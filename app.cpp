#include "crow_all.h"
#include <map>
#include <string>
#include <sstream>

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
            res.set_header("Access-Control-Allow-Origin", "*");
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
                session_manager_middleware.set_value("username", crow::json::load(username));
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

namespace Library {

    std::vector<crow::json::wvalue> Books;

    std::string load(const std::string& key) {
        std::string ret = "[";
        for (auto&& i: Books) {
            auto name = crow::json::dump(i["name"]);
            name = name.substr(1, name.length() - 2);
            cerr << "this" << endl;
            if (name.find(key) != std::string::npos) {
                ret.append(crow::json::dump(i));
                ret.append(", ");
            }
        }
        ret.erase(ret.length() - 2, ret.length());
        ret.append("]");
        return std::move(ret);
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
                res.end("wrong username or password");
            }
        }
        else {
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

        // get query_string
        auto bookname_json = req.url_params.get("bookname");
        string bookname;
        if (bookname_json == nullptr) {
            bookname = "";
        }
        else {
            bookname = bookname_json;
        }
        cerr << Library::load(bookname) << endl;
        res.end(Library::load(bookname));
    });




    CROW_ROUTE(app, "/api/set")
               .methods("POST"_method)
    ([](const crow::request & req, crow::response & res){

        // get req body
        auto book = crow::json::load(req.body);
        string name = crow::json::dump(book["name"]);
        name = name.substr(1, name.length() - 2);
        string image = crow::json::dump(book["image"]);
        image = image.substr(1, name.length() - 2);
        string description = crow::json::dump(book["description"]);
        description.substr(1, description.length() - 2);

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
            Library::Books.push_back(book);
            res.end("post succesfully");
        }
        else {
            res.end("You are not the admin");
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
    Library::Books.push_back(crow::json::load("{\"index\": 0, \"name\": \"ReactJS\", \"image\": \"./media/book1.jpg\", \"description\": \"The best way to code component UI I have seen.\", \"borrowed\": false}"));
    Library::Books.push_back(crow::json::load("{\"index\": 1, \"name\": \"javascript the good part\", \"image\": \"./media/book2.jpg\", \"description\": \"Strongly Suggest that every javascript coder should read it!\", \"borrowed\": false}"));
    Library::Books.push_back(crow::json::load("{\"index\": 2, \"name\": \"javascript\", \"image\": \"./media/book3.jpg\", \"description\": \"Include the bad part of javascript\", \"borrowed\": false}"));
    Library::Books.push_back(crow::json::load("{\"index\": 3, \"name\": \"我的青春恋爱物语果然有问题\", \"image\": \"./media/book4.jpg\", \"description\": \"高端后宫玩家\", \"borrowed\": false}"));

    app.port(8964)
            .multithreaded()
            .run();
}
