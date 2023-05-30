/*********************************************************************************
    *Copyright(C), 2021, Company
    *FileName:  cdcs_msg.h
    *Author:  yangqi
    *Version:  V1.0.0
    *Date: 2023-05-18 10:02:25
    *Description:


    *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/

#ifndef  __CDCS_MSG_H__
#define  __CDCS_MSG_H__

/***************************************includes***********************************/
//#include"xxx.h"
#include "cdcs/cdcs_pb.h"
#include "mycdcs.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/
//static int i
typedef void (*ivi_msg_proc)(unsigned char *msg, unsigned int len);
typedef void (*ivi_msg_handler)(int fd, unsigned char *msg, unsigned int len);

/***************************************Functions***********************************/
//void test(void);
int cdcs_net_top_message_pack( Tbox__Net__TopMessage *TopMsg,unsigned char *pro_buf,unsigned int in_size,size_t *out_size);
int cdcs_net_top_message_send( int fd ,unsigned char *pro_buf,unsigned int in_size,int loglv);
int cdcs_msg_heart_request_send( int fd, int heart_type);

int cdcs_read_tbox_data(struct mycdcs_t *param, int list,uint8_t *dstbuf,int len);
void cdcs_msg_decodex(int fd, MSG_RX *rx, ivi_msg_handler ivi_msg_proc);
void cdcs_msg_decode_callback(int fd, unsigned char *data, unsigned int len);

int cdcs_sm_callback(int* fd, Tbox__Net__TopMessage *msg);

#endif
/* [] END OF FILE */