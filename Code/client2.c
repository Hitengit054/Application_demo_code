/**
* @file  : 	client.c
* @brief : 	This file contains source code for server using multiple threads.
*		There are 3 threads which are execute concurrently.
*		server is provide below three services for client.
*		1. Create C file 
*		2. Chatting
*		3. Read content.
*		
* @author:	Hiten Padiya( hiten.padiya@vvdntech.in )
*
*
*/

/* -------------- Header file ---------------- */

#include "socket.h"

/* -------------- End of Header file ---------------- */
int main(int argc, char *argv[])
{
	/* -------------- local variables ---------------- */	
	
	int socketfd, len, connectid;
	char buffer[1024];
	int str_cmp;
	struct sockaddr_in address;
	
	/* -------------- End of local variables ----------- */
	
	if(argc!=2)
	{
		fprintf(stderr, "Usage: %s <filename> <port number>\n", *argv);
		exit(EXIT_FAILURE);
	}
	
	/* -------------- Create socket -------------*/	
						
	socketfd= socket(AF_INET,SOCK_STREAM,0);  		/* create a socket for client*/			
	address.sin_family=AF_INET;			    		/* name the socket as the agree with server*/
	address.sin_addr.s_addr=inet_addr("127.0.0.1");
	address.sin_port=atoi(argv[1]);
	len=sizeof(address);
	
	connectid = connect(socketfd,(struct sockaddr *)&address,len);	/* connect your socket to server socket*/
	
	if(connectid==-1)
	{
		printf("ERROR in connection of socket\n");
		return 1;
	}
	
	printf("-----client connected with server----\n");
	printf("-----Here you can add library, macro and variable ----\n");
	
	read(socketfd,&buffer,1024);
	printf("%s", buffer);
	
	while(1)
	{
		
		fflush(stdout);
		printf("\n Client:");
		fflush(stdin);
		fgets(buffer,1024,stdin);
		write(socketfd, buffer, bufsize);
		
		str_cmp=strcmp("bye\n" , buffer);
		
		if(str_cmp==0)
		{
			break;
		}	
 	 	fflush(stdout);
 	 	printf("\n Server:");
        	memset(buffer,0,sizeof(buffer)); 
        	read(socketfd, &buffer,bufsize);
  	     	fflush(stdout);
        	printf("%s", buffer);
            	
            		
        } 
        printf("\n");

	close(socketfd);
	return 0;
}

