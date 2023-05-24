#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <math.h>
#include <dirent.h>
#include <unistd.h>

#include <signal.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <pthread.h>

#include "cdcs_api.h"
#include "cdcs_pb.h"

#include <errno.h>
#include <sys/prctl.h>


//poll cycle
#define CDCS_POLL_INTERVAL 100

//reconnect time
#define CDCS_SOCKET_RETRY     (5000)
//heart max time
#define CDCS_HEART_MAX_TIME   (3000)
#define TIMEOUT (3)


//Connection default state timestamp
cdcs_link_status_t cdcs_link_status[CDCS_CLIENT_NUM_MAX];





cdcs_state_t cdcs_state;



int cdcs_get_3call_tcp_st(void)
{

    for(unsigned char i = 0; i < CDCS_CLIENT_NUM_MAX; i++)
    {
        if((cdcs_clients[i].fd > 0) && (cdcs_clients[i].type == TBOX__NET__HEART__TYPE__HEART_3CALL))
        {
            return 1;
        }
    }

    return 0;
}

int cdcs_get_nmea_tcp_st(void)
{

    for(unsigned char i = 0; i < CDCS_CLIENT_NUM_MAX; i++)
    {
        if((cdcs_clients[i].fd > 0) && (cdcs_clients[i].type == TBOX__NET__HEART__TYPE__HEART_NMEA))
        {
            return 1;
        }
    }

    return 0;
}



int pb_bytes_set(ProtobufCBinaryData * des, uint8_t *buf, int len)
{
    if( len > 0 )
    {
        memcpy(des->data,buf,len);
        des->len = len;

        return 0;
    }
    return -1;
}


//init link
void cdcs_socket_link_init(unsigned char ch)
{
	signal_power = 0;   
	signal_type = 0;
	active_flag = 1;	
	appointment_sync = 1;
    call_flag = 5;

}




typedef struct cdcs_msg_t
{
    int used;
    int size;
    unsigned char *data;
} MSG_RX;

/**
 * @brief Checks whether the car machina message is decoded 
 * and calls the callback function if it is decoded
 * @param MSG_RX *rx
 * @param ivi_msg_handler ivi_msg_proc
 * @param void *para
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
void cdcs_msg_decodex(MSG_RX *rx, ivi_msg_handler ivi_msg_proc, void *para)
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

#if 0
                if (len <= 0 || 0 != ivi_msg_check(rx->data + start_pos, &len))
                {
                    start_pos = end_pos;
                    end_pos = -1;
                }
                else
#endif                    
                {
                    ivi_msg_proc(rx->data , len, para);
                    start_pos = -1;//
                    end_pos = -1;//
                }
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
 * @brief show send msg result success or fail
 * @file cdcs.c 
 * @name 
 * @param[in] int len // send msg len
 * @param[in] size_t szlen //ToMsg len
 * @param[in] int loglevel // 0 DEBUG,other INFOR
 * @return void 
 * @note 
 * @date 2022-02-23 10:19:31 
 * @version V1.0.0 
*/
void cdcs_msg_send_result_show(int len, size_t szlen, int loglevel)
{
    if (len < (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + szlen))
    {
        ERROR("ivi msg send response failed!!!");
    }
    else
    {
        if(loglevel <= LOG_LEVEL_INFOR) 
        {
            INFOR("ivi msg send response success");
        }
        else 
        {
            DEBUG("ivi msg send response success");
        }
    }
}





void cdcs_msg_error_response_send( int fd ,Tbox__Net__Messagetype id,char *error_code)
{
    size_t szlen = 0;
    unsigned char pro_buf[2048] = {0};
	
    if( fd < 0 )
    {
        ERROR("ivi_msg_error_response_send fd = %d.",fd);
        return ;
    }

    Tbox__Net__TopMessage TopMsg ;
    Tbox__Net__MsgResult result;

    tbox__net__top_message__init( &TopMsg );
    tbox__net__msg_result__init( &result );
	TopMsg.has_message_type = PARAM_EXISTS;
    TopMsg.message_type = id;
 	result.has_result = PARAM_EXISTS;
    result.result = false;
    pb_bytes_set( &result.error_code, (uint8_t *)error_code, strlen(error_code));
    
    TopMsg.msg_result = &result;
    
    if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
    {
        return ;
    }

    cdcs_net_top_message_send(fd,pro_buf,szlen,LOG_LEVEL_INFOR);
    return;
}

void cdcs_360mode_start_send(cdcs_360mode_t *param)
{
    size_t szlen = 0;
    unsigned char pro_buf[2048] = {0};
    int fd = cdcs_get_3call_fd();
    Tbox__Net__Messagetype id = TBOX__NET__MESSAGETYPE__REQUEST_START_SENTINELMODE;
	

    if( fd < 0 )
   	{
        ERROR("ivi_msg_response_send fd = %d.",fd);
        return ;
    }

    Tbox__Net__TopMessage TopMsg;
    tbox__net__top_message__init( &TopMsg ); 

	switch( id )
	{
        //360 find car
        case TBOX__NET__MESSAGETYPE__REQUEST_START_SENTINELMODE:
        {
			unsigned int len = 0;
            char vin[18] = {0};
            Tbox__Net__StartSentinelMode startsentinemode;
			tbox__net__start_sentinel_mode__init(&startsentinemode);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_START_SENTINELMODE;

            startsentinemode.timestamp = tbox_ivi_getTimestamp();
            startsentinemode.mode = param->mode;

            startsentinemode.type = param->type;
            len = sizeof(vin);
            cfg_get_by_id(CFG_TBOX_VIN, vin, &len);
            startsentinemode.vin = vin;
            startsentinemode.eventid = param->eventid;
            startsentinemode.has_timestamp = PARAM_EXISTS;
            startsentinemode.has_mode = PARAM_EXISTS;
            startsentinemode.has_type = PARAM_EXISTS;
            startsentinemode.has_eventid = PARAM_EXISTS;
            startsentinemode.url = param->url;
            startsentinemode.filename = param->filename;

			INFOR(" startsentinemode.timestamp = %d",startsentinemode.timestamp);
			INFOR(" startsentinemode.mode  = %d", startsentinemode.mode );
            INFOR(" startsentinemode.type = %d",startsentinemode.type);
			INFOR(" startsentinemode.vin  = %s", vin );
            INFOR(" startsentinemode.eventid = %d",startsentinemode.eventid);
            INFOR(" startsentinemode.url = %s",startsentinemode.url);
            INFOR("startsentinemode.filename = %s",startsentinemode.filename);

            TopMsg.start_sentinel_mode = &startsentinemode;
            INFOR("TopMsg.start_sentinel_mode.->mode:%d", TopMsg.start_sentinel_mode->mode);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }

        }
		break;

		default:
            return;
		break;
	}
    
    cdcs_net_top_message_send(fd,pro_buf,szlen,LOG_LEVEL_INFOR);
    INFOR("BUF LEN = %d", (IVI_PKG_S_MARKER_SIZE + IVI_PKG_E_MARKER_SIZE + IVI_PKG_MSG_LEN + szlen));
	
}

/** 
 * @brief 
 * @file cdcs.c 
 * @name 
 * @param[in] void 
 * @return 0 is ok, other is fail 
 * @note 
 * return 0 is ok
 * return -1 3call heart no cennect
 * return -2 param error, check param
 * @date 2022-08-22 11:15:58 
 * @version V1.0.0 
*/
int cdcs_net_state_sync_send(cdcs_net_state_t *param) 
{
    size_t szlen = 0;
    unsigned char pro_buf[2048] = {0};


    int fd = cdcs_get_3call_fd();
    if(fd < 0) {
        ERROR("no 3call heart [%d] no connect, exit", fd);
        return -1;
    }
    Tbox__Net__Messagetype id = TBOX__NET__MESSAGETYPE__REQUEST_SYNC_NETWORK_STATUS;

    Tbox__Net__TopMessage TopMsg;
    tbox__net__top_message__init( &TopMsg ); 

	switch( id )
	{
        //net state sync
        case TBOX__NET__MESSAGETYPE__REQUEST_SYNC_NETWORK_STATUS:
        {
            Tbox__Net__NetworkStatus net_state;
			tbox__net__network_status__init(&net_state);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_SYNC_NETWORK_STATUS;

            if(net_state.channel != 0 && net_state.channel != 1) {
                ERROR("param channel %d error ", net_state.channel);
                return -2;
            }

            if(net_state.networksts != 0 && net_state.networksts != 1) {
                ERROR("param networksts %d error ", net_state.networksts);
                return -2;
            }
            // net_state.timestamp = tbox_ivi_getTimestamp();
            net_state.timestamp = param->timestamp;
            net_state.channel = param->channel;
            net_state.networksts = param->networksts;

            net_state.has_timestamp = PARAM_EXISTS;
            net_state.has_channel = PARAM_EXISTS;
            net_state.has_networksts = PARAM_EXISTS;

            TopMsg.network_status = &net_state;
            INFOR("TopMsg.network_status->timestamp = %d", TopMsg.network_status->timestamp);
            INFOR("TopMsg.network_status->channel = %d", TopMsg.network_status->channel);
            INFOR("TopMsg.network_status->networksts = %d", TopMsg.network_status->networksts);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return 0 ;
            }
        }
		break;

		default:
            return 0;
		break;
	}
    
    cdcs_net_top_message_send(fd,pro_buf,szlen,LOG_LEVEL_INFOR);

    return 0;
}


void cdcs_message_request(int fd ,Tbox__Net__Messagetype id,void *para)
{
    size_t szlen = 0;
	static int fotapush_cnt = 0;
    unsigned char pro_buf[2048] = {0};
	

    if( fd < 0 )
   	{
        ERROR("ivi_msg_response_send fd = %d.",fd);
        return ;
    }

    Tbox__Net__TopMessage TopMsg;
    tbox__net__top_message__init( &TopMsg ); 

	switch( id )
	{
		//	fota
		case TBOX__NET__MESSAGETYPE__REQUEST_OTAUPDATE_TASK:
		{	
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_OTAUPDATE_TASK;
			DEBUG("fota push success.....");
			fotapush_cnt++;
		
			if(fotapush_cnt > 3)
			{
				fotapush_cnt = 0;
				otaupdate_flag = 0;//

				//PP_FIP_InfoPush_cb(IVI_FOTA_PUSH_FAIL_TIMEOUT);
				ERROR("OTA PUSH HU responed timeout");
			}
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //logfile
		case TBOX__NET__MESSAGETYPE__REQUEST_IHU_LOGFILE:
		{	//
			Tbox__Net__IhuLogfile logfile;
			tbox__net__ihu_logfile__init(&logfile);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_IHU_LOGFILE;
			logfile.has_aid = PARAM_EXISTS;
			logfile.has_mid = PARAM_EXISTS;
			logfile.has_starttime = PARAM_EXISTS;
			logfile.durationtime = PARAM_EXISTS;
			logfile.has_timestamp = PARAM_EXISTS;
			logfile.has_channel = PARAM_EXISTS;
			logfile.has_level = PARAM_EXISTS;
			logfile.aid = tsplogfile.aid;
			logfile.mid = tsplogfile.mid;
			logfile.eventid = tsplogfile.eventid;
			logfile.starttime = tsplogfile.starttime;
			logfile.durationtime = tsplogfile.durationtime;
			logfile.timestamp = tsplogfile.timestamp;
			logfile.channel = tsplogfile.channel;
			logfile.level = tsplogfile.level;
			TopMsg.ihu_logfile = &logfile;
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //tbox activestate
		case TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_ACTIVESTATE_RESULT:
		{	//
			Tbox__Net__TboxActiveState state;
			tbox__net__tbox_active_state__init( &state );
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_ACTIVESTATE_RESULT;
            if(PP_rmtCfg_enable_actived() == 6 )
            {
            	state.active_state = 1;
            }
            else
            {
            	state.active_state = 0;
            }
			state.has_active_state = PARAM_EXISTS;
			INFOR("state:%d",state.active_state);
			TopMsg.tbox_activestate = &state;
            INFOR("TopMsg.tbox_activestate state %d",TopMsg.tbox_activestate->active_state);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //tbox info
		case TBOX__NET__MESSAGETYPE__REQUEST_TBOX_INFO:
		{   //
			char vin[18] = {0};
			char iccid[21] = {0};
			char imei[16] = {0};
			char tboxsn[19] = {0};
            unsigned int len = 0;
            char hw[32] = {0};
            len = sizeof(hw);
			Tbox__Net__TboxInfo tboxinfo;
			tbox__net__tbox_info__init(&tboxinfo);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_INFO;
			tboxinfo.software_version = DID_F1B0_SW_UPGRADE_VER;
			len = sizeof(hw);
            cfg_get_by_id(CFG_TBOX_HW, hw, &len);

    		if(hw[0] == 0)
    		{
                memcpy(hw,"00.00",5);
    		}
			tboxinfo.hardware_version = hw;
			if(cdcs_get_iccid(iccid) == 0)
			{
				tboxinfo.iccid = iccid;	
			}
			else
			{
				tboxinfo.iccid  = "00000000000000000000";
			}
			cdcs_get_imei(imei);
			tboxinfo.imei = imei;
            len = sizeof(vin);
            cfg_get_by_id(CFG_TBOX_VIN, vin, &len);
			tboxinfo.vin = vin ;
            len = sizeof(tboxsn) - 1;
            cfg_get_by_id(CFG_TBOX_SN, tboxsn, &len);
			tboxinfo.pdid = tboxsn;
			TopMsg.tbox_info = &tboxinfo;
            INFOR("tboxinfo......");
            INFOR("vin:%s",vin);
            INFOR("iccid:%s",iccid);
            INFOR("imei:%s",imei);
            INFOR("tboxsn:%s",tboxsn);
            INFOR("sw:%s",DID_F1B0_SW_UPGRADE_VER);
            INFOR("hw:%s",hw);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //IHU charge appointment state
		case TBOX__NET__MESSAGETYPE__REQUEST_IHU_CHARGEAPPOINTMENTSTS:
		{
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_IHU_CHARGEAPPOINTMENTSTS;
            appointment_sync = 1;
            Tbox__Net__IhuChargeAppoointmentSts chager;
            tbox__net__ihu_charge_appoointment_sts__init(&chager);
			lcctrl_Charge_AppointBook_t appoint_time;
			lcctrl_get_charge_appoint(&appoint_time);
            INFOR("IHU charge appointment state");

			chager.timestamp        = cdcs_get_timestamp();
			chager.id               = appoint_time.id;
			chager.hour             = appoint_time.hour;
			chager.min              = appoint_time.min;
			chager.targetpower      = appoint_time.targetSOC;
			chager.effectivestate   = appoint_time.validFlg;
            chager.longtime         = appoint_time.timelong;
            chager.iscontinuecharge = appoint_time.iscontinuecharge;
			
            chager.has_timestamp        = PARAM_EXISTS;
            chager.has_hour             = PARAM_EXISTS;
            chager.has_min              = PARAM_EXISTS;
            chager.has_id               = PARAM_EXISTS;
            chager.has_targetpower      = PARAM_EXISTS;
            chager.has_effectivestate   = PARAM_EXISTS;
            chager.has_longtime         = PARAM_EXISTS;
            chager.has_iscontinuecharge = PARAM_EXISTS;

            TopMsg.ihu_charge_appoointmentsts = &chager;
            INFOR("chager.timestamp = %d",          TopMsg.ihu_charge_appoointmentsts->timestamp);
            INFOR("chager.id = %d",                 TopMsg.ihu_charge_appoointmentsts->id);
            INFOR("chager.hour = %d",               TopMsg.ihu_charge_appoointmentsts->hour);
            INFOR("chager.min = %d",                TopMsg.ihu_charge_appoointmentsts->min);
            INFOR("chager.targetpower = %d",        TopMsg.ihu_charge_appoointmentsts->targetpower);
            INFOR("chager.effectivestate = %d",     TopMsg.ihu_charge_appoointmentsts->effectivestate);
            INFOR("chager.longtime = %d",           TopMsg.ihu_charge_appoointmentsts->longtime);
            INFOR("chager.iscontinuecharge = %d",   TopMsg.ihu_charge_appoointmentsts->iscontinuecharge);

            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;


        //charge record
		case TBOX__NET__MESSAGETYPE__RESPONSE_CHARGE_RECORD_RESULT:
		{
			Tbox__Net__TboxChargeRecord chager;
			tbox__net__tbox_charge_record__init(&chager);
			lcctrl_Charge_AppointBook_t appoint_time;
			lcctrl_get_charge_appoint(&appoint_time);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__RESPONSE_CHARGE_RECORD_RESULT;

			chager.timestamp        = cdcs_get_timestamp();
			chager.id               = appoint_time.id;
			chager.hour             = appoint_time.hour;
			chager.min              = appoint_time.min;
			chager.targetpower      = appoint_time.targetSOC;
			chager.effectivestate   = appoint_time.validFlg;;
            chager.longtime         = appoint_time.timelong;
            chager.iscontinuecharge = appoint_time.iscontinuecharge;
			
            chager.has_timestamp        = PARAM_EXISTS;
            chager.has_hour             = PARAM_EXISTS;
            chager.has_min              = PARAM_EXISTS;
            chager.has_id               = PARAM_EXISTS;
            chager.has_targetpower      = PARAM_EXISTS;
            chager.has_effectivestate   = PARAM_EXISTS;
            chager.has_longtime         = PARAM_EXISTS;
            chager.has_iscontinuecharge = PARAM_EXISTS;
			TopMsg.tbox_charge_record   = &chager;

            INFOR("chager.timestamp = %d",          TopMsg.tbox_charge_record->timestamp);
            INFOR("chager.id = %d",                 TopMsg.tbox_charge_record->id);
            INFOR("chager.hour = %d",               TopMsg.tbox_charge_record->hour);
            INFOR("chager.min = %d",                TopMsg.tbox_charge_record->min);
            INFOR("chager.targetpower = %d",        TopMsg.tbox_charge_record->targetpower);
            INFOR("chager.effectivestate = %d",     TopMsg.tbox_charge_record->effectivestate);
            INFOR("chager.longtime = %d",           TopMsg.tbox_charge_record->longtime);
            INFOR("chager.iscontinuecharge = %d",   TopMsg.tbox_charge_record->iscontinuecharge);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //battery mode
		case TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_BATTERYMODE_RESULT:
		{
			uint32_t result  = *((uint32_t *)para);
			Tbox__Net__TboxBatteryModeResult battery;
			tbox__net__tbox_battery_mode_result__init(&battery);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__RESPONSE_TBOX_BATTERYMODE_RESULT;
			battery.timestamp = tbox_ivi_getTimestamp();
			battery.has_result = PARAM_EXISTS;
			battery.result = result;
			TopMsg.tbox_battery_mode_result = &battery;
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
		}
		break;

        //360 find car
        case TBOX__NET__MESSAGETYPE__REQUEST_START_SENTINELMODE:
        {
			unsigned int len = 0;
            char vin[18] = {0};
            Tbox__Net__StartSentinelMode startsentinemode;
			tbox__net__start_sentinel_mode__init(&startsentinemode);
			TopMsg.message_type = TBOX__NET__MESSAGETYPE__REQUEST_START_SENTINELMODE;

            startsentinemode.timestamp = tbox_ivi_getTimestamp();
            //sentrymode.mode = tbox_ivi_get_sentry_mode_swtich();
            startsentinemode.mode = 1;
            startsentinemode.type = 1;
            len = sizeof(vin);
            cfg_get_by_id(CFG_TBOX_VIN, vin, &len);
            startsentinemode.vin = vin;
            startsentinemode.eventid = 1;
            startsentinemode.has_timestamp = PARAM_EXISTS;
            startsentinemode.has_mode = PARAM_EXISTS;
            startsentinemode.has_type = PARAM_EXISTS;
            startsentinemode.has_eventid = PARAM_EXISTS;
            char url[128] = "ftp://172.16.1.50/usrdata/current/data/image/cdcs.jpg";
            startsentinemode.url = url;

			INFOR(" startsentinemode.timestamp = %d",startsentinemode.timestamp);
			INFOR(" startsentinemode.mode  = %d", startsentinemode.mode );
            INFOR(" startsentinemode.type = %d",startsentinemode.type);
			INFOR(" startsentinemode.vin  = %s", startsentinemode.vin );
            INFOR(" startsentinemode.eventid = %d",startsentinemode.eventid);
            INFOR(" startsentinemode.url = %s",startsentinemode.url);

            TopMsg.start_sentinel_mode = &startsentinemode;
            INFOR("TopMsg.start_sentinel_mode.->mode:%d", TopMsg.start_sentinel_mode->mode);
            TopMsg.has_message_type = PARAM_EXISTS;
            if(cdcs_net_top_message_pack(&TopMsg,pro_buf,sizeof(pro_buf),&szlen))
            {
                return ;
            }
        }
		break;

		default:
            return;
		break;
	}
    
    cdcs_net_top_message_send(fd,pro_buf,szlen,LOG_LEVEL_INFOR);
}







void cdcs_heart_type_decode(int fd, int heart_type) {
    int i = 0;
    int ret = 0;
    //heartbeat type 3call or nmea
    switch ( heart_type ) 
    {
        //3call
        case TBOX__NET__HEART__TYPE__HEART_3CALL: 
        {
            DEBUG("heartbeat 3call...");
            ret = cdcs_get_client_type(TBOX__NET__HEART__TYPE__HEART_3CALL);
            if(ret < 0) 
            {
                for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++) 
                {
                    if(cdcs_clients[i].fd == fd) 
                    {
                        INFOR("set fd[%d] 3call heart", cdcs_clients[i].fd);
                        cdcs_clients[i].type =TBOX__NET__HEART__TYPE__HEART_3CALL;
                        cdcs_clients[i].lasthearttime = time_msec();
                        cdcs_state.connect_3call_state = CDCS_CONNECT_FIRST;
                    }
                }
            }
            else 
            {
                for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++) 
                {
                    if(cdcs_clients[i].fd == fd) 
                    {
                        if(i != ret) 
                        {
                            close(fd);
                            cdcs_clients[i].fd = -1;
                            cdcs_clients[i].type = -1;
                            ERROR("3call client type duplicate,close");
                            break;
                        }
                        else 
                        {
                            DEBUG("3call last heart");
                            cdcs_clients[i].lasthearttime = time_msec();
                        }        
                    }
                }   
            }
            cdcs_heart_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_HEARTBEAT_SIGNAL);
            break;
        }
        //nmea
        case TBOX__NET__HEART__TYPE__HEART_NMEA: 
        {
            DEBUG("heartbeat nmea...");
            ret = cdcs_get_client_type(TBOX__NET__HEART__TYPE__HEART_NMEA);
            if(ret < 0) 
            {
                for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++) 
                {
                    if(cdcs_clients[i].fd == fd) 
                    {
                        INFOR("set fd [%d] nmea heart", cdcs_clients[i].fd);
                        cdcs_clients[i].type =TBOX__NET__HEART__TYPE__HEART_NMEA;
                        cdcs_clients[i].lasthearttime = time_msec();
                    }
                }
            }
            else 
            {
                for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++) 
                {
                    if(cdcs_clients[i].fd == fd) 
                    {
                        if(i != ret) 
                        {
                            close(fd);
                            cdcs_clients[i].fd = -1;
                            cdcs_clients[i].type = -1;
                            ERROR("nmea client type duplicate,close");
                            break;
                        }
                        else 
                        {
                            DEBUG("nmea last heart");
                            cdcs_clients[i].lasthearttime = time_msec();       
                        }        
                    }
                }   
            }       
            cdcs_heart_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_HEARTBEAT_SIGNAL);
            break;
        }
        //other
        default: 
        {
            ERROR("unkonw heart onoff type[%d]!!!", heart_type);
            break;
        }
    }
}





//1s check appointment_sync & ota update
void cdcs_msg_request_timer_1s(void)
{
    int fd = cdcs_get_3call_fd();
    static int count = 0;
    if(fd < 0)
    {
        count++;
        if(count >= 10)
        {
            ERROR("no 3call fd,exit");
            count = 0;
        }

        return;
    }
    cdcs_signal_type_sync(fd);
    cdcs_netmodecfg_type_sync(fd);
    cdcs_stealth_mode_first_sync_to_tsp();


    if(cdcs_vin_change_flag)
    {
        INFOR("vin charge,sync cdcs");
        cdcs_vin_change_flag = 0;
        cdcs_message_request(fd ,TBOX__NET__MESSAGETYPE__REQUEST_TBOX_INFO, NULL);
    }
}


/**
 * @brief  ivi message decode process
 * call cdcs_msg_request_process
 * @param unsigned char *data
 * @param unsigned int datalen
 * @param void *para
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
void cdcs_tcp_protobuf_process(unsigned char *data, unsigned int datalen, void *para)
{
    int fd = *(int *)para;
    cdcs_msg_request_process( data, datalen ,fd);
    return;
}





/**
 * @brief heartbeat time check,timeout close client
 * @param unsigned time_ms
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
static void cdcs_check_client_connect_st(unsigned time_ms)
{
    /* check client heart time out */
    for(unsigned char i = 0; i < CDCS_CLIENT_NUM_MAX; i++)
    {
        //heart timout judge
        if ((-1 != cdcs_clients[i].fd) \
            && (time_ms - cdcs_clients[i].lasthearttime > CDCS_HEART_MAX_TIME))
        {
            INFOR("close client [type:%d]", cdcs_clients[i].type);
            if(cdcs_clients[i].type == TBOX__NET__HEART__TYPE__HEART_NMEA) client_nmea_flag = 0;
            if (cdcs_clients[i].type == TBOX__NET__HEART__TYPE__HEART_3CALL) client_3call_flag = 0;
            close(cdcs_clients[i].fd);
            cdcs_clients[i].fd = -1;
            cdcs_clients[i].type = -1;
        }

        if(-1 != cdcs_clients[i].fd)
        {
            //all_disconnect = 0;
        }
    }

}


/**
 * @brief  add new client and print
 * @param none
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
static int cdcs_handle_server(void)
{
    int i = 0;
    int new_conn_fd = -1;
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    memset(&cli_addr, 0, sizeof(cli_addr));
    
    new_conn_fd = accept(tcp_fd, (struct sockaddr *)&cli_addr, &len);
    INFOR("new client comes ,fd=%d\n", new_conn_fd);
    if (new_conn_fd < 0)//connect fail
    {
        ERROR("Fail to accept");
        return -1;
    }
    else//add new client
    {
        for (i = 0; i < CDCS_CLIENT_NUM_MAX; i++)
        {
            if (cdcs_clients[i].fd == -1)
            {
                cdcs_clients[i].type = -1;
                cdcs_clients[i].fd = new_conn_fd;
                cdcs_clients[i].addr = cli_addr;
                cdcs_clients[i].lasthearttime = time_msec();
                INFOR("add client_fd[%d]=%d", i, cdcs_clients[i].fd);
                cdcs_socket_link_init(i);
                netmodecfg_type_sync = -1;
                break;
            }
        }
        //over client max num
        if (i >= CDCS_CLIENT_NUM_MAX)
        {
            ERROR("client max num is %d, close new_conn_fd ", i);
            close(new_conn_fd);
            return -1;
        }
    }
    return 0;
}
/**
 * @brief  cdcs task cdcs handle client,client exit & message decode
 * @param struct cdcs_client_t* client//client+i(0 or 1)
 * @param MSG_RX* msg//client(cdcs) send msg to server(tbox)//
 * call cdcs_msg_decodex to call-back cdcs_tcp_protobuf_process
 * @return none
 * @date 2021/10/22
 * @version V1.0
*/
static int cdcs_handle_client( cdcs_client_t *client,MSG_RX *msg)
{
    int num = 0;
    if(client->fd < 0)
    {

        DEBUG("invalid Client fd:%d\n",client->fd);
        return -1;
    }
    
    DEBUG("read Client msg:%d\n",client->fd);
    if (msg->used >= msg->size)
    {
        msg->used =  0;
    }

    num = recv(client->fd, (msg->data + msg->used), msg->size - msg->used, 0);
    if (num > 0)//message decode//call-back cdcs_tcp_protobuf_process
    {
        
        msg->used += num;
        //decode msg
        cdcs_msg_decodex(msg, cdcs_tcp_protobuf_process, &client->fd);
    }
    else//client exit
    {
        if (num == 0 && (EINTR != errno))
        {
            ERROR("TCP client disconnect!!!!");
        }

        ERROR("Client(%d) exit\n", client->fd);
        close(client->fd);
        if(client->type == TBOX__NET__HEART__TYPE__HEART_NMEA) 
        {
            client_nmea_flag = 0;
        }
        if (client->type == TBOX__NET__HEART__TYPE__HEART_3CALL) 
        {
            client_3call_flag = 0;
        }
        client->fd = -1;
        client->type = -1;
    }
  
    return 0;
}

static void cdcs_3call_first_sync(void)
{
    int fd = cdcs_get_3call_fd();
    //deinit
    if(fd < 0 )
    {
        DEBUG("set 3call state [%d] no connect", cdcs_state.connect_3call_state);
        cdcs_state.connect_3call_state = CDCS_CONNECT_NO;
        active_flag = 1;
        tboxinfo_flag = 0;
        return ;
    }

    //sync
    if(CDCS_CONNECT_FIRST == cdcs_state.connect_3call_state)
    {
        INFOR("cdcs first syunc active");
        cdcs_msg_request_active_state_res();

        INFOR("cdcs first sync tboxinfo");
        cdcs_msg_request_tboxinfo();

        INFOR("cdcs first sync network state");
        cdcs_net_state_t param;
        param.timestamp = cdcs_get_timestamp();
        if(0 == get_network_st())
        {
            param.networksts = 0;
        }
        else
        {
            param.networksts = 1;
        }
        INFOR("param.networksts = %d", param.networksts);
        param.channel = 1;
        cdcs_net_state_sync_send(&param);


        INFOR("cdcs first sync charge record");
		cdcs_message_request( fd ,TBOX__NET__MESSAGETYPE__RESPONSE_CHARGE_RECORD_RESULT,NULL);

        INFOR("cdcs first sync bat record");
        cdcs_batheat_response_send( fd ,TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTHEATMODE, 1, true);

        INFOR("cdcs first sync bat mode");
        cdcs_msg_response_send( fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_BATTERYMODE);

        INFOR("cdcs first sync discharge");
        cdcs_discharge_response_send(fd, TBOX__NET__MESSAGETYPE__REQUEST_QUERY_DISCHARGE, NULL, 0);

        INFOR("set 3call state [%d] long connect", cdcs_state.connect_3call_state);
        cdcs_state.connect_3call_state = CDCS_CONNECT_LONG;
        cdcs_frist_callstate_send(fd);
       
    }
    
}

//tbox active & appointment & ota & info
static void cdcs_poll_timer_1s(unsigned time_ms)
{
    static unsigned tiemr_tag = 0;
    static unsigned time_1min = 0;

    cdcs_msg_request_active_state_res();// just now link reporting status
    if(((time_ms > tiemr_tag) &&((time_ms - tiemr_tag) > 1000))
        || ((tiemr_tag == 0) || (time_ms < tiemr_tag)))
    {
        tiemr_tag = time_ms; 
        //cdcs_check_fault_st(time_ms); 
        cdcs_msg_request_timer_1s();
        cdcs_3call_first_sync();
        //cdcs_msg_request_tboxinfo();   
    }

    if(time_msec() - time_1min > 60*1000)
    {
        time_1min = time_msec();
        for(unsigned char i = 0; i < CDCS_CLIENT_NUM_MAX;i++)
        {
            INFOR("fd(%d),type(%d),lasttime(%lld)",cdcs_clients[i].fd, cdcs_clients[i].type, cdcs_clients[i].lasthearttime);
        }
    }
     
}

//do nothing
static void cdcs_poll_timer_5s(unsigned time_ms)
{
    static unsigned tiemr_tag_5 = 0;

    if(((time_ms > tiemr_tag_5) &&((time_ms - tiemr_tag_5) > 5000))
        || ((tiemr_tag_5 == 0) || (time_ms < tiemr_tag_5)))
    {
        tiemr_tag_5 = time_ms;

        for(unsigned char i = 0; i < CDCS_CLIENT_NUM_MAX;i++)
        {
            if((cdcs_clients[i].fd > 0) && (cdcs_clients[i].type == TBOX__NET__HEART__TYPE__HEART_3CALL))
            {
                // cdcs_msg_request_tboxinfo();
                //cdcs_msg_response_send(cdcs_clients[i].fd, TBOX__NET__MESSAGETYPE__REQUEST_NETWORK_SIGNAL_STRENGTH);
            }
        }
    }
}

//do nothing
static void cdcs_poll_timer_10s(unsigned time_ms)
{
    static unsigned tiemr_tag = 0;
    
    if(((time_ms > tiemr_tag) &&((time_ms - tiemr_tag) > 10000))
        || ((tiemr_tag == 0) || (time_ms < tiemr_tag)))
    {
        tiemr_tag = time_ms;
    }
}


static int 
task_CDCS(void *none)
{
	short i = 0;
    int ret = -1;
	static MSG_RX rx_msg[CDCS_CLIENT_NUM_MAX];
    static uint16_t task_poll_inv = CDCS_POLL_INTERVAL;
    static uint64_t task_poll_timer = 0;
    uint64_t systime_msec64 = 0;
    uint64_t systime_msec64_5 = 0;
    uint64_t SocketCreateTimeTag = 0;
	

    
    INFOR("create cdcs tcp socket");
    ret = cdcs_create_tcp_socket();
	if( ret != 0 )
	{
		if (tcp_fd > 0)
		{
            close(tcp_fd);
            tcp_fd = -1;
            ERROR("tbox_cdcs_create_tcp_socket failed,retry.ret:%d",ret);
		} 
        
        SocketCreateTimeTag = time_msec64();
	}

    while (1)
    {
        systime_msec64 =  time_msec64();   
        systime_msec64_5 =  time_msec64();

       /* recreate server socket */
        if(tcp_fd == -1 && (time_msec64() - SocketCreateTimeTag > CDCS_SOCKET_RETRY))
        {
            ERROR("assist recreate server socket");
            if (cdcs_create_tcp_socket())
            {
                if (tcp_fd > 0)
                {
                    close(tcp_fd);
                    tcp_fd = -1;
                }
            }
            SocketCreateTimeTag = time_msec64();
        }

        systime_msec64 =  time_msec(); 
         
        cdcs_check_client_connect_st(systime_msec64);
        //tbox active & appointment & ota & info
        cdcs_poll_timer_1s(systime_msec64);
        //do nothing
        cdcs_poll_timer_5s(systime_msec64_5);
        cdcs_poll_timer_10s(systime_msec64);
    }
	return 0;
}


static int 
init_CDCS(void)
{
    memset(&cdcs_state, 0, sizeof(cdcs_state));
    


				
   return 0;
}















