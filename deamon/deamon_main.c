/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  deamon_main.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-05-19 13:56:55
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
//#include"xxx.h"
//#include"xxx.h"


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>

const char *Binary1Path =   "/home/yq22/Desktop/code/sdk/bin/sdk_cdcs"; // 第一个bin文件的路径
const char *Binary2Path =   "/home/yq22/Desktop/code/sdk/bin/sdk_app"; // 第二个bin文件的路径
const char *PIDFile =       "/home/yq22/Desktop/code/sdk/bin/sdk_daemon.pid"; // 守护进程的pid文件路径
const int RestartInterval = 10; // 重新拉起应用的时间间隔，单位秒

volatile int KeepRunning = 1;

void SignalHandler(int sig)
{
    switch (sig)
    {
    case SIGHUP:
        // 处理挂起信号
        // 可以在这里重新加载配置文件等
        break;
    case SIGTERM:
        // 处理终止信号
        KeepRunning = 0;
        break;
    }
}

void CreatePIDFile()
{
    // 创建pid文件
    pid_t pid;
    char buf[16];

    pid = getpid();
    int fd = open(PIDFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open pid file");
        exit(EXIT_FAILURE);
    }
    if (lockf(fd, F_TLOCK, 0) == -1)
    {
        perror("lock pid file");
        exit(EXIT_FAILURE);
    }
    snprintf(buf, sizeof(buf), "%ld", (long)pid);
    write(fd, buf, strlen(buf));
}

void CheckPIDFile()
{
    // 检查pid文件是否存在，如果存在，尝试向进程发送0号信号判断进程是否存活
    // 如果进程不存在，则删除pid文件
    int fd = open(PIDFile, O_RDONLY);
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            // pid文件不存在
            return;
        }
        else
        {
            perror("open pid file");
            exit(EXIT_FAILURE);
        }
    }
    char buf[16];
    int len = read(fd, buf, sizeof(buf) - 1);
    if (len == -1)
    {
        perror("read pid file");
        exit(EXIT_FAILURE);
    }
    buf[len] = '\0';
    pid_t pid = atol(buf);
    if (pid == 0)
    {
        perror("invalid pid file");
        exit(EXIT_FAILURE);
    }
    if (kill(pid, 0) == -1 && errno == ESRCH)
    {
        // 进程不存在
        unlink(PIDFile);
    }
}

void CloseFileDescriptors()
{
    // 关闭所有文件描述符
    for (int i = sysconf(_SC_OPEN_MAX); i >= 0; i--)
    {
        close(i);
    }
}

void Daemonize()
{
    // 守护进程化
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        perror("getrlimit");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid != 0)
    {
        exit(EXIT_SUCCESS);
    }

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid != 0)
    {
        exit(EXIT_SUCCESS);
    }

    if (chdir("/") < 0)
    {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        fprintf(stderr, "unexpected file descriptors %d %d %d\n", fd0, fd1, fd2);
        exit(EXIT_FAILURE);
    }
}

int CheckBinary(const char *path)
{
    // 检查二进制文件是否可执行
    if (access(path, F_OK | X_OK) == -1)
    {
        return 0;
    }
    return 1;
}

int LaunchBinary(const char *path)
{
    // 启动二进制文件
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {
        // 子进程
        if (execl(path, path, NULL) == -1)
        {
            perror("execl");
            return -1;
        }
    }
    return pid;
}

int CheckProcessAlive(pid_t pid)
{
    // 检查进程是否存活
    if (kill(pid, 0) == -1 && errno == ESRCH)
    {
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s [start|stop]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "start") == 0)
    {
        // 启动守护进程
        Daemonize();
        CreatePIDFile();
        signal(SIGTERM, SignalHandler);
        signal(SIGHUP, SignalHandler);
        // 检查二进制文件是否存在并且可执行
        if (!CheckBinary(Binary1Path))
        {
            fprintf(stderr, "Binary1 does not exist or is not executable\n");
            exit(EXIT_FAILURE);
        }
        if (!CheckBinary(Binary2Path))
        {
            fprintf(stderr, "Binary2 does not exist or is not executable\n");
            exit(EXIT_FAILURE);
        }
        pid_t pid1 = LaunchBinary(Binary1Path);
        pid_t pid2 = LaunchBinary(Binary2Path);
        while (KeepRunning)
        {
            // 轮询两个bin文件是否存活，如果停止了，则重新启动
            if (!CheckProcessAlive(pid1))
            {
                pid1 = LaunchBinary(Binary1Path);
            }
            if (!CheckProcessAlive(pid2))
            {
                pid2 = LaunchBinary(Binary2Path);
            }
            sleep(RestartInterval);
        }
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        system("pkill sdk_cdcs");
        system("pkill sdk_app");
        // 停止守护进程
        CheckPIDFile();
        int fd = open(PIDFile, O_RDONLY);
        if (fd == -1)
        {
            perror("open pid file");
            exit(EXIT_FAILURE);
        }
        char buf[16];
        int len = read(fd, buf, sizeof(buf) - 1);
        if (len == -1)
        {
            perror("read pid file");
            exit(EXIT_FAILURE);
        }
        buf[len] = '\0';
        pid_t pid = atol(buf);
        if (pid == 0)
        {
            perror("invalid pid file");
            exit(EXIT_FAILURE);
        }
        if (kill(pid, SIGTERM) == -1)
        {
            perror("kill");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        fprintf(stderr, "usage: %s [start|stop]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}


/* [] END OF FILE */