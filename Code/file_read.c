/**
* @file  : 	file_read.c
* @brief : 	This file contains source code for file read of multiple threads.
*		There are many threads which are execute concurrently.
*		server is provide below service Read content for multiply client. 
		1. Read content
*		
* @author:	Hiten Padiya( hiten.padiya@vvdntech.in )
*
*
*/

/**
* @function : read_content
* @param1   : client socket fd
* @return	: void
* @brief	: This function will perform read operation of client services concurrently  
*
*/

#include "socket.h"


void read_content(int client_socketfd)
{
	/* -------------- local variables ---------------- */
	
	char buffer[1024];
	FILE *fd;
	char buffer2[1024];		/* To store the content of the string */
	char *p_buffer=buffer;					
	int count_s=0, flag=0;
	char buff[4];			/* to detect '=>' */
	char c;				/* To take the character from the file */
	int enter;				/* To detect the enter */
	int content_length=0;	
	
	/* -------------- end of local variables ---------------- */
	
	/* file Open operation */

	fd=fopen("commands.txt","r+");
	if(fd == NULL)
	{
        	printf("\n fopen() Error \n");
        	return;
    	}
	printf("\n File opened successfully through fopen()\n");

	strcpy(buffer, "Kindly provide word to read the content.");
	write(client_socketfd,buffer,strlen(buffer)+2);
	read(client_socketfd, &buffer, bufsize);
	
	buffer[strlen(buffer)-1]='\0';		      /* To escape the '\n' */
	printf("Client : ");
	
	printf("%s %ld",buffer, strlen(buffer));		/* To check the received string */
	memset(buffer2,0,sizeof(buffer2));
	memset(buff,0,sizeof(buff));
	while(1)
	{
		fread(&c, 1, 1, fd);				/* To read the first character */
		while(1)
		{
			if(feof(fd))				/* if end of file reached the it will break it. */
			break;
			
			if((*p_buffer >= 64) && (*p_buffer <= 91) )		/* convert received string to lower alphabet */
			{	
				*p_buffer=tolower(*p_buffer);
			}
			
			if((c >= 64) && (c <= 91) )			/* convert file's character to lower alphabet */
			{	
				c=tolower(c);
			}  
			
			if(*p_buffer==c)					/* check received string in the commands.txt file */
			{
				p_buffer++;
				count_s++;
				
				if(count_s==strlen(buffer))
				{
					flag=1;
					break;
				}
					
			}
			else
			{
				count_s=0;
				p_buffer=buffer;
			}
			
			fread(&c, 1, 1, fd);
		}
		
		printf("Flag=%d\n", flag);				/* flag=1 => Received string exist in the file & flag=0 => Not exist */		
		
		if(flag==1)
		{
			puts("\nstring found\n");			/* Store the content in buffer2 */
			fread(&buff, 2, 1, fd);
			printf("%s", buff);
			count_s=0;
			if(!(strcmp(buff,"=>")))
			{
				printf(" in buff\n");
				fseek(fd, -strlen(buffer)-2, SEEK_CUR);
				
				fread(&c, 1, 1, fd);
				while(enter!=5)
				{
					buffer2[content_length]=c;
					fread(&c, 1, 1, fd);
					if(c=='\n')
					enter++;
					content_length++;
				}
				buffer2[content_length]='\0';
			}	
			break;	
			
		}
		else
		{	
			strcat(buffer2, buffer);
			strcat(buffer2, " string not found.  Do you want to try another.? type 'yes' ");
			write(client_socketfd,buffer2,strlen(buffer2)+2);
			
		}
				
		read(client_socketfd, &buffer, bufsize);				/* again check the string */
		if(!(strcmp("yes\n" , buffer)))
		{
			write(client_socketfd,"Kindly provide another:",28);
			memset(buffer,0,sizeof(buffer));
			read(client_socketfd, &buffer, bufsize);
			rewind(fd);								/* to set file position at the begining */
			count_s=0;
			p_buffer=buffer;									
			buffer[strlen(buffer)-1]='\0';
		}
		else			
		{
			fclose(fd);
			return;
		}
		
	}
	
	write(client_socketfd,buffer2,strlen(buffer2)+2);			/* Send content to client */
	
	fclose(fd);
	return;
}
