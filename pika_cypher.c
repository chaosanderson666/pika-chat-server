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
#include <errno.h>
#include <unistd.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "pika_cypher.h"
/*
*********************************************************************************************************
*/
static const char *key_dir = "./key/";
char pri_key[] = \
    "-----BEGIN RSA PRIVATE KEY-----\n"\
    "MIIEowIBAAKCAQEAy8Dbv8prpJ/0kKhlGeJYozo2t60EG8L0561g13R29LvMR5hy\n"\
    "vGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+vw1HocOAZtWK0z3r26uA8kQYOKX9\n"\
    "Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQApfc9jB9nTzphOgM4JiEYvlV8FLhg9\n"\
    "yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68i6T4nNq7NWC+UNVjQHxNQMQMzU6l\n"\
    "WCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoVPpY72+eVthKzpMeyHkBn7ciumk5q\n"\
    "gLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUywQIDAQABAoIBADhg1u1Mv1hAAlX8\n"\
    "omz1Gn2f4AAW2aos2cM5UDCNw1SYmj+9SRIkaxjRsE/C4o9sw1oxrg1/z6kajV0e\n"\
    "N/t008FdlVKHXAIYWF93JMoVvIpMmT8jft6AN/y3NMpivgt2inmmEJZYNioFJKZG\n"\
    "X+/vKYvsVISZm2fw8NfnKvAQK55yu+GRWBZGOeS9K+LbYvOwcrjKhHz66m4bedKd\n"\
    "gVAix6NE5iwmjNXktSQlJMCjbtdNXg/xo1/G4kG2p/MO1HLcKfe1N5FgBiXj3Qjl\n"\
    "vgvjJZkh1as2KTgaPOBqZaP03738VnYg23ISyvfT/teArVGtxrmFP7939EvJFKpF\n"\
    "1wTxuDkCgYEA7t0DR37zt+dEJy+5vm7zSmN97VenwQJFWMiulkHGa0yU3lLasxxu\n"\
    "m0oUtndIjenIvSx6t3Y+agK2F3EPbb0AZ5wZ1p1IXs4vktgeQwSSBdqcM8LZFDvZ\n"\
    "uPboQnJoRdIkd62XnP5ekIEIBAfOp8v2wFpSfE7nNH2u4CpAXNSF9HsCgYEA2l8D\n"\
    "JrDE5m9Kkn+J4l+AdGfeBL1igPF3DnuPoV67BpgiaAgI4h25UJzXiDKKoa706S0D\n"\
    "4XB74zOLX11MaGPMIdhlG+SgeQfNoC5lE4ZWXNyESJH1SVgRGT9nBC2vtL6bxCVV\n"\
    "WBkTeC5D6c/QXcai6yw6OYyNNdp0uznKURe1xvMCgYBVYYcEjWqMuAvyferFGV+5\n"\
    "nWqr5gM+yJMFM2bEqupD/HHSLoeiMm2O8KIKvwSeRYzNohKTdZ7FwgZYxr8fGMoG\n"\
    "PxQ1VK9DxCvZL4tRpVaU5Rmknud9hg9DQG6xIbgIDR+f79sb8QjYWmcFGc1SyWOA\n"\
    "SkjlykZ2yt4xnqi3BfiD9QKBgGqLgRYXmXp1QoVIBRaWUi55nzHg1XbkWZqPXvz1\n"\
    "I3uMLv1jLjJlHk3euKqTPmC05HoApKwSHeA0/gOBmg404xyAYJTDcCidTg6hlF96\n"\
    "ZBja3xApZuxqM62F6dV4FQqzFX0WWhWp5n301N33r0qR6FumMKJzmVJ1TA8tmzEF\n"\
    "yINRAoGBAJqioYs8rK6eXzA8ywYLjqTLu/yQSLBn/4ta36K8DyCoLNlNxSuox+A5\n"\
    "w6z2vEfRVQDq4Hm4vBzjdi3QfYLNkTiTqLcvgWZ+eX44ogXtdTDO7c+GeMKWz4XX\n"\
    "uJSUVL5+CVjKLjZEJ6Qc2WZLl94xSwL71E41H4YciVnSCQxVc4Jw\n"\
    "-----END RSA PRIVATE KEY-----\n";
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
size_t encrypt_msg(char *user_name, char *msg, int msg_len, char *en_msg, int *enmsg_len)
{
    int len;
    char cmd[128];
    FILE *fd = NULL;

    if((user_name == NULL)||(msg == NULL)||(msg_len <= 0)||
       (en_msg == NULL)||(enmsg_len == NULL)){
        return -1;
    }
    
    fd = fopen("forward.msg", "wb");
	if(fd == NULL){
		perror("fopen");
		return -1;
	}
    fwrite(msg, msg_len, 1, fd);
    fclose(fd);
    fd = NULL;
    
    sprintf(cmd, "openssl rsautl -encrypt -in forward.msg \
        -inkey %s%s.pub -pubin -out fwd_encrypt.msg", key_dir, user_name);
    system(cmd);

    fd = fopen("fwd_encrypt.msg", "rb");
	if(fd == NULL){
		perror("fopen");
		return -1;
	}
   	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	rewind(fd);
    fread(en_msg, len, 1, fd);
    *enmsg_len = len;
    fclose(fd);
    fd = NULL;
    return 0;
}
/*
*********************************************************************************************************
* Description : This function parse the command and execute it.
* Arguments   : None.
* Returns     : the execute status.
* Notes       : none.
*********************************************************************************************************
*/
size_t decrypt_msg(char *enmsg, int enmsg_len, char *msg, int *msg_len)
{
    char cmd[128];
    FILE *fd = NULL;
    int size;

    if((enmsg == NULL)||(enmsg_len <= 0)||
       (msg_len == 0) ||(msg == NULL)){
        return -1;
    }
    
    fd = fopen("encrypt.msg", "wb");
	if(fd == NULL){
		perror("fopen");
		return -1;
	}
    fwrite(enmsg, enmsg_len, 1, fd);
    fclose(fd);
    fd = NULL;

    sprintf(cmd, "openssl rsautl -decrypt -in encrypt.msg \
        -inkey %sserver.pri -out user.msg", key_dir);
    system(cmd);

    fd = fopen("user.msg", "rb");
	if(fd == NULL){
		perror("fopen");
		return -1;
	}
  	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	rewind(fd);
    fread(msg, size, 1, fd);
    *msg_len = size;
    fclose(fd);
    fd = NULL;
    return 0;
}
/*
*********************************************************************************************************
*/