/*
    created at 11th June, 2015
 */

// TODO: column - row layout

var slice = Array.prototype.slice.call;

var MessageListener = Object.create({});


// login
var LoginWidget = React.createClass({
    getInitialState: function() {
        return {display: "hidden", username: "", password: ""};
    },

    componentDidMount: function() {
        $(MessageListener).on("activateLogin", function(){
            $(".mask").removeClass("hidden");
            $(".mask").addClass("mask-fade-in");
            this.setState({display: "fadeIn"});
        }.bind(this));
        $(MessageListener).on("deactivateLogin", function(){
            $(".mask").removeClass("mask-fade-in");
            $(".mask").addClass("hidden");
            this.setState({display: "hidden"});
        }.bind(this));
    },

    componentWillUnmount: function() {
        $(MessageListener).off("activateLogin");
    },

    submit: function() {
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/login",
            method: "POST",
            data: JSON.stringify({
                "username": this.state.username,
                "password": this.state.password
            })
        })
            .then(function(result){
                alert(result);
                $(MessageListener).trigger("deactivateLogin");
            });
    },

    usernameChange: function(l) {
        this.setState({
            "username": l.target.value
        });
    },

    passwordChange: function(l) {
        this.setState({
            "password": l.target.value
        });
    },

    render: function() {
        var classString = "login";
        classString += " " + this.state.display;
        return(
            <div className={classString}>
                <img src="./media/closelabel.png" className="closelabel" onClick={function(){$(MessageListener).trigger("deactivateLogin")}} />
                <div className="text">LOGIN</div>
                <input type="search" className="username" name="username" placeholder="username" onChange={this.usernameChange} value={this.state.username} required />
                <input type="password" className="password" name="password" placeholder="password" onChange={this.passwordChange} value={this.state.password} required />
                <a className="submit" onClick={this.submit}>Submit</a>
            </div>
        );
    }
});

React.render(<LoginWidget />, document.getElementsByClassName("login-place-holder")[0]);


// register
var RegisterWidget = React.createClass({
    getInitialState: function() {
        return {display: "hidden", username: "", password: "", repeatPassword: ""};
    },

    componentDidMount: function() {
        $(MessageListener).on("activateRegister", function(){
            $(".mask").removeClass("hidden");
            $(".mask").addClass("mask-fade-in");
            this.setState({display: "fadeIn"});
        }.bind(this));
        $(MessageListener).on("deactivateRegister", function(){
            $(".mask").removeClass("mask-fade-in");
            $(".mask").addClass("hidden");
            this.setState({display: "hidden"});
        }.bind(this));
    },

    componentWillUnmount: function() {
        $(MessageListener).off("activateRegister");
    },

    submit: function() {
        if (this.state.password !== this.state.repeatPassword){
            console.log(this.state.password);
            console.log(this.state.repeatPassword);
            alert("password doesn't match");
        }
        else {
            $.ajax({
                url: "http://" + window.location.hostname + ":8964/api/register",
                method: "POST",
                data: JSON.stringify({
                    "username": this.state.username,
                    "password": this.state.password
                })
            })
                .then(function (result) {
                    alert(result);
                    $(MessageListener).trigger("deactivateRegister");
                });
        }
    },

    usernameChange: function(l) {
        this.setState({
            "username": l.target.value
        });
    },

    passwordChange: function(l) {
        this.setState({
            "password": l.target.value
        });
    },

    repeatChange: function(l) {
        this.setState({
            "repeatPassword": l.target.value
        });
    },

    render: function() {
        var classString = "register";
        classString += " " + this.state.display;
        return(
            <div className={classString}>
                <img src="./media/closelabel.png" className="closelabel" onClick={function(){$(MessageListener).trigger("deactivateRegister")}} />
                <div className="text">SIGN UP</div>
                <input type="search" className="username" name="username" placeholder="username" onChange={this.usernameChange} value={this.state.username} required />
                <input type="password" className="password" name="password" placeholder="password" onChange={this.passwordChange} value={this.state.password} required />
                <input type="password" className="password" name="password" placeholder="repeat" onChange={this.repeatChange} value={this.state.repeatePassword} required />
                <a className="submit" onClick={this.submit}>Submit</a>
            </div>
        );
    }
});

React.render(<RegisterWidget />, document.getElementsByClassName("register-place-holder")[0]);


var LoginRegister = React.createClass({

    login: function() {
        $(MessageListener).trigger("deactivateRegister");
        $(MessageListener).trigger("activateLogin");
        $(".mask").removeClass("hidden");
    },

    register: function() {
        $(MessageListener).trigger("deactivateLogin");
        $(MessageListener).trigger("activateRegister");
        $(".mask").removeClass("hidden");
    },

    render: function() {
        return (
            <div className="login-register" >
                <a id="loginButton" onClick={this.login}>Sign in</a>
                <a id="registerButton" onClick={this.register} >Sign up</a>
            </div>
        );
    }
});


var DisplayBoard = React.createClass({

    componentDidMount(){
        var self = this;
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/load",
        })
        .then(function(result){
            result = JSON.parse(result);
            for (var i = 0; i < result; ++i)
                result[i].key= i;
            self.setState({Books: result});
        });

        $(MessageListener).on("borrow", function(e, index){
            var library = self.state.Books;
            library[index].borrowed = true;
            self.setState({Books: library});
        });

        $(MessageListener).on("return", function(e, index){
            var library = self.state.Books;
            library[index].borrowed = false;
            self.setState({books: library});
        });
    },

    getInitialState: function() {
        var searchString = "";

        return {
            searchString: searchString,
            Books: []
        };
    },

    handleChange: function(l){
        this.setState({
            searchString: l.target.value,
        });
    },

    borrow: function(index) {
        $(MessageListener).trigger("borrow", index);
        this.setState({Books: Books});
    },

    returnBook: function(index) {
        $(MessageListener).trigger("return", index);
        this.setState({Books: Books});
    },

    render: function() {

        var libraries = this.state.Books;
        var searchString = this.state.searchString.trim().toLowerCase();

        if (searchString.length > 0) {
            libraries = libraries.filter(function(book){
                for (var i in book)
                    if (typeof book[i] === 'string') {
                        if (book[i].toLowerCase().match(searchString))
                            return true;
                    }
                return false;
            });
        }

        var self = this;

        return (
            <div>
                <div className="header">
                    <div className="logo">SimpleLibrary</div>
                    <input
                        className="search-bar"
                        type="search"
                        onChange={this.handleChange}
                        value={this.state.searchString}
                        placeholder="Search books"
                        />
                    <LoginRegister />
                </div>
                <div className="display-board">
                    <ul className="row">
                        {
                            libraries.map(function(book, index){
                                return (
                                    <li className="book">
                                        <img src={book.image} className="book-image" />
                                        <div className="book-title">
                                            {
                                                book.borrowed ?
                                                    <a className="glyphicon glyphicon-eject return" onClick={self.returnBook.bind(self, index)} >Return</a>:
                                                    <a className="glyphicon glyphicon-plus borrow" onClick={self.borrow.bind(self, index)} >Borrow</a>
                                            }
                                            <span className="book-name">{book.name}</span>
                                        </div>
                                        <div className="book-description">{book.description}</div>
                                    </li>
                                );
                            })
                        }
                    </ul>
                </div>
            </div>
            );
    }
});

// for test
var Books = [
    {index: 0, name: "ReactJS", image: './media/book1.jpg', description: "The best way to code component UI I have seen.", "borrowed": false},
    {index: 1, name: "javascript the good part", image: './media/book2.jpg', description: "Strongly Suggest that every javascript coder should read it!", "borrowed": false},
    {index: 2, name: "javascript 权威指南", image: "./media/book3.jpg", description: "Include the bad part of javascript", "borrowed": false},
    {index: 3, name: "我的青春恋爱物语果然有问题", image: "./media/book4.jpg", description: "高端后宫玩家", borrowed: false},
    {index: 4, name: "ReactJS", image: './media/book1.jpg', description: "The best way to code component UI I have seen.", "borrowed": false},
    {index: 5, name: "ReactJS", image: './media/book1.jpg', description: "The best way to code component UI I have seen.", "borrowed": false},
    {index: 6, name: "javascript the good part", image: './media/book2.jpg', description: "Strongly Suggest that every javascript coder should read it!", "borrowed": false},
    {index: 7, name: "javascript 权威指南", image: "./media/book3.jpg", description: "Include the bad part of javascript", "borrowed": false},
    {index: 8, name: "我的青春恋爱物语果然有问题", image: "./media/book4.jpg", description: "高端后宫玩家", borrowed: false},
    {index: 9, name: "javascript the good part", image: './media/book2.jpg', description: "Strongly Suggest that every javascript coder should read it!", "borrowed": false},
    {index: 10, name: "javascript 权威指南", image: "./media/book3.jpg", description: "Include the bad part of javascript", "borrowed": false},
    {index: 11, name: "我的青春恋爱物语果然有问题", image: "./media/book4.jpg", description: "高端后宫玩家", borrowed: false},
];


React.render(<DisplayBoard />, document.getElementsByClassName("search-widget")[0]);

