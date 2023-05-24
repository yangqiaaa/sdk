#ifndef __CDCS_API_H__
#define __CDCS_API_H__



//

#define CDCS_BATTHEAT_MAX                 3



#define IVI_GPS_TIME                     1000



#define IVI_SENTRY_MODE                (0)
#define IVI_360_FIND                   (1)

#define IVI_TYPE_PICTURE               (1)
#define IVI_TYPE_VIDEO                 (0)

#define GPS_NMEA_SIZE                  (1024)
//#define TBOX_PKI_IHU  1
//
#define PKI_IDLE      0
#define PKI_INIT      1
#define PKI_ACCEPT    2
#define PKI_RECV      3
#define PKI_END       4

#define IVI_FOTA_PUSH_SUCCESS         1
#define IVI_FOTA_PUSH_FAIL            2
#define IVI_FOTA_PUSH_FAIL_TIMEOUT    3
#define IVI_FOTA_PUSH_FAIL_offline  4

//
#define TBOX_HU_LINK_TIMEOUT  1
#define TBOX_HU_LINK_NORMAL   0
typedef enum
{	
    CDCS_CLIENT_PANEL = 0,
	CDCS_CLIENT_GNSS,
}CDCS_CLIENT_TYPE;//
//
typedef enum
{	ECALL_NULL = 0,
	ECALL_KEY = 5,
	ECALL_VOICE = 6,
	ECALL_CAN = 7,
}ECALL_STRR_STYLE;//

//
typedef enum
{
	UNKNOWN_YTPE = 0,
	ECALL_TYPE,
	BCALL_TYPE,
	ICALL_TYPE,
}TBOX_IVI_CALLTYPE;//

//
typedef enum
{
	unknown_action = 0,
	START_YTPE,
	END_TYPE,
}TBOX_IVI_CALLACTION;//

typedef enum
{
	SENTINEL_BUSINESS  = 0,
	SEARCH_BUSINESS  ,
	BIOLOGY_BUSINESS ,
}TBOX_IVI_360;//



typedef struct
{
    uint64_t lasthearttime;  //
	uint8_t stage;           //
	uint8_t states;
	uint32_t accept_flag;    //
	uint8_t re_create_ssl_sock_flag; //
	uint8_t close_syscall_count;
} pki_client;

//
typedef struct{
    signed char link_index;
	//uint8_t link_faultflag;//
	//uint64_t link_timestamp;//
}cdcs_link_status_t;

//
typedef struct{
	uint8_t call_type;//
	uint8_t call_action;//
	uint8_t call_style;//
}ivi_callrequest;

typedef struct{
	uint32_t eventid;
	uint32_t timestamp;
	uint8_t datatype;
	uint8_t cameraname;
	uint32_t aid;
	uint32_t mid;
	uint32_t effectivetime;
	uint32_t sizelimit;
}ivi_remotediagnos;

typedef struct{
	 char *vin;
	  uint32_t eventid;
	  uint32_t timestamp;
	  uint32_t aid;
	  uint32_t mid;
	  uint32_t starttime;
	  uint32_t durationtime;//1:TBOX, 2:IHU
	  uint32_t channel;//1:ERROR, 2:WARN, 3:INFO, 4:DEBUG
	  uint32_t level;
}ivi_logfile;

//
typedef struct {
	uint32_t timestamp;
    uint32_t hour;
    uint32_t min;
    uint32_t id;
    uint32_t targetpower;
	uint16_t cmd;
	uint8_t mode;//true:effective, false:Invalid
	uint32_t longtime;
    uint8_t effectivestate;//
	uint8_t iscontinuecharge;
}ivi_chargeAppointSt;

typedef struct
{
  uint32_t timestamp;
  uint32_t result;
}ivi_batterymode;

//360 find car
typedef struct
{
  uint32_t timestamp;
  uint32_t result;
}ivi_360mode;

typedef struct _cdcs_360mode_t {
  uint32_t timestamp;
  /*
   *0: SentinelMode, 1:360 find car, 2:biological detection
   */
  uint32_t mode;
  /*
   *0: video, 1:photo
   */
  uint32_t type;
  char url[512];
  char filename[128];
  uint32_t eventid;
}cdcs_360mode_t;

typedef struct _battheat_mode
{
	uint32_t timestamp;
	uint32_t id;
	uint32_t hour;
	uint32_t min;
	/*
	*0: off, 1: charge heat, 2: appointment heat
	*/
	uint32_t mode;
}cdcs_battheat_mode_t;

typedef struct _route_mode
{
	uint32_t timestamp;
	uint32_t id;
  	// char *name;
  	// char *data;
	char name[8];
  	char data[1024];
	/*
	*0: off, 1: charge heat, 2: appointment heat
	*/
	uint32_t event;
}cdcs_route_mode_t;

typedef struct _cdcs_battery {
	uint32_t timestamp;
	uint32_t mode;
} cdcs_battery_t;

typedef struct _cdcs_discharge
{
	uint32_t timestamp;
	int command;
	uint32_t targetpower;

} cdcs_discharge_t;

typedef struct _cdcs_net_state
{
	uint32_t timestamp;
	uint32_t channel;
	uint32_t networksts;

} cdcs_net_state_t;

typedef enum _CDCS_CONNECT_STATE
{
    CDCS_CONNECT_NO = 0,
	CDCS_CONNECT_FIRST,
	CDCS_CONNECT_LONG,
} CDCS_CONNECT_STATE;

/** 
 * @brief 
 * @file cdcs_api.h 
 * @name 
 * @param[in] void 
 * @return void 
 * @note 
 * connect_3call_state
 * 0 no connect
 * 1 first connect
 * 2 long connect
 * @date 2022-08-26 11:04:30 
 * @version V1.0.0 
*/
typedef struct _cdcs_state
{
	uint8_t connect_3call_state;

} cdcs_state_t;

//mbox msg ################################################
typedef struct 
{
    char sender[16];
    unsigned char msgid;
    unsigned int msglen ;
}cdcs_mbox_msg_header_t;

typedef struct
{
    unsigned int event;
    unsigned int active;
} cdcs_event_t;

typedef struct _cdcs_msg_t
{
    cdcs_mbox_msg_header_t header;
    cdcs_event_t msg;
} cdcs_mbox_msg_t;


typedef enum CDCS_MSG_EVENT
{
    CDCS_MSG_GPS_EVENT = 1,
    CDCS_MSG_MOB_EVENT ,
    CDCS_MSG_CALL_EVENT ,
    CDCS_MSG_SOS_EVENT ,
    CDCS_MSG_SRS_EVENT ,
    CDCS_MSG_SOCKET_RESET,//
    CDCS_MSG_SENTRY_CHECK,//
	CDCS_MSG_SENTRY_CLEANVON,//
    CDCS_MSG_360_SERACH,//
    CDCS_MSG_RMT_CFG,//
	//active cdcs
	CDCS_MSG_ACTIVE,
	//tbox set charge to cdcs
	CDCS_MSG_TBOX_SET_CHARGE,
	//cdcs set charge
	CDCS_MSG_CDCS_SET_CHARGE,
	//tbox sync batterymode
	CDCS_MSG_SYNC_BATTERYMODE,
	//
	CDCS_MSG_SYNC_BATTHEAT,
	CDCS_MSG_SYNC_ROUTE,
} CDCS_MSG_EVENT;

typedef enum _CDCS_SIGNAL_TYPE
{
    CDCS_SIGNAL_TYPE_AUTO = 0,
	CDCS_SIGNAL_TYPE_NR5G,
	CDCS_SIGNAL_TYPE_NR5G_LTE,
	CDCS_SIGNAL_TYPE_LTE,
	CDCS_SIGNAL_TYPE_3G,
	CDCS_SIGNAL_TYPE_2G,
	
} CDCS_CDCS_SIGNAL_TYPE;

int cdcs_get_gnss_onoff(void);
void cdcs_gnss_send(void);

uint8_t tbox_ivi_get_call_action(void);

uint8_t tbox_ivi_get_call_type(void);

void tbox_ivi_clear_call_flag(void);

void tbox_ivi_clear_bcall_flag(void);

void tbox_ivi_clear_icall_flag(void);

extern void tbox_ivi_set_tspInformHU(ivi_remotediagnos *tsp);

extern void tbox_ivi_set_tsplogfile_InformHU(ivi_logfile *tsp);

extern void tbox_ivi_set_tspchager_InformHU(ivi_chargeAppointSt *tsp);

extern long tbox_ivi_getTimestamp(void);

extern void tbox_ivi_pki_renew_pthread();

extern uint8_t tbox_ivi_ecall_srs(void);

extern uint8_t tbox_ivi_ecall_key(void);

extern void tbox_ivi_ecall_srs_deal(uint8_t dt);

extern void tbox_ivi_ecall_key_deal(uint8_t dt);

extern void tbox_ivi_tsp_active(void);

extern void tbox_ivi_push_fota_informHU(uint8_t flag);

extern uint8_t tbox_ivi_get_link_fault(uint64_t *timestamp);

extern uint8_t tbox_ivi_ecall_type();

extern void tbox_ivi_charge_ctrl(long cmd);

extern void tbox_ivi_charge_appoint(long cmd,uint8_t *data);

extern void tbox_ivi_discharge_ctrl(long cmd,uint8_t *data);

extern void tbox_ivi_charge_batmode(long cmd,uint8_t *data);


extern void tbox_ivi_charge_batheat(long cmd,uint8_t *data);

extern void tbox_ivi_charge_baterry_appoint(long cmd,uint8_t *data);


int cdcs_mbox_msg_event_send(enum CDCS_MSG_EVENT id, const char *sender, cdcs_mbox_msg_t msg);
int cdcs_get_3call_tcp_st(void);
int cdcs_get_nmea_tcp_st(void);

extern void cdcs_360mode_start_send(cdcs_360mode_t *param);
extern void cdcs_sync_guard();
uint32_t get_cdcs_mode(void);
void cdcs_batt_sync_send(void);
int cdcs_discharge_sync(void) ;
int cdcs_net_state_sync_send(cdcs_net_state_t *param);
uint8_t cdcs_get_restart_360_flag(void);
void cdcs_set_restart_360_flag(uint8_t param);
void cdcs_set_360_search_cnt(uint8_t param);
void cdcs_lcc_bat_sync(void);
void cdcs_bat_set_result_send(int result);

#endif

