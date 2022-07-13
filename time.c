#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#define CST (+8)
#define IND (-5)

int main()
{
	time_t tnow;
	time_t tnow_utc;
	struct tm *tmnow;
	struct tm *tmnow_utc;

	time(&tnow);
	tmnow = localtime(&tnow);

	printf("hour - %d min - %d  sec - %d year - %d month - %d day - %d", tmnow->tm_hour,tmnow->tm_min, tmnow->tm_sec
										, tmnow -> tm_year + 1900, tmnow -> tm_mon + 1, tmnow -> tm_mday);

	time_t rawtime;
  	struct tm * timeinfo;

  	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
  	printf ( "Current local time and date: %s", asctime (timeinfo) );
  	printf ( "Current local hour : %d", timeinfo->tm_hour);
	printf ( "Current local hour : %d", timeinfo->tm_mon);
	

	struct timeval tv;
	time_t t;
	struct tm *info;
	char buffer[64];

	gettimeofday(&tv, NULL);
	t = tv.tv_sec;

	info = localtime(&t);
	printf("%s",asctime (info));
	strftime (buffer, sizeof buffer, "Today is %A, %B %d.\n", info);
	printf("%s",buffer);
	strftime (buffer, sizeof buffer, "The time is %I:%M %p.\n", info);
	printf("%s",buffer);

	// object
    time_t current_time;
  
    // pointer
    struct tm* ptime;
  
    // use time function
    time(&current_time);
  
    // gets the current-time
    ptime = gmtime(&current_time);
  
    // print the current time
    printf("Current time:\n");
  
    printf("Beijing ( China ):%2d:%02d:%02d\n",
           (ptime->tm_hour + CST) % 24, ptime->tm_min, ptime->tm_sec);
  
    printf("Delhi ( India ):%2d:%02d:%02d\n",
           (ptime->tm_hour + IND) % 24, ptime->tm_min, ptime->tm_sec);

}


