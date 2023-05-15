# Library Management System
# Grosu Gheorghe - 324CD

## Project description
This is a simple library management system implemented in C++ language. The system allows users to register, log in, and perform various operations related to library management, such as adding books, getting books, deleting books, and logging out.

## Client-server architecture
The system is based on a client-server architecture. The server is implemented using the C++ programming language and the client is implemented using the C/C++ programming language. The communication between the client and the server is done using TCP sockets.

## Nlohmann JSON library
The communication between the client and the server is done using JSON objects. The JSON objects are serialized and deserialized using the Nlohmann JSON library. I chose this library because it is very easy to use and it is also very well documented. The library is known for its ease of use, high performance, and flexibility. It offers a variety of features, including support for standard JSON types, such as objects, arrays, strings, numbers, and booleans.

## Steps to run the application
1. Clone the repository
2. Open the terminal and navigate to the project directory
3. Run the following commands:
```
make
./client
```

## Commands explanation
- `register_user` function takes a socket file descriptor as an argument and prompts the user to input their username and password. It then creates a JSON object using the create_json function with the given username and password. It then sends a POST request to the server with the JSON object as its body and prints out the status message and the registration message obtained from the response.
- `login_user` function takes a socket file descriptor as an argument and prompts the user to input their username and password. It then creates a JSON object using the create_json function with the given username and password. It then sends a POST request to the server with the JSON object as its body and prints out the status message and the login message obtained from the response. If the login is successful, the function extracts the cookie from the response and returns it.
- `get_access` function uses the socket file descriptor, the cookie_session, and a reference to a boolean variable access_flag as arguments. It sends a GET request to the server to get the access token required to access the library. It then prints out the status message and the message obtained from the response. If the response contains a body, the function parses it as JSON and extracts the access token from it and returns it. If the response contains an error message, the function prints it out and sets the access_flag to false.
- `get_books` function sends a GET request to the server to get the list of books from the library, using also cookie_session and an accessToken. It then prints out the status message and the message obtained from the response. If the response contains a body, the function parses it as JSON and prints out the list of books. If the response contains an error message, the function prints it out.
- `add_book` function takes the same arguments It prompts the user to input the book title, author, genre, and publisher and page_count. It then creates a JSON object using the create_json function with the given book title, author, genre, and publisher. It then sends a POST request to the server with the JSON object as its body and prints out the status message and the message obtained from the response. If the response contains an error message, the function prints it out and sets the access_flag to false.
- `get_book` function takes the same arguments It prompts the user to input the book id. It then creates a JSON object using the create_json function with the given book id. It then sends a GET request to the server with the JSON object as its body and prints out the status message and the message obtained from the response.
- `delete_book` function is very similar with get_book It sends a DELETE request to the server with the JSON object as its body and prints out the status message or error message obtained from the response.
- `logout_user` function needs only the cookie_sesion. It sends a GET request to the server to log out the user and prints out the status message and the message obtained from the response.

## Helper functions
- `create_json` function takes the necesarry arguments and creates a JSON object with the given fields.
- `parse_json` function takes a string as an argument and parses it as JSON.
- `get_status_message` function takes a string as an argument and parses it as JSON. It then extracts the status message from the JSON object and returns it.
- `get_user_credentials` function that prompts the user to input their username and password and returns them.
- `create_cookies_array` function takes the cookie_session as an argument and creates an array with the given cookie_session.
- `is_number` function takes a string as an argument and checks if it is a number.
- `read_id` function takes a string as an argument and checks if it is a number. It obliges the user to input a number.

The rest of the functions are self-explanatory. I started from the lab scheleton and I added the necesarry code to implement the required functionality. I also added some extra functions to make the code more readable and to avoid code duplication. 

## Endpoints
This is the list of endpoints used in the application:

- `POST /api/v1/tema/auth/register` - registers a user
- `POST /api/v1/tema/auth/login` - logs in a user
- `GET /api/v1/tema/library/access` - gets the access token
- `GET /api/v1/tema/library/books` - gets the list of books
- `POST /api/v1/tema/library/books` - adds a book
- `GET /api/v1/tema/library/books/{id}` - gets a book
- `DELETE /api/v1/tema/library/books/{id}` - deletes a book
- `GET /api/v1/tema/auth/logout` - logs out a user


## Status messages
I also created a few maps for storing the status messages and the error messages. I used maps because they are very efficient for storing key-value pairs. I used the status code as the key and the status message as the value.

Maps are created that associate HTTP status codes with corresponding status messages for the various API endpoints. These maps are used to return appropriate status codes and messages to API clients.

- status_messages - used for general status messages
- register_messages - used for the /api/v1/tema/auth/register endpoint
- login_messages - used for the /api/v1/tema/auth/login endpoint
- logout_messages - used for the /api/v1/tema/auth/logout endpoint
- library_access_messages - used for the /api/v1/tema/library/access endpoint
- view_books_messages - used for the /api/v1/tema/library/books endpoint

