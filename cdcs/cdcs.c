/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-12 16:28:29
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


struct mycdcs_t mycdcs;
static MSG_RX rx_msg[CDCS_CLIENT_NUM_MAX];
unsigned char recv_buf[CDCS_CLIENT_NUM_MAX][IVI_MSG_SIZE];
/***************************************Functions***********************************/
long 
cdcs_get_timestamp(void)
{
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);
	
	return timestamp.tv_sec;
}

long cdcs_get_s(struct timeval *param)
{
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);
    return labs(timestamp.tv_sec - param->tv_sec) ;
}

long cdcs_get_ms(struct timeval *param)
{
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);
    return ( (labs(timestamp.tv_sec - param->tv_sec)*1000) + (labs(timestamp.tv_usec - param->tv_usec)/1000) );
}



static int cdcs_get_3call_fd(struct mycdcs_t *param)
{
    int list = 0;
    for(list=0;list < CDCS_CLIENT_NUM_MAX;list++) 
    {
        if((param->mycdcs_data.cdcs_client[list].fd > 0)  \
        && (param->mycdcs_data.cdcs_client[list].type == TBOX__NET__HEART__TYPE__HEART_3CALL))
        {
            return param->mycdcs_data.cdcs_client[list].fd;
        }
    }
    return -1;
}

static int cdcs_get_nmea_fd(struct mycdcs_t *param)
{
    int list = 0;
    for(list=0;list < CDCS_CLIENT_NUM_MAX;list++) 
    {
        if((param->mycdcs_data.cdcs_client[list].fd > 0)  \
        && (param->mycdcs_data.cdcs_client[list].type == TBOX__NET__HEART__TYPE__HEART_NMEA))
        {
            return param->mycdcs_data.cdcs_client[list].fd;
        }
    }
    return -1;
}

static int cdcs_get_fd(struct mycdcs_t *param, int list)
{
    if((param->mycdcs_data.cdcs_client[list].fd > 0)  )
    {
        return param->mycdcs_data.cdcs_client[list].fd;
    }
    return -1;
}
/**
 * @brief init msg
 * @param MSG_RX *rx
 * @param unsigned char *buf
 * @param int size
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
static inline void msg_init_rx(MSG_RX *rx, unsigned char *buf, int size)
{
    assert(rx || buf);
    memset(buf, 0, size);
    rx->data  = buf;
    rx->size  = size;
    rx->used = 0;
}

void cdcs_init(struct mycdcs_t *param, int list){
    uint8_t i = 0;
    if(0 == list){
        param->mycdcs_data.cdcs_client[list].fd    = -1;
        param->mycdcs_data.cdcs_client[list].type  = list+1;
    }
    else if(1 == list){
        param->mycdcs_data.cdcs_client[list].fd    = -1;
        param->mycdcs_data.cdcs_client[list].type  = list+1;
    }
    else{
        for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++){
            param->mycdcs_data.cdcs_client[i].fd    = -1;
            param->mycdcs_data.cdcs_client[i].type  = list+1;
        }
    }

    if(0 == list){
        msg_init_rx(&rx_msg[list], recv_buf[list], sizeof(recv_buf[list]));
    }
    else if(1 == list){
        msg_init_rx(&rx_msg[list], recv_buf[list], sizeof(recv_buf[list]));
    }
    else{
        for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++){
            msg_init_rx(&rx_msg[i], recv_buf[i], sizeof(recv_buf[i]));
        }
    }

    my_zlog_info_cdcs("cdcs init");
}


void cdcs_poll_1s(void){
    static long time_1s = 0;
    if ( cdcs_get_timestamp() - time_1s > 0 ) {
        // my_zlog_info_cdcs("test 1s(%ld),clock(%ld),diff(%ld)", clock_1s, clock(), (clock()-clock_1s) );
        cdcs_poll_heart_send(&mycdcs);
        time_1s = cdcs_get_timestamp();
    }
    
}

void cdcs_client_handle(void){
    int list = 0;
    int max_fd = -1;
    int ret = 0;
    fd_set read_set;
    struct timeval tm;
    FD_ZERO(&read_set);
    tm.tv_sec = 0;
    tm.tv_usec = 100000;

    for (list = 0; list < CDCS_CLIENT_NUM_MAX; list++) {
        if (cdcs_get_fd(&mycdcs, list) > 0) {
            FD_SET(cdcs_get_fd(&mycdcs, list), &read_set);
            if (max_fd < cdcs_get_fd(&mycdcs, list)) {
                max_fd = cdcs_get_fd(&mycdcs, list);
            }
        }
    }

    // my_zlog_info_cdcs("test 1");
    ret = select(max_fd + 1, &read_set, NULL, NULL, &tm);
    /* the file deccriptor is readable */
    if (ret >= 0) {
        // tbox_ivi_link_process();
        // tbox_ivi_server_setup_process(); // 未建立成功后尝试重新建立
        // if (FD_ISSET(tbox_ivi_get_server_fd(), &read_set))
        // {
        //     tbox_ivi_client_accept();    // 监听客户端连接
        // }
        
        for(list = 0; list < CDCS_CLIENT_NUM_MAX; list++) {
            if (cdcs_get_fd(&mycdcs, list) > 0) {
                // tbox_ivi_clien_timeout(i);  // 客户端超时处理

                if (FD_ISSET(cdcs_get_fd(&mycdcs, list), &read_set) ) {
                    if (rx_msg[list].used >= rx_msg[list].size) {
                        rx_msg[list].used =  0;
                    }

                    int num = cdcs_read_tbox_data(&mycdcs, list, (rx_msg[list].data + rx_msg[list].used),(rx_msg[list].size - rx_msg[list].used));
                    if(num > 0) {
                        rx_msg[list].used += num;
                        // log_buf_dump(LOG_IVI, "tcp recv", rx_msg[i].data, rx_msg[i].used);
                        cdcs_msg_decodex(cdcs_get_fd(&mycdcs, list), &rx_msg[list], cdcs_msg_decode_callback);
                    }
                }
            }
        }
    }	
    else if (0 == ret)   /* timeout */ {
        // continue;   /* continue to monitor the incomging data */
        my_zlog_info_cdcs("cdcs ret = 0");
    }
    else {
        if (EINTR == errno)  /* interrupted by signal */ {
            // continue;
        }
        my_zlog_error_cdcs("ivi_main exit, error:%s", strerror(errno));
        // break;  /* thread exit abnormally */
    }

    // my_zlog_info_cdcs("test 2");
}

void cdcs_run(void) {
    cdcs_client_handle();
    cdcs_poll_1s();
    usleep(50000);
}

/** 
 * @brief 
 * @file cdcs.c 
 * @name 
 * @param[in] void 
 * @return void 
 * @note 
 * @date 2023-05-12 17:26:51 
 * @version V1.0.0 
*/
void *cdcs_pthread(void* param) {
    my_zlog_info_cdcs("cdcs start");
    cdcs_create_tcp_client(&mycdcs, 2);

    while(1) {
        cdcs_run();
    }

    my_zlog_info_cdcs("cdcs end");
    return param;
}



/** 
 * @brief 
 * @file cdcs.c 
 * @name 
 * @param[in] void 
 * @return void 
 * @note 
 * @date 2023-05-12 16:48:56 
 * @version V1.0.0 
*/
int
cdcs_task(void){
    int ret = 0;
    pthread_t cdcs_pthread_t;
    ret = pthread_create(&cdcs_pthread_t, NULL, cdcs_pthread, NULL);
    if(ret < 0)
    {
        printf("cdcs pthread create fail,ret(%d)", ret);
    }
    printf("cdcs pthread create ok,ret(%d)", ret);

    return ret;
}

int main(void){
    int zlog_init_flag = 0;
    const char zlog_conf_path[100] = "/home/yq22/Desktop/code/sdk/bin/zlog_cdcs.conf";
    const char zlog_cname[30] = "cdcs";
    struct myzlog_t myzlog;

    printf("cdcs main start\n");
    myzlog_init(&myzlog, ZLOG_CATEGORY_CDCS);
    myzlog_set(&myzlog);
    zlog_init_flag = myzlog.myzlog_api.init(myzlog.myzlog_data, zlog_conf_path);
    // zlog_init_flag = zlog_init("./zlog.conf");
    print(PRINT_INFOR, "zlog init flag %d\n", zlog_init_flag);
    myzlog.myzlog_api.get_category(&myzlog.myzlog_data, zlog_cname, ZLOG_CATEGORY_CDCS);
    my_zlog_info_cdcs("cdcs start");

    cdcs_create_tcp_client(&mycdcs, 2);

    while (1) {
        cdcs_run();
    }
    my_zlog_info_cdcs("cdcs end");
}


/* [] END OF FILE */