/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs_signal.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-30 15:42:17
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "cdcs/cdcs_pb.h"
#include "cdcs/cdcs_msg.h"
#include "myzlog.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
int cdcs_callback_signal(int *fd, Tbox__Net__TopMessage *msg)
{
    if(*fd < 0)
    {
        return 1;
    }
    if(msg == NULL)
    {
        return 2;
    }

    switch (msg->message_type)
    {
        case TBOX__NET__MESSAGETYPE__RESPONSE_NETWORK_SIGNAL_STRENGTH:
        {
            my_zlog_info_cdcs("cdcs signal type(%d) power(%d)", msg->signal_type, msg->signal_power);
            break;
        }

    
        default:
            break;
    }
    return 0;
}

/* [] END OF FILE */