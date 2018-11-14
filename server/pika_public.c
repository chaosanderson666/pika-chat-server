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
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "pika_public.h"
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
int set_fd_nblk(int fd)
{
	int ret;
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0){
		return -1;
	}
	
	flags |= O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if(ret < 0){
		return -1;
	}

	return fd;
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
int set_fd_into_evt(struct epoll_event *evt, int epollfd, int fd)
{
	int ret;

	evt->data.fd = fd;
	evt->events = EPOLLIN | EPOLLET;
	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, evt);
	if(ret < 0){
		return -1;
    }
    return 0;
}
/*
*********************************************************************************************************
*/