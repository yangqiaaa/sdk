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
#include "cdcs/cdcs_msg.h"
#include "myzlog.h"

#define CDCS_MSG_TYPE_MAX TBOX__NET__MESSAGETYPE__REQUEST_SYNC_SIGNALTYPE_RESULT
/***************************************Macros***********************************/
//#define
typedef struct _ctrl_sm
{
    int type;
	int (*callback)( int*, Tbox__Net__TopMessage *);	
} cdcs_sm_t;

typedef struct
{
    cdcs_sm_t cdcs_sm[CDCS_MSG_TYPE_MAX];        //State Machines Corresponding to Different Control Commands
} cdcs_state_t;

/***************************************Variables***********************************/
cdcs_state_t cdcs_state =
{
    .cdcs_sm =
    {   
    	/*type                                 ,   funtion,            */                                         
    	{(int)TBOX__NET__MESSAGETYPE__RESPONSE_NETWORK_SIGNAL_STRENGTH	, 	cdcs_callback_signal,  		},
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
int cdcs_sm_callback(int* fd, Tbox__Net__TopMessage *msg)
{
	int i = 0;	
    for (i = 0; i < CDCS_MSG_TYPE_MAX; i++)
    {
        if (msg->message_type == cdcs_state.cdcs_sm[i].type)
        {
            if(cdcs_state.cdcs_sm[i].callback != NULL)
            {
                my_zlog_info_cdcs("cdcs msg type(%d) callback", msg->message_type);
                cdcs_state.cdcs_sm[i].callback(fd, msg);
                return 0;
            }
            else
            {
                my_zlog_error_cdcs("callbacak list(%d),*type(%d) callback is NULL",i, msg->message_type);
                return 2;
            }

        }
    }

    if(i == CDCS_MSG_TYPE_MAX)
    {
        my_zlog_error_cdcs("*no type(%d) callback", msg->message_type);
    }
    return 1;
}

/* [] END OF FILE */