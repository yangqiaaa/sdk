/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs_handle.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-30 14:45:27
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
#include "cdcs/cdcs_pb.h"
#include "myzlog.h"

#define CDCS_MSG_TYPE_MAX TBOX__NET__MESSAGETYPE__REQUEST_SYNC_SIGNALTYPE_RESULT
/***************************************Macros***********************************/
//#define
typedef struct _ctrl_sm
{
    int type;
	int (*callback)( int*);	
} cdcs_sm_t;

typedef struct
{
    cdcs_sm_t cdcs_sm[1];        //State Machines Corresponding to Different Control Commands
} cdcs_state_t;

/***************************************Variables***********************************/
cdcs_state_t cdcs_state =
{
    .cdcs_sm =
    {   
    	/*type                                 ,   funtion,            */                                         
    	{(int)TBOX__NET__MESSAGETYPE__RESPONSE_HEARTBEAT_RESULT	, 	NULL,  		},
    },
};


/***************************************Functions***********************************/
/** 
 * @brief 
 * @file cdcs_handle.c 
 * @name 
 * @param[in] void 
 * @return void 
 * @note 
 * @date 2023-05-30 15:09:00 
 * @version V1.0.0 
*/
int cdcs_sm_callback(int* type)
{
	int i = 0;	
    for (i = 0; i < CDCS_MSG_TYPE_MAX; i++)
    {
        if (*type == i)
        {
            if(cdcs_state.cdcs_sm[i].callback != NULL)
            {
                my_zlog_info_cdcs("*no type(%d) callback", *type);
                cdcs_state.cdcs_sm[i].callback(type);
                return 0;
            }
            else
            {
                my_zlog_error_cdcs("*type(%d) callback is NULL", *type);
                return 2;
            }

        }
    }

    if(i == CDCS_MSG_TYPE_MAX)
    {
        my_zlog_error_cdcs("*no type(%d) callback", *type);
    }
    return 1;
}

/* [] END OF FILE */