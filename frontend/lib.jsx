/*
 created at 11th June, 2015
 */

// TODO: column - row layout

var slice = Array.prototype.slice.call;

var MessageListener = Object.create({});


// login
var LoginWidget = React.createClass({
    getInitialState: function () {
        return {display: "hidden", username: "", password: ""};
    },

    componentDidMount: function () {
        $(MessageListener).on("activateLogin", function () {
            setTimeout(function(){
                $("input[name='login-username']").focus();
            }, 10);
            $(".mask").removeClass("hidden");
            $(".mask").addClass("mask-fade-in");
            this.setState({display: "fade-in"});
        }.bind(this));
        $(MessageListener).on("deactivateLogin", function () {
            $(".mask").removeClass("mask-fade-in");
            $(".mask").addClass("hidden");
            this.setState({display: "hidden"});
        }.bind(this));
    },

    componentWillUnmount: function () {
        $(MessageListener).off("activateLogin");
    },

    submit: function () {
        var self = this;
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/login",
            method: "POST",
            data: JSON.stringify({
                "username": this.state.username,
                "password": this.state.password
            }),
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function (result) {
            alert(result);
            $(MessageListener).trigger("deactivateLogin");
            $(MessageListener).trigger("loggedin");
            $(MessageListener).trigger("check_admin");
        }, function(result){
            alert(result.responseText);
            self.setState({username: "", password: ""});
        });
    },

    usernameChange: function (l) {
        this.setState({
            "username": l.target.value
        });
    },

    passwordChange: function (l) {
        this.setState({
            "password": l.target.value
        });
    },

    render: function () {
        var classString = "login";
        classString += " " + this.state.display;
        return (
            <div className={classString}>
                <img src="./media/closelabel.png" className="closelabel"
                     onClick={function(){$(MessageListener).trigger("deactivateLogin")}}/>

                <div className="text">LOGIN</div>
                <input type="search" className="username" name="login-username" placeholder="username"
                       onChange={this.usernameChange} value={this.state.username} required/>
                <input type="password" className="password login-lastInput" name="password" placeholder="password"
                       onChange={this.passwordChange} value={this.state.password} required/>
                <button className="submit" id="login-submit" onClick={this.submit}>Submit</button>
            </div>
        );
    }
});

React.render(<LoginWidget />, document.getElementsByClassName("login-place-holder")[0]);


// register
var RegisterWidget = React.createClass({
    getInitialState: function () {
        return {display: "hidden", username: "", password: "", repeatPassword: ""};
    },

    componentDidMount: function () {
        $(MessageListener).on("activateRegister", function () {
            setTimeout(function() {
                $(".register-username").focus()
            }, 10);
            $(".mask").removeClass("hidden");
            $(".mask").addClass("mask-fade-in");
            this.setState({display: "fade-in", password: "", username: "", repeatPassword: ""});
        }.bind(this));
        $(MessageListener).on("deactivateRegister", function () {
            $(".mask").removeClass("mask-fade-in");
            $(".mask").addClass("hidden");
            this.setState({display: "hidden"});
        }.bind(this));
    },

    componentWillUnmount: function () {
        $(MessageListener).off("activateRegister");
    },

    submit: function () {
        if (this.state.password !== this.state.repeatPassword) {
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
                }),
                crossDomain: true,
                xhrFields: {
                    withCredentials: true
                }
            }).then(function (result) {
                alert(result);
                $(MessageListener).trigger("deactivateRegister");
            });
        }
    },

    usernameChange: function (l) {
        this.setState({
            "username": l.target.value
        });
    },

    passwordChange: function (l) {
        this.setState({
            "password": l.target.value
        });
    },

    repeatChange: function (l) {
        this.setState({
            "repeatPassword": l.target.value
        });
    },

    render: function () {
        var classString = "register";
        classString += " " + this.state.display;
        return (
            <div className={classString}>
                <img src="./media/closelabel.png" className="closelabel"
                     onClick={function(){$(MessageListener).trigger("deactivateRegister")}}/>

                <div className="text">SIGN UP</div>
                <input type="search" className="register-username" name="username" placeholder="username"
                       onChange={this.usernameChange} value={this.state.username} required/>
                <input type="password" className="password" name="password" placeholder="password"
                       onChange={this.passwordChange} value={this.state.password} required/>
                <input type="password" className="password register-lastInput" name="password" placeholder="repeat"
                       onChange={this.repeatChange} value={this.state.repeatPassword} required/>
                <button className="submit" id="register-submit" onClick={this.submit}>Submit</button>
            </div>
        );
    }
});

React.render(<RegisterWidget />, document.getElementsByClassName("register-place-holder")[0]);


var LoginRegister = React.createClass({

    getInitialState: function() {
        return {loggedin: false, is_admin: false};
    },

    componentDidMount: function() {
        var self = this;
        $(MessageListener).on("loggedin", function(){
            self.setState({"loggedin": true});
        });
        $(MessageListener).on("loggedout", function(){
            self.setState({"loggedin": false});
        });
        $(MessageListener).on("is_admin", function(){
            self.setState({"is_admin": true});
        });
    },

    login: function () {
        $(MessageListener).trigger("deactivateRegister");
        $(MessageListener).trigger("deactivateAddBook");
        $(MessageListener).trigger("activateLogin");
        $(".mask").removeClass("hidden");
    },

    register: function () {
        $(MessageListener).trigger("deactivateLogin");
        $(MessageListener).trigger("deactivateAddBook");
        $(MessageListener).trigger("activateRegister");
        $(".mask").removeClass("hidden");
    },

    addBook: function() {
        $(MessageListener).trigger("deactivateLogin");
        $(MessageListener).trigger("activateAddBook");
        $(MessageListener).trigger("deactivateRegister");
        $(".mask").removeClass("hidden");
    },

    logout: function() {
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/logout",
            method: "POST",
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function(result) {
            $(MessageListener).trigger("loggedout");
        }, function(result) {
            alert(result.responseText);
        });
    },

    render: function () {
        var login_class = (this.state.loggedin === false) ? "" : "hidden";
        return (
            <div className="login-register {}">
                <a id="loginButton" onClick={this.login} className={(this.state.loggedin===false) ? "" : "hidden"}>Sign in</a>
                <a id="registerButton" onClick={this.register} className={(this.state.loggedin===false) ? "": "hidden"}>Sign up</a>
                <a id="addBook" onClick={this.addBook} className={(this.state.loggedin===false|| this.state.is_admin===false) ? "hidden" : ""} >Add Book </a>
                <a id="logoutButton" onClick={this.logout} className={(this.state.loggedin===false) ? "hidden": ""}>Sign Out</a>
            </div>
        );
    }
});


var DisplayBoard = React.createClass({

    componentDidMount(){
        var self = this;
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/load",
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function (result) {
            result = JSON.parse(result);
            self.setState({Books: result});
        });

        $(MessageListener).on("borrow", function (e, index) {
            var library = self.state.Books;
            library[index].borrowed = true;
            self.setState({Books: library});
        });

        $(MessageListener).on("return", function (e, index) {
            var library = self.state.Books;
            library[index].borrowed = false;
            self.setState({Books: library});
        });

        $(MessageListener).on("AddedBook", function(e, book) {
            var library = self.state.Books;
            book.index = library.length;
            library.push(book);
            self.setState({
                Books: library
            });
        });

        $(MessageListener).on("is_admin", function(){
            self.setState({is_admin: true});
        });
    },

    getInitialState: function () {
        var searchString = "";

        return {
            searchString: searchString,
            Books: [],
            is_admin: false
        };
    },

    handleChange: function (l) {
        this.setState({
            searchString: l.target.value,
        });
    },

    borrow: function (index) {
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/borrow",
            method: "POST",
            data: JSON.stringify({
                index: index
            }),
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function (e) {
                alert(e);
                $(MessageListener).trigger("borrow", index);
            },
            function(e) {
                alert(e.responseText);
            }
        );
    },

    returnBook: function (index) {
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/return",
            method: "POST",
            data: JSON.stringify({
                index: index
            }),
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function (e) {
                alert(e);
                $(MessageListener).trigger("return", index);
            },
            function(e) {
                alert(e.responseText);
            }
        );
    },

    deleteBook: function(pk, index) {

        var self = this;
        console.log(index);

        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/delete",
            method: "POST",
            data: JSON.stringify({
                index: pk
            }),
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            }
        }).then(function (e) {
                alert(e);
                var library = self.state.Books;
                for (var i = 0; i < library.length; ++i)
                    if (library[i].index === pk)
                        library.splice(i, 1);

                // to fade out
                var toBeDelete = $(".book"+index);
                setTimeout(function() {
                    toBeDelete.css("position", "absolute");
                }, 0);
                var toBeDeleteOpacity = 1;
                var toBeDeleteHandle = setInterval(function(){
                    toBeDeleteOpacity -= 0.01;
                    toBeDelete.css("opacity", toBeDeleteOpacity);
                    if (toBeDelete <= 0)
                        clearInterval(toBeDeleteHandle);
                }, 10);

                var moveUpElement = $(".book"+(index+1));
                // if not null
                if (moveUpElement.length > 0) {
                    console.log(123);
                    // init
                    var stdMarginTop = parseFloat(moveUpElement.css("margin-top"));
                    var stdHeight = parseFloat(toBeDelete.css("height"));
                    var nowMarginTop = stdMarginTop * 2 + stdHeight;
                    moveUpElement.css("margin-top", nowMarginTop);
                    var delta = (stdMarginTop + stdHeight) / 100;
                    console.log(delta);

                    var moveUpElementHandle = setInterval(function(){
                        nowMarginTop -= delta;
                        moveUpElement.css("margin-top", nowMarginTop);
                        if (nowMarginTop <= stdMarginTop) {
                            moveUpElement.css("margin-top", stdMarginTop);
                            clearInterval(moveUpElementHandle);
                        }
                    }, 10);
                }

                setTimeout(function() {
                    self.setState({Books: library})
                }, 1000);
            },
            function(e) {
                alert(e.responseText);
            }
        );
    },

    render: function () {

        var libraries = this.state.Books;
        var searchString = this.state.searchString.trim().toLowerCase();

        if (searchString.length > 0) {
            libraries = libraries.filter(function (book) {
                for (var i in book)
                    if (typeof book[i] === 'string') {
                        if (book[i].toLowerCase().match(searchString))
                            return true;
                    }
                return false;
            });
        }

        var self = this;
        var deleteLabelClass = "book-delete ";
        if (self.state.is_admin === false)
            deleteLabelClass += "hidden";

        return (
            <div>
                <div className="header">
                    <div className="logo">
                        <img src="./media/logo.png" />
                        <div className="logo-text">SimpleLibrary</div>
                    </div>
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
                    <ul className="column">
                        {
                            libraries.map(function (book, index) {
                                var bookClass = "book " + "book" + index;
                                return (
                                    <li className={bookClass} key={book.index}>
                                        <div className="book-image">
                                            <img src={book.image} className="book-image"/>
                                        </div>
                                        <div className="book-title">
                                            <span className="book-name">{book.name}</span>
                                            {
                                                book.borrowed ?
                                                    <a className="glyphicon glyphicon-eject return"
                                                       onClick={self.returnBook.bind(self, index)}>Return</a> :
                                                    <a className="glyphicon glyphicon-plus borrow"
                                                       onClick={self.borrow.bind(self, index)}>Borrow</a>
                                            }
                                        </div>
                                        <div className="book-description">{book.description}</div>
                                        <img src="./media/closelabel.png" className={deleteLabelClass} onClick={self.deleteBook.bind(self, book["index"], index)} />
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
/*
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
 */


React.render(<DisplayBoard />, document.getElementsByClassName("search-widget")[0]);

var AddBook = React.createClass({

    getInitialState: function() {
        return {
            name: "",
            description: "",
            image: "",
            visible: "hidden"
        }
    },

    componentDidMount: function() {
        var self = this;
        $(MessageListener).on("activateAddBook", function(){
            $(".mask").removeClass("hidden");
            self.setState({
                visible: ""
            });
            setTimeout(function() {
                $("input[name=book-name]").focus();
            });
        });

        $(MessageListener).on("deactivateAddBook", function(){
            $(".mask").addClass("hidden");
            self.setState({
                visible: "hidden"
            });
        })
    },

    nameChange: function(l) {
        this.setState({name: l.target.value});
    },

    descriptionChange: function(l) {
        this.setState({description: l.target.value});
    },

    imageChange: function(l) {
        this.setState({image: l.target.value});
    },

    submit: function() {
        var self = this;
        $.ajax({
            url: "http://" + window.location.hostname + ":8964/api/set",
            method: "POST",
            crossDomain: true,
            xhrFields: {
                withCredentials: true
            },
            data: JSON.stringify({
                name: this.state.name,
                description: this.state.description,
                image: this.state.image
            })
        }).then(function(e){
            alert(e);
            $(MessageListener).trigger("deactivateAddBook");
            $(MessageListener).trigger("AddedBook", {
                name: self.state.name,
                description: self.state.description,
                image: self.state.image,
                borrowed: false
            });
        }, function(e){
            alert(e);
        });
    },

    render: function() {
        var classString = "add-book " + this.state.visible;
        return (
            <div className={classString} >
                <img src="./media/closelabel.png" className="closelabel"
                     onClick={function(){$(MessageListener).trigger("deactivateAddBook")}}/>
                <span className="text">ADD BOOK</span>
                <input name="book-name" value={this.state.name} onChange={this.nameChange} placeholder="name" required />
                <input name="book-description" value={this.state.description} onChange={this.descriptionChange} placeholder="description" required />
                <input name="book-image" value={this.state.image} onChange={this.imageChange} className="addBook-lastInput" placeholder="image link" />
                <button onClick={this.submit} id="add-book-submit" >ADD</button>
            </div>
        );

    }
});

React.render(<AddBook />, document.getElementsByClassName("addBook-place-holder")[0]);


// check login
$(MessageListener).on("check_admin", function() {
    $.ajax({
        url: "http://" + window.location.hostname + ":8964/api/is_login",
        method: "POST",
        crossDomain: true,
        xhrFields: {
            withCredentials: true
        }
    }).then(function (e) {
        $(MessageListener).trigger("loggedin");
        if (e === '0')
            $(MessageListener).trigger("is_admin");
    }, function (e) {
        $(MessageListener).trigger("loggedout");
    });
});

$(MessageListener).trigger("check_admin");


$(function(){

    $(".login-lastInput").on("keydown", function(e){
        if (e.keyCode === 13) {
            setTimeout(function(){
                $("#login-submit").click();
            }, 10);
        }
    });

    $(".register-lastInput").on("keydown", function(e){
        if (e.keyCode === 13) {
            setTimeout(function() {
                $("#register-submit").click();
            }, 10);
        }
    });

    $(".addBook-lastInput").on("keydown", function(e){
        if (e.keyCode === 13) {
            setTimeout(function() {
                $("#add-book-submit").click();
            });
        }
    });
});