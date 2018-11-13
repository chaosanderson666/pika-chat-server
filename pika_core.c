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
#include <unistd.h>
#include "pika_core.h"
#include "pika_config.h"
#include "pika_public.h"
#include "pika_cypher.h"

/*
*********************************************************************************************************
*/
#define   MAX_EVENTS     32
/*
*********************************************************************************************************
*/
typedef enum user_state {
    USER_STATE_DISCONNECT = 0x01,
    USER_STATE_CONNECTED  = 0x02,
} user_state_e;

typedef enum msg_type {
    MSG_TYPE_REGISTER = 0x01,
    MSG_TYPE_FW_DATA  = 0x02,
} msg_type_e;

typedef enum user_id {
    USER_ID_PIKA = 0x01,
    USER_ID_PIKI = 0x02,
} user_id_e;
/*
*********************************************************************************************************
*/
typedef struct user{
	int sockfd;
    int id;
	char *name;
    char *key_file;
    user_state_e state;
} user_t;

typedef struct msg{
	int sockfd;
	char *msg;
    int msg_len;
} msg_t;
/*
*********************************************************************************************************
*/
int epollfd;
static char enmsg_buf[RECIEVE_BUF];
static char msg_buf[RECIEVE_BUF];
struct epoll_event evt, events[MAX_EVENTS];
user_t pika = {
    .sockfd = -1, 
    .id = USER_ID_PIKA, 
    .name = "pika", 
    .key_file = "pika.pub", 
    .state = USER_STATE_DISCONNECT
};
user_t piki = {
    .sockfd = -1, 
    .id = USER_ID_PIKI, 
    .name = "piki", 
    .key_file = "piki.pub",
    .state = USER_STATE_DISCONNECT
};
extern volatile sig_atomic_t sig_terminal;
/*
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
static void got_conn(int epollfd, int listen_sockfd)
{
	int sockfd;
	int ret;
    
	sockfd = accept(listen_sockfd, NULL, NULL);
	if(sockfd < 0){
        perror("accept");
		return;
	}
    printf("\ngot a connection, fd: %d\n", sockfd);
    
	ret = set_fd_nblk(sockfd);
	if(ret < 0){
		close(sockfd);
		return;
	}
	ret = set_fd_into_evt(&evt, epollfd, sockfd);
	if(ret < 0){
		close(sockfd);
		return;
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
void reg_user(char *data, int data_len, int sockfd)
{
    unsigned char user_id;

    user_id = data[data_len - 1];
    switch(user_id){
        case USER_ID_PIKA: 
            pika.sockfd = sockfd;
            pika.state = USER_STATE_CONNECTED;
            log2file("%s has connected, fd: %d.\n", pika.name, sockfd);
            break;
        case USER_ID_PIKI:
            piki.sockfd = sockfd;
            piki.state = USER_STATE_CONNECTED;
            log2file("%s has connected, fd: %d.\n", piki.name, sockfd);
            break;
  		default:
            log2file("unkown user id.\n");
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
void forward_data(char *data, int len, int sockfd)
{
	int fd, ret;
    char *enmsg_buf = NULL;
    int enmsg_len;
	char *name = NULL;
    char *msg = NULL;
    user_t *user = NULL;
    unsigned char user_id;

    user_id = data[len - 1];
    switch(user_id){
        case USER_ID_PIKA: 
            user = &pika;
            break;
        case USER_ID_PIKI:
            user = &piki;
            break;
        default:
            printf("unkown user id.\n");
			return;
    }
    
    if(user->sockfd < 0){
        printf("%s has not connected yet.\n", user->name);
        return;
    }

    enmsg_buf = (char *)calloc(RECIEVE_BUF, sizeof(char));
    if(enmsg_buf == NULL){
        perror("calloc");
        return;
    }
    ret = encrypt_msg(user->name, data, len - 1, enmsg_buf, &enmsg_len);
    if(ret < 0){
        printf("encrypt_msg");
        free(enmsg_buf);
        return;
    }
    
	ret = send(user->sockfd, enmsg_buf, enmsg_len, 0);
	if(ret < 0){
		printf("send");
	}
    free(enmsg_buf);
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
void process_msg(char *enmsg, int enmsg_len, int sockfd)
{
    int i;
    int ret;
    int msg_len;
    unsigned char msg_type;

    memset(msg_buf, 0, sizeof(msg_buf));
    ret = decrypt_msg(enmsg, enmsg_len, msg_buf, &msg_len);
    if(ret < 0){
        printf("decrypt msg error.\n");
        return;
    }

    msg_type = msg_buf[msg_len - 1];
    switch(msg_type){
        case MSG_TYPE_REGISTER:
            reg_user(msg_buf, msg_len - 1, sockfd);
            break;
        case MSG_TYPE_FW_DATA:
            forward_data(msg_buf, msg_len - 1, sockfd);
            break;
		default:
            return;
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
void user_offline(int sockfd)
{
    if(sockfd == pika.sockfd){
        pika.sockfd = -1;
        pika.state = USER_STATE_DISCONNECT;
        printf("%s close connection, fd: %d\n", pika.name, sockfd);
        return;
    }
    
    if(sockfd == piki.sockfd){
        piki.sockfd = -1;
        piki.state = USER_STATE_DISCONNECT;
        printf("%s close connection, fd: %d\n", piki.name, sockfd);
        return;
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
void read_msg(struct epoll_event *evt)
{
    int i;
	int len;
    int offset;

    offset = 0;
    memset(enmsg_buf, 0, sizeof(enmsg_buf));
    
    while(evt->events & EPOLLIN){
        len = read(evt->data.fd, enmsg_buf + offset, sizeof(enmsg_buf) - offset);
        if(len == 0){
            close(evt->data.fd);
            user_offline(evt->data.fd);
            return;
        }
        if(len < 0 ){
           break;
        }
        offset += len;
    }
    
    if(offset > 0){
        process_msg(enmsg_buf, offset, evt->data.fd);
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
void pika_main(void)
{
    int nfds;
	int i, ret;
    int listen_sockfd;

    listen_sockfd = get_listen_sockfd();
    
	epollfd = epoll_create(MAX_EVENTS);
	if(epollfd < 0){
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	evt.data.fd = listen_sockfd;
	evt.events = EPOLLIN;
	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sockfd, &evt);
	if(ret < 0){
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
    
	while(sig_terminal == 0){
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(nfds == -1){
           perror("epoll_wait");
           goto stop;
        }
		for(i = 0; i < nfds; ++i){
			if((events[i].events & EPOLLERR)||
			   (events[i].events & EPOLLHUP)){
				close(events[i].data.fd);
				continue;
			}else if(events[i].data.fd == listen_sockfd){
				got_conn(epollfd, listen_sockfd);
            }else{
				read_msg(&events[i]);
			}
		}
	}
stop:
    close(listen_sockfd);
	close(epollfd);
	log2file("stop.");
}
/*
*********************************************************************************************************
*/
