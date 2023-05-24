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


#include "myzlog.h"
#include "mycdcs.h"



/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/



void cdcs_poll_1s(void){
    static clock_t clock_1s = 0;
    if(clock() - clock_1s > 1000*1000 || 0 == clock_1s)
    {
        clock_1s = clock();
    }
    
}



void cdcs_run(void){
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
void
*cdcs_pthread(void* param){
    my_zlog_info("cdcs start");
    while(1)
    {
        cdcs_run();
    }
    my_zlog_info("cdcs end");
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


/* [] END OF FILE */