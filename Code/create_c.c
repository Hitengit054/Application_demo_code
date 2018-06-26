#include "socket.h"

/**
* @function : To chack weather library exist in the Library.txt
* @param1   : User Input Library
* @return	: Interger
* @brief	: This function will preform library check.
		  flag=1=> Library available 
		  flag=0=> Not Available 
*
*/

int check_string(char buff[])  
{
	/* -------------- local variables ---------------- */
	
	char *p_buff=buff,ch_lib;
	FILE *fs= NULL;
	int check_length=0,flag=0;
	
	/* -------------- end of local variables ------------*/
	
	/* file Open operation */
	printf(" --- in check library --- ");
	printf("%s\n", buff);
	fs = fopen("Library.txt","r+");
	
	if(fs==NULL)
	{
        	printf("\n fopen() Error \n");
        	return 1;
    	}
	
	fread(&ch_lib, 1, 1, fs);
	
	while(1)
	{
		if(*p_buff==ch_lib)
		{
			p_buff++;
			check_length++;
		}
		if(check_length==strlen(buff))
		{ 
			flag=1;
			break;
		}
			
		fread(&ch_lib, 1, 1, fs);
		if(feof(fs))		/* Will check end of file & when end of file reach then it equal to 1 */
		break;
	}
	return flag;
}	

/**
* @function : To generate C file 
* @param1   : client_socket file descriptor
* @return	: void 
* @brief	: It will take from the user like input library, macro and variable & generate the
		  C file 
*
*/

void create_c(int client_socketfd)
{
	/* -------------- local variables ---------------- */
	char buffer[1024];
	char buffer2[1024];
	char buff[1024];
	char delim[]=" ";		/* Take delimiter as a space to seperate the string token */ 
	char *p_strtok;
	int flag;			/* for checking string in Library.txt */
	int count=0;		/* To give the total byte to file write of library */
	int check_again=1;	/* Again check the string in Library.txt */
	int macro_i=0,macro_set=0;	/* To write the Macro in file */ 
	char filename[20];
	
	int str_cmp;
	FILE* fd = NULL;

	/* -------------- end of local variables ---------------- */
	
	memset(buffer2,0,sizeof(buffer2));
	memset(buff,0,sizeof(buff));
	memset(buffer,0,sizeof(buffer));
	
	/* file Open operation */
	sprintf(filename,"%d.c",rand()%100);		/* to generate random filename */

	fd = fopen(filename,"w+");
	if(fd==NULL)
	{
        	printf("\n fopen() Error in test.c \n");
        	return;
      }
	printf("\n %s file opened successfully through fopen()\n",filename);
	printf("\n\n");
	strcpy(buffer, "--- Create c file---- you can add library, macro, variable by typing keyword\n"); 
	write(client_socketfd,buffer,strlen(buffer)+2);
	
	while(1)							 /* Library part */
	{
		memset(buffer,0,sizeof(buffer));
            read(client_socketfd, &buffer, bufsize);
           	printf("%s", buffer);
           	
            while((strcmp("library\n" , buffer)))	 /* if wrong input then prompt the user */
            {
            	strcpy(buffer,"Provide valid input ex.'library' :");
           		write(client_socketfd,buffer,strlen(buffer)+2);
           		read(client_socketfd, &buffer, bufsize);
            }
            		
            write(client_socketfd, "which library you want to add. ?" , 35);
            printf("\n Client:");
            memset(buffer,0,sizeof(buffer));
 		read(client_socketfd, &buffer, bufsize);
 		
		p_strtok=strtok(buffer, delim);		/* strtok initialization */				
 				
 		while( (p_strtok!=NULL) && (check_again==1) )         /* Libraries sperate by ' ' */
 		{
 			printf(" ---- In library----- \n");
 			strcpy(buff, p_strtok);			/* Take token string to buff for check */
			printf("%s ", buff);
			p_strtok=strtok(NULL, delim);			/* To take next token string */
					
 			flag=check_string(buff);		/* Check_string function call */
 			if(flag==1)					/* Flag=1 => String found */
 			{
 				printf("--- String found in file--\n");
 				fwrite("#include<",9, 1, fd);
 				if(buff[strlen(buff)-1]=='\n')	/* To eliminate '\n' in Last token */ 
 				buff[strlen(buff)-1]='\0';		/* Last token string should be terminated with NULL */
 				count=strlen(buff);
 				printf("Size of %s is %d",buff, count);
 				fwrite(buff,count,1, fd);		/* library write operation */
 				fwrite(">\n",2, 1, fd);
 				strcat(buffer2, buff);			/* Give multiply libarary status to client */
 				strcat(buffer2, " Added\n");
 				memset(buff,0,sizeof(buff));
 		
 			}
 			else if(flag==0)					 /* In case invalid library and user want to change it. */
 			{
 				strcat(buff, " invalid library.  Do you want to change it.? type 'y'");
 				write(client_socketfd, buff, strlen(buff)+1);
 				memset(buff,0,sizeof(buff));
 					
 				read(client_socketfd, &buffer, sizeof(buffer));
 					
 				if(!(strcmp("y\n" , buffer)))
 				{
 					check_again=1;
 					write(client_socketfd,"which library you want to add. ?", 35);
 					memset(buffer,0,sizeof(buffer));
            			printf("\n Client:");
 					read(client_socketfd, &buffer, bufsize);	
 					p_strtok=strtok(buffer, delim);	/* To re initialize the buffer */
 					count=0;
 				}
 				else
 				check_again=0;
 					
 			}
 			count=0;
 		}
 			
 		write(client_socketfd, buffer2, strlen(buffer2)+2);	/* Send the status of library to client */
 		memset(buffer2,0,sizeof(buffer2));
 		
 		memset(buffer,0,sizeof(buffer));
 		read(client_socketfd, &buffer, bufsize);
 	
 		 /* if wrong input then prompt the user */
 		while( (strcmp("macro\n" , buffer)) && (strcmp("variable\n" , buffer)))
        	{
            	strcpy(buffer,"Provide valid input ex.'macro' or 'variable':");
           		write(client_socketfd,buffer,strlen(buffer)+2);
           		read(client_socketfd, &buffer, bufsize);
         	}
 	
 		if(!(strcmp("macro\n" , buffer)))			/* macro part */
 		{
 			macro_set=1;
 			fwrite("\n#define ",9,1, fd);
 			strcpy(buffer, "Define macro name & value :");
 			write(client_socketfd,buffer, strlen(buffer)+2);
 			memset(buffer,0,sizeof(buffer));
 			
 			read(client_socketfd,&buffer, bufsize);
 			printf("%s", buffer);
 			
 
			while(buffer[macro_i]!='\0')
			{
				buffer2[macro_i]=buffer[macro_i];
				macro_i++;
			}
			
			printf("%s", buffer2);
			fwrite(buffer2,strlen(buffer2),1, fd);	/* write macro into file */
			memset(buffer2,0,sizeof(buffer2));
			write(client_socketfd, "Macro Added", 11); /* Send the status of Macro to client */
			memset(buffer,0,sizeof(buffer));
		}	
 		
 		
 		fwrite("\nint main() \n{",14,1, fd);

 		if((strcmp("variable\n" , buffer) || (macro_set==1)))
 		read(client_socketfd, &buffer, bufsize);

 		while((strcmp("variable\n" , buffer)))		/* if wrong input then prompt the user */
         	{
            	strcpy(buffer,"Provide valid input ex.'variable':");
           		write(client_socketfd,buffer,strlen(buffer)+2);
           		read(client_socketfd, &buffer, bufsize);
        	}
 		
 		if(!(strcmp("variable\n" , buffer)))			 /* variable part */
            {	
            	int input=0,total_v;
            	int vari_l=1, invaild_ip=0;
            	printf(" ---- In Variable ----- \n");
            	while(1)
			{
				if(invaild_ip==0)
				{
					strcpy(buffer,"which variable you want to add.?  1. Interger  2. Character  3. float 4. Finished");
            			write(client_socketfd,buffer,strlen(buffer)+1);
            			printf("\n Client2:");
            			
            			read(client_socketfd, &buffer, bufsize);
            			sscanf(buffer, "%d", &input);
            			
            		}
            			
            		if(input==4) break;
            			
            		if( (input==1 ) || (input==2) || (input==3) )
            		{
            			write(client_socketfd,"How many variable.?",20);
            			memset(buffer,0,sizeof(buffer));	
            			read(client_socketfd, &buffer, bufsize);
            			sscanf(buffer, "%d", &total_v);
            			
            			while(total_v==0)			/* if wrong input then prompt the user */
            			{	
            				memset(buffer,0,sizeof(buffer));
            				strcpy(buffer,"Provide valid input interms of digits");
           					write(client_socketfd,buffer,strlen(buffer)+2);
           					read(client_socketfd, &buffer, bufsize);
            				sscanf(buffer, "%d", &total_v);
            		
            			}
            				
            			if(input==1)	
            			fwrite("\n int ",6, 1, fd);
            			else if(input==2)
            			fwrite(" char ",6, 1, fd);
            			else if(input==3)
            			fwrite(" float ",7, 1, fd);
            
            				
            				
            			while(vari_l<=total_v)
            			{
            				static char c='a';
            				fwrite(&c,1, 1, fd);
            				if(vari_l!=total_v)
            				fwrite(",", 1, 1, fd);
            				vari_l++;
            				c++;
            			}
            			fwrite(";\n",2, 1, fd);
            			vari_l=1;
            			total_v=0;
            			input=0;
            			printf("\n Added");
            			invaild_ip=0;
            				
        			}
        			else					  /* if wrong input then prompt the user */
        			{	memset(buffer,0,sizeof(buffer));
        				strcpy(buffer,"Provide valid input ex.'1', '2', '3' or '4' :");
           				write(client_socketfd,buffer,strlen(buffer)+2);
           				read(client_socketfd, &buffer, bufsize);
            			sscanf(buffer, "%d", &input);
            			invaild_ip=1;
        				
        			}
        		}   
        		
            }
            	
   		fwrite("\n\n return 0;\n",13, 1, fd);
		fwrite("}",1, 1, fd);
		            	
            fclose(fd);
            	
            write(client_socketfd,"Anything else",14);
            memset(buffer,0,sizeof(buffer));
            read(client_socketfd, &buffer, bufsize);
            str_cmp=strcmp("bye\n" , buffer);
		
		if(str_cmp==0)						 /* exit by client say "bye" */
		{
			break;
			
		}
      	break;
			      	
     }
	return;
//	close(client_socketfd);

}

