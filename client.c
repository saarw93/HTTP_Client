//
// Created by saar on 12/17/18.
//

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <arpa/inet.h>


//#define DEBUG
#define ERR (-1)
#define ON 1
#define BUFFER_SIZE 5000
#define USAGE "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\r\n"

int check_if_post(char* str);
int check_for_r(char* str);
char* save_text_for_post(char* text);
char* append_params(char* argv[],int r_index, int num_of_params);
int find_url(char* str);
char** split_url_to_tokens(char* url);
int make_socket_connect(char* url_tokens_arr[]);
char* get_request(char* url_tokens_arr[],int r_flag, char* params);
char* post_request(char* url_tokens_arr[], char* text, int r_flag, char* params);
int count_digits_p_text(char* p_text);
void free_program(int post_flag, int r_flag, char* p_text, char* params, char* url_tokens_arr[], char* request);


int main(int argc, char* argv[])
{
    int post_flag = 0, r_flag = 0, sockfd = 0;
    int i, p_index = 0, r_index = 0, num_of_params = -1;
    char* p_text = NULL, *params = NULL;
    char* url = NULL;
    char** url_tokens_arr = NULL;
    char* request = NULL;

    for (i = 1; i < argc; i++)
    {
        if (check_if_post(argv[i]) == 1)
        {
            p_index = i;  //the index where p is in the cmd line
            i++;
            p_text = save_text_for_post(argv[i]);
            post_flag = ON;
            continue;
        }

        if (check_for_r(argv[i]) == 1)
        {
            r_index = i;
            num_of_params = atoi(argv[i+1]);

            if (num_of_params == 0) {
                printf(USAGE);
                exit(1);
            }
            params = append_params(argv, r_index, num_of_params);
            r_flag = ON;

            #if defined(DEBUG)
                printf("The parameters append is: %s\n", params);
            #endif
        }

        if (find_url(argv[i]) != -1 )
        {
            if ( r_flag == ON)
            {
                if (i > r_index + num_of_params + 1 || i < r_index)
                {
                    url = argv[i];
                    #if defined(DEBUG)
                    printf("The url is: %s\n", url);
                    #endif

                    url_tokens_arr = split_url_to_tokens(argv[i]);

                    #if defined(DEBUG)
                    for (int j = 0; j < 3; ++j)
                        printf("%s\n", url_tokens_arr[j]);
                    #endif
                }
            }
            else
            {
                url = argv[i];
                #if defined(DEBUG)
                printf("The url is: %s\n", url);
                #endif

                url_tokens_arr = split_url_to_tokens(argv[i]);

                #if defined(DEBUG)
                for (int j = 0; j < 3; ++j)
                    printf("%s\n", url_tokens_arr[j]);
                #endif
            }
        }
    }

    if (post_flag == ON)
        request = post_request(url_tokens_arr, p_text, r_flag, params);
    else
    request = get_request(url_tokens_arr, r_flag, params);

    #if defined(DEBUG)
    printf("Request:\r\n%s\r\n", request);
    #endif

    printf("HTTP request =\n%s\nLEN = %d\n", request, (int)strlen(request));

    sockfd = make_socket_connect(url_tokens_arr);

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    printf("sockfd is: %d\r\n", sockfd);

    int sum = 0, nbytes = 0;
    int wr = (int)write(sockfd, request, strlen(request)+1);
    printf("wr = %d\r\n", wr);


    while(1)
        {
            nbytes = (int)read(sockfd, buffer+sum, BUFFER_SIZE-sum);
            sum+= nbytes;

            if (nbytes == 0)
                break;
            if (nbytes < 0)
            {
                perror("ERROR: cannot read from socket");
                exit(1);
            }
        }

    printf("Response from server:\n%s\n", buffer);
    printf("\n Total received response bytes: %d\n", sum);

    close(sockfd);


    #if defined(DEBUG)
        printf("we have -p at place %d , we have %s text after -p, and we have -r at place %d in the cmd input\r\n", p_index,p_text, r_index);
    #endif
    free_program(post_flag, r_flag, p_text, params, url_tokens_arr, request);
    return 0;
}


int check_if_post(char* str)
{
    if (strcmp(str, "-p") == 0)
        return 1;
    return -1;

}


int check_for_r(char* str)
{
    if (strcmp(str, "-r") == 0)
        return 1;
    return -1;
}

char* save_text_for_post(char* text)
{
    char* text_ptr = (char*)malloc(strlen(text)+1);
    if (text_ptr == NULL)
        return NULL;

    strcpy(text_ptr, text);

    return text_ptr;
}

char* append_params(char* argv[],int r_index, int num_of_params)
{
    int params_start = r_index + 2;   //the index where the first parameter is in agrv[]
    int i, size = 0;
    for (i = params_start; i < params_start+num_of_params ; i++)
    {
        size += strlen(argv[i]);
    }

    size += num_of_params;
    char* params = (char*)malloc(size * sizeof(char));  //num_of_params-1 is for the ampersands between the parameters
    if (params == NULL)
        return NULL;
    bzero(params, size* sizeof(char));  // initial with '/0' the params malloc


    for (i = params_start; i < params_start+num_of_params ; i++)
    {
        strcat(params, argv[i]);
        if(i != params_start+num_of_params-1)  //in case we are in the last parameter to be appended
            strcat(params, "&");
    }
    return params;
}

int find_url(char* str)
{
    char* sub_str = strstr(str, "http");

    if (sub_str)
        return 1;
    return -1;
}

char** split_url_to_tokens(char* url)
{
    if (url)
    {
        char* host = NULL;  //cuts off the "http://" from url
        char** url_tokens_arr = (char**)malloc(sizeof(char*)*3); //array of size 3 which contains: |host|port|path|
        if (url_tokens_arr == NULL)
            return NULL;
        bzero(url_tokens_arr, 3* sizeof(char*));  // initial with '/0' the url_tokens_arr malloc

        host = url+7*sizeof(char);   // 7 is the length of string "http://"

        char* check_port = strchr(host, ':'); //check if there is a port in the url or not

        host = strtok(host, ":/");  // get the first token (i.e, the host)


        int i = 0;
        while (host != NULL)   // walk through other tokens (i.e, the port and path)
        {
            if (i == 0)
            {
                url_tokens_arr[i] = host;
                i = i+1;
                if (check_port != NULL)
                    host = strtok(NULL, ":/");
            }

            else if (i == 1)
            {
                if (check_port == NULL)
                    url_tokens_arr[i] = "80"; //there is no port in the url input, so need to put the well known port number of http: 80
                else
                    url_tokens_arr[i] = host; //there is port in the url input

                i++;
                host = strtok(NULL, "");  //to get the rest of the file, which is the path
            }

            else if (i == 2)
            {
                //host = strtok(NULL, "");  //to get the rest of the file, which is the path
                //url_tokens_arr[i] = strcat("/", host); //TODO check if need the "/" at the start of the path. strcat her dont work

                    url_tokens_arr[i] = host;
                    i++;
            }
            else
            {
                host = strtok(NULL, "/:");
            }
        }
        return url_tokens_arr;
    }
    return NULL;
}


int make_socket_connect(char* url_tokens_arr[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *hp;


    if (url_tokens_arr == NULL) {
        perror("ERROR: url is NULL\n");
        exit(1);
    }


    sockfd = socket(PF_INET, SOCK_STREAM, 0);    //create an endpoint for communication
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;


    hp = gethostbyname(url_tokens_arr[0]);
    if (hp == NULL) {
        herror("ERROR, no such host\r\n");
        exit(1);
    }

    serv_addr.sin_port = htons(atoi(url_tokens_arr[1]));   //htons function converts the unsigned short integer (the port) from host byte order to network byte order

    serv_addr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr))->s_addr;

    printf("host:%s\npath:%s\nport:%s\r\n\r\n",url_tokens_arr[0],url_tokens_arr[2],url_tokens_arr[1]);

    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)      //initiate a connection on a socket
    {
        perror("ERROR connecting\r\n");
        exit(1);
    }
    return sockfd;
}


char* get_request(char* url_tokens_arr[],int r_flag, char* params)
{
    char* hostName = url_tokens_arr[0];
    char* filePath = url_tokens_arr[2];
    char* request_str = NULL;
    int size = 0;

    size+= strlen(hostName) + strlen(filePath); //adds the size of the hostName and the filePath

    size+= 29;     //adds the size of GET /, HTTP/1.0, HOST:,\r\n\r\n, spaces, \r\n\r\n at the end and '/0' to the end of string


    if (r_flag == ON)  //we have parameters in the request
        size += (strlen(params)+1);  //+1 is for the question mark

    request_str = (char*)malloc(size*sizeof(char));
    if (request_str == NULL)
    {
        perror("malloc of request_str failed\r\n");
        return NULL;
    }
    bzero(request_str, size*sizeof(char));  // initial with '/0' the request_str malloc

    #if defined (DEBUG)
    int strLength = (int)sizeof(request_str);
    printf("the length of request_str = %d\r\n", strLength);
    #endif

    if (r_flag == ON)
        sprintf(request_str, "GET /%s?%s HTTP/1.0\r\nHost: %s\r\n\r\n", filePath, params, hostName);
    else
        sprintf(request_str, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", filePath, hostName);

    return request_str;
}


char* post_request(char* url_tokens_arr[], char* text, int r_flag, char* params)
{
    char* hostName = url_tokens_arr[0];
    char* filePath = url_tokens_arr[2];
    char* request_str = NULL;
    int size = 0;

    size+= strlen(hostName) + strlen(filePath); //adds the size of the hostName and the filePath
    size+= strlen(text);  //adds the size of the text of the post request

    int p_text_digits = count_digits_p_text(text);
    size+= p_text_digits; //adds place to the num of digits that the Content-Length needs

    size+= 47;    //adds the size of POST /, HTTP/1.0, HOST:,\r\n\r\n, Content-Length: spaces, \r\n\r\n at the end and '/0' to the end of string


    if (r_flag == ON)  //we have parameters in the request
        size += (strlen(params)+1);  //+1 is for the question mark

    request_str = (char*)malloc(size*sizeof(char));
    if (request_str == NULL)
    {
        perror("malloc of request_str failed\r\n");
        return NULL;
    }
    bzero(request_str, size*sizeof(char));  // initial with '/0' the request_str malloc

    #if defined (DEBUG)
    int strLength = (int)sizeof(request_str);
    printf("the length of request_str = %d\r\n", strLength);
    #endif

    if (r_flag == ON)
        sprintf(request_str, "POST /%s?%s HTTP/1.0\r\nHost: %s\r\nContent-length:%d\r\n\r\n%s\r\n", filePath, params, hostName, (int)strlen(text), text);
    else
        sprintf(request_str, "POST /%s HTTP/1.0\r\nHost: %s\r\nContent-length: %d\r\n\r\n%s\r\n", filePath, hostName, (int)strlen(text), text);

    return request_str;
}

/*method to count from how many numbers does the text's size of p is made of*/
int count_digits_p_text(char* p_text)
{
    int counter = 0;
    int num = strlen(p_text);

    while(num != 0)
    {
        num/= 10;
        counter++;
    }
    return counter;
}

void free_program(int post_flag, int r_flag, char* p_text, char* params, char* url_tokens_arr[], char* request)
{
    if (post_flag == 1)
        free(p_text);

    if (r_flag == 1)
        free(params);

    free(url_tokens_arr);
    free(request);
}