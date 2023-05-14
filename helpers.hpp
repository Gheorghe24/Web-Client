#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(const char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

char *advanced_extract_json_response(char *str);

// get status code from a server response
int get_status_code(char *str);

// create a json with username and password
string create_json(string username, string password);

// function to get username and password from user
void get_user_credentials(string &username, string &password);

// function to create array of cookies from just one cookie
char **create_cookies_array(string cookie);

// function to create the json for adding a book
string create_json(string title, string author, string genre, string publisher, string page_count);

// function to check if a string is a number
bool is_number(string s);

string read_id();

#endif
