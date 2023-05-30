/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs_msg.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-18 09:37:03
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "mycdcs.h"
#include "cdcs/cdcs_pb.h"
#include "cdcs/cdcs_protobuf.h"
#include "cdcs/cdcs_msg.h"
#include "myzlog.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
//send  ######################################
int cdcs_net_top_message_pack( Tbox__Net__TopMessage *TopMsg,unsigned char *pro_buf,unsigned int in_size,size_t *out_size)
{
    if((pro_buf == NULL) || (TopMsg == NULL) || (out_size == NULL))
    {
        my_zlog_error_cdcs("*TopMsg or *pro_buf or *out_size is null!");
        return -1;
    }

    *out_size = tbox__net__top_message__get_packed_size(TopMsg);

    if(*out_size >= in_size)
    {
        my_zlog_error_cdcs("tbox__net__top_message__get_packed_size too long!out_size(%lu),inbuf_size(%u)",*out_size,in_size);
        return -1;
    }
    
    tbox__net__top_message__pack(TopMsg,pro_buf);

    return 0;
}

int cdcs_net_top_message_send( int fd ,unsigned char *pro_buf,unsigned int in_size,int loglv)
{
    char send_buf[5120] = {0};
    int ret = -1,i = 0;
    int j = 0;

    if(pro_buf == NULL)
    {
        my_zlog_error_cdcs("*pro_buf is null!");
        return -1;
    }

    if((IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size) >= sizeof(send_buf))
    {
        my_zlog_error_cdcs("tbox__net__top_message gps send_buf size error!szlen(%u)",(IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size));
        return ret;
    }

    memcpy(send_buf,IVI_PKG_MARKER,IVI_PKG_S_MARKER_SIZE);
    send_buf[IVI_PKG_S_MARKER_SIZE] = in_size >> 8;
    send_buf[IVI_PKG_S_MARKER_SIZE + 1] = in_size;

    for( i = 0; i < in_size; i ++ )
    {
        send_buf[ i + IVI_PKG_S_MARKER_SIZE + 2 ] = pro_buf[i];
    }

    memcpy(( send_buf + IVI_PKG_S_MARKER_SIZE + in_size + 2),IVI_PKG_ESC,IVI_PKG_E_MARKER_SIZE);

    ret = send(fd, send_buf, (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size), 0);

    if (ret < (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size))
    {
        my_zlog_error_cdcs("cdcs msg send failed,close fd(%d)", fd);
        for(j =0; j < CDCS_CLIENT_NUM_MAX; j++)
        {
            // if((cdcs_clients[j].fd > 0) && (cdcs_clients[j].fd == fd))
            // {
            //     my_zlog_error_cdcs("close fd(%d),type(%d)",cdcs_clients[j].fd, cdcs_clients[j].type);
            //     // close(cdcs_clients[j].fd);
            //     // cdcs_clients[j].fd = -1;
            //     // cdcs_clients[j].type = -1;
            //     break;
            // }
        }
    }
    else 
    {
        my_zlog_debug_cdcs("cdcs msg send response success");
    }

    // if(loglv <= LOG_LEVEL_INFOR)
    // {
    //      WDUMP("tbox send", (const char *)send_buf, (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size));
    // }
    // else if(loglv == LOG_LEVEL_TRACE)
    // {
    //     TDUMP("tbox send", (const char *)send_buf, (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size) );
    // }
    // else
    // {
    //     DDUMP("tbox send", (const char *)send_buf, (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + in_size));
    // }
   
    // cdcs_msg_send_result_show(ret, in_size, loglv);

    return ret;
}

//recv###########################

int cdcs_read_tbox_data(struct mycdcs_t *param, int list,uint8_t *dstbuf,int len)
{
    int num = 0;
    if(dstbuf == NULL)
    {
        my_zlog_error_cdcs("dstbuf is NULL!!!");
        return -1;
    }
    if(param->mycdcs_data.cdcs_client[list].fd > 0)
    {
        num = recv(param->mycdcs_data.cdcs_client[list].fd, dstbuf, len, 0);
        if(num < 0)
        {
            if (num == 0 && (EINTR != errno))
            {
                my_zlog_error_cdcs("TCP client disconnect!!!!");
            }
            my_zlog_error_cdcs("recv data error,client exit\n");
            my_zlog_error_cdcs("recv data error,Client(%d) exit\n", param->mycdcs_data.cdcs_client[list].fd);
            close(param->mycdcs_data.cdcs_client[list].fd);
            param->mycdcs_data.cdcs_client[list].fd = -1;
            return -1;
        }
    }
    return num;                            
}

int str_find(const char * string, unsigned int strlen, const char *substring, int unsigned sublen)
{
	int i = 0;
	int j = 0;
	if( ( string == NULL ) || ( substring == NULL ) )
	{
        my_zlog_error_cdcs("%s","str_find string is null!!!");
		return -1;
	}
	
	if ( strlen < sublen )
	{
        my_zlog_error_cdcs("strlen = %d, sublen = %d.",strlen,sublen);
		return -1;
	}
	
	for ( i = 0; i <= strlen - sublen; i++ )
	{
		for ( j = 0; j < sublen; j++ )
		{
			if ( string[i + j] != substring[j] )
			{
				break;
			}
		}
		
		if ( j == sublen )
		{
			return i;
		}
	}
	return -1;
}


void cdcs_msg_decodex(int fd, MSG_RX *rx, ivi_msg_handler ivi_msg_proc)
{
    int ret, len, i;
    int r_pos = 0, start_pos = -1, end_pos = -1,flag = 0;

    while(rx->used > IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE)
    { 
        if( start_pos < 0 )
        {
            ret = str_find( (const char *)(rx->data + r_pos) ,rx->used,IVI_PKG_MARKER,IVI_PKG_S_MARKER_SIZE);
            flag++;
            if( ret >= 0 )
            {
                start_pos = r_pos + ret;
                r_pos = start_pos + IVI_PKG_S_MARKER_SIZE;
            }
            else
            {
                r_pos = rx->used;
            }
        }

        /* nothing is found in the buffer, ignore it */
        if (-1 == start_pos)
        {
            rx->used = 0;
        }
        /* start tag is found, but the end tag is not found in a very long buffer, ignore it */
        else if ((r_pos - start_pos) >= (rx->size - 20))
        {
            rx->used = 0;
        }
        /* start tag is found */
        else
        {
            if(flag == 1)
            {
                len = rx->used - start_pos;
            }
            else
            {
                len = rx->used;
            }
            
            if (start_pos != 0)
            {
                for (i = 0; i < len; i++)
                {
                    rx->data[i] = rx->data[i + start_pos];
                }
            }
            rx->used = len;
            ret = str_find( (const char *)(rx->data + r_pos) ,rx->used ,IVI_PKG_ESC,IVI_PKG_E_MARKER_SIZE);
            if( ret >= 0 )
            {
                end_pos = r_pos + ret + IVI_PKG_E_MARKER_SIZE;
                r_pos = end_pos;
                len = end_pos - start_pos;
                rx->used -= len;   
                ivi_msg_proc(fd, rx->data , len);
                start_pos = -1;
                end_pos = -1;
            }
            else
            {
                r_pos = rx->used;
            }
        }
        if(rx->used < IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE)
        {
            rx->used = 0;
        }
    }  
}


/**
 * @brief cdcs message request process.
 * if msg true ,call cdcs_msg_response_send
 * @param unsigned char *data
 * @param unsigned int datalen
 * @param int fd
 * @return none
 * @date 2021/10/22
 * @version V1.0
**/
void cdcs_msg_decode_callback(int fd, unsigned char *data, unsigned int len)
{
    int i = 0;
    short msg_len = 0;
    short msg_len1 = 0;
    short msg_len2 = 0;
    unsigned char proto_buf[2048] = {0};

    Tbox__Net__TopMessage *TopMsg;
    
    // DDUMP("cdcs msg:", (const char *)data, len);


    msg_len1 = data[ IVI_PKG_S_MARKER_SIZE ];
    msg_len2 = data[ IVI_PKG_S_MARKER_SIZE + 1 ];

    msg_len = (msg_len1 << 8) + msg_len2;

	if(msg_len > 2048)
	{
		msg_len = 2048;
	}
    // DEBUG("msg_len = %d.",msg_len);

    for(i = 0; i < msg_len; i ++ )
    {
        proto_buf[i] = data[ i + IVI_PKG_S_MARKER_SIZE + IVI_PKG_MSG_LEN ];
    }

	// DDUMP("PROTO BUF", (const char *)proto_buf, msg_len);

    TopMsg = tbox__net__top_message__unpack(NULL, msg_len , proto_buf);

    if( NULL == TopMsg )
    {
        my_zlog_error_cdcs("top_message__unpack failed !!!");
        // EDUMP("cdcs msg:", (const char *)data, len);
        return;
    }

    //DEBUG("TopMsg->message_type = %d.",TopMsg->message_type);
    if(TopMsg->message_type != TBOX__NET__MESSAGETYPE__REQUEST_HEARTBEAT_SIGNAL)
    {
        my_zlog_info_cdcs("TopMsg->message_type = %d.",TopMsg->message_type);
    } 
    else 
    {
        my_zlog_debug_cdcs("TopMsg->message_type = %d.",TopMsg->message_type);
    }

    switch( TopMsg->message_type )
    {
        //0 none
        case TBOX__NET__MESSAGETYPE__REQUEST_RESPONSE_NONE:
        {
            my_zlog_error_cdcs("msg request is none");
            break;
        }

        //4 tbox send signal type and power
        case TBOX__NET__MESSAGETYPE__RESPONSE_NETWORK_SIGNAL_STRENGTH:
        {
            cdcs_sm_callback(&fd, TopMsg);
            break;
        }
        
        // //1 heartbeat
        // case TBOX__NET__MESSAGETYPE__REQUEST_HEARTBEAT_SIGNAL:
        // {
        //     if(NULL == TopMsg->ihu_heart_type)
		// 	{
		// 			ERROR("TopMsg->ihu_heart_type == NULL !!!");
		// 			return;
		// 	}
        //     DEBUG("heartbeat...");
            
        //     //heartbeat type 3call or nmea
        //     cdcs_heart_type_decode(fd, TopMsg->ihu_heart_type->heart_type);
        //     break;
        // } 

        // //2 heartbeat trsult
        // case TBOX__NET__MESSAGETYPE__RESPONSE_HEARTBEAT_RESULT:
        // {
        //     ERROR("message type is response hearbeat result,should tbox send");
        //     break;
        // }
        
        // //5 3call
        // case TBOX__NET__MESSAGETYPE__REQUEST_CALL_ACTION:
        // {
        // 	DEBUG("call action request");
        //     memset(&callrequest,0 ,sizeof(ivi_callrequest));
			
		// 	if( NULL == TopMsg->call_action )
		// 	{
		// 			ERROR("TopMsg->call_action == NULL !!!");
		// 			return;
		// 	}

        //     cdcs_call_action(fd, TopMsg->call_action->type, TopMsg->call_action->action);
        //     break;
        // }
		
        // //6 call result
        // case TBOX__NET__MESSAGETYPE__RESPONSE_CALL_ACTION_RESULT:
        // {
        //     ERROR("message type is response call result,should tbox send");
        //     break;
        // }

        // //7 call result
        // case TBOX__NET__MESSAGETYPE__RESPONSE_CALL_STATUS:
        // {
        //     ERROR("message type is response call state,should tbox send");
        //     break;
        // }

        // //8 tbox info
		// case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_INFO:
		// {
		// 	DEBUG("tbox infor request");
        //     cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_INFO);
		// 	break;
		// }

        // //10 gps on off
        // case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_GPS_SET:
        // {   
        // 	DEBUG("gps set request");
        //     if( NULL == TopMsg->tbox_gps_ctrl )
		// 	{
		// 			ERROR("TopMsg->tbox_gps_ctrl == NULL !!!");
		// 			return;
		// 	}
        //     //DEBUG("gps state %d.",TopMsg->tbox_gps_ctrl->onoff);
        //     switch ( TopMsg->tbox_gps_ctrl->onoff )
        //     {
        //         //gps open
        //         case TBOX__NET__GPS__SEND__ON_OFF__GPS_ON:
        //         {
        //             gnss_onoff = 1;
        //             DEBUG("gps on request");
                    
        //             cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_GPS_SET);

        //             break;
        //         }

        //         //gps stop
        //         case TBOX__NET__GPS__SEND__ON_OFF__GPS_OFF:
        //         {
        //             gnss_onoff = 0;

        //             DEBUG("gps off request");
                    
        //             cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_GPS_SET);

        //             break;
        //         }

        //         //gps once
        //         case TBOX__NET__GPS__SEND__ON_OFF__GPS_ONCE:
        //         {
        //             DEBUG("gps once request");
        //             cdcs_gnss_response_send(fd);
        //             break;
        //         }

        //         default:
        //         {
        //             ERROR("unkonw gps onoff type!!!");
        //             break;
        //         }
        //     }
        //     break;
        // }
		
	    // //15 remotediagnose result
        // case TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_REMOTEDIAGNOSE_RESULT:
	    // {
	    //     if( NULL == TopMsg->msg_result )
		//     {
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
        //     DEBUG("remotediagnose result request");
	    // 	if(TopMsg->msg_result->result == true)
	    // 	{
		// 		INFOR("remotediagnos success...");
		// 		//tspdiagnos_flag = 0;
	   	// 	}
		// 	break;
	    // }
	   
	    // //17 logfile request
	    // case TBOX__NET__MESSAGETYPE__RESPONSE_IHU_LOGFILE_RESULT:
	   	// {
	   	// 	if( NULL == TopMsg->msg_result )
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
        //     DEBUG("logfile request request");
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		DEBUG("tsplogfile success......");
		// 		//tsplogfile_flag = 0;
		// 	}
			
		// 	break;
	   	// }
	   
	    // //19 IHU chargeappointment result
	    // case TBOX__NET__MESSAGETYPE__RESPONSE_IHU_CHARGEAPPOINTMENTSTS_RESULT:
	   	// {
	   	// 	if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
        //     INFOR("IHU chargeappointment est result request");
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		INFOR("chager appointment update success......");
		// 		appointment_sync = 0;  
		// 	}
			
		// 	break;
	   	// }
	   
	    // //20 charge appointment set result
	    // case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_CHARGEAPPOINTMENTSET:
	    // {   
        //     uint8_t data[11] = {0};
        //     uint32_t cmd ;
		// 	// ivi_chargeAppointSt HuChargeAppoint;
		// 	if(TopMsg->tbox_charge_appoointmentset == NULL)
		// 	{
		// 		ERROR("TopMsg->tbox_charge_appoointmentset == NULL");
		// 		return;
		// 	}
        //     INFOR("charge appointment set request");

		// 	INFOR("HuChargeAppoint.effectivestate = %d",TopMsg->tbox_charge_appoointmentset->effectivestate);
		// 	if(TopMsg->tbox_charge_appoointmentset->effectivestate == 1)
		// 	{
		// 		cmd = PP_RMTCTRL_APPOINTCHARGE;
        //         INFOR("set request");
		// 	}
		// 	else
		// 	{
		// 		cmd = PP_RMTCTRL_CHRGCANCELAPPOINT;
        //         INFOR("abort request");
        //     }
			
		// 	cdcs_appoint_time->timestamp = TopMsg->tbox_charge_appoointmentset->timestamp;
		// 	cdcs_appoint_time->id = TopMsg->tbox_charge_appoointmentset->id;
		// 	cdcs_appoint_time->hour = TopMsg->tbox_charge_appoointmentset->hour;
		// 	cdcs_appoint_time->min = TopMsg->tbox_charge_appoointmentset->min;
		// 	cdcs_appoint_time->targetpower = TopMsg->tbox_charge_appoointmentset->targetpower;
		// 	cdcs_appoint_time->effectivestate = TopMsg->tbox_charge_appoointmentset->effectivestate;
        //     cdcs_appoint_time->longtime = TopMsg->tbox_charge_appoointmentset->longtime;
        //     cdcs_appoint_time->iscontinuecharge = TopMsg->tbox_charge_appoointmentset->iscontinuecharge;

        //     INFOR("cdcs_appoint_time->timestamp = %d",cdcs_appoint_time->timestamp);
        //     INFOR("cdcs_appoint_time->id = %d",cdcs_appoint_time->id);
        //     INFOR("cdcs_appoint_time->hour = %d",cdcs_appoint_time->hour);
        //     INFOR("cdcs_appoint_time->min = %d",cdcs_appoint_time->min);
        //     INFOR("cdcs_appoint_time->targetpower = %d",cdcs_appoint_time->targetpower);
        //     INFOR("cdcs_appoint_time->effectivestate = %d",cdcs_appoint_time->effectivestate);
		// 	INFOR("cdcs_appoint_time->longtime = %d",cdcs_appoint_time->longtime);
        //     INFOR("cdcs_appoint_time->iscontinuecharge = %d",cdcs_appoint_time->iscontinuecharge);
        //     data[0] = (uint8_t)(TopMsg->tbox_charge_appoointmentset->id >> 24);
		// 	data[1] = (uint8_t)(TopMsg->tbox_charge_appoointmentset->id >> 16);
        //     data[2] = (uint8_t)(TopMsg->tbox_charge_appoointmentset->id >> 8);
        //     data[3] = (uint8_t)(TopMsg->tbox_charge_appoointmentset->id );
        //     data[4] = TopMsg->tbox_charge_appoointmentset->hour;
        //     data[5] = TopMsg->tbox_charge_appoointmentset->min;
        //     data[6] = TopMsg->tbox_charge_appoointmentset->targetpower;
        //     data[7] = 0xff;  
        //     data[8] = (uint8_t) (TopMsg->tbox_charge_appoointmentset->longtime >> 8);
        //     data[9] = (uint8_t) TopMsg->tbox_charge_appoointmentset->longtime;
        //     data[10] = (uint8_t) TopMsg->tbox_charge_appoointmentset->iscontinuecharge;

        //     tbox_ivi_charge_appoint( cmd,data);
		// 	cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_CHARGEAPPOINTMENTSET);
		// 	break;
	   	// }
	    // //22 charge control
	    // case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_CHARGECTRL:
	    // {
		// 	//ivi_chargeAppointSt HuChargeCtrl;
		// 	if(TopMsg->tbox_chargectrl == NULL)
		// 	{
		// 		ERROR("TopMsg->tbox_chargectrl == NULL");
		// 		return ;
		// 	}
        //     INFOR("charge control request");
        //     INFOR("TopMsg->tbox_chargectrl->timestamp %d",TopMsg->tbox_chargectrl->timestamp);
        //     INFOR("TopMsg->tbox_chargectrl->commend %d",TopMsg->tbox_chargectrl->commend);
        //     INFOR("TopMsg->tbox_chargectrl->targetpower %d",TopMsg->tbox_chargectrl->targetpower);
			
			
		// 	if(TopMsg->tbox_chargectrl->commend == 0)
		// 	{
        //         tbox_ivi_charge_ctrl(PP_RMTCTRL_STOPCHARGE);
        //         DEBUG("commend off");
		// 	}
		// 	else
		// 	{
        //         tbox_ivi_charge_ctrl(PP_RMTCTRL_STARTCHARGE);
        //         DEBUG("comment on");
		// 	}
			
		// 	cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_CHARGECTRL);
			
		// 	break;
	    // }
		
        // //24 call state
		// case TBOX__NET__MESSAGETYPE__IHU_CALL_STATUS_RESULT:
		// {
		// 	INFOR("ivi response call state");

        //     if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		DEBUG("ivi response call state success......");
		// 	}
        //     else ERROR("ivi response call state fail......");
			
		// 	break;
		// }

        // //25 active state
		// case TBOX__NET__MESSAGETYPE__IHU_ACTIVESTATE_RESULT:
		// {
		// 	DEBUG("ivi response active state");

        //     if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
        //         INFOR("fd[%d] ivi response active state success......", fd);
		// 	}
        //     else 
        //     {
        //         ERROR("ivi response active state fail......");
        //     }
			
		// 	break;
		// }
        // //29 OTP update task result
	    // case TBOX__NET__MESSAGETYPE__RESPONSE_OTAUPDATE_TASK_RESULT:
	    // {
	    // 	if( NULL == TopMsg->msg_result )
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
        //     DEBUG("OTP update task result request");
        //     ERROR("Look at the next paragraph");
        //     ERROR("no this cmd,I need interface,byebye");
        //     return;
        //     #if 0
		// 	otaupdate_flag = 0;

		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		DEBUG("TSP FOTA UODATing DEBUGM HU success......");
				
		// 		PP_FIP_InfoPush_cb(IVI_FOTA_PUSH_SUCCESS);
		// 	}
		// 	else
		// 	{
		// 		DEBUG("TSP FOTA UODATing DEBUGM HU fail......");
				
		// 		PP_FIP_InfoPush_cb(IVI_FOTA_PUSH_FAIL);
		// 	}
		// 	#endif
		// 	break;	
	    // }

        // //26 request query charge record
		// case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_CHARGE_RECORD:
		// {
        //     DEBUG("query charge record request");
		// 	cdcs_message_request( fd ,TBOX__NET__MESSAGETYPE__RESPONSE_CHARGE_RECORD_RESULT,NULL);
		// 	break;
		// }

        // //30 set battery mode
		// case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_BATTERYMODE:
		// {
        //     uint8_t data;
		// 	if(NULL == TopMsg->set_tbox_battery_mode)
		// 	{
		// 		ERROR("TopMsg->set_tbox_battery_mode == NULL !!!");
		// 		return;
		// 	}
        //     else
		// 	{
		// 		INFOR("cdcs set battery mode request");
        //         INFOR("timestamp %d, mode =%d", \
        //             TopMsg->set_tbox_battery_mode->timestamp, \
        //             TopMsg->set_tbox_battery_mode->mode);
        //         data = TopMsg->set_tbox_battery_mode->mode;
        //         if(0 == TopMsg->set_tbox_battery_mode->mode) 
        //         {
        //             INFOR("health mode 80");
        //         }
        //         else if(1 == TopMsg->set_tbox_battery_mode->mode) 
        //         {
        //             INFOR("long distance mode 100");
        //         }
        //         else if(2 == TopMsg->set_tbox_battery_mode->mode) 
        //         {
        //             INFOR("health mode 90");
        //         }
        //         else 
        //         {
        //             INFOR("unknow battery mode");
        //         }
        //         cdcs_battery_mode_request.timestamp = TopMsg->set_tbox_battery_mode->timestamp;
        //         cdcs_battery_mode_request.result = TopMsg->set_tbox_battery_mode->mode;
        //         tbox_ivi_charge_batmode(PP_RMTCTRL_SETCHARGEMODE, &data);

        //         cdcs_battery.timestamp = TopMsg->set_tbox_battery_mode->timestamp;
        //         cdcs_battery.mode = TopMsg->set_tbox_battery_mode->mode;

        //         INFOR("START SET battery mode");
        //         // cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_BATTERYMODE);
		// 	}
		// 	break;
		// }

        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTERYMODE:
        // {
        //     INFOR("query batterymode");
        //     cdcs_msg_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTERYMODE);
        // }

        // //33 start 360 fid car result
		// case TBOX__NET__MESSAGETYPE__RESPONSE_START_SENTINELMODE_RESULT:
		// {
		// 	DEBUG("ivi response start 360 find car");

        //     if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		DEBUG("ivi response start 360 find car success......");
		// 	}
        //     else ERROR("ivi response start 360 find car fail......");
			
		// 	break;
		// }

        // //34 end 360 fid car
		// case TBOX__NET__MESSAGETYPE__REQUEST_END_SENTINELMODE:
		// {
		// 	if(NULL == TopMsg->end_sentinel_mode)
		// 	{
		// 		ERROR("TopMsg->set_tbox_battery_mode == NULL !!!");
		// 		return;
		// 	}
        //     else
		// 	{
		// 		DEBUG("end find car mode request");
        //         INFOR("TopMsg->end_sentinel_mode->timestamp %d", TopMsg->end_sentinel_mode->timestamp);
		// 		INFOR("TopMsg->end_sentinel_mode->mode =%d",TopMsg->end_sentinel_mode->mode);
        //         INFOR("TopMsg->end_sentinel_mode->eventid =%d",TopMsg->end_sentinel_mode->eventid);
        //         INFOR("TopMsg->end_sentinel_mode->uploadsts =%d",TopMsg->end_sentinel_mode->uploadsts);
        //         if( TopMsg->end_sentinel_mode->filename != NULL)
        //         {
        //             INFOR("TopMsg->end_sentinel_mode->filename =%s",TopMsg->end_sentinel_mode->filename);
        //             tbox_respoend_result_to_tsp(TopMsg->end_sentinel_mode->uploadsts,TopMsg->end_sentinel_mode->filename,strlen(TopMsg->end_sentinel_mode->filename),TopMsg->end_sentinel_mode->mode);
        //         }
        //         else
        //         {
        //             INFOR("TopMsg->end_sentinel_mode->filename == NULL");
        //             tbox_respoend_result_to_tsp(TopMsg->end_sentinel_mode->uploadsts,NULL,0,TopMsg->end_sentinel_mode->mode);
        //         }
        //         DEBUG("START end find car mode");
        //         cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_END_SENTINELMODE);
        //         //ask lcc search end
        //         if(TopMsg->end_sentinel_mode->mode == 1)
        //         {
        //             cdcs_360_search_cnt++;
        //             INFOR("cdcs 360 search cnt %d", cdcs_360_search_cnt);
        //             if(cdcs_360_search_cnt == 2)
        //             {
        //                 lcctrl_set_search_flag(LCCTRL_SEARCH_END);
        //             }
        //         }
		// 	}
		// 	break;
		// }

        // //68 restart 360 find car
        // case TBOX__NET__MESSAGETYPE__REQUEST_RESTART_SENTINELMODE:
        // {
        //     if(NULL == TopMsg->restart_sentinel_mode)
		// 	{
		// 		ERROR("TopMsg->end_sentinel_mode == NULL !!!");
		// 		return;
		// 	}
        //     else
		// 	{
		// 		DEBUG("restart find car mode request");
        //         INFOR("TopMsg->restart_sentinel_mode->timestamp %d", TopMsg->restart_sentinel_mode->timestamp);
		// 		INFOR("TopMsg->restart_sentinel_mode->mode =%d",TopMsg->restart_sentinel_mode->mode);
        //         INFOR("TopMsg->restart_sentinel_mode->resaon =%d",TopMsg->restart_sentinel_mode->resaon);
        //         cdcs_set_restart_360_flag(TopMsg->restart_sentinel_mode->resaon+1);
                
        //         DEBUG("START end find car mode");
        //         cdcs_msg_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_RESTART_SENTINELMODE);
		// 	}
		// 	break;
        // }

        // //36 update
        // case TBOX__NET__MESSAGETYPE__REQUEST_TBOXUPDATE_TASK:
        // {
        //     INFOR("recv update msg request");
        //     cdcs_msg_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOXUPDATE_TASK);
        //     break;
        // }
        // //38 reboot
        // case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_REBOOT:
        // {
        //     INFOR("recv reboot msg request");
        //     cdcs_reboot_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_TBOX_REBOOT);
        //     break;
        // }
        // //40 set battheat mode
        // case TBOX__NET__MESSAGETYPE__REQUEST_SET_BATTHEATMODE:
        // {
        //     if(NULL == TopMsg->set_batt_heat_mode)
		// 	{
		// 		ERROR("TopMsg->set_tbox_battery_mode == NULL !!!");
		// 		return;
		// 	}
        //     else
		// 	{
		// 		INFOR("set battheat mode request");
        //         INFOR("timestamp %d", TopMsg->set_batt_heat_mode->timestamp);
		// 		INFOR("hour =%d,min =%d,id =%d,mode =%d,event =%d,cycle =%d", \
        //             TopMsg->set_batt_heat_mode->hour, \
        //             TopMsg->set_batt_heat_mode->min, \
        //             TopMsg->set_batt_heat_mode->id, \
        //             TopMsg->set_batt_heat_mode->mode, \
        //             TopMsg->set_batt_heat_mode->event, \
        //             TopMsg->set_batt_heat_mode->cycle);

        //         int lcctrl_warm_status = 0;

        //         if(TopMsg->set_batt_heat_mode->mode == 0)
        //         {
        //             uint8_t cmd = 0;
        //             lcctrl_warm_status = 0;
		// 		    cfg_set_by_id(CFG_HZ_LCCTRL_BETTERY_STATUS,&lcctrl_warm_status,1);
        //             // cdcs_lcc_bat_sync();
        //             tbox_ivi_charge_batheat(PP_RMTCTRL_KEEPWARM,&cmd);
        //             // cdcs_batheat_response_send( fd, 
        //             //     TBOX__NET__MESSAGETYPE__REQUEST_SET_BATTHEATMODE, 
        //             //     TopMsg->set_batt_heat_mode->mode, 
        //             //     true);
        //             break;
        //         }
        //         else if(TopMsg->set_batt_heat_mode->mode == 1)
        //         {
        //             //do something
        //             uint8_t cmd = 1;
        //             lcctrl_warm_status = 1;
		// 		    cfg_set_by_id(CFG_HZ_LCCTRL_BETTERY_STATUS,&lcctrl_warm_status,1);
        //             // cdcs_lcc_bat_sync();
        //             tbox_ivi_charge_batheat(PP_RMTCTRL_KEEPWARM,&cmd);
        //             // cdcs_batheat_response_send( fd,TBOX__NET__MESSAGETYPE__REQUEST_SET_BATTHEATMODE,TopMsg->set_batt_heat_mode->mode,true);                    
        //             break;
        //         }
        //         else if(TopMsg->set_batt_heat_mode->mode == 2)
        //         {
        //             if(TopMsg->set_batt_heat_mode->event == 0 || TopMsg->set_batt_heat_mode->event == 1)
        //             {
        //                 cdcs_bat_decode_init();
        //                 lcctrl_warm_status = 2;
		// 	            cfg_set_by_id(CFG_HZ_LCCTRL_BETTERY_STATUS,&lcctrl_warm_status,1);
                        
        //                 //数据相同
        //                 for(i=0;i<BATTERY_APPOINT_NUM;i++)
        //                 {		
        //                     if(cdcs_bat[i].validFlg == 1)
        //                     {
        //                         if(cdcs_bat[i].id != 0)
        //                         {
                                    
        //                             if(	cdcs_bat[i].hour == TopMsg->set_batt_heat_mode->hour && \
        //                             cdcs_bat[i].min == TopMsg->set_batt_heat_mode->min && \
        //                             cdcs_bat[i].period == TopMsg->set_batt_heat_mode->cycle )
        //                             {
        //                                 INFOR("cdcs set data is same as, sync");
        //                                 // msg->battery_heat_index = index;
        //                                 // lcctrl_batinfo_to_tsp(&sm->typ, msg, &result, &sm->typ);
        //                                 // return 1;
        //                                 // cdcs_lcc_bat_sync();
        //                                 break;
        //                             }
        //                         }
        //                     }
        //                 }
        //                 uint8_t data[10] = {0};
        //                 data[0] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 24);
        //                 data[1] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 16);
        //                 data[2] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 8);
        //                 data[3] = (uint8_t)(TopMsg->set_batt_heat_mode->id );
        //                 data[4] = (uint8_t)TopMsg->set_batt_heat_mode->hour;
        //                 data[5] = (uint8_t)TopMsg->set_batt_heat_mode->min;
        //                 data[6] = (uint8_t)TopMsg->set_batt_heat_mode->cycle;
        //                 data[7] = 0;
        //                 tbox_ivi_charge_baterry_appoint(PP_RMTCTRL_KEEPWARMAPPOINT,data);
        //             }
        //             else if(TopMsg->set_batt_heat_mode->event == 2)
        //             {
        //                 cdcs_bat_decode_init();
        //                 uint8_t data[10] = {0};
        //                 data[0] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 24);
        //                 data[1] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 16);
        //                 data[2] = (uint8_t)(TopMsg->set_batt_heat_mode->id >> 8);
        //                 data[3] = (uint8_t)(TopMsg->set_batt_heat_mode->id );
        //                 data[4] = (uint8_t)TopMsg->set_batt_heat_mode->hour;
        //                 data[5] = (uint8_t)TopMsg->set_batt_heat_mode->min;
        //                 data[6] = (uint8_t)TopMsg->set_batt_heat_mode->cycle;
        //                 data[7] = 0;
        //                 tbox_ivi_charge_baterry_appoint(PP_RMTCTRL_KEEPWARMAPPOINTCANCEL,data);
        //             }
        //         }
		// 	}
        //     break;
        // }
        // //42 query batheart mode
        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTHEATMODE:
        // {
        //     INFOR("recv query batheart mode msg request");
        //     cdcs_batheat_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTHEATMODE, 1, true);
        //     break;       
        // }

        // //53 syuc battheatmode
        // case TBOX__NET__MESSAGETYPE__RESPONSE_SYNC_BATTHEATMODE_RESULT:
        // {
        //     if(NULL == TopMsg->set_battheatmode_result)
        //     {
        //         ERROR("TopMsg->set_battheatmode_result is NILL");
        //     }
        //     else
        //     {
        //         INFOR("TopMsg->set_battheatmode_result->timestamp %d",TopMsg->set_battheatmode_result->timestamp);
        //         if(TopMsg->set_battheatmode_result->result == true)
        //         {
        //             INFOR("ivi response sync batheat success......");
        //             INFOR("TopMsg->set_battheatmode_result->timestamp %d",TopMsg->set_battheatmode_result->timestamp);
        //         }
        //         else
        //         {
        //             ERROR("ivi response sync batheat fail......");
        //         }
        //     }
        //     break;
        // }

        // //49 sync batterymode
        // case TBOX__NET__MESSAGETYPE__RESPONSE_SYNC_BATTERYMODE_RESULT:
        // {
		// 	INFOR("sync bat result");

        //     if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		INFOR("ivi response sync bat result success......");
		// 	}
        //     else 
        //     {
        //         ERROR("ivi response sync bat result fail......");
        //     }
		// 	break;
        // }
        // //44 set parkuing route
        // case TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE:
        // {
        //     int route_flag = 1;
        //     if(NULL == TopMsg->parking_route)
		// 	{
		// 		ERROR("TopMsg->set_tbox_battery_mode == NULL !!!");
		// 		return;
		// 	}
        //     INFOR("TopMsg->parking_route->timestamp %d", TopMsg->parking_route->timestamp);
        //     INFOR("TopMsg->parking_route->id %d", TopMsg->parking_route->id);
        //     INFOR("TopMsg->parking_route->name %s", TopMsg->parking_route->name);
        //     INFOR("TopMsg->parking_route->event %d", TopMsg->parking_route->event);
        //     INFOR("TopMsg->parking_route->data %s", TopMsg->parking_route->data);

        //     if(TopMsg->parking_route->event == 0)
        //     {
        //         //no use
        //         for (unsigned char i = 0; i < CDCS_BATTHEAT_MAX; i++)
        //         {
        //             INFOR("cdcs_route[i].id %d", cdcs_route[i].id);
        //             if(cdcs_route[i].id == -1)
        //             {
        //                 // INFOR("no use, add route");
        //                 cdcs_route[i].timestamp = TopMsg->parking_route->timestamp;
        //                 cdcs_route[i].id = TopMsg->parking_route->id;
        //                 cdcs_route[i].event = TopMsg->parking_route->event;
        //                 strcpy(cdcs_route[i].name , TopMsg->parking_route->name);
        //                 strcpy(cdcs_route[i].data , TopMsg->parking_route->data);
        //                 cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, true);
        //                 route_flag = 0;
        //                 INFOR("cdcs_route[i].id %d", cdcs_route[i].id);
        //                 break;
        //             }
        //             else if(cdcs_route[i].id == TopMsg->parking_route->id)
        //             {
        //                 ERROR("route id aready use");
        //                 route_flag = 1;
        //                 break;
        //             }
        //         }

        //         if(route_flag)
        //         {
        //             // all use
        //             ERROR("all use, route fail");
        //             cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, false);
        //         }
        //         break;

        //     }
        //     else if(TopMsg->parking_route->event == 1)
        //     {
        //         //have
        //         for (unsigned char i = 0; i < CDCS_BATTHEAT_MAX; i++)
        //         {
        //             if(cdcs_route[i].id == TopMsg->parking_route->id)
        //             {
        //                 INFOR("have id, change");
        //                 cdcs_route[i].timestamp = TopMsg->parking_route->timestamp;
        //                 cdcs_route[i].id = TopMsg->parking_route->id;
        //                 cdcs_route[i].event = TopMsg->parking_route->event;
        //                 strcpy(cdcs_route[i].name , TopMsg->parking_route->name);
        //                 strcpy(cdcs_route[i].data , TopMsg->parking_route->data);
        //                 cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, true);
        //                 route_flag = 0;
        //                 break;
        //             }
        //         }
        //         if(route_flag)
        //         {
        //             // all use
        //             ERROR("no id, change");
        //             cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, false);
        //         }
        //         break;
        //     }
        //     else if(TopMsg->parking_route->event == 2)
        //     {//delete
        //         //delete
        //         for (unsigned char i = 0; i < CDCS_BATTHEAT_MAX; i++)
        //         {
        //             if(cdcs_route[i].id == TopMsg->parking_route->id)
        //             {
        //                 INFOR("have id, change");
        //                 cdcs_route[i].timestamp = -1;
        //                 cdcs_route[i].id = -1;
        //                 cdcs_route[i].event = -1;
        //                 strcpy(cdcs_route[i].name , "-1");
        //                 strcpy(cdcs_route[i].data , "-1");
        //                 cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, true);
        //                 return;
        //             }
        //         }
        //         // all use
        //         ERROR("no id, change");
        //         cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, false);
        //         break;
        //     }
        //     else
        //     {
        //         ERROR("TopMsg->parking_route->event %d is unknow", TopMsg->parking_route->event);
        //         cdcs_route_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE, false);
        //         break;
        //     }

        //     // cdcs_msg_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_PARKING_ROUTE); 
        //     // break;
        // }

        // //50 query route mode
        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_PARKINGROUTE:
        // {
        //     INFOR("recv query route mode msg request");
        //     cdcs_route_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_QUERY_PARKINGROUTE, true);
        //     break;       
        // }
        // //47 sync route
        // case TBOX__NET__MESSAGETYPE__RESPONSE_SYNC_PARKINGROUTE_RESULT:
        // {
		// 	DEBUG("ivi response call state");

        //     if(NULL == TopMsg->msg_result)
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true)
		// 	{
		// 		DEBUG("ivi response sync route success......");
		// 	}
        //     else ERROR("ivi response sync route fail......");
			
		// 	break;
        // }
        // //cdcs set guard
        // case TBOX__NET__MESSAGETYPE__REQUEST_SET_SECURITY_GUARD: 
        // {
        //     INFOR("recv guard request");
        //     uint8_t data[64] = {0};
        //     unsigned int len = 0;
        //     len = sizeof(data);
        //     int timestamp = 0;
        //     int valetmode = 0;
        //     int stealthmode = 0;
        //     if(NULL == TopMsg->security_guard) 
        //     {
        //         ERROR("TopMsg->security_guard IS NULL");
        //         return;
        //     }
        //     INFOR("TopMsg->security_guard->timestamp %d", TopMsg->security_guard->timestamp);
        //     INFOR("TopMsg->security_guard->valetmode %d", TopMsg->security_guard->valetmode);
        //     INFOR("TopMsg->security_guard->stealthmode %d", TopMsg->security_guard->stealthmode);
        //     //param check
        //     if(TopMsg->security_guard->valetmode < 0 || TopMsg->security_guard->valetmode > 3)
        //     {
        //         ERROR("param TopMsg->security_guard->valetmode [%d] error", TopMsg->security_guard->valetmode);
        //         return;
        //     }
        //     else if(TopMsg->security_guard->stealthmode < 0 || TopMsg->security_guard->stealthmode > 2)
        //     {
        //         ERROR("param TopMsg->security_guard->stealthmode [%d] error", TopMsg->security_guard->stealthmode);
        //         return;
        //     }

        //     cfg_get_by_id(CFG_HZ_LCCTRL_GUARD, data, &len);
        //     sscanf((const char *)data, GUARD_FORMAT, &timestamp, &valetmode, &stealthmode);
        //     INFOR("old valetmode [%d], stealthmode [%d]", valetmode, stealthmode);
        //     memset(data, 0, len);

        //     if(0 != TopMsg->security_guard->valetmode)
        //     {
        //         valetmode = TopMsg->security_guard->valetmode;
        //     }

        //     if(0 != TopMsg->security_guard->stealthmode)
        //     {
        //         stealthmode = TopMsg->security_guard->stealthmode;
        //     }
        //     sprintf((char*)data, GUARD_FORMAT, TopMsg->security_guard->timestamp, valetmode, stealthmode);
        //     INFOR("new valetmode [%d], stealthmode [%d]", valetmode, stealthmode);
        //     cfg_set_by_id(CFG_HZ_LCCTRL_GUARD, data, 64);
            
        //     if(1 == TopMsg->security_guard->valetmode)
        //     {
        //         INFOR("open valet");
        //         PP_rmtCfg_15_tbox_to_tsp(0x30, 1);
        //     }
        //     else if(2 == TopMsg->security_guard->valetmode)
        //     {
        //         INFOR("close valet");
        //         PP_rmtCfg_15_tbox_to_tsp(0x30, 0);
        //     }
        //     else if(3 == TopMsg->security_guard->valetmode)
        //     {
        //         INFOR("valet not init");
        //         PP_rmtCfg_15_tbox_to_tsp(0x30, 2);
        //     }

        //     if(2 == TopMsg->security_guard->stealthmode)
        //     {
        //         INFOR("open stealth");
        //         PP_rmtCfg_15_tbox_to_tsp(0x52, 1);
        //     }
        //     else if(1 == TopMsg->security_guard->stealthmode)
        //     {
        //         INFOR("close stealth");
        //         PP_rmtCfg_15_tbox_to_tsp(0x52, 0); 
        //     }
        //     cdcs_guard_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_SECURITY_GUARD, NULL, 0);
        //     break;
        // }
        // //query guard
        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_SECURITY_GUARD: 
		// {
        //     INFOR("query guard");
        //     cdcs_guard_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_SECURITY_GUARD, NULL, 0);
        //     break;
        // }

        // //sync guard
        // case TBOX__NET__MESSAGETYPE__RESPONSE_SYNC_SECURITY_GUARD_RESULT: 
        // {
		// 	INFOR("sync guard result");
        //     if(NULL == TopMsg->msg_result) 
        //     {
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}

		// 	if(TopMsg->msg_result->result == true) 
        //     {
		// 		INFOR("ivi response sync guard result success......");
		// 	}
        //     else 
        //     {
        //         ERROR("ivi response sync guard result fail......");
        //     }
		// 	break;
        // }
        // //set discharge
        // case TBOX__NET__MESSAGETYPE__REQUEST_SET_DISCHARGE: 
        // {
        //     unsigned int len = 0;
        //     int ret = 0;
        //     int timestamp = 0;
        //     int cmd = 0;
        //     int targetpower = 0;
        //     INFOR("cdcs set discharge");
        //     uint8_t data[64] = {0};
        //     uint8_t soc = 0;
        //     if(NULL == TopMsg->discharge_state) 
        //     {
        //         ERROR("TopMsg->discharge_state IS NULL");
        //     }
        //     len = sizeof(data);
        //     cfg_get_by_id(CFG_HZ_LCCTRL_DISCHARGE, data, &len);
        //     ret = sscanf((const char *)data, DISCHARGE_FORMAT, &timestamp, &cmd, &targetpower);
        //     if(  ret == 3 )
        //     {
        //         INFOR("discharge mode(%d),targetpower(%d)",cmd, targetpower);
        //     }
        //     else
        //     {
        //         INFOR("ret %d", ret);
        //     }
        //     INFOR("timestamp = %d, command = %d, targetpower = %d", \
        //         TopMsg->discharge_state->timestamp, TopMsg->discharge_state->command, TopMsg->discharge_state->targetpower);
        //     // sprintf((char*)data, DISCHARGE_FORMAT, TopMsg->discharge_state->timestamp, TopMsg->discharge_state->command, TopMsg->discharge_state->targetpower);
		// 	// cfg_set_by_id(CFG_HZ_LCCTRL_DISCHARGE, data, 64);
        //     soc = TopMsg->discharge_state->targetpower;
        //     if(0 == TopMsg->discharge_state->command) 
        //     {
        //         soc = 0;
        //         cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_DISCHARGE, NULL, 0);
        //         tbox_ivi_discharge_ctrl(PP_RMTCTRL_STOPOUTCHARGET,&soc);

        //     }
        //     else if(1 == TopMsg->discharge_state->command) 
        //     {
        //         //set
        //         if(cmd)
        //         {
        //             cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_DISCHARGE, NULL, 0);
        //             tbox_ivi_discharge_ctrl(PP_RMTCTRL_SETDISCHARGESOC,&soc);
        //         }
        //         //start
        //         else
        //         {
        //             cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_DISCHARGE, NULL, 0);
        //             tbox_ivi_discharge_ctrl(PP_RMTCTRL_STARTOUTCHARGET,&soc);
        //         }

        //     }
        //     else 
        //     {
        //         ERROR("unkown TopMsg->discharge_state->command %d", TopMsg->discharge_state->command);
        //         cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_DISCHARGE, NULL, 1);
                
        //     }
        //     break;
        // }

        // // query
        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_DISCHARGE: 
        // {
        //     INFOR("query discharge");
        //     cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_DISCHARGE, NULL, 0);
        //     break;
        // }

        // case TBOX__NET__MESSAGETYPE__REQUEST_SYNC_DISCHARGE_RESULT: 
        // {
		// 	INFOR("sync guard result");
        //     if(NULL == TopMsg->msg_result) 
        //     {
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true) 
        //     {
		// 		INFOR("ivi response sync guard result success......");
		// 	}
        //     else 
		// 	{
		// 		ERROR("ivi response sync guard result fail......");
		// 	}
			
		// 	break;
        // }

        // //sync guard result
        // case TBOX__NET__MESSAGETYPE__RESPONSE_SYNC_NETWORK_STATUS_RESULT:
		//  {
		// 	INFOR("sync guard result");
        //     if(NULL == TopMsg->msg_result) 
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true) 
		// 	{
		// 		INFOR("ivi response sync net result success......");
		// 	}
        //     else 
		// 	{
		// 		ERROR("ivi response sync net result fail......");
		// 	}
			
		// 	break;
        // }

        // //set signaltype
        // case TBOX__NET__MESSAGETYPE__REQUEST_SET_SIGNALTYPE:
        // {
        //     if(NULL == TopMsg->tbox_signal_type)
        //     {
        //         ERROR("no send TopMsg->tbox_signal_type");
        //         cdcs_signal_handle(fd, TBOX__NET__MESSAGETYPE__REQUEST_SET_SIGNALTYPE, NULL, 1);
        //     }
        //     else
        //     {
        //         if(CDCS_SIGNAL_TYPE_LTE == TopMsg->tbox_signal_type->signaltype)
        //         {
        //             INFOR("net mode lock 4G mode");
        //             cdcs_tbox_net_type_set_respone(fd,CDCS_SIGNAL_TYPE_LTE);
        //         }
        //         else
        //         {
        //             INFOR("net mode lock 4/5G mode");
        //             cdcs_tbox_net_type_set_respone(fd,CDCS_SIGNAL_TYPE_NR5G_LTE);
        //         } 
        //     }
        //     break; 
        // }

        // //query
        // case TBOX__NET__MESSAGETYPE__REQUEST_QUERY_SIGNALTYPE:
        // {
        //     cdcs_signal_handle(fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_SIGNALTYPE, NULL, 0);
        //     break;
        // }

        // //sync
        // case TBOX__NET__MESSAGETYPE__REQUEST_SYNC_SIGNALTYPE_RESULT:
		//  {
		// 	INFOR("sync net result");
        //     if(NULL == TopMsg->msg_result) 
		// 	{
		// 		ERROR("TopMsg->msg_result == NULL !!!");
		// 		return;
		// 	}
			
		// 	if(TopMsg->msg_result->result == true) 
		// 	{
		// 		INFOR("ivi response sync net result success......");
		// 	}
        //     else 
		// 	{
		// 		ERROR("ivi response sync net result fail......");
		// 	}
			
		// 	break;
        // }

        default:
        {
            my_zlog_error_cdcs("recv ivi unknown message type!!!");
            break;
        }
    }
    
}

/* [] END OF FILE */