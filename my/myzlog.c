/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  myzlog.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-16 15:23:04
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "myzlog.h"
#include "myprint.h"
#include <stdio.h>


/***************************************Macros***********************************/
//#define
enum init_e
{
    INIT_IDLE,
    INIT_OK,
    INIT_FAIL,
};


/***************************************Variables***********************************/

// struct myzlog_t myzlog;
struct myzlog_category_t myzlog_category;
/***************************************Functions***********************************/
extern int myzlog_init(struct myzlog_t *param, int type)
{
    int ret = 0;
    int i = 0;

    param->myzlog_data.init_flag = INIT_IDLE;
    param->myzlog_data.type = type;
    printf("##type (%d)\n", type);
    for(i = ZLOG_CATEGORY_MIN; i<ZLOG_CATEGPRY_MAX; i++)
    {
        printf("##i (%d)\n", i);
        if(i == type)
        {
            myzlog_category.type[i] = ZLOG_CATEGORY_MIN +i;
            // myzlog_category.category[i] = NULL;
            printf("zlog init type(%d)\n", myzlog_category.type[i] );
            memset(&(myzlog_category.category[i]), 0, sizeof(zlog_category_t) );
        }
    }
    param->myzlog_data.category = NULL;

    

    param->myzlog_api.init = NULL;
    param->myzlog_api.get_category = NULL;



    print(PRINT_INFOR, "myzlog init\n");
    return ret;
}

extern int myzlog_set(struct myzlog_t *param)
{
    int ret = 0;
    param->myzlog_api.init = my_zlog_init;
    param->myzlog_api.get_category = my_zlog_get_category;
    print(PRINT_INFOR, "myzlog set\n");
    return ret;  
}

static int my_zlog_init(struct myzlog_data_t param, const char* zlog_conf_path)
{
    int ret = 0;
    ret = zlog_init(zlog_conf_path);
    if(ret < 0) {
        print(PRINT_ERROR, "zlog init fail(%d)\n", ret);
    }
    else{
        print(PRINT_INFOR, "zlog init ok(%d)\n", ret);
        param.init_flag = INIT_OK;
    }
    return ret;
}

void my_zlog_show(void)
{
    int i = 0;
    for(i = ZLOG_CATEGORY_MIN; i<ZLOG_CATEGPRY_MAX; i++)
    {
        printf("zlog init type(%d)\n", myzlog_category.type[i] );
        printf("zlog init category(%s)\n", myzlog_category.category[i].name );
    }
}

static int my_zlog_get_category(struct myzlog_data_t *param, const char* cname, int type)
{
    int ret = 0;
    param->category = zlog_get_category(cname);
    struct zlog_category_s *p = NULL;
    if(NULL == param->category){
        print(PRINT_ERROR, "zlog get category fail(%s)\n", cname);
    }
    else{
        print(PRINT_INFOR, "zlog get category ok(%s)\n", cname);
        param->type = type;
        p = param->category;
        myzlog_category.category[type] = *(param->category);
        printf("type(%d)\n", param->type);
        printf("1 cayegory(%s)\n", param->category->name);
        printf("2 cayegory(%s)\n", p->name);
        printf("3 cayegory(%s)\n", myzlog_category.category[type].name);
        my_zlog_show();
    }

    return ret;
}

extern zlog_category_t *my_zlog_get_category_to_type(int type)
{
    static int i = 1;
    struct zlog_category_s *p = NULL;
    if(myzlog_category.type[type] == type)
    {
        p = &(myzlog_category.category[type]);
        if(i){
            i = 0;
            printf("4 type(%d),cayegory(%s)\n",type, myzlog_category.category[type].name);
            printf("4 type(%d),cayegory(%s)\n",type, p->name);
        }
        // return &(myzlog_category.category[type]);
        return p;
    }
    else
    {
        return NULL;
    }
    

}

/* [] END OF FILE */