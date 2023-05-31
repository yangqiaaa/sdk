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

        //2 tbox send heart result
        case TBOX__NET__MESSAGETYPE__RESPONSE_HEARTBEAT_RESULT:
        //4 tbox send signal type and power
        case TBOX__NET__MESSAGETYPE__RESPONSE_NETWORK_SIGNAL_STRENGTH:
        {
            cdcs_sm_callback(&fd, TopMsg);
            break;
        }

        default:
        {
            my_zlog_error_cdcs("recv unknown message type(%d), no handle", TopMsg->message_type);
            break;
        }
    }
    
}

/* [] END OF FILE */