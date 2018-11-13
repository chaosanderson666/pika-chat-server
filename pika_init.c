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
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include "pika_init.h"
#include "pika_config.h"
#include "pika_public.h"
/*
*********************************************************************************************************
*/
static int listen_sockfd;
/*
*********************************************************************************************************
*/
volatile sig_atomic_t sig_child;
volatile sig_atomic_t sig_terminal;
volatile sig_atomic_t sig_user1;
volatile sig_atomic_t sig_user2;
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
int get_listen_sockfd(void)
{
    return listen_sockfd;
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
static void sig_handler(int signal)
{
	switch (signal) {
		case SIGINT:
		case SIGTERM:
			sig_terminal = 1;
            printf("received SIGTERM\n");
			break;
		case SIGCHLD:
			sig_child = 1;
            printf("received SIGCHLD\n");
			break;
		case SIGUSR1:
            printf("received SIGUSR1\n");
			sig_user1 = 1;
			break;
		case SIGUSR2:
			sig_user2 = 1;
            printf("received SIGUSR2\n");
			break;
	}
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
static void install_signal(int sig, void(*handler)(int))
{
	struct sigaction act;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGTERM);
	act.sa_flags = 0;
    sigaction(sig, &act, NULL);
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
void start_up(int listen_port)
{
	int ret;
	struct sockaddr_in server_addr;

	listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sockfd < 0){
		perror("socket");
		abort();
	}
	listen_sockfd = set_fd_nblk(listen_sockfd);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(listen_port);

	ret = bind(listen_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret < 0){
		perror("bind");
		abort();
	}

	ret = listen(listen_sockfd, 10);
	if(ret < 0){
		perror("listen");
		abort();
	}

  	install_signal(SIGINT , sig_handler);
    install_signal(SIGTERM, sig_handler);
	install_signal(SIGCHLD, sig_handler);
	install_signal(SIGUSR1, sig_handler);
	install_signal(SIGUSR2, sig_handler);
}
/*
*********************************************************************************************************
*/
