# SimpleLib

2015 FDU C++ Final Project 

# Installation

## Dependency 
	boost cmake
	
## Back-End	
### Ubuntu
	apt-get install boost-all-dev cmake
	git clone https://github.com/voidrank/SimpleLib.git
	cd SimpleLib
	cmake -H. -Bbuild
	cd build 
	make && ./app
	## if you have an i5, you can run
	make -j4
	## for shorter compile time
	
### OSX
	## Suppose you have installed [Homebrew](http://brew.sh/)
	brew install boost cmake
	git clone https://github.com/voidrank/SimpleLib.git
	cd SimpleLib
	cmake -H. -Bbuild
	cd build 
	make && ./app
	## if you have an i5, you can run
	make -j4
	## for shorter compile time

## Front-End
	## Suppose you have installed python2.7
	## open another terminal and then go to /path/to/SimpleLib/
	python -m SimpleHTTPServer 8080
	## And then browse http://localhost:8080/Lib.html
	
	
# Some Explanation

## Back-End
Base on [CROW webframework](https://github.com/ipkn/crowd)
### Middleware
I build 4 middleware for this app

1. Cookie Parser. It's a widget of CROW. Simply use it for load Cookie from HTTP header.

2. Cookie middleware, generate a ramdon cookie (without encryption. It's an TODO to add a secret key for it. However, it isn't important regardless of the security.)

3. Session middleware. Map a session to Cookie. I store **username** in the session.

4. UserManager middleware. It an **unordered_map** of C++ STL.(unordered_map<username, pair<password, auth>>). **auth** stores identity of the user {anonymous user, user, admin}. 

### Backend:  

> namespace Library manage stored books. Library::Books is a **vector** in C++ STL. **Library::Books** store books with **crow::json::wvalue** of CROW which can manage **JSON datatype** in C++.  
> 
> namespace BorrowAndReturn store the relationship of books and users.


### Web API

Login
> @method post  
> @url /api/login  
> @data  JSON {"username": string, "password": string}

Logout
> @method post  
> @url /api/logout  

Register
> @method post  
> @url /api/login  
> @data  JSON {"username": string, "password": string}

Load
> @mothed get  
> @url /api/login  
> @param ?bookname=:string  
> @description load book which contains **bookname**. null **bookname** means load all.  
> @return array of books

Set
> @method get  
> @url /api/set  
> @data JSON book    
> @description store a book (admin only)

Borrow
> @mothod post  
> @url /api/borrow  
> @data JSON {"index": number}  

Return
> @method post  
> @url /api/return  
> @data JSON {"index": number}  

Delete
> @method post  
> @url /api/delete  
> @data JSON {"index": number}   
> @description admin only  

Is_login
> @method post   
> @url /api/is_login  
> @return **auth** of current user 