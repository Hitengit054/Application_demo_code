
/* -------------- Header file ---------------- */

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<ctype.h>
#include<stdlib.h>

/* -------------- End of Header file -------------*/

/* ------------------- Macro ---------------- */

#define bufsize 1024

/* -------------- end of Macro ---------------- */


/* -------------- function prototype ---------------- */

int check_string(char buff[]);
void create_c(int client_socketfd);
void read_content(int client_socketfd);

/* -------------- End of function prototype -----------*/
