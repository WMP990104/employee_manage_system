#ifndef __STAFF_H__
#define __STAFF_H__

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

#define LOGIN 1 // 登录
#define ADMIN 0 // 管理员身份
#define STAFF 1 // 员工身份
#define ERR_MSG(msg)                         \
    do                                       \
    {                                        \
        fprintf(stderr, "__%d__", __LINE__); \
        perror(msg);                         \
    } while (0) // 定义错误宏函数

/*用户类型:0、管理员   1、普通用户
  通信指令类型:1、登录  2、按人名查询 3、查询全部 4#、修改 5、添加用户 6、删除用户 7、查询历史记录   8、退出
  'N'：姓名  'A'：年龄  'F'：家庭住址  'T'：电话  'P'：职位  'S'：工资  'D'：入职年月  'G'：评级
*/
// 员工信息结构体 大小212byte
struct info
{
    int num;           // 工号
    int type;          // 用户类型   0：管理员   1：普通用户
    char name[32];     // 姓名
    char password[6];  // 密码
    int age;           // 年龄
    char tel_num[12];   // 电话
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

// 定义服务器回复客户端信息结构体
/* 0：失败  1：成功 2：回复信息包 */
struct recvs
{
    int cmd; // 指令类型
    char buf[256];
    int row;
};

void menu1(void);       // menu1函数
void menu_admin(void);  // 管理员菜单
void menu_staff(void);  // 员工菜单
void menu_search(void); // 查询菜单

int do_login(const int sfd, const int type);    // 登录函数
int do_research(const int sfd, const int type);     // 查询函数
int do_insert(const int sfd);    // 添加用户函数
int do_delete(const int sfd);    // 删除用户函数
int do_update(const int sfd, int num);    // 修改信息函数
int do_review(const int sfd);    // 查询历史记录

// 菜单1函数
void menu1(void)
{
    system("clear");
    printf("*************************************************************\n");
    printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
    printf("*************************************************************\n");
    printf("请输入你的选择(数字)>>");
}
// 管理员菜单
void menu_admin(void)
{
    system("clear");
    printf("*************************************************************\n");
    printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
    printf("* 6：退出                                                   *\n");
    printf("*************************************************************\n");
    printf("请输入您的选择(数字)>>");
}
// 员工菜单
void menu_staff(void)
{
    system("clear");
    printf("*************************************************************\n");
    printf("*************  1：查询  	2：修改		3：退出	 *************\n");
    printf("*************************************************************\n");
    printf("请输入您的选择(数字)>>");
}
// 查询菜单
void menu_search(void)
{
    system("clear");
    printf("*************************************************************\n");
    printf("******* 1：按人名查找  	2：查找所有 	3：退出	 *******\n");
    printf("*************************************************************\n");
    printf("请输入您的选择(数字)>>");
}
// 修改菜单
void menu_update(void)
{
    printf("*******************请输入要修改的选项********************\n");
    printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
    printf("******	5：职位	   6：工资  7：入职年月   8：评级  ******\n");
    printf("******	9：密码	 10：退出				  *******\n");
    printf("********************************************************\n");
}
#endif