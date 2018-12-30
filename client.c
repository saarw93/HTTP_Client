/**Author: Saar Weitzman
 * Date: 30.12.18
 * HTTP Client implementation
 * **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define DEBUG
#define ON 1
#define BUFFER_SIZE 5000
#define USAGE "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\r\n"

int check_if_post(char* str);
int check_for_r(char* str);
char* save_text_for_post(char* text, int r_flag, char* params, char* url_tokens_arr[], char* request);
char* append_params(char* argv[],int r_index, int num_of_params);
int find_url(char* str);
char** split_url_to_tokens(char* url, int post_flag, int r_flag, char* p_text, char* params, char* request);
int make_socket_connect(char* url_tokens_arr[], int post_flag, int r_flag, char* p_text, char* params, char* request);
char* get_request(char* url_tokens_arr[],int r_flag, char* params);
char* post_request(char* url_tokens_arr[], char* text, int r_flag, char* params);
int count_digits_p_text(char* p_text);
void free_program(int post_flag, int r_flag, char* p_text, char* params, char* url_tokens_arr[], char* request);
int check_params_validation(int num_of_params, int r_index, char* argv[], int argc);
int is_only_digits(char* str);


int main(int argc, char* argv[])
{
    int post_flag = 0, r_flag = 0, url_flag = 0, sockfd = 0;
    int i, p_index = 0, r_index = 0, num_of_params = -1, counter = argc-1;
    char* p_text = NULL, *params = NULL, *url, *request = NULL;
    char** url_tokens_arr = NULL;

    for (i = 1; i < argc; i++)
    {
        if (check_if_post(argv[i]) == 1)
        {
            if (post_flag == ON)
            {
                printf(USAGE);
                free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
                exit(1);
            }

            p_index = i;  //the index where -p is in argv
            i++;

            p_text = save_text_for_post(argv[i], r_flag, params, url_tokens_arr, request);
            post_flag = ON;
            counter-= 2; //sub the -p and text from counter
            continue;
        }

        if (check_for_r(argv[i]) == 1)
        {
            if (r_flag == ON)
            {
                printf(USAGE);
                free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
                exit(1);
            }
            r_index = i;   //the index where -r is in argv

            if (is_only_digits(argv[r_index+1]) == -1)  //there is a non digit char in argv[r_index+1], or -r is in the last index of argv
            {
                printf(USAGE);
                free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
                exit(1);
            }

            num_of_params = atoi(argv[i+1]);

            if (check_params_validation(num_of_params, r_index, argv, argc) == -1)
            {
                printf(USAGE);
                free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
                exit(1);
            }

            params = append_params(argv, r_index, num_of_params); //appends the parameters
            r_flag = ON;
            i = i + num_of_params + 1;
            counter = counter - (2 + num_of_params);   //sub -r, the number after it and the number of parameters

            #if defined(DEBUG)
                printf("The parameters append = %s\n", params);
            #endif
            continue;
        }

        if (find_url(argv[i]) != -1)
        {

            url_tokens_arr = split_url_to_tokens(argv[i], post_flag, r_flag, p_text, params, request);
            url_flag = ON;
            counter -= 1;

            #if defined(DEBUG)
            printf("The url is: %s\r\n", argv[i]);
            for (int j = 0; j < 3; ++j)
                printf("url_token_arr[%d] = %s\r\n", j, url_tokens_arr[j]);
            #endif
        }
    }

    if (counter != 0)  //there are too much or too many arguments in argv than what was supposed to be
    {
        printf(USAGE);
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }

    if (url_flag == ON)
    {
        if (post_flag == ON)
            request = post_request(url_tokens_arr, p_text, r_flag, params);
        else  //it's a GET request
            request = get_request(url_tokens_arr, r_flag, params);

        printf("HTTP request =\n%s\nLEN = %d\n", request, (int) strlen(request));

        sockfd = make_socket_connect(url_tokens_arr, post_flag, r_flag, p_text, params, request);

        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);

        int sum = 0, nbytes = 0;
        write(sockfd, request, strlen(request) + 1);

        #if defined(DEBUG)
            printf("sockfd is: %d\r\n", sockfd);
            printf("wr = %d\r\n", (int)write(sockfd, request, strlen(request) + 1));
        #endif

        while(1)
        {
            nbytes = (int)read(sockfd, buffer + sum, BUFFER_SIZE - sum);
            sum += nbytes;

            if (nbytes == 0)
                break;
            if (nbytes < 0)
            {
                perror("ERROR: cannot read from socket");
                exit(1);
            }
        }
        printf("The Server Response =\r\n%s\r\n", buffer);
        printf("\r\n Total received response bytes: %d\r\n", sum);

        close(sockfd);

        #if defined(DEBUG)
        printf("we have -p at place %d , we have %s text after -p, and we have -r at place %d in the cmd input\r\n", p_index,p_text, r_index);
        #endif
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
    }
    else
    {
        printf(USAGE);
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }
    return 0;
}


/*Method to check if 'str' input is a post or not */
int check_if_post(char* str)
{
    if (strcmp(str, "-p") == 0)
        return 1;
    return -1;
}


/*Method to check if 'str' input is '-r', which symbols that there are parameters*/
int check_for_r(char* str)
{
    if (strcmp(str, "-r") == 0)
        return 1;
    return -1;
}


/*Method to check that the parameters which were given in the cmd input are valid*/
int check_params_validation(int num_of_params, int r_index, char* argv[], int argc)
{
    if (r_index+1 >= argc)  //check if -r is in the last index of argv
        return -1;

    if (num_of_params < 0 || (num_of_params == 0 && strcmp("0", argv[r_index+1]) != 0)) //check if num_of_params is negative or not a number (i.e 0)
        return -1;

    if (num_of_params == 0)  //num_of_params is a real zero, there should be no parameters
        return 1;

    char* ptr = NULL;
    int i, first_param = r_index+2;

    if (r_index+ 1 + num_of_params >= argc)  //check if there are less indexes left in argv than the given number of parameters
        return -1;

    for (i = first_param; i < first_param + num_of_params ; i++)  //check the equals validation of the parameters
    {
        ptr = strchr(argv[i], '=');

        if (ptr == NULL) //there is no '=' sign in the parameters string
            return -1;

        if (argv[i] == ptr)   //there is no parameter's name before the '=' sign
            return -1;

        if(*(ptr+1) == '\0')  //there is no parameter's value after the '=' sign
            return -1;
    }
    return 1;
}


/*Method to save the text of the post in a new allocation on the heap*/
char* save_text_for_post(char* text, int r_flag, char* params, char* url_tokens_arr[], char* request)
{
    if (text == NULL) //that happens in case -p is in the last place in argv
        return NULL;

    char* text_ptr = (char*)malloc(strlen(text)+1);
    if (text_ptr == NULL)
        return NULL;
    bzero(text_ptr, strlen(text)+1);

    strcpy(text_ptr, text);

    char* temp_ptr = text;
    while(*temp_ptr != '\0')  //TODO check if need this while check loop
    {
        if (*temp_ptr == '/' || *temp_ptr == ':' || *temp_ptr == '?' || *temp_ptr == '&')
            {
            printf(USAGE);
            free_program(ON, r_flag, text_ptr, params, url_tokens_arr, request);
            exit(1);
            }
        temp_ptr++;
    }

    return text_ptr;
}


/*Method to append the parameters in the way it needs to be for the HTTP client's request*/
char* append_params(char* argv[],int r_index, int num_of_params)
{
    if (num_of_params == 0)
        return NULL;

    int params_start = r_index + 2;   //the index where the first parameter is in agrv[]
    int i, size = 0;
    for (i = params_start; i < params_start+num_of_params ; i++)
    {
        size += strlen(argv[i]);
    }

    size += num_of_params+1;  //num_of_params+1 is for the ampersands between the parameters, the question mark, and '\0' at the end of params
    char* params = (char*)malloc(size * sizeof(char));
    if (params == NULL)
        return NULL;
    bzero(params, size* sizeof(char));  // initial with '/0' the params malloc

    strcat(params, "?");

    for (i = params_start; i < params_start+num_of_params ; i++)
    {
        strcat(params, argv[i]);
        if(i != params_start+num_of_params-1)  //in case we are in the last parameter to be appended
            strcat(params, "&");
    }
    return params;
}


/*Method to check if 'str' is the URL for the request*/
int find_url(char* str)
{
    char* sub_str = strstr(str, "http://");

    if (sub_str)
        return 1;
    return -1;
}


/*Method to split the URL to it's 3 parts: hostName, port and path*/
char** split_url_to_tokens(char* url, int post_flag, int r_flag, char* p_text, char* params, char* request)
{
    if (url)
    {
        char* temp_token = NULL;  //cuts off the "http://" from url
        char** url_tokens_arr = (char**)malloc(sizeof(char*)*3); //array of size 3 which contains: |host|port|path|
        if (url_tokens_arr == NULL)
            return NULL;
        bzero(url_tokens_arr, 3* sizeof(char*));  // initial with '/0' the url_tokens_arr malloc

        temp_token = url+7*sizeof(char);   // 7 is the length of string "http://"


        char* check_port = strchr(temp_token, ':'); //check if there is a port in the url or not
        char* first_slash = strchr(temp_token, '/');
        if (check_port != NULL) {
            if (check_port + 1 * sizeof(char) == first_slash)  //checks if the char after ':' is '/', i.e, we have no port although there was ':' in the url
            {
                printf(USAGE);
                free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
                exit(1);
            }
        }

        temp_token = strtok(temp_token, ":/");  // get the first token (i.e, the host)
        int i = 0;
        while (temp_token != NULL)   // walk through other tokens (i.e, the port and path)
        {
            if (i == 0)
            {
                url_tokens_arr[i] = temp_token;
                i = i+1;
                if (check_port != NULL)
                    temp_token = strtok(NULL, ":/");
            }

            else if (i == 1)
            {
                if (check_port == NULL)
                    url_tokens_arr[i] = "80"; //there is no port in the url input, so need to put the well known port number of http: 80
                else
                    url_tokens_arr[i] = temp_token; //there is port in the url input

                i++;
                temp_token = strtok(NULL, "");  //to get the rest of the file, which is the path
            }

            else if (i == 2)
            {
                url_tokens_arr[i] = temp_token;
                i++;
            }
            else
            {
                temp_token = strtok(NULL, "/:");
            }
        }
        return url_tokens_arr;
    }
    return NULL;
}


/*Method that makes the socket and builds the connection with the server it need to turn to by the URL it gets*/
int make_socket_connect(char* url_tokens_arr[], int post_flag, int r_flag, char* p_text, char* params, char* request)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    struct hostent *hp;

    if (url_tokens_arr == NULL) {
        perror("ERROR: url is NULL\n");
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }

    sockfd = socket(PF_INET, SOCK_STREAM, 0);    //create an endpoint for communication
    if (sockfd < 0) {
        perror("ERROR opening socket");
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }

    serv_addr.sin_family = AF_INET;

    hp = gethostbyname(url_tokens_arr[0]);
    if (hp == NULL)
    {
        herror("ERROR, no such host\r\n");
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }

    bcopy(hp->h_addr, &serv_addr.sin_addr, hp->h_length);

    #if defined(DEBUG)
    int i = 0;
    while(hp->h_addr_list[i] != NULL) {
        printf("h_addr_list[i]: %s\n", inet_ntoa( (struct in_addr) *((struct in_addr *) hp->h_addr_list[i])));
        i++;
    }
    #endif

    if (is_only_digits(url_tokens_arr[1]) == -1)
        serv_addr.sin_port = htons(atoi("portHasNonDigitChar"));  //give a non digits port because atoi takes off the non digits and give only the nums
    else
        serv_addr.sin_port = htons(atoi(url_tokens_arr[1]));   //htons function converts the unsigned short integer (the port) from host byte order to network byte order

    serv_addr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;

    printf("host:%s\npath:%s\nport:%s\r\n\r\n",url_tokens_arr[0],url_tokens_arr[2],url_tokens_arr[1]);

    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0)      //initiate a connection on a socket
    {
        perror("ERROR connecting\r\n");
        free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
        exit(1);
    }
    return sockfd;
}


/*Method to make the GET request*/
char* get_request(char* url_tokens_arr[],int r_flag, char* params)
{
    char* hostName = url_tokens_arr[0];
    char* filePath = url_tokens_arr[2];
    char* request_str = NULL;
    int size = 0;

    if (filePath == NULL)
        size+= strlen(hostName);    //adds the size of the hostName
    else
        size+= strlen(hostName) + strlen(filePath); //adds the size of the hostName and the filePath

    size+= 29;     //adds the size of GET /, HTTP/1.0, HOST:,\r\n\r\n, spaces, \r\n\r\n at the end and '/0' to the end of string

    if (r_flag == ON && params != NULL )  //we have parameters in the request
        size += (strlen(params));

    request_str = (char*)malloc(size*sizeof(char));
    if (request_str == NULL)
    {
        perror("malloc of request_str failed\r\n");
        return NULL;
    }
    bzero(request_str, size*sizeof(char));  // initial with '/0' the request_str malloc

    if (r_flag == ON && params != NULL)
    {
        if (filePath != NULL)
            sprintf(request_str, "GET /%s%s HTTP/1.0\r\nHost: %s\r\n\r\n", filePath, params, hostName);
        else   //filePath = NULL
            sprintf(request_str, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", params, hostName);
    }
    else
    {
        if (filePath != NULL)
            sprintf(request_str, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", filePath, hostName);
        else   //filePath = NULL
            sprintf(request_str, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", hostName);
    }
    return request_str;
}


/*Method to make the POST request*/
char* post_request(char* url_tokens_arr[], char* text, int r_flag, char* params)
{
    char* hostName = url_tokens_arr[0];
    char* filePath = url_tokens_arr[2];
    char* request_str = NULL;
    int size = 0;

    if (filePath == NULL)
        size+= strlen(hostName);    //adds the size of the hostName
    else
        size+= strlen(hostName) + strlen(filePath); //adds the size of the hostName and the filePath

    size+= strlen(text);  //adds the size of the text of the post request

    int p_text_digits = count_digits_p_text(text);
    size+= p_text_digits; //adds place to the num of digits that the Content-Length needs

    size+= 50;    //adds the size of POST /, HTTP/1.0, HOST:,\r\n\r\n, Content-Length: spaces, \r\n\r\n at the end and '/0' to the end of string

    if (r_flag == ON && params != NULL)  //we have parameters in the request
        size += (strlen(params));

    request_str = (char*)malloc(size*sizeof(char));
    if (request_str == NULL)
    {
        perror("malloc of request_str failed\r\n");
        return NULL;
    }
    bzero(request_str, size*sizeof(char));  // initial with '/0' the request_str malloc

    if (r_flag == ON && params != NULL)
    {
        if (filePath != NULL)
            sprintf(request_str, "POST /%s%s HTTP/1.0\r\nHost: %s\r\nContent-length:%d\r\n\r\n%s\r\n", filePath, params, hostName, (int)strlen(text), text);
        else
            sprintf(request_str, "POST /%s HTTP/1.0\r\nHost: %s\r\nContent-length:%d\r\n\r\n%s\r\n", params, hostName, (int)strlen(text), text);
    }
    else
    {
        if (filePath != NULL)
            sprintf(request_str, "POST /%s HTTP/1.0\r\nHost: %s\r\nContent-length:%d\r\n\r\n%s\r\n", filePath, hostName, (int)strlen(text), text);
        else
            sprintf(request_str, "POST / HTTP/1.0\r\nHost: %s\r\nContent-length:%d\r\n\r\n%s\r\n", hostName, (int)strlen(text), text);

    }
    return request_str;
}


/*Method to count from how many chars does the text's size of p is made of*/
int count_digits_p_text(char* p_text)
{
    int counter = 0;
    int text_length = (int)strlen(p_text);

    while(text_length != 0)
    {
        text_length/= 10;
        counter++;
    }
    return counter;
}


/*Method to check if there is a non digit char in a string*/
int is_only_digits(char* str)
{
    if (str != NULL)
    {
        char* ptr_check = str;
        while(*ptr_check != '\0')
        {
            if (*ptr_check < '0' || *ptr_check > '9')  //if returns 1 (true), then we have a non digit char in the port
                return -1;  //there is a non digit char in the string 'str'

            ptr_check+= 1*sizeof(char);
        }
        return 1;  //there is not a non digit char in the string 'str'
    }
    else
        return -1;  //the string 'str' is NULL
}


/*Method to free all the memory which was allocated during the running of the program*/
void free_program(int post_flag, int r_flag, char* p_text, char* params, char* url_tokens_arr[], char* request)
{
    if (post_flag == ON)
        if (p_text != NULL)
            free(p_text);

    if (r_flag == ON)
        if (params != NULL)
            free(params);

    if (url_tokens_arr != NULL)
        free(url_tokens_arr);

    if (request != NULL)
        free(request);
}
