/*
*********************************************************************************************************
*                                             Pika Client
*
*                            (c) Copyright 1999, John, All Rights Reserved
*
*********************************************************************************************************
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
/*
*********************************************************************************************************
*/
#define   MAXLINE     4096
static char *prog_name = NULL;
static const char *ver = "pika-1.0";
static const char *key_dir = "./cli_key/";
char time_buf[1000];
unsigned int sec;
static char flag = 1;
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
	fprintf(stderr, "Usage: ./%s [-h/-v]\n", prog_name);
}
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

    if(flag == 1){
        flag = 2;
        sec = (t->tm_hour) * 3600 + (t->tm_min) * 60 + t->tm_sec;
    }else{
        flag = 1;
        sec = ((t->tm_hour) * 3600 + (t->tm_min) * 60 + t->tm_sec) - sec;
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
int main(int argc, char *argv[])
{	
    int i;
	static unsigned int cnt = 0;
    int size, len;
    int sockfd;
	int ret, option;
    char cmd[128];
    char recv_buf[MAXLINE];
    char key_filename[50];
    char *buf = NULL;
    char *send_data = NULL;
    char delim[] = "reg$";
    FILE *key_fd = NULL;
    struct sockaddr_in  servaddr;
    char msg_buf[1024];
    char userid;
    char msgid;
    char *msg = NULL;
    
	prog_name = argv[0];
	opterr = 0;
	while((option = getopt(argc, argv, "hvt:u:d:")) != EOF){
		switch(option){
		case 'h':
			usage();
			return 0;
		case 'v':
			fprintf(stdout, "%s\n", ver);
			return 0;
  		case 'u':
			userid = atoi(optarg);
            break;
  		case 't':
			msgid = atoi(optarg);
            break;
  		case 'd':
			msg = optarg;
            break;
		default:
			usage();
			return 0;
		}
	}
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
		perror("socket");
		abort();
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(8080);
    if(inet_pton(AF_INET, "172.22.0.69", &servaddr.sin_addr) <= 0){  
		perror("inet_pton");
		abort();
    }
    printf("connect the server...\n");
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){  
		perror("connect");
		abort();
    }
    /******************************************************************/
	printf("register user pika...\n");
    memset(msg_buf, 0, sizeof(msg_buf));
    msg_buf[0] = 0x01;
    msg_buf[1] = 0x01;
    printf("msg id: %02x\n", msg_buf[0]);
    printf("user  id: %02x\n", msg_buf[1]);
    
    key_fd = fopen("client.msg", "wb");
	if(key_fd == NULL){
		fprintf(stderr, "open file failed.\n");
		return 0;
	}   
    fwrite(msg_buf, 2, 1, key_fd);
    fclose(key_fd);
    key_fd == NULL;
    
    sprintf(cmd, "openssl rsautl -encrypt -in client.msg -inkey \
        %sserver.pub -pubin -out client_encrypt.msg", key_dir);
    system(cmd);

    key_fd = fopen("client_encrypt.msg", "rb");
	if(key_fd == NULL){
		fprintf(stderr, "open file failed.\n");
		return;
	}
  	fseek(key_fd, 0, SEEK_END);
	size = ftell(key_fd);
	rewind(key_fd);
    
    buf = (char *)calloc(size, sizeof(char));
    if(buf == NULL){
        printf("calloc error.\n");
        return;
    }
    
    fread(buf, size, 1, key_fd);
    fclose(key_fd);
    key_fd == NULL;
    
	ret = send(sockfd, buf, size, 0);
	if(ret < 0){
		perror("send");
		goto err;
	}
    free(buf);
	printf("go to sleep 1s\n");
	sleep(1);
	/******************************************************************/
    while(1){
		memset(msg_buf, 0, sizeof(msg_buf));
		msg_buf[0] = cnt >> 24;
		msg_buf[1] = cnt >> 16;
		msg_buf[2] = cnt >> 8;
		msg_buf[3] = cnt;
        msg_buf[4] = 0x01;
        msg_buf[5] = 0x02;
		fprintf(stdout, "send msg: ");
		for(i = 0; i < 4; i++){
			fprintf(stdout, "%02x", (unsigned char)msg_buf[i]);
		}
		fprintf(stdout, "\n");	
		key_fd = fopen("client.msg", "wb");
		if(key_fd == NULL){
			fprintf(stderr, "open file failed.\n");
			return 0;
		}   
		fwrite(msg_buf, 6, 1, key_fd);
		fclose(key_fd);
		key_fd == NULL;
		
		sprintf(cmd, "openssl rsautl -encrypt -in client.msg -inkey \
			%sserver.pub -pubin -out client_encrypt.msg", key_dir);
		system(cmd);

		key_fd = fopen("client_encrypt.msg", "rb");
		if(key_fd == NULL){
			fprintf(stderr, "open file failed.\n");
			return;
		}
		fseek(key_fd, 0, SEEK_END);
		size = ftell(key_fd);
		rewind(key_fd);
		
		buf = (char *)calloc(size, sizeof(char));
		if(buf == NULL){
			printf("calloc error.\n");
			return;
		}
		
		fread(buf, size, 1, key_fd);
		fclose(key_fd);
		key_fd == NULL;
		
		ret = send(sockfd, buf, size, 0);
		if(ret < 0){
			perror("send");
			goto err;
		}
        
        fget_time(time_buf, 1000);
		free(buf);
        
		/******************************************************************/
		memset(recv_buf , 0, sizeof recv_buf);
		len = read(sockfd, recv_buf, sizeof(recv_buf));
		if(len < 0 ){
		}
       
		if(len == 0){
			printf("close connection at fd: %d\n", sockfd);
			return;
		}
		 
		key_fd = fopen("server_encrypt.msg", "wb");
		if(key_fd == NULL){
			fprintf(stderr, "open file failed.\n");
			return;
		}
		fwrite(recv_buf, len, 1, key_fd);
		fclose(key_fd);
		key_fd == NULL;

		sprintf(cmd, "openssl rsautl -decrypt -in server_encrypt.msg -inkey %spika.pri -out server.msg", key_dir);
		system(cmd);
		
		key_fd = fopen("server.msg", "rb");
		if(key_fd == NULL){
			fprintf(stderr, "open file failed.\n");
			goto err;
		}
		fseek(key_fd, 0, SEEK_END);
		size = ftell(key_fd);
		rewind(key_fd);
		buf = (char *)calloc(size, sizeof(char));
		
		if(buf == NULL){
			printf("calloc error.\n");
			return;
		}
		
		fread(buf, size, 1, key_fd);
		fclose(key_fd);
		key_fd == NULL;

		printf("reci msg: ");
		for(i = 0; i < size; i++){
			printf("%02x", (unsigned char)buf[i]);
		}
		printf(" - ");
        
        fget_time(time_buf, 1000);
        printf("%d\n", sec);
        
		free(buf);
		cnt++;
    }
    exit(0);
err:
    printf("Client error...\n");
    close(sockfd);
    exit(0);
}
/*
*********************************************************************************************************
*/
