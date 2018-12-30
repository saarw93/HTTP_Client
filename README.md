Name of Student: Saar Weitzman

HTTP Client

-------------List of files------------

1. client.c - Code file written in c programming language, contains the main function, and all the implementaions of the functions of the program.


-------------Description-------------

This program implements a data structure of an hash table, and the handling with collisions is done with linked lists.
The hash table support these three operations: 1. Adding an element 2. Removing an element 3. Searching for an element.
The hash table can be from the type of integers, or from the type of strings. Each type has little different implementation of the hash function.
In this program we limit the size of the linked lists that can be made. In case there is an object that the hash function sends to a full linked
list, the way this hash table deal with it is by duplicating the hash table size (i.e, expand the table).  
In case of expansion, all the existed objects of the table in entries 0-(n-1) will be mapped to the respective even entries in the new hash table.


This program implements an HTTP client that constructs an HTTP request based on user’s command line input, sends the request to a Web server,
receives the reply from the server, and displays the reply message on screen (That program supports only IPv4 connections). This program can build
a GET request and a POST request.


--------Installation/Compilation of the program---------

open linux terminal, navigate to the folder containing "ex2" folder using the "cd" command (confirm it by using ls command)
type in the terminal: gcc -Wall –o client client.c .


-----------Activation of the program--------------------

open linux terminal, navigate to tester executeable file location using "cd" command (confirm it using ls command) 
and type: ./client . To run the program with valgrind, type: valgrind ./client .


----------program functions and their output---------------

int check_if_post(char* str);
output: returns 1 if 'str' input is a post, if not, returns -1.

int check_for_r(char* str);
output: returns 1 if there is the '-r' sign for parameters, if not, it returns -1. 

char* save_text_for_post(char* text, int r_flag, char* params, char* url_tokens_arr[], char* request);
output: returns a pointer to the text of the post request. If there is no text, it returns NULL.

char* append_params(char* argv[],int r_index, int num_of_params);
output: returns the append of the parameters in the way it needs to be for the HTTP client's request. If there are no parameters, it returns NULL.

int find_url(char* str);
output: returns 1 if 'str' is the URL for the request, if not, it returns -1.

char** split_url_to_tokens(char* url, int post_flag, int r_flag, char* p_text, char* params, char* request);
output: returns an array with the URL's 3 parts: hostName (index 0), port (index 1) and path (index 2).

int make_socket_connect(char* url_tokens_arr[], int post_flag, int r_flag, char* p_text, char* params, char* request);
output: returns the file descriptor of the socket.

char* get_request(char* url_tokens_arr[],int r_flag, char* params);
output: returns a pointer to the string of the GET request.

char* post_request(char* url_tokens_arr[], char* text, int r_flag, char* params);
output: returns a pointer to the string of the POST request.

int count_digits_p_text(char* p_text);
output: returns the number of digits 'p_text' is made of.

int is_only_digits(char* str);
output: returns 1 is there is not a non digit char in 'str, otherwise it returns -1. 

void free_program(int post_flag, int r_flag, char* p_text, char* params, char* url_tokens_arr[], char* request);
output: frees all the memory which was allocated during the running of the program.

int check_params_validation(int num_of_params, int r_index, char* argv[], int argc);
output: returns 1 if the parameters which were given in the cmd input are valid, otherwise ot returns -1.
