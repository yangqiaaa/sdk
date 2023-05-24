/*********************************************************************************
    *Copyright(C), 2021, Company
    *FileName:  myzlog.h
    *Author:  yangqi
    *Version:  V1.0.0
    *Date: 2023-05-16 14:57:57
    *Description:


    *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/

#ifndef  __MYZLOG_H__
#define  __MYZLOG_H__

/***************************************includes***********************************/
#include "zlog.h"
#include "myprint.h"
#include "category.h"


/***************************************Macros***********************************/
//#define
enum zlog_category_type
{
    ZLOG_CATEGORY_MIN = 0,
    ZLOG_CATEGORY_APP = 0,
    ZLOG_CATEGORY_CDCS,
    ZLOG_CATEGPRY_MAX,
};

/***************************************Variables***********************************/
struct myzlog_category_t
{
    int type[ZLOG_CATEGPRY_MAX];
    struct zlog_category_s category[ZLOG_CATEGPRY_MAX];
};


struct myzlog_data_t
{
    /*init flag*/
    int init_flag;
    int type;
    struct zlog_category_s *category;
};

struct myzlog_api_t
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
    int (*init)(struct myzlog_data_t myzlog, const char* zlog_conf_path);
    int (*get_category)(struct myzlog_data_t *myzlog_data, const char* cname, int type);
};

struct myzlog_t
{
    struct myzlog_data_t    myzlog_data;
    struct myzlog_api_t     myzlog_api;
};

extern zlog_category_t *my_zlog_get_category_to_type(int type);

#define my_zlog_info(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_APP) ) zlog_info(my_zlog_get_category_to_type(ZLOG_CATEGORY_APP), format ,## arg); \
    else { \
        printi("zlog no init\n");\
        printi( format, ## arg); \
    }\
}while(0)


#define my_zlog_error(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_APP) ) zlog_error(my_zlog_get_category_to_type(ZLOG_CATEGORY_APP), format ,## arg); \
    else { \
        printe("zlog no init\n");\
        printe( format, ## arg); \
    }\
}while(0)

#define my_zlog_debug(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_APP) ) zlog_debug(my_zlog_get_category_to_type(ZLOG_CATEGORY_APP), format ,## arg); \
    else { \
        printd("zlog no init\n");\
        printd( format, ## arg); \
    }\
}while(0)


// #if 1
#define my_zlog_info_cdcs(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS) ) zlog_info(my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS), format ,## arg); \
    else { \
        printi("zlog no init\n");\
        printi( format, ## arg); \
    }\
}while(0)


#define my_zlog_error_cdcs(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS) ) zlog_error(my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS), format ,## arg); \
    else { \
        printe("zlog no init\n");\
        printe( format, ## arg); \
    }\
}while(0)

#define my_zlog_debug_cdcs(format, arg...) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS) ) zlog_debug(my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS), format ,## arg); \
    else { \
        printd("zlog no init\n");\
        printd( format, ## arg); \
    }\
}while(0)

#define my_hzlog_info_cdcs(format, buf, buf_len) do{ \
    if(NULL != my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS) ) hzlog_info(my_zlog_get_category_to_type(ZLOG_CATEGORY_CDCS), buf, buf_len); \
    else { \
        printi("zlog no init, buf len(%d)\n", buf_len);\
    }\
}while(0)
// #endif

/***************************************Functions***********************************/
//void test(void);
extern int myzlog_init(struct myzlog_t *myzlog, int type);
extern int myzlog_set(struct myzlog_t *myzlog);
static int my_zlog_init(struct myzlog_data_t myzlog, const char* zlog_conf_path);
static int my_zlog_get_category(struct myzlog_data_t *myzlog_data, const char* cname, int type);

#endif
/* [] END OF FILE */