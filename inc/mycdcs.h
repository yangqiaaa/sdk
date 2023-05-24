/*********************************************************************************
    *Copyright(C), 2021, Company
    *FileName:  mycdcs.h
    *Author:  yangqi
    *Version:  V1.0.0
    *Date: 2023-05-17 16:43:53
    *Description:


    *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/

#ifndef  __mycdcs_H__
#define  __mycdcs_H__

/***************************************includes***********************************/
#include <netinet/in.h>


/***************************************Macros***********************************/
#define CDCS_CLIENT_NUM_MAX     (2)
#define IVI_MSG_SIZE            (2048)
#define CDCS_SERVER_PORT         (5757)
#define CDCS_SERVER_ADDR         ("172.16.1.50")
#define IVI_PKG_MARKER                 "#START*"
#define IVI_PKG_ESC                    "#END*"
#define IVI_PKG_S_MARKER_SIZE          (7)
#define IVI_PKG_E_MARKER_SIZE          (5)
#define IVI_PKG_CS_SIZE                (1)
#define IVI_PKG_ENCRY_SIZE             (1)
#define IVI_PKG_MSG_LEN                (2)
#define IVI_PKG_MSG_CNT                (4)
#define PARAM_EXISTS                   (1)



/***************************************Variables***********************************/
//static int i
typedef struct
{
    int used;
    int size;
    unsigned char *data;
} MSG_RX;

typedef struct
{
    int fd;
    int type;
    unsigned long long lasthearttime;
    struct sockaddr_in addr;
} cdcs_client_t;


struct mycdcs_data_t
{
    /*init flag*/
    int init_flag;
    cdcs_client_t cdcs_client[CDCS_CLIENT_NUM_MAX];
};

struct mycdcs_api_t
{
    /** 
     * @brief 
     * @file myzlog.h 
     * @name 
     * @param[in] const char* zlog_conf_path
     * @return <0 is fail, =0 is ok
     * @note 
     * @date 2023-05-16 15:51:35 
     * @version V1.0.0 
    */
    int (*init)(const char* zlog_conf_path);
    int (*get_category)(const char* cname);
};

struct mycdcs_t
{
    struct mycdcs_data_t    mycdcs_data;
    struct mycdcs_api_t     mycdcs_api;
};


/***************************************Functions***********************************/
//void test(void);
int cdcs_task(void);

#endif
/* [] END OF FILE */