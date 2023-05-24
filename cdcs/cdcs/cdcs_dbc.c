/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  cdcs_dbc.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2022-05-29 10:40:16
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "dbc.h"
#include "log.h"
#include "assert.h"
#include "can.h"
#include "shell.h"
#include "task.h"
#include "cdcs_api.h"

#include <stdio.h>
#include <string.h>


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/
static int cdcs_mode = -1;
// uint32_t cdcs_sig_id = 0;

/***************************************Functions***********************************/
uint32_t get_cdcs_mode(void) {
    return cdcs_mode;
}
#if 0

static int cdcs_data_parse_surfix(int sig_id, const char *sfx)
{
    uint32_t  cdcs_index;

    assert(sig_id > 0 && sfx != NULL);
	

    if (1 != sscanf(sfx, "G07%3x", &cdcs_index))
    {
        // INFOR("get fail");
        return 0;
    }
    INFOR("cdcs index %d", cdcs_index);

	//get it by gb32960 and dbc
    if (cdcs_index  == 0x23D) 
    {
		cdcs_sig_id = sig_id;
		INFOR("cdcs mode status is %d", cdcs_sig_id);
		return 0;
    }

	return -1;

}



static int dbccb_cdcs(int event, long arg1, long arg2) 
{
	int ret = 0;
	const dbcsig_t* dbc_sig = NULL;
    switch(event)
    {
		case DBC_EVENT_RELOAD:
		{
			cdcs_sig_id = 0;
			break;
		}

    	case DBC_EVENT_SURFIX:
      	{
          	ret = cdcs_data_parse_surfix((int)arg1, (const char *)arg2);
      		break;
		}

		#if 1
		case DBC_EVENT_UPDATE:
		{
			if(cdcs_sig_id == (int)arg1)
			{
				INFOR("cdcs state %d", cdcs_sig_id);
                if(NULL != (dbc_sig = (const dbcsig_t*)dbc_getmsg(cdcs_sig_id))) {
                    INFOR("get data");
                    // INFOR("mode %f", dbc_sig->curr_val);
                }
                else ERROR("get error");
				// if( (NULL != (dbc_sig = dbc_getsig(srs_sig_id)))
				//    && (dbc_sig->curr_val != dbc_sig->last_val) )
				// {
				// 	if(dbc_sig->curr_val == 1.0)
				// 	{
				// 		INFOR("srs");
				// 		PrvtProtCfg_ecallTrig();
				// 		cfg_get_by_id(CFG_HZ_EXT_ECALL, xcall_num, &xcall_len);
				// 		xcall_mbox_msg_call_event_send("SRS", ECALL_E, XCALL_EVENT_CALL, xcall_num);
				// 	}
				// }

			}
			break;
		}
		#else
		case DBC_EVENT_UPDATE:
		{
			if(srs_sig_id == (int)arg1)
			{
				INFOR("srs state: %d",dbc_sig->curr_val);
				if( (NULL != (dbc_sig = dbc_getsig(srs_sig_id)))
				   && (dbc_sig->curr_val == (double)1.00 ))

				{
					PrvtProtCfg_ecallTrig();
					cfg_get_by_id(CFG_HZ_EXT_ECALL, xcall_num, &xcall_len);
					xcall_mbox_msg_call_event_send("SRS", ECALL_E, XCALL_EVENT_CALL, xcall_num);
				}

			}
			break;
		}
		#endif

    }
    return ret;

}

DECLARE_DBCCB(cdcs);
#endif

#if 1
static unsigned char cdcs_recvdata[8];

static void
cancb_cdcs_data(int event, long arg1, long arg2)
{
    if (event == CAN_EVENT_DATA)
    {
        can_pdu_t *pdu = (void*)arg1;
        for (int i = 0; i < arg2; i++, pdu = CAN_PDU_NEXT(pdu)) 
        {
            if(pdu->ident == 0x278)
            {
				memset(cdcs_recvdata, 0, 8);
				// INFOR("GET 278");
				memcpy(cdcs_recvdata, pdu->data, 8);
				// for(int i = 0; i<8; i++){
				// 	INFOR("cdcs %x", cdcs_recvdata[i])
				// }
				if(0x64 == cdcs_recvdata[3] && cdcs_mode != 1) {
					INFOR("get mode value 100");
					cdcs_mode = 1;
				}
				else if(0x5A == cdcs_recvdata[3] && cdcs_mode != 2) {
					INFOR("get mode value 90");
					cdcs_mode = 2;
				}
				else if(0x50 == cdcs_recvdata[3] && cdcs_mode != 0) {
					INFOR("get mode value 80");
					cdcs_mode = 0;
				}
				// memset(cdcs_recvdata, 0, 8);
                // lcctrl_get_random(pdu->data);
            }
        }
    }
    else if (event == CAN_EVENT_ACTIVE)
    {
        // remote_can_status = 1;
        INFOR("cdcs nm active");
    }
    else if (event == CAN_EVENT_DEACTIVE)
    {
        // remote_can_status = 0;
        ERROR("cdcs nm deactivated");
		cdcs_mode = -1;
    }
}
DECLARE_CANCB(cdcs_data);


static int shcmd_cdcsshow(int argc, const char **argv)
{
    int i = 0;
    shell_print("cdcs mode %d\n", get_cdcs_mode());
	for(i = 0; i<8; i++){
		shell_print("data[%d], %x\n", i, cdcs_recvdata[i]);
	}

    shell_exit(0);
}


DECLARE_SHCMD(
    cdcsshow,
    "cdcsshow.", 
    "Usage: cdcsshow\n"
    "cdcsshow\n");

#endif


/* [] END OF FILE */