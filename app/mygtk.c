/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  mygtk.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-16 21:06:05
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
#include "mygtk.h"
#include "myzlog.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <gtk/deprecated/gtkhbox.h>
#include <gdk/gdkscreen.h>


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/

static GtkWidget *image;

/***************************************Functions***********************************/

// 更新图片
// void update_image(const char *filepath) {
void update_image(gpointer data) {
    // gchar *filepath = (gchar *) data;
    char filepath[100] = "0";
    strcpy(filepath, (char *)data);
    
    my_zlog_info("gtk image path(%s)", (char *)data);
    g_print("path %s", (char *)data);
    my_zlog_info("gtk image path(%s)", filepath);
    g_print("path %s", filepath);
    strcpy(filepath, "/home/yq22/Desktop/code/sdk/bin/image/signal/signal_type_4.png");
    // 加载图片
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filepath, NULL);
    // // 获取图片大小
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // // 缩放图片
    GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width / 2, height / 2, GDK_INTERP_BILINEAR);
    // 设置图片
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);
    // 释放资源
    g_object_unref(pixbuf);
    g_object_unref(scaled_pixbuf);
}
#if 0
static void
activate(GtkApplication *app,
         gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *button_box;
    my_zlog_info("gtk app create windows");
    /*create a windows*/
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "Window");
    gtk_window_set_default_size(GTK_WINDOW (window), 500, 500);

    button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER (window), button_box);

    my_zlog_info("gtk app create button");
    button = gtk_button_new_with_label("Hello World");
    g_signal_connect (button, "clicked", G_CALLBACK(print_hello), NULL);
    g_signal_connect_swapped (button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    gtk_container_add(GTK_CONTAINER (button_box), button);


    // 创建窗口和布局控件
    GtkWidget *fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);


    // 创建图片控件，并设置图片
    GtkWidget *image = gtk_image_new();
    update_image(GTK_IMAGE(image), "./image/signal/signal_type_0.png");

    // 设置图片大小和对齐方式
    gtk_widget_set_size_request(image, 200, 200);
    // gtk_image_set_form_alignment(GTK_IMAGE(image), 0.5, 0.5);

    // 将图片控件添加到布局容器中
    gtk_box_pack_start(GTK_BOX(button_box), image, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
}
#endif

static void
activate(GtkApplication *app,
         gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;
    GtkWidget *table;
    GdkScreen* screen;
    int width = 0;
    int height = 0;
    screen = gdk_screen_get_default();
    width = gdk_screen_get_width(screen);
    height = gdk_screen_get_height(screen);
    my_zlog_info("width(%d),height(%d)", width, height);
    my_zlog_info("gtk app create windows");
    /*create a windows*/
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "Window");
    gtk_window_set_default_size(GTK_WINDOW (window), 700, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    my_zlog_info("gtk app create box");

    // 创建垂直布局容器
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    table = gtk_table_new (4, 4, TRUE);
    gtk_container_add(GTK_CONTAINER (window), box);

    my_zlog_info("gtk app add image");

    // 创建图片
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/home/yq22/Desktop/code/sdk/bin/image/signal/signal_type_0.png", NULL);
    image = gtk_image_new_from_pixbuf(pixbuf);

    // 设置图片位置和大小
    gtk_layout_put(GTK_LAYOUT(window), image, 50, 50);
    gtk_widget_set_size_request(image, 300, 200);

    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);
    g_object_unref(pixbuf); // 释放资源

    my_zlog_info("gtk app add button");

    // 创建按钮
    printf("1\n");
    button = gtk_button_new_with_label("Update Image");
    printf("2\n");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    printf("1\n");

    my_zlog_info("gtk app create button");

    // 绑定按钮点击事件
    g_signal_connect(button, "clicked", G_CALLBACK(update_image), (gpointer)"/home/yq22/Desktop/code/sdk/bin/image/signal/signal_type_4.png");
    // g_signal_connect (button, "clicked", G_CALLBACK(print_hello), NULL);
    // g_signal_connect_swapped (button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_add(GTK_CONTAINER (box), button);



    gtk_widget_show_all(window);

    gtk_main();
}

int mygtk_main(int argc,
     char **argv) {
    GtkApplication *app;
    int status;
    my_zlog_info("gtk app start");
    // app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    /*set attivate signal connect*/
    g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);
    /*run app and send signal*/
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);
    my_zlog_info("gtk app end");
    return status;
}

/* [] END OF FILE */