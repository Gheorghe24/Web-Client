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
const char *VIEW_BOOKS_ID = "/api/v1/tema/library/books/";  // GET, DELETE
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
    {201, "Created"},
    {400, "Username already exists"}};

// function to get status code from first line of response
// this is the first line "Response: HTTP/1.1 200 OK"
int get_status_code(char *response)
{
    char *status_code = strtok(response, " ");
    status_code = strtok(NULL, " ");
    return atoi(status_code);
}

// function to get username and password from user
void get_user_credentials(string &username, string &password)
{
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;
}

// function to register a new user
void register_user(int sockfd, string username, string password)
{
    json j;
    j["username"] = username;
    j["password"] = password;
    // content of json to string
    string j_string = j.dump();

    char *message = compute_post_request(HOST, REGISTER, APP_TYPE_JSON, j_string.c_str(), NULL, 0, NULL);
    // message to chr array
    printf("Message: %s\n", message);

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    printf("Response: %s\n", response);

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
string login_user(int sockfd, string username, string password, bool &logged_in_flag)
{
    string cookie = "";
    json j;
    j["username"] = username;
    j["password"] = password;
    string j_string = j.dump();
    char *message = compute_post_request(HOST, LOGIN, APP_TYPE_JSON, j_string.c_str(), NULL, 0, NULL);
    printf("Message: %s\n", message);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);
    printf("Response: %s\n", response);

    // check if response has body, get the cookie if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;
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
string get_access(int sockfd, string cookie)
{
    string access_token = "";
    char **cookies = (char **)malloc(sizeof(char *));
    cookies[0] = (char *)malloc(BUFLEN);
    strcpy(cookies[0], cookie.c_str());
    // print cookie from parameter
    printf("Cookie1: %s\n", cookie.c_str());
    printf("Cookie2: %s\n", cookies[0]);

    char *message = compute_get_request(HOST, LIBRARY_ACCESS, NULL, cookies, 1, NULL);
    printf("Message: %s\n", message);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);
    printf("Response: %s\n", response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;
    json j = json::parse(body);

    if (body != NULL)
    {
        // get access token from response
        access_token = j["token"].get<string>();
        cout << access_token << endl;
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
void get_books(int sockfd, string access_token)
{
    char **cookies = (char **)malloc(sizeof(char *));
    cookies[0] = (char *)malloc(BUFLEN);
    strcpy(cookies[0], access_token.c_str());
    char *message = compute_get_request(HOST, VIEW_BOOKS, NULL, cookies, 1, access_token.c_str());
    printf("Message: %s\n", message);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *copy = (char *)malloc(BUFLEN);
    strcpy(copy, response);
    printf("Response: %s\n", response);

    // check if response has body, get the access token if it does
    char *body = basic_extract_json_response(response);
    // get first line of response
    char *first_line = strtok(copy, "\n");
    // status code
    int status_code = get_status_code(first_line);
    cout << status_messages[status_code] << endl;

    // the response is like this
    /*
    Întoarce o listă de obiecte json:
[
{
id: Number,
title: String
}
    */

    // if status code is 200, print books
    if (status_code == 200)
    {
        cout << body << endl;
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
    int sockfd = -1, n, ret;
    struct sockaddr_in serv_addr;
    bool logged_in_flag = false;
    string cookie = "";
    string accessToken = "";
    status_codes status;

    // read user input
    while (true)
    {
        string command;
        cin >> command;
        if (command.compare("exit") == 0) // exit command
        {
            logged_in_flag = false;
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
            string username, password;
            get_user_credentials(username, password);
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                error("ERROR opening socket");
            }
            register_user(sockfd, username, password);
            close_connection(sockfd);
        }
        else if (command.compare("login") == 0)
        {
            if (logged_in_flag)
            {
                cout << "You are already logged in!" << endl;
                continue;
            }
            string username, password;
            get_user_credentials(username, password);
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                error("ERROR opening socket");
            }
            cookie = login_user(sockfd, username, password, logged_in_flag);
            printf("Cookie from LOG: %s\n", cookie.c_str());
            close_connection(sockfd);
        }
        else if (command.compare("enter_library") == 0)
        {
            if (!logged_in_flag)
            {
                cout << "You are not logged in!" << endl;
                continue;
            }
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                error("ERROR opening socket");
            }
            accessToken = get_access(sockfd, cookie);
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

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                error("ERROR opening socket");
            }
            get_books(sockfd, accessToken);
            close_connection(sockfd);
        } else {
            cout << "Invalid command!" << endl;
        }

        
    }

    return 0;
}