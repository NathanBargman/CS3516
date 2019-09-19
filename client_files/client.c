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
#include <fcntl.h>

#define RCVBUFSIZE 2056
#define TIMEOUT 1 //TIMEOUT X2 is the time (sec) for the client to disconnect after recv nothing

void error(char *msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct addrinfo *res;;
    int status, port, sock, option_setting;
    char *input, *host, *path, *header, *port_char, *option_char;
    
    //parse command line
    if(argc < 3){
        error("Usage Error(less than 3 arguments): ./client [-option] hostname/path port");
    }
    else if(argc == 3){
        if(strlen(argv[1]) == 2){error("INVALID hostname/path");}
        if(atoi(argv[2]) == 0){error("Usage Error: 0 or a non-number was inputed for the port");}
        input = argv[1]; port_char = argv[2];
    }
    else if(argc == 4){
        option_char = argv[1]; input = argv[2]; port_char = argv[3];
        if(strchr(option_char, '-') != NULL)
        {
            if(strchr(option_char, 'p') != NULL){
                option_setting = 1;
            }
            else if(strchr(option_char, 'r') != NULL){
                option_setting = 2;
            }
            else{
                error("Usage Error: ./client [-option] hostname/path port\nCheck README for proper option commands");
            }
        }
    }
    else{
        error("Usage Error(more than 4 arguments): ./client [-option] hostname/path port");
    }
    
    if(strchr(input, '/') == NULL)
        error("Usage Error: hostname must end in '/'");
    
    //Separate the url into host and path
    path = strchr(input, '/');
    int cpylen = (strlen(input) - strlen(path));
    host = malloc(cpylen + 1);
    host[cpylen + 1] = '\0';
    strncpy(host, input, cpylen);
    
    //printf("Host: %s\nPath: %s\n", host,  path);
    
    //fill addrinfo struct
    port = atoi(port_char);
    if (getaddrinfo(host, port_char, NULL, &res) < 0)
        error("ERROR GETTING ADDRINFO");
    
    //create the socket
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sock < 0)
        error("ERROR MAKING SOCKET");
    
    //if getting rtt use these structs
    struct timeval rtt_s, rtt_f;
    double rtt_t;
    if(option_setting > 0) //if rtt option
        gettimeofday(&rtt_s, NULL);
    
    //connect to server socket
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        error("ERROR MAKING CONNECTION");
    
    if(option_setting > 0) //if rtt option
    {
        gettimeofday(&rtt_f, NULL);
        rtt_t = ((rtt_f.tv_sec - rtt_s.tv_sec)) + (1e-6 * (rtt_f.tv_usec - rtt_s.tv_usec));
    }
    
    //create header "GET part HTTP/1.1\r\nHOST: host\r\n\r\n"
    char *head1 = "GET ";
    char *head2 = " HTTP/1.1\r\nHOST: ";
    char *head3 = "\r\n\r\n";
    header = malloc(strlen(head1) + strlen(head2) + strlen(head3) + strlen(head3) + strlen(host) + strlen(path));
    strcpy(header, head1);
    strcat(header, path);
    strcat(header, head2);
    strcat(header, host);
    strcat(header, head3);
    
    //printf("Sending\n%s", header);
    
    //send header to server socket
    if (send(sock, header, strlen(header), 0) < 0)
        error("ERROR SENDING");
    
    if(option_setting > 0) //if rtt option
        printf("---\nRTT: %.2f milliseconds\n---\n", rtt_t * 1000);
    
    //cleanup
    free(header);
    free(host);
    freeaddrinfo(res);
    if(option_setting == 2) //if we just want rtt no need to continue
    {
        printf("\n---\nEND CLIENT\n---\n");
        exit(1);
    }
    
    //prepare to recv
    int size_recv, total_size = 0;
    char chunk[512]; //buffer for recv data in groups
    struct timeval start, current;
    double timepass, first_recv;
    int data_recv = 0;
    fcntl(sock, F_SETFL, O_NONBLOCK);
    gettimeofday(&start, NULL); //start timeout clock
    
    //enter loop
    while(1)
    {
        //get current time
        gettimeofday(&current, NULL);
        timepass = ((current.tv_sec - start.tv_sec)) + (1e-6 * (current.tv_usec - start.tv_usec));
        
        //check time and if we have data or not
        if(timepass > TIMEOUT  && size_recv > 0)
            break;
        else if(timepass > 2 * TIMEOUT)
            break;
        
        //clear chuck
        memset(chunk, 0, 512);
        //recv data into chunck
        if ((size_recv = recv(sock, chunk, 512, 0)) <  0)
        {
            //usleep(1000); //in case the recv is too intensive
        }
        else //if we got data
        {
            if(!data_recv){first_recv = timepass; data_recv = 1;} //record first recv
            total_size += size_recv; //collect total size
            printf("%s", chunk); //print chunk
        }
    }
    
    //final print{
    printf("\nTotal Size: %d bytes\nTime Till First Data: %.2f milliseconds\nTimed Out @ %.2f milliseconds\n", total_size, (first_recv * (double)  1000), (timepass * (double) 1000));
    printf("\n---\nEND CLIENT\n---\n");
}	
