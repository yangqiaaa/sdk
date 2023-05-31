/*********************************************************************************
   *Copyright(C),1996-2021, Company
   *FileName:  cdcs_connect.c
   *Author:  wyp
   *Version:  V1.0
   *Date:  2023-05-31 14:14:19
   *Description:


   *History:
        1.Date:
            Author:
            Modification:
        2.
**********************************************************************************/


/***************************************Includes***********************************/
#include <stdio.h>
#include <pthread.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h> 
#include <fcntl.h>
#include <assert.h>
#include<sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <math.h>


#include "myzlog.h"
#include "mycdcs.h"
#include "cdcs/cdcs_pb.h"
#include "cdcs/cdcs_msg.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
/** 
 * @brief tcp socket create
 * @file cdcs.c 
 * @name 
 * @param[in] struct mycdcs_t *param // cdcs struct
 * @param[in] int list (list max is 2, 0 or 1 init corresponding list, other all list )
 * @return void 
 * @note 
 * @date 2023-05-31 14:12:08 
 * @version V1.0.0 
*/
int cdcs_tcp_socket_client(struct mycdcs_t *param, int list)
{
    int i = 0;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len = 0;
    memset(&serv_addr, 0, sizeof(serv_addr));

    cdcs_init(param, list);

    param->mycdcs_data.cdcs_client[list].fd = socket(AF_INET, SOCK_STREAM, 0);
    if (param->mycdcs_data.cdcs_client[list].fd < 0) {
        my_zlog_error_cdcs("(%d)tcp socket fail:%s", i, strerror(errno));
        return -1;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(CDCS_SERVER_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(CDCS_SERVER_ADDR);

    my_zlog_info_cdcs("server addr : %d, port(%d)",serv_addr.sin_addr.s_addr, serv_addr.sin_port);

    struct sockaddr_in client_addr;  
    bzero(&client_addr, sizeof(client_addr));  
    client_addr.sin_family = AF_INET; // internet协议族IPv4  
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); // INADDR_ANY表示自动获取本机地址  
    client_addr.sin_port = htons(5757); // auto allocated, 让系统自动分配一个空闲端口  

    
    // if (bind(param->mycdcs_data.cdcs_client[list].fd, (struct sockaddr *)&client_addr, serv_addr_len) < 0)
    // {
    //     my_zlog_error_cdcs("fail to bind,fd:%d,error:%s", param->mycdcs_data.cdcs_client[i].fd, strerror(errno) );
    //     return -3;
    // }
    
    // if (listen(param->mycdcs_data.cdcs_client[i].fd, CDCS_CLIENT_NUM_MAX) < 0)
    // {
    //     my_zlog_error_cdcs("Fail to listen,error:%s", strerror(errno));
    //     return -4;
    // }

    serv_addr_len = sizeof(serv_addr);
    if (connect(param->mycdcs_data.cdcs_client[list].fd, (struct sockaddr*)&serv_addr, serv_addr_len) < 0)
    {
        my_zlog_error_cdcs("fail to connect,fd:%d,error:%s", param->mycdcs_data.cdcs_client[list].fd, strerror(errno) );
        return -5;
    }


    int flags = fcntl(param->mycdcs_data.cdcs_client[list].fd, F_GETFL, 0);
    fcntl(param->mycdcs_data.cdcs_client[list].fd, F_SETFL, flags | O_NONBLOCK);
    
    unsigned int value = 1;
    if (setsockopt(param->mycdcs_data.cdcs_client[list].fd, SOL_SOCKET, SO_REUSEADDR,(void *)&value, sizeof(value)) < 0)
    {
        my_zlog_error_cdcs("Fail to setsockop, error:%s", strerror(errno));
        return -2;
    }
    serv_addr_len = sizeof(serv_addr);

    my_zlog_info_cdcs("cdcs list(%d) create server socket success", list);
}

/**
 * @brief  create cdcs tcp client socket
 * @param unsigned char *data
 * @param unsigned int datalen
 * @param[in] struct mycdcs_t *param // cdcs struct
 * @param[in] int list (list max is 2, 0 or 1 init corresponding list, other all list )
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
int cdcs_create_tcp_client(struct mycdcs_t *param, int list)
{
    int i = 0;
    if(0 == list){
        cdcs_tcp_socket_client(param, list);
    }
    else if(1 == list){
        cdcs_tcp_socket_client(param, list);
    }
    else{
        for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++){
            cdcs_tcp_socket_client(param, i);
        } 
    }

    return 0;
}

/* [] END OF FILE */