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
#include "pika_init.h"
#include "pika_core.h"
/*
*********************************************************************************************************
*/
static char *prog_name = NULL;
static const char *ver = "pika-1.0";
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
static void usage()
{
    fprintf(stderr, "pika-1.0 compiled on %s %s.\n", __DATE__, __TIME__);
	fprintf(stderr, "Usage: %s [option] parameter\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h  This help text.\n");
    fprintf(stderr, "  -v  Show pika version numbers.\n");
    fprintf(stderr, "  -p  The pika listen port.\n");
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
int main(int argc, char *argv[])
{	
	int ret;
    int option;
    int listen_port = -1;

	prog_name = argv[0];
	opterr = 0;
	while((option = getopt(argc, argv, "hvp:")) != EOF){
		switch(option){
		case 'h':
			usage();
			return 0;
		case 'v':
			fprintf(stdout, "%s\n", ver);
			return 0;
   		case 'p':
			listen_port = atoi(optarg);
            break;
		default:
            fprintf(stderr, "Invalid option.\n");
			return 0;
		}
	}
    if(listen_port == -1){
        usage();
        return 0;
    }
    log2file("start.");
	start_up(listen_port);
	pika_main();
}
/*
*********************************************************************************************************
*/