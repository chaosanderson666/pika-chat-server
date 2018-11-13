/*
*********************************************************************************************************
*                                             Pika Server
*
*                            (c) Copyright 1999, John, All Rights Reserved
*
*********************************************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/sysinfo.h>
#include <time.h>
#include "pika_log.h"
/*
*********************************************************************************************************
*/
#define   TIME_BUF_SIZE        (30)
#define   FILE_NAME_BUF_SIZE   (30)
/*
*********************************************************************************************************
*/
static char time_buf[TIME_BUF_SIZE];
static char log_filename[FILE_NAME_BUF_SIZE];
static const char *log_dir = "./log/";
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
static void fget_time(char *buf, int len)
{
	struct tm *t;
	time_t local_time;
	
	time(&local_time);
	t = localtime(&local_time);
    
    memset(buf, 0, len);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
		(1900 + t->tm_year), (1 + t->tm_mon), t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec );
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
void log2file(const char *fmt, ...)
{
	va_list ap;
	FILE *logfd;
    char *ymd;  /* year, month, day. */
    
    fget_time(time_buf, sizeof(time_buf));
    ymd = strtok(time_buf, " ");
    
	memset(log_filename, 0, sizeof(log_filename));
	sprintf(log_filename, "%s%s.log", log_dir, ymd);
	logfd = fopen(log_filename, "a+");
    if(logfd == NULL){
        perror("fopen");
        return;
    }
    
	fget_time(time_buf, sizeof(time_buf));
	fprintf(logfd, "%s >>> ", time_buf);
	va_start(ap, fmt);
	vfprintf(logfd, fmt, ap);
	va_end(ap);
	fprintf(logfd, "\n");
    fclose(logfd);
}
/*
*********************************************************************************************************
*/