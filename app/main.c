/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  main.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-04-12 09:07:46
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include <stdio.h>
#include <string.h>
#include "api.h"
#include <gtk/gtk.h>
#include "zlog.h"
#include "myprint.h"
#include "myzlog.h"
#include "mygtk.h"
#include "mycdcs.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/


int
main(int argc,
     char **argv) {
    int status;

    int zlog_init_flag = 0;
    const char zlog_conf_path[100] = "/home/yq22/Desktop/code/sdk/bin/zlog.conf";
    const char zlog_cname[30] = "app";

    my_zlog_info("test 2 hello, zlog\n");
    struct myzlog_t myzlog;
    myzlog_init(&myzlog, ZLOG_CATEGORY_APP);
    myzlog_set(&myzlog);
    zlog_init_flag = myzlog.myzlog_api.init(myzlog.myzlog_data, zlog_conf_path);
    // zlog_init_flag = zlog_init("./zlog.conf");
    print(PRINT_INFOR, "zlog init flag %d\n", zlog_init_flag);
    myzlog.myzlog_api.get_category(&myzlog.myzlog_data, zlog_cname, ZLOG_CATEGORY_APP);
    zlog_info(myzlog.myzlog_data.category, "hello, zlog");
    my_zlog_info("test hello, zlog");
    my_zlog_info("22222");

    // cdcs_task();

    mygtk_main(argc, argv);

    printe("1\n");
    zlog_fini();
    printe("2\n");

    return status;
}

/* [] END OF FILE */