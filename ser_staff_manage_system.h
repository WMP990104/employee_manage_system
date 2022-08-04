#ifndef __SER_STAFF_MANAGE_SYSTEM_H__
#define __SER_STAFF_MANAGE_SYSTEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ERR_MSG(msg)                         \
    do                                       \
    {                                        \
        fprintf(stderr, "__%d__", __LINE__); \
        perror(msg);                         \
    } while (0) // 定义错误宏函数

/*用户类型:0、管理员   1、普通用户
  通信指令类型:1、登录  2、按人名查询 3、查询全部 4#、修改 5、添加用户 6、删除用户 7、查询历史记录   8、退出
  'N'：姓名  'A'：年龄  'F'：家庭住址  'T'：电话  'P'：职位  'S'：工资  'D'：入职年月  'G'：评级 'W':密码
*/
// 员工信息结构体 大小212byte
struct info
{
    int num;           // 工号
    int type;          // 用户类型   0：管理员   1：普通用户
    char name[32];     // 姓名
    char password[6];  // 密码
    int age;           // 年龄
    char tel_num[12];  // 电话
    char address[64];  // 地址
    char position[32]; // 职位
    char date[12];     // 入职年月
    int grade;         // 等级
    int salary;        // 工资
};
// 定义通信结构体
struct trans
{
    int usertype;   // 用户类型:1、管理员   2、普通用户
    int cmdtype;    // 通信指令类型
    char buf[32];   // 通信的消息
    struct info st; // 员工信息
};

// 定义需要传入分支线程的结构体参数
struct msg
{
    int newfd;              // 文件描述符
    struct sockaddr_in cin; // 客户端信息结构体
    sqlite3 *db;            // 数据库
};

// 定义回复信息结构体
/* 0：失败  1：成功 2：完毕*/
struct recvs
{
    int cmd;                // 指令类型
    char buf[256];
    int row;
};

sqlite3 *create_table(void);                    // 打开数据库函数
void *recv_cli(void *arg);                      // 定义分支线程函数
int do_history(sqlite3 *db, const char *sql);    // 写入历史记录中 历史记录仅存放修改用户信息的记录
int do_judge(sqlite3 *db, int num, char (* name)[32], char (* password)[6], int flag);    // 判断是否有该用户

#endif