/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  sdk.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-04-12 09:55:52
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "api.h"
#include<stdio.h>


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
extern int api_init(struct api *api)
{
    api->test = NULL;
}

extern int api_set(struct api *api)
{
    api->ntp_get_time = ntp_get_time;
    api->timestamp_to_time = timestamp_to_time;
    api->ntp_to_time = ntp_to_time;
}

/* [] END OF FILE */