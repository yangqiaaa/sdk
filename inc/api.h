/*********************************************************************************
    *Copyright(C), 2021, Company
    *FileName:  api.h
    *Author:  yangqi
    *Version:  V1.0.0
    *Date: 2023-04-12 08:15:15
    *Description:


    *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/

#ifndef  __API_H__
#define  __API_H__

/***************************************includes***********************************/
//#include"xxx.h"
#include  <stdint.h>


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/
//static int i
struct api
{
    long timestamp;
    char time[30];
    int (*test)(void);
    long (*ntp_get_time)(char* hostname);
    char* (*timestamp_to_time)(long timestamp);
    int (*ntp_to_time)(uint64_t ntp_time);
};



/***************************************Functions***********************************/
extern int api_init(struct api *api);

extern int api_set(struct api *api);

extern int test(void);

extern long ntp_get_time(char* hostname);

extern char* timestamp_to_time(long timestamp);

extern int ntp_to_time(uint64_t ntp_time);

#endif
/* [] END OF FILE */