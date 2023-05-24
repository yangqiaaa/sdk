#include <string.h>
#include "cdcs_api.h"
#include <time.h>
#include <sys/time.h>
#include "lcctrl.h"
#include "task.h"

extern ivi_callrequest callrequest; 

uint8_t tbox_ivi_get_call_type(void) 
{
	return callrequest.call_type;
}


uint8_t tbox_ivi_get_call_action(void) 
{

	return callrequest.call_action;
}

void tbox_ivi_clear_call_flag(void)
{
	memset(&callrequest,0 ,sizeof(ivi_callrequest));
}

void tbox_ivi_clear_bcall_flag(void)
{
	//callrequest.bcall = 0;
}

void tbox_ivi_clear_icall_flag(void)
{
	//callrequest.icall = 0;
}


long tbox_ivi_getTimestamp(void)
{
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);
	
	return (long)(timestamp.tv_sec);
}

