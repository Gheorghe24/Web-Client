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

#define HOST "34.254.242.81"
#define PORT 8080

// application types
#define APP_TYPE_JSON "application/json"

// addresses
#define REGISTER "/api/v1/tema/auth/register" // POST
// #define LOGIN "/api/v1/tema/auth/login" // GET
#define LIBRARY_ACCESS "/api/v1/tema/library/access" // GET
#define LIBRARY_INFO "/api/v1/tema/library/books"    // GET
#define BOOKS "/api/v1/tema/library/books"           // GET, POST
#define BOOKS_ID "/api/v1/tema/library/books/"       // GET, DELETE
#define LOGOUT "/api/v1/tema/auth/logout"            // GET

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

int main()
{
    return 0;
}