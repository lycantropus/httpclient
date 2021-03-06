#include <stdio.h> // printf, sprintf 
#include <stdlib.h> // exit, atoi, malloc, free 
#include <unistd.h> // read, write, close 
#include <string.h> // memcpy, memset 
#include <sys/socket.h> // socket, connect 
#include <time.h>	// time functions
#include <math.h>	//sqrt, pow
#include <netinet/in.h> // struct sockaddr_in, struct sockaddr 
#include <netdb.h> // struct hostent, gethostbyname 


// Example calls
// ./client 45.62.226.182 80 GET / 2
// ./client 45.62.226.182 80 POST /cgi-bin/action.php 2 "item1=6" "Content-Type: application/x-www-form-urlencoded"

void error(const char *msg) { perror(msg); exit(0); }

int main(int argc,char *argv[])
{
    int i;

    // first where are we going to send it? 
    int portno = atoi(argv[2])>0?atoi(argv[2]):80;
    char *host = strlen(argv[1])>0?argv[1]:"localhost";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *message, response[4096];



    
    int nofrequests = atoi(argv[5])>0?atoi(argv[5]):1;

    

    
    if (argc < 6) { puts("Parameters: <host> <port> <method> <path> <number of requests> [<data> [<headers>]]"); exit(0); }

    // How big is the message? 
    message_size=0;
    if(!strcmp(argv[3],"GET"))
    {
        message_size+=strlen("%s %s%s%s HTTP/1.0\r\n");        // method         
        message_size+=strlen(argv[3]);                         // path           
        message_size+=strlen(argv[4]);                         // headers        
        if(argc>6)
            message_size+=strlen(argv[6]);                     // query string   
        for(i=7;i<argc;i++)                                    // headers        
            message_size+=strlen(argv[i])+strlen("\r\n");
        message_size+=strlen("\r\n");                          // blank line     
    }
    else//POST
    {
        message_size+=strlen("%s %s HTTP/1.0\r\n");
        message_size+=strlen(argv[3]);                         // method         
        message_size+=strlen(argv[4]);                         // path           
        for(i=7;i<argc;i++)                                    // headers        
            message_size+=strlen(argv[i])+strlen("\r\n");
        if(argc>6)
            message_size+=strlen("Content-Length: %zd\r\n")+10; // content length 
        message_size+=strlen("\r\n");                          // blank line     
        if(argc>6)
            message_size+=strlen(argv[6]);                     // body           
    }

    // allocate space for the message 
    message=malloc(message_size);

    // fill in the parameters 
    if(!strcmp(argv[3],"GET"))
    {
        if(argc>6)
            sprintf(message,"%s %s%s%s HTTP/1.0\r\n",
                strlen(argv[3])>0?argv[3]:"GET",               // method         
                strlen(argv[4])>0?argv[4]:"/",                 // path           
                strlen(argv[6])>0?"?":"",                      // ?              
                strlen(argv[6])>0?argv[6]:"");                 // query string   
        else
            sprintf(message,"%s %s HTTP/1.0\r\n",
                strlen(argv[3])>0?argv[3]:"GET",               // method         
                strlen(argv[4])>0?argv[4]:"/");                // path           
        for(i=7;i<argc;i++)                                    // headers        
            {strcat(message,argv[i]);strcat(message,"\r\n");}
        strcat(message,"\r\n");                                // blank line     
    }
    else//POST
    {
        sprintf(message,"%s %s HTTP/1.0\r\n",
            strlen(argv[3])>0?argv[3]:"POST",                  // method         
            strlen(argv[4])>0?argv[4]:"/");                    // path           
        for(i=7;i<argc;i++)                                    // headers        
            {strcat(message,argv[i]);strcat(message,"\r\n");}
        if(argc>6)
            sprintf(message+strlen(message),"Content-Length: %zd\r\n",strlen(argv[6]));
        strcat(message,"\r\n");                                // blank line     
        if(argc>6)
            strcat(message,argv[6]);                           // body           
    }

     

    struct timeval tBeginReply, tEndResponse;
    double elapsedTime;
    
    double totalTime;


    //getting timestamp for file
    time_t t;
    struct tm * timeinfo;
    time(&t);
   	timeinfo = localtime(&t);

   	char* formatedtime = asctime(timeinfo);
   	formatedtime[strlen(formatedtime)-1] = 0;

   	char *fn = malloc(strlen(formatedtime)+5);

   	sprintf(fn, "%s.txt", formatedtime);
   	printf("output: %s\n", fn );
   	
    FILE *f = fopen(fn, "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	// What are we going to send?
	printf("Request:\n%s\n",message);
    fprintf(f, "Request:\n%s\n",message);


    int round;
    int packetnumber;

    double deviationArray[nofrequests];
    memset(deviationArray, 0, nofrequests*sizeof(double));

    for(round=0; round<nofrequests; round++)
    {
    	//reset time counters
    	elapsedTime=0;
    	memset(&tBeginReply, 0, sizeof(tBeginReply));
    	memset(&tEndResponse, 0, sizeof(tEndResponse));
	//reset packetnumber
	packetnumber=0;

    // create the socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // lookup the ip address 
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    // fill in the structure 
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);



    // connect the socket 
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    
    
    total = strlen(message);
    sent = 0;

    // send the request 
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    // receive the response 
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
	if(packetnumber==0)
	{
		packetnumber=1;
		gettimeofday(&tBeginReply, NULL);
	}
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    //get time after response is complete
    gettimeofday(&tEndResponse, NULL);

    if (received == total)
        error("ERROR storing complete response from socket");

    // close the socket 
    close(sockfd);

    //process time metrics
    double startTime = tBeginReply.tv_sec *1000.0; //sec to ms
    startTime += tBeginReply.tv_usec / 1000.0;		//usec to ms

    double endTime = tEndResponse.tv_sec *1000.0;
    endTime += tEndResponse.tv_usec / 1000.0;

    printf("Begin of Reply: %f\n", startTime);
    fprintf(f, "Begin of Reply: %f\n", startTime);
    printf("End of Response: %f\n", endTime);
    fprintf(f,"End of Response: %f\n", endTime);

    elapsedTime = endTime - startTime;
    printf("Time: %f\n", elapsedTime);
    fprintf(f, "Time: %f\n", elapsedTime);
    deviationArray[round]= elapsedTime;   


    totalTime += elapsedTime;

    printf("Response:\n%s\n",response);
    fprintf(f, "Response:\n%s\n",response);
    }

    //statistics
    double mean = totalTime/nofrequests;
    double sd=0.0;

    for(i=0; i<nofrequests; ++i)
    {
        sd += pow(deviationArray[i] - mean, 2);
    }

    double standardDeviation= sqrt(sd/nofrequests);

    //at 95%
    double intervalLeft = mean - 1.96*standardDeviation/sqrt(nofrequests);
    double intervalRight = mean + 1.96*standardDeviation/sqrt(nofrequests);
    
    printf("Median time of %d requests: %f ms\n", nofrequests, mean );
    fprintf(f, "Median time of %d requests: %f ms\n", nofrequests, mean );

    printf("Standard deviation: %f\n",standardDeviation );
    fprintf(f, "Standard deviation: %f\n",standardDeviation );

    printf("Confidence Interval at 95%%: [%f,%f] \n",intervalLeft, intervalRight );
    fprintf(f, "Confidence Interval at 95%%: [%f,%f] \n",intervalLeft, intervalRight );

    free(message);

    return 0;
}
