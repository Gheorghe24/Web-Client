// 323CB Cazan Bogdan-Marian

#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"
#include "buffer.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include "nlohmann/json.hpp"

// for convenience
using json = nlohmann::json;

using namespace std;

const char *HOST = "34.254.242.81";
#define PORT 8080

// application types
const char *APP_TYPE_JSON = "json";

// define endpoints
const char *REGISTER = "/api/v1/tema/auth/register";        // POST
const char *LOGIN = "/api/v1/tema/auth/login";              // GET
const char *LIBRARY_ACCESS = "/api/v1/tema/library/access"; // GET
const char *VIEW_BOOKS = "/api/v1/tema/library/books";      // GET, POST
const char *LOGOUT = "/api/v1/tema/auth/logout";            // GET

// define data status codes in a structure for easier access
typedef struct StatusCodes
{
    int OK = 200;
    int CREATED = 201;
    int BAD_REQUEST = 400;
    int UNAUTHORIZED = 401;
    int NOT_FOUND = 404;
    int INTERNAL_SERVER_ERROR = 500;
} status_codes;

// complete a map between status codes and their messages
map<int, string> status_messages = {
    {200, "OK"},
    {201, "Created"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {404, "Not Found"},
    {500, "Internal Server Error"}};

// complete a map between status codes and their messages for the register function
map<int, string> register_messages = {
    {201, "You have registered successfully"},
    {400, "Username already exists"}};

// complete a map between status codes and their messages for the login function
map<int, string> login_messages = {
    {200, "You have logged in successfully"},
    {400, "Wrong username or password"}};

// complete a map between status codes and their messages for the logout function
map<int, string> logout_messages = {
    {200, "You have logged out successfully"},
    {400, "You are not logged in"}};

map<int, string> library_access_messages = {
    {200, "You have access to the library"},
    {400, "You are not logged in"}};

map<int, string> view_books_messages = {
    {200, "Books retrieved successfully"},
    {400, "You are not logged in"}};
    

// function to register a new user
void register_user(int sockfd)
{
    string username, password;
    get_user_credentials(username, password);

    string j_string = create_json(username, password);

    char *message = compute_post_request(HOST, REGISTER, APP_TYPE_JSON, j_string.c_str(), NULL, 0, NULL);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // check response status code and print message from map
    int status_code = get_status_code(response);
    cout << status_messages[status_code] << endl;
    cout << register_messages[status_code] << endl;

    // check if response has body and print it
    if (strstr(response, "{") != NULL)
    {
        cout << response << endl;
    }

    free(response);
}

// function to login a user
string login_user(int sockfd, bool &logged_in_flag)
{
    string username, password;
    get_user_credentials(username, password);
    string cookie = "";
    string j_string = create_json(username, password);
    char *message = compute_post_request(HOST, LOGIN, APP_TYPE_JSON, j_string.c_str(), NULL, 0, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the cookie if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;
    cout << login_messages[status_code] << endl;
    if (body != NULL)
    {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
        logged_in_flag = false;
    }
    else
    {
        // get cookie from response
        char *cookie_ptr = strstr(response, "connect.sid");
        cookie_ptr = strtok(cookie_ptr, ";");
        cookie = cookie_ptr;
        logged_in_flag = true;
    }

    free(response);
    free(copy);
    return cookie;
}

// function to get access to library
string get_access(int sockfd, string cookie, bool &access_flag)
{
    string access_token = "";
    char **cookies = create_cookies_array(cookie);

    char *message = compute_get_request(HOST, LIBRARY_ACCESS, NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;
    cout << library_access_messages[status_code] << endl;
    json j = json::parse(body);

    if (body != NULL)
    {
        // get access token from response
        access_token = j["token"].get<string>();
        access_flag = true;
    }
    else
    {
        // print error message if login failed, use parsed json
        cout << "Error" << endl;
        cout << j["error"] << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
    return access_token;
}

// function to get books from library
void get_books(int sockfd, string access_token, string cookie)
{
    char **cookies = create_cookies_array(cookie);
    char *message = compute_get_request(HOST, VIEW_BOOKS, NULL, cookies, 1, access_token.c_str());
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = advanced_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;
    cout << view_books_messages[status_code] << endl;

    // if status code is 200, print books
    if (status_code == 200)
    {
        
        if (body != NULL)
        {
            // print books
            cout << body << endl;
        } else {
            cout << "No books in library" << endl;
        }

    } else {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
}

// function to add a book to library
void add_book(int sockfd, string access_token, string cookie)
{
    string title, author, genre, publisher, page_count;
    cout << "title: ";
    // the title can contain spaces
    getline(cin, title);
    cout << "author: ";
    cin >> author;
    cout << "genre: ";
    cin >> genre;
    cout << "publisher: ";
    cin >> publisher;
    cout << "page_count: ";
    cin >> page_count;

    // check if page count is a number
    while (!is_number(page_count))
    {
        cout << "Page count must be a number" << endl;
        cout << "page_count: ";
        cin >> page_count;
    }

    string j_string = create_json(title, author, genre, publisher, page_count);

    char **cookies = create_cookies_array(cookie);
    char *message = compute_post_request(HOST, VIEW_BOOKS, APP_TYPE_JSON, j_string.c_str(), cookies, 1, access_token.c_str());
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_code << " " << status_messages[status_code] << endl;

    // if status code is 200, print books
    if (status_code != 200)
    {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
    } else {
        cout << "Book added successfully" << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
}

// function to get a book from library
void get_book(int sockfd, string access_token, string cookie)
{
    string id = read_id();

    char *url = (char *)malloc(BUFLEN);
    sprintf(url, "%s/%s", VIEW_BOOKS, id.c_str());

    char **cookies = create_cookies_array(cookie);
    char *message = compute_get_request(HOST, url, NULL, cookies, 1, access_token.c_str());
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;

    // if status code is 200, print books
    if (status_code == 200)
    {
        json j = json::parse(body);
        cout << "Title: " << j["title"] << endl;
        cout << "Author: " << j["author"] << endl;
        cout << "Genre: " << j["genre"] << endl;
        cout << "Publisher: " << j["publisher"] << endl;
        cout << "Page count: " << j["page_count"] << endl;
    }
    else
    {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
}

// function to delete a book from library
void delete_book(int sockfd, string access_token, string cookie)
{
    string id = read_id();

    char **cookies = create_cookies_array(cookie);

    char *url = (char *)malloc(BUFLEN);
    sprintf(url, "%s/%s", VIEW_BOOKS, id.c_str());
    char *message = compute_delete_request(HOST, url, NULL, cookies, 1, access_token.c_str());
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;

    // if status code is 200, print books
    if (status_code != 200)
    {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
    } else {
        cout << "Book deleted successfully" << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
}

// function to logout from server
void logout(int sockfd, string cookie, bool &logged_in)
{
    char **cookies = create_cookies_array(cookie);

    char *message = compute_get_request(HOST, LOGOUT, NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;

    // if status code is 200, print books
    if (status_code == 200)
    {
        logged_in = false;
        cout << "Logged out successfully" << endl;
    }
    else
    {
        // print error message if login failed, use parsed json
        json j = json::parse(body);
        cout << "Error" << endl;
        cout << j["error"] << endl;
    }

    free(response);
    free(copy);
    free(cookies[0]);
    free(cookies);
}

int main(int argc, char *argv[])
{
    int sockfd = -1;
    bool logged_in_flag = false;
    bool library_flag = false;
    string cookie = "";
    string accessToken = "";

    // read user input
    while (true)
    {
        string command;
        // cin an entire line
        getline(cin, command);
        // get first word from line
        command = command.substr(0, command.find(" "));

        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            error("ERROR opening socket");
        }

        if (command.compare("exit") == 0) // exit command
        {
            if (sockfd > 0)
                close_connection(sockfd);
            break;
        }
        else if (command.compare("register") == 0) // register command
        {
            if (logged_in_flag)
            {
                cout << "You are already logged in!" << endl;
                continue;
            }
            cout << "--Register account--" << endl;
            string username, password;
            register_user(sockfd);
            close_connection(sockfd);
        }
        else if (command.compare("login") == 0)
        {
            if (logged_in_flag)
            {
                cout << "You are already logged in!" << endl;
                continue;
            }
            cout << "--Login--" << endl;
            string username, password;
            cookie = login_user(sockfd, logged_in_flag);
            close_connection(sockfd);
        }
        else if (command.compare("enter_library") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }
            cout << "--Enter library--" << endl;
            accessToken = get_access(sockfd, cookie, library_flag);
            close_connection(sockfd);
        }
        else if (command.compare("get_books") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }

            if (accessToken == "")
            {
                cout << "You don't have access to the library!" << endl;
                continue;
            }
            cout << "--Get books--" << endl;
            get_books(sockfd, accessToken, cookie);
            close_connection(sockfd);
        }
        else if (command.compare("add_book") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }

            if (accessToken == "")
            {
                cout << "You don't have access to the library!" << endl;
                continue;
            }
            cout << "--Add book--" << endl;
            add_book(sockfd, accessToken, cookie);
            close_connection(sockfd);
        }
        else if (command.compare("get_book") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }

            if (accessToken == "")
            {
                cout << "You don't have access to the library!" << endl;
                continue;
            }
            cout << "--Get book--" << endl;
            get_book(sockfd, accessToken, cookie);
            close_connection(sockfd);
        }
        else if (command.compare("delete_book") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }

            if (accessToken == "")
            {
                cout << "You don't have access to the library!" << endl;
                continue;
            }
            cout << "--Delete book--" << endl;
            delete_book(sockfd, accessToken, cookie);
            close_connection(sockfd);
        }
        else if (command.compare("logout") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }
            cout << "--Logout--" << endl;
            logout(sockfd, cookie, logged_in_flag);
            close_connection(sockfd);
        }
        else
        {
            close_connection(sockfd);
            if (command.compare("") == 0) // junk command
                continue;
            cout << "Invalid command!" << endl;
        }
    }

    return 0;
}