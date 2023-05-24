/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs_heart.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-18 09:31:44
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "myzlog.h"
#include "mycdcs.h"
#include "cdcs/cdcs_pb.h"
#include "cdcs/cdcs_protobuf.h"
#include "cdcs/cdcs_msg.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
int cdcs_msg_heart_request_send( int fd, int heart_type)
{
    int ret = 0;
    size_t szlen = 0;
    unsigned char pro_buf[2048] = {0};
    Tbox__Net__Messagetype message_type = TBOX__NET__MESSAGETYPE__REQUEST_HEARTBEAT_SIGNAL;
	

    if( fd < 0 )
   	{
        my_zlog_error_cdcs("ivi_msg_response_send fd = %d.",fd);
        return -1;
    }

    Tbox__Net__TopMessage TopMsg;
    tbox__net__top_message__init( &TopMsg ); 

	unsigned int len = 0;
    Tbox__Net__IhuHeartType ihu_heart_type;
    tbox__net__ihu_heart_type__init(&ihu_heart_type);
	TopMsg.message_type = message_type;

    if(TBOX__NET__HEART__TYPE__HEART_3CALL == heart_type \
    || TBOX__NET__HEART__TYPE__HEART_NMEA == heart_type)
    {
        ihu_heart_type.heart_type = heart_type;
    }
    else
    {
        my_zlog_error_cdcs("unknow heart type(%d)", heart_type);
    }

    ihu_heart_type.has_heart_type = PARAM_EXISTS;

	my_zlog_info_cdcs("ihu_heart_type.heart_type = %d", ihu_heart_type.heart_type);


    TopMsg.ihu_heart_type = &ihu_heart_type;
    my_zlog_info_cdcs("TopMsg.ihu_heart_type.heart_type:%d", TopMsg.ihu_heart_type->heart_type);
    TopMsg.has_message_type = PARAM_EXISTS;
    if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
    {
        return -3;
    }

    cdcs_net_top_message_send(fd,pro_buf,szlen,ZLOG_LEVEL_INFO);
    my_zlog_info_cdcs("BUF LEN = %ld", (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + szlen));
	
}

/* [] END OF FILE */