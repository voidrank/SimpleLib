#include "crow_all.h"
#include <map>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>

// for random string
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using std::string;
using std::cerr;
using std::endl;


// TODO : anti-XSS, anti-CSRF, Database(permanent data storage)

namespace CookieManager {

    std::string chars(
            "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "1234567890"
    );
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

    const int COOKIE_LENGTH = 64;

    std::string gen(int length) {
        std::ostringstream o;
        for (int i = 0; i < length; ++i)
            o << chars[index_dist(rng)];
        return o.str();
    }

    struct CookieManagerMiddleware {

        struct context {

            std::string cookieid;

            std::string get_cookieid() {
                return cookieid;
            }

            void set_cookieid(std::string ci) {
                cookieid = ci;
            }
        };

        template<typename AllContext>
        void before_handle(crow::request &req, crow::response &res, context &ctx, AllContext &all_ctx) {

            auto cookie_parser_context = all_ctx.template get<crow::CookieParser>();
            auto crowid = cookie_parser_context.get_cookie("crowid");

            cerr << "crowid: " << crowid << endl;

            if (crowid == "") {
                cerr << "crowid doesn't exists" << endl;
                std::string crowid = gen(COOKIE_LENGTH);
                ctx.set_cookieid(crowid);
                res.set_header("Set-Cookie", string("crowid=") + crowid);
            }

        }

        template<typename AllContext>
        void after_handle(crow::request &req, crow::response &res, context &ctx, AllContext all_ctx) {
            // this is a hack
            res.add_header("Access-Control-Allow-Origin", "http://localhost:63343");
            res.add_header("Access-Control-Allow-Credentials", "true");
        }
    };
};


// Cookie -> Session Middleware
namespace SessionManager {

    typedef crow::json::wvalue session;
    std::unordered_map <std::string, crow::json::wvalue> Session;

    /*
    session & get_session(const crow::request & req) {
        auto cookie = req.get_header_value("Cookie");
        retunr nullptr;
    }
    */

    struct SessionMiddleware {


        struct context {

            crow::json::wvalue *session;

            crow::json::wvalue &get_session() {
                return *session;
            }

            crow::json::wvalue &get_value(const std::string &key) {
                return (*session)[key];
            }

            void set_value(const std::string &key, crow::json::wvalue &&value) {
                (*session)[key] = std::move(value);
            }

            void remove(const std::string &key) {
                (*session)[key].clear();
            }
        };

        template<typename AllContext>
        void before_handle(crow::request &req, crow::response &res, context &ctx, AllContext &all_ctx) {

            auto cookies_manager_context = all_ctx.template get<CookieManager::CookieManagerMiddleware>();

            std::string crowid = cookies_manager_context.get_cookieid();

            // session doesn't exist
            if (Session.find(crowid) == Session.end()) {
                Session[crowid] = crow::json::wvalue();
            }
            else {
            }

            ctx.session = &Session[crowid];
        }

        void after_handle(crow::request &req, crow::response &res, context &ctx) {

        }
    };

}


namespace UserManager {

    // backends
    namespace backends {
        std::string chars(
                "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "1234567890"
        );
        boost::random::random_device rng;
        boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

        const int USER_ID_LENGTH = 64;

        std::string gen(int length = USER_ID_LENGTH) {
            std::ostringstream o;
            for (int i = 0; i < length; ++i)
                o << chars[index_dist(rng)];
            return o.str();
        }

        typedef std::pair <std::string, int> User;
        // <string>username : pair{<string>password, <int>member }
        // member:
        // 0 means root
        // 1 means registed user
        // 2 means anonymous user

        // TODO: add salt & hash
        // Users pool
        std::unordered_map <std::string, User> users =
        decltype(users)();

        // Anonymous User
        User AnonymousUser =
                std::make_pair(
                        std::string("Anonymous User"),
                        3
                );
    }


    class UserManagerMiddleware {

    public:

        struct context {
            backends::User *user;

            auto &get_user() {
                return *user;
            }

            void set_user(backends::User &o) {
                user = &o;
            }
        };

        template<typename AllContext>
        void before_handle(crow::request &req, crow::response &res, context &ctx, AllContext &all_ctx) {

            using namespace backends;

            auto session_manager_middleware = all_ctx.template get<SessionManager::SessionMiddleware>();

            // check if username is in session
            std::string username = crow::json::dump(session_manager_middleware.get_value("username"));
            username = username.substr(1, username.length() - 2);
            if (username == "") {
                cerr << "UserManagerMiddleware: username not set" << endl;
                ctx.set_user(AnonymousUser);
            }
            // check if username is in user manager
            else {
                auto me = users.find(username);
                // found
                if (me != users.end()) {
                    cerr << "UserManagerMiddleware: user found" << endl;
                    ctx.set_user(me->second);
                }
                    // anonymous
                else {
                    cerr << "UserManagerMiddleware: user not found, username: " << username << endl;
                    ctx.set_user(AnonymousUser);
                }
            }
        }

        template<typename AllContext>
        void after_handle(crow::request &req, crow::response &, context &ctx, AllContext &all_ctx) {

        }
    };

}

namespace BorrowAndReturn {
    typedef std::unordered_map<int, bool> relation;
    std::unordered_map<string, relation> relations;
    // {username: {index: bool(1 means book is borrowed, 0 means book is returned)}}
}

namespace Library {

    std::vector<crow::json::wvalue> Books;
    int primary_key_count;

    crow::json::wvalue load(const std::string& key, const std::string& username) {

        using crow::json::dump;
        using crow::json::load;

        auto ret = crow::json::wvalue();
        auto count = 0;
        cerr << username << endl;
        auto p_rel = BorrowAndReturn::relations.find(username);

        if (p_rel == BorrowAndReturn::relations.end()){
            p_rel = BorrowAndReturn::relations.emplace(username, BorrowAndReturn::relation()).first;
            cerr << "not found" << endl;
        }

        auto rel = p_rel->second;

        for (auto& i: Books) {
            string name = load(dump(i["name"])).s();

            if (name.find(key) != std::string::npos) {

                int index = (int)load(dump(i["index"])).i();
                ret[count] = load(dump(i));

                auto p = rel.find(index);
                if (p != rel.end()) {
                    ret[count]["borrowed"] = p->second;
                    cerr << p->second << endl;
                    cerr << "right branch" << endl;
                }
                else {
                    ret[count]["borrowed"] = false;
                }

                count ++;
            }
        }
        return std::move(ret);
    }

    bool empty(const int& index) {

        using crow::json::load;
        using crow::json::dump;

        for (auto& i:Books) {
            if (load(dump(i["index"])).i() == index) {
                if (load(dump(i["rest"])).i() > 0)
                    return false;
                else
                    return true;
            }
        }

        return false;
    }

    bool borrow(const int & index) {
        using crow::json::load;
        using crow::json::dump;

        for (auto& i:Books) {
            if (load(dump(i["index"])).i() == index) {
                cerr << "Book found" << endl;
                if (load(dump(i["rest"])).i() > 0) {
                    i["rest"] = load(dump(i["rest"])).i() - 1;
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        return false;
    }

    bool returnBook(const int & index) {

        using crow::json::load;
        using crow::json::dump;

        for (auto& i:Books) {
            if (load(dump(i["index"])).i() == index) {
                i["rest"] = load(dump(i["rest"])).i() + 1;
                return true;
            }
        }

        return false;
    }
}

namespace BorrowAndReturn {

    bool borrowBook(std::string username, int index) {
        auto rit = relations.find(username);
        if (rit == relations.end()){
            auto r_pair = relations.emplace(username, std::move(relation()));
            rit = std::move(r_pair.first);
        }
        auto bit = rit->second.find(index);
        if (bit == rit->second.end()){
            // record not exists
            // if not empty
            if (Library::borrow(index)) {
                auto r_pair = rit->second.emplace(index, 1);
                return true;
            }
            else {
                auto r_pair = rit->second.emplace(index, 0);
                return false;
            }
        }
        else {
            // if you hvae borrowed the book
            if (bit->second == true)
                return false;
            else {
                // normal case
                if (Library::borrow(index)) {
                    bit->second = true;
                    return true;
                }
                else {
                    // there is no books rest
                    return false;
                }
            }
        }
    }

    bool returnBook(std::string username, int index) {
        auto rit = relations.find(username);
        if (rit == relations.end()){
            auto r_pair = relations.emplace(username, std::move(relation()));
            rit = std::move(r_pair.first);
        }
        auto bit = rit->second.find(index);
        if (bit == rit->second.end()){
            auto r_pair = rit->second.emplace(index, 0);
            // record not exists
            return false;
        }
        else {
            // if you have returned the book
            if (bit->second == false)
                return false;
            // normal case
            else {
                bit->second = false;
                Library::returnBook(index);
                return true;
            }
        }
    }
}


int main() {


    crow::App<
            crow::CookieParser,
            CookieManager::CookieManagerMiddleware,
            SessionManager::SessionMiddleware,
            UserManager::UserManagerMiddleware
    > app;


    CROW_ROUTE(app, "/")
            ([](const crow::request &req, crow::response &res) {
                res.write("Hello World");
                res.end();
            });


    CROW_ROUTE(app, "/api/login")
            .methods("POST"_method)
    ([](const crow::request &req, crow::response &res) {

        // get user in memory
        using UserManager::backends::users;

        auto req_body = crow::json::load(req.body);
        string username = crow::json::dump(req_body["username"]);
        string password = crow::json::dump(req_body["password"]);
        string username_str = std::move(username.substr(1, username.size()-2));
        string password_str = std::move(password.substr(1, password.size()-2));

        auto user = users.find(username_str);

        // found a user named {username}
        if (user != users.end()) {
            cerr << "user found" << endl;
            // check password
            if (user->second.first == password_str) {
                cerr << "password right" << endl;
                auto *context =
                        static_cast<
                                crow::detail::partial_context
                                        <
                                                crow::CookieParser,
                                                CookieManager::CookieManagerMiddleware,
                                                SessionManager::SessionMiddleware,
                                                UserManager::UserManagerMiddleware
                                        > *
                                >
                        (req.middleware_context);
                auto session_middleware = context->get<SessionManager::SessionMiddleware>();
                session_middleware.set_value("username", std::move(crow::json::load(username)));
                auto str = crow::json::dump(session_middleware.get_value("username"));
                cerr << req.middleware_context << endl;
            }
            else {
                res.code = 403;
                res.end("wrong username or password");
            }
        }
        else {
            res.code = 403;
            res.end("wrong username or password");
        }
        res.end("login successfully");
    });

    CROW_ROUTE(app, "/api/logout")
               .methods("POST"_method)
    ([](const crow::request &req, crow::response &res){

        // get user in memory
        auto *context =
                static_cast<
                    crow::detail::partial_context
                        <
                            crow::CookieParser,
                            CookieManager::CookieManagerMiddleware,
                            SessionManager::SessionMiddleware,
                            UserManager::UserManagerMiddleware
                        >*
                >(req.middleware_context);
        auto session_middleware = context->get<SessionManager::SessionMiddleware>();
        session_middleware.remove("username");
        res.end("logout successfully");
    });

    CROW_ROUTE(app, "/api/register")
               .methods("POST"_method)
    ([](const crow::request & req, crow::response &res){

        // get registet information
        auto req_body = crow::json::load(req.body);
        string username = crow::json::dump(req_body["username"]);
        string password = crow::json::dump(req_body["password"]);
        string username_str = username.substr(1, username.length() - 2);
        string password_str = password.substr(1, password.length() - 2);

        using UserManager::backends::users;
        // check if the username is occupied
        cerr << username_str << endl;
        auto user = users.find(username_str);
        cerr << (user == users.end()) << endl;
        // fail
        if (user != users.end()) {
            res.end("This username is occupied");
        }
        // success
        else {
            // 1 means user
            users.emplace(make_pair(username_str, make_pair(password_str, 1)));
            cerr << "Current total of users is " << users.size() << endl;
            res.end("register successfully");
        }
    });



    CROW_ROUTE(app, "/api/load")
               .methods("GET"_method)
    ([](const crow::request & req, crow::response & res){

        using crow::json::dump;
        using crow::json::load;

        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto session_middlerware = context->get<SessionManager::SessionMiddleware>();
        string username;
        try {
            username = std::move(load(dump(session_middlerware.get_value("username"))).s());
        }
        catch (std::runtime_error) {
            username = "anonymous";
        }

        // get query_string
        auto bookname_json = req.url_params.get("bookname");
        string bookname;
        if (bookname_json == nullptr) {
            bookname = "";
        }
        else {
            bookname = bookname_json;
        }
        auto books = Library::load(bookname, username);
        res.end(crow::json::dump(books));
    });




    CROW_ROUTE(app, "/api/set")
               .methods("POST"_method)
    ([](const crow::request & req, crow::response & res){

        // get req body
        crow::json::wvalue book = crow::json::load(req.body);

        // check the request is sent from admin
        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto user_middlerware = context->get<UserManager::UserManagerMiddleware>();
        auto user = user_middlerware.get_user();
        cerr << user.second << endl;
        cerr << user.first << endl;
        if (user.second == 0) {
            ++Library::primary_key_count;
            book["index"] = Library::primary_key_count;
            Library::Books.push_back(std::move(book));
            res.end("post succesfully");
        }
        else {
            res.end("You are not the admin");
        }
    });



    CROW_ROUTE(app, "/api/borrow")
               .methods("POST"_method)
    ([](const crow::request &req, crow::response & res){

        using crow::json::load;
        using crow::json::dump;

        auto book_json = crow::json::load(req.body);
        auto book_index = std::move(atoi(crow::json::dump(book_json["index"]).c_str()));
        cerr << book_index << endl;
        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto session_middlerware = context->get<SessionManager::SessionMiddleware>();
        string username;
        try {
            username = std::move(load(dump(session_middlerware.get_value("username"))).s());
        }
        catch (std::runtime_error) {
            res.code = 403;
            res.end("Login first!");
        }
        if (BorrowAndReturn::borrowBook(username, book_index)){
            res.end("Success!");
        }
        else {
            res.code = 403;
            res.end("Wrong Borrow Operation");
        }

    });


    CROW_ROUTE(app, "/api/return")
            .methods("POST"_method)
    ([](const crow::request &req, crow::response & res){

        using crow::json::load;
        using crow::json::dump;

        auto book_json = crow::json::load(req.body);
        auto book_index = std::move(atoi(crow::json::dump(book_json["index"]).c_str()));
        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto session_middlerware = context->get<SessionManager::SessionMiddleware>();
        string username;
        try {
            username = std::move(load(dump(session_middlerware.get_value("username"))).s());
        }
        catch (std::runtime_error) {
            res.code = 403;
            res.end("Login first!");
        }
        if (BorrowAndReturn::returnBook(username, book_index)){
            res.end("Success!");
        }
        else {
            res.end("You didn't borrow this book!");
        }
    });



    CROW_ROUTE(app, "/api/delete")
            .methods("POST"_method)
    ([](const crow::request &req, crow::response & res){

        using crow::json::load;
        using crow::json::dump;

        auto book_json = crow::json::load(req.body);
        auto book_index = std::move(atoi(crow::json::dump(book_json["index"]).c_str()));
        cerr << book_index << endl;
        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto session_middlerware = context->get<SessionManager::SessionMiddleware>();
        string username;
        try {
            username = std::move(load(dump(session_middlerware.get_value("username"))).s());
        }
        catch (std::runtime_error) {
            res.code = 403;
            res.end("Login first!");
        }
        auto user_middleware = context->get<UserManager::UserManagerMiddleware>();
        auto user = user_middleware.get_user();
        auto auth = user.second;
        if (auth != 0) {
            res.end("You aren't the admin.");
        }
        else {
            using Library::Books;
            using crow::json::load;
            using crow::json::dump;
            for (auto i = Books.begin(); i != Books.end(); ++i)
                if (load(dump((*i)["index"])).i() == book_index) {
                    Books.erase(i);
                    break;
                }
            res.end("Success!");
        }

    });



    CROW_ROUTE(app, "/api/is_login")
            .methods("POST"_method)
    ([](const crow::request &req, crow::response & res){

        using crow::json::load;
        using crow::json::dump;

        auto *context =
                static_cast<
                        crow::detail::partial_context
                                <
                                        crow::CookieParser,
                                        CookieManager::CookieManagerMiddleware,
                                        SessionManager::SessionMiddleware,
                                        UserManager::UserManagerMiddleware
                                >*
                        >(req.middleware_context);
        auto session_middleware = context->get<SessionManager::SessionMiddleware>();
        auto user_middleware = context->get<UserManager::UserManagerMiddleware>();
        string username;
        cerr << "here" << endl;
        try {
            username = std::move(load(dump(session_middleware.get_value("username"))).s());
            auto user_middleware = context->get<UserManager::UserManagerMiddleware>();
            auto user = user_middleware.get_user();
            res.end(std::to_string(user.second));
        }
        catch (std::runtime_error) {
            res.code = 403;
            res.end("Login first!");
        }
    });

    /*
     *  set admin
     */
    UserManager::backends::users.emplace("lancy", std::make_pair("lancy", 0));
    cerr << (UserManager::backends::users.find("lancy") == UserManager::backends::users.end()) << endl;

    /*
     * set initial library
     */
    Library::Books.push_back(crow::json::load("{\"index\": 0, \"name\": \"ReactJS\", \"image\": \"./media/book1.jpg\", \"description\": \"The best way to code component UI I have seen.\", \"borrowed\": false, \"rest\": 2}"));
    Library::Books.push_back(crow::json::load("{\"index\": 1, \"name\": \"javascript the good part\", \"image\": \"./media/book2.jpg\", \"description\": \"Strongly Suggest that every javascript coder should read it!\", \"borrowed\": false, \"rest\": 2}"));
    Library::Books.push_back(crow::json::load("{\"index\": 2, \"name\": \"javascript\", \"image\": \"./media/book3.jpg\", \"description\": \"Include the bad part of javascript\", \"borrowed\": false, \"rest\": 10}"));
    Library::Books.push_back(crow::json::load("{\"index\": 3, \"name\": \"我的青春恋爱物语果然有问题\", \"image\": \"./media/book4.jpg\", \"description\": \"高端后宫玩家\", \"borrowed\": false, \"rest\": 1}"));
    Library::primary_key_count = 3;

    app.port(8964)
            .multithreaded()
            .run();
}
