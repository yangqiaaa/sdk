/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  ntp.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-04-20 20:14:13
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
//#include"xxx.h"
//#include"xxx.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>
#include "api.h"
/** 
 * * 连接到 NTP 服务器并获取当前时间 * 
 * * @param hostname NTP 服务器的主机名或 IP 地址 
 * * @return 当前时间的时间戳，以秒为单位，如果无法连接到服务器，则返回 -1 
 * */
extern long ntp_get_time(char* hostname) 
{    
    int sockfd;    
    char buffer[48] = {0};    
    struct sockaddr_in servaddr;    
    struct hostent *server;    
    long timestamp;  
    int i = 0;  
    // 创建 socket    
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    if (sockfd < 0) 
    {        
        perror("socket error");        
        return -1;    
    }  
    printf("ntp create socket ok\n");
    // 获取服务器的 IP 地址    
    server = gethostbyname(hostname);    
    if (server == NULL) 
    {        
        perror("gethostbyname error");        
        return -1;    
    } 
    printf("ntp get server ip ok\n");
    // 设置服务器地址    
    bzero(&servaddr, sizeof(servaddr));    
    servaddr.sin_family = AF_INET;    
    bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);    
    servaddr.sin_port = htons(123); // NTP 端口号    
    // 发送 NTP 请求
    // 0001 1011 2 3 3 版本3    模式3  
    // Mode：模式, 0－预留；1－对称行为；3－客户机；4－服务器；5－广播；6－NTP 控制信息。
    buffer[0] = 0x1B; // NTP 协议版本号、模式等信息    
    if (sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    {        
        perror("sendto error");      
        return -1;    
    }    
    printf("ntp send data ok\n");
    // 接收服务器的响应    
    if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) 
    {        
        perror("recv error");        
        return -1;    
    }  
    printf("ntp recv data ok\n"); 
    // 解析服务器响应的时间戳   
    for(i = 0; i < sizeof(buffer); i++) {
        printf("%02x ", buffer[i]);
    }
    printf("recv buf end");
    timestamp = ntohs(*(unsigned short*)&buffer[40]); // 获取秒数    
    timestamp -= 2208988800UL; // 将 1900 年 1 月 1 日作为起始时间    
    return timestamp;
}

/* 将时间戳转换为当前时间，返回一个字符串 */
extern char* timestamp_to_time(long timestamp) 
{    
    /* 根据时间戳创建时间结构体 */    
    time_t t = timestamp;    
    struct tm *lt = localtime(&t);      
    /* 根据时间结构体创建字符串 */    
    static char str[20];    
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", lt);     
    return str;
}

/*从ntp校时获取到的时间戳转换为当前时间*/
time_t ntp_to_unix_time(uint64_t ntp_time) 
{    
    /*NTP时间戳从1900年1月1日开始*/    
    static const uint64_t ntp_epoch_diff = 2208988800ULL;    
    /*将NTP时间戳转换为Unix时间戳，并考虑32位时间戳溢出问题*/    
    uint32_t ntp_time_high = (uint32_t)(ntp_time >> 32);    
    uint32_t ntp_time_low = (uint32_t)(ntp_time & 0xFFFFFFFF);    
    uint64_t unix_time = ((uint64_t)ntp_time_high << 32) | ntp_time_low;    
    unix_time -= ntp_epoch_diff;    
    if (unix_time > (uint64_t)0x7FFFFFFF) 
    {        
        /*超过32位时间戳范围，返回错值*/        
        return (time_t)-1;    
    }    
    return (time_t)unix_time;
}

extern int ntp_to_time(uint64_t ntp_time) 
{    
    /*从ntp校时获取到的时间戳*/    
    time_t unix_time = ntp_to_unix_time(ntp_time);    
    if (unix_time != (time_t)-1) 
    {        
        /*将Unix时间戳转换为字符串形式*/        
        char time_str[64];        
        strftime(time_str, sizeof(time_str), 
        "%Y-%m-%d %H:%M:%S", localtime(&unix_time));        
        printf("Current time: %s\n", time_str);    
    }    
    return 0;
}

/* [] END OF FILE */