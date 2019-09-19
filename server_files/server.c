#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define HOST_LEN 64

void error(char *msg)
{
    printf("%s", msg);
    exit(1);
}

//error via errno handler
void sigchld_handler(int s)
{
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

//get address of client
void *get_in_addr (struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sock, new_sock;
    struct addrinfo *servinfo;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    struct sigaction sa;
    
    //get hostname of this server
    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) < 0)
        error("NO HOSTNAME\n");
    printf("Starting...\nHostname: %s\n", hostname);
    
    //fill addrinfo struct
    if(getaddrinfo(hostname, argv[1], NULL, &servinfo) < 0)
        error("FAILED ADDRESS\n");
    printf("Found Self...\n");
    
    //create the socket
    if((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0)
        error("SOCKET FAILED\n");
    printf("Made Socket...\n");
    
    //bind socket to server
    if(bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        error("FAILED BIND: TRY NEW PORT\n");
    printf("Bound Socket...\n");
    
    //listen for clients
    if(listen(sock, 10) < 0)
        error("LISTEN FAILED\n");
    
    //sigaction
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction (SIGCHLD, &sa, NULL) < 0)
        error("SIGACTION ERROR");
    
    //cleanup
    freeaddrinfo(servinfo);
    
    printf("Waiting for clients...\n");
    
    //enter loop
    while(1){
        
        //create sockets for new clients
        sin_size = sizeof(client_addr);
        new_sock = accept(sock, (struct sockaddr *) &client_addr, &sin_size);
        if(new_sock < 0)
            continue;
        
        //display connectiom
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr), s, sizeof(s));
        printf("Got Connection from %s\n", s);
        
        //create a new process for each client
        if(!fork())
        {
            close(sock); //clean old socket
            char buffer[1024]; //buffer for client headder
            char *input, *cmd, *path, *temp, *temp2;
            int size, index;
            
            //recv header from client
            if(recv(new_sock, buffer, sizeof(buffer), 0) != -1)
            {
                //get first word from header
                input = buffer;
                temp = strchr(input, ' ');
                index = (int)(temp - input);
                cmd = malloc(index + 1);
                cmd[index] = '\0';
                strncpy(cmd, input, index);
                
                if(strstr(cmd, "GET") == NULL)
                {
                    printf("BAD HEADER\n");
                    continue;
                }
                
                //send OK
                char *rtn;
                rtn = malloc(64);
                rtn = "HTTP/1.1 200 OK\n";
                
                //get path from header
                temp = temp + 1;
                temp2 = strchr(temp, ' ');
                index = (int)(temp2 - temp - 1);
                path = malloc(index + 1);
                path[index] = '\0';
                strncpy(path, temp + 1, index);
                
                //find file at path
                FILE *fptr = fopen(path, "r");
                size_t nread;
                char buf[512];
                
                if(!strcmp(path, ""))
                {
                    rtn = "HTTP/1.1 200 OK\nNo File Requested\n";
                    if(send(new_sock, rtn, 64, 0) < 0)
                        error("SEND ERROR");
                    continue;
                }
                else if(fptr == NULL) //check of the file exists
                {
                    printf("404 Not Found\n");
                    rtn = "HTTP/1.1 404 Not Found\n";
                    if(send(new_sock, rtn, 64, 0) < 0)
                        error("SEND ERROR");
                    continue;
                }
                
                //read and send file
                if(send(new_sock, rtn, 64, 0) < 0)
                    error("SEND ERROR");
                while ((nread = fread(buf, 1, sizeof(buf), fptr)) > 0)
                    send(new_sock, buf, nread, 0);
                
                //cleanup
                fclose(fptr);
                free(cmd);
                free(path);
            }
            else
            {
                printf("NOTHING RECIVED\n");
            }
            close(new_sock);
            exit(0);
        }
    }
    
    return 0;
}	
