/**
* @file  : 	server.c
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
/* -------------- Global Variable ---------------- */


/* -------------- End of Global Variable -----------*/

/* -------------- function prototype ---------------- */

void *thread_funct(void *arg);
void check_again();
void create_chat();

/* for checking github commit */

/* -------------- End of function prototype -----------*/


int main(int argc, char *argv[])
{
	/* -------------- local variables ---------------- */	
	
	int server_len,client_len,i;
	int server_socketfd,client_socketfd;
	pthread_t thread;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
		
	/* -------------- End of local variables ----------- */
	if(argc!=2)
	{
		fprintf(stderr, "Usage: %s <filename> <port number>\n", *argv);
		exit(EXIT_FAILURE);
	}
	
	/* -------------- Create socket -------------*/	
	
	server_socketfd = socket(AF_INET,SOCK_STREAM,0);
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_address.sin_port=atoi(argv[1]);
	server_len=sizeof(server_address);
	bind(server_socketfd,(struct sockaddr *)&server_address,server_len);

	/*  Create connection queue and wait for client  */
	
	listen(server_socketfd,5);		
	printf("Server waiting...\n");	
	client_len = sizeof(client_address);	
	
	
	for(i=1;i<=10;i++)
	{
		client_socketfd=accept(server_socketfd,(struct sockaddr *)&client_address,&client_len);


		pthread_create(&thread,NULL,thread_funct,(void*)&client_socketfd);
	
		printf("Server connected with client %d with IP address is: %s and port %d\n",i, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		
	}
	
	return 0;
}

/**
* @function : Thread function
* @param1   : pointer of p_client_socketfd
* @return	: void
* @brief	: This function will preform thread operation of client services concurrently  
*
*/
	
void *thread_funct(void *client_desc)
{
	/* -------------- local variables ---------------- */
	
	int input;
	int client_socketfd= *(int*)client_desc;
	int read_byte;
	/* -------------- end of local variables ----------*/
	
	char buffer[1024]="Hi, How can i help you.? 1. Create c file 2. Chat 3. Read Content";
	write(client_socketfd,buffer,strlen(buffer)+1);
	

	read_byte=read(client_socketfd, &buffer, bufsize);

	if(read_byte==0)
	{
		printf(" Client disconnect\n");
		close(client_socketfd);
	}

     sscanf(buffer, "%d", &input);
	
	while((input!=1) && (input!=2) && (input!=3))
	{
		write(client_socketfd,"Provide valid input",23);
    		memset(buffer,0, sizeof(buffer));
    		read(client_socketfd, &buffer, bufsize);
    		sscanf(buffer, "%d", &input);
    		fflush(stdout);
    	}


   	switch(input)                     
    	{	
	  case 1 : create_c(client_socketfd);
        break; 
        case 2 : create_chat(client_socketfd);
        break; 
        case 3 : read_content(client_socketfd);
        break;
	  default : close(client_socketfd);
	  break;    
		
     	}
	
	
	
	close(client_socketfd);
      pthread_exit(0);

}


/**
* @function : create_chat
* @param1   : client socket fd
* @return	: void
* @brief	: Chat function program
*
*/


void create_chat(int client_socketfd)				/* Chat function */
{
	/* -------------- local variables ---------------- */
	
	int exit=0;
	char buffer[1024];
	int str_cmp;

	/* -------------- end of local variables ------------ */
	
	write(client_socketfd,"---- Create chat----\n",22);
	while(1)
	{
		printf("\n Client:");
            read(client_socketfd, &buffer, bufsize);
            printf("%s", buffer);
            
            str_cmp=strcmp("bye\n" , buffer);
		
		if(str_cmp==0)
		{
			break;
		}
            	
            printf("\n Server:");
            fgets(buffer,1024,stdin);
		write(client_socketfd, buffer, bufsize);
        }
        return;
} 	
