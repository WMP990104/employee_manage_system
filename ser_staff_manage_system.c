/* 员工管理系统服务器 
    IP和PORT由中断输入
*/
#include "ser_staff_manage_system.h"

int main(int argc, char const *argv[]) // 终端传过来的第二个参数是ip地址 第三个参数是端口
{
    // 判断 如果传进来的参数没有三个，则打印参数不够
    if (argc < 3)
    {
        printf("请检查传入的参数:IP and PORT\n");
        return -1;
    }
    // 打开数据库
    sqlite3 *db = create_table();
    if (NULL == db)
    {
        printf("数据库初始化失败\n");
        return -1;
    }
    printf("数据库初始化成功\n");

    // 创建流式套接字
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (EOF == sfd)
    {
        ERR_MSG("socket");
        return -1;
    }

    // 填充服务器地址信息结构体
    int port = atoi(argv[2]);// 转换一下参数的类型
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    // 设置允许端口快速复用
    int reuse = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        ERR_MSG("setsockopt");
        return -1;
    }
    printf("允许端口快速复用成功\n");

    // 将IP和PORT绑定到套接字上
    if (EOF == bind(sfd, (struct sockaddr*)&sin, sizeof(sin)))
    {
        ERR_MSG("bind");
        return -1;
    }
    printf("bind succeed\n");

    // 将套接字设置成被动监听状态
    if (EOF == listen(sfd, 2))
    {
        ERR_MSG("listen");
        return -1;
    }
    printf("listen succeed\n");

    // 定义客户端信息结构体
    struct sockaddr_in cin;
    socklen_t addrlen = sizeof(cin);

    int newfd = 0;
    pthread_t tid;
    struct msg cliInfo; // 创建一个结构体用于分支线程传参

    while (1)
    {
        // 主线程负责连接
        newfd = accept(sfd, (struct sockaddr*)&cin, &addrlen);
        if (newfd < 0)
        {
            ERR_MSG("accept");
            return -1;
        }
        printf("[%s:%d]connect succeed\n", \
            inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
        
        cliInfo.newfd = newfd;
        cliInfo.cin = cin;
        cliInfo.db = db;

        // 创建分支线程负责通信
        if (pthread_create(&tid, NULL, recv_cli, &cliInfo) != 0)
        {
            ERR_MSG("pthread_create");
            return -1;
        }
    }
    // 关闭文件描述符和数据库
    close(sfd);
    sqlite3_close(db);

    return 0;
}
// 打开数据库函数
sqlite3 *create_table(void)
{
    // 打开数据库
    sqlite3 *db = NULL;
    if (sqlite3_open("./staff_manage_sys.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "__%d__ sqlite3_open:%s\n", __LINE__, sqlite3_errmsg(db));
        return NULL;
    }
    // 创建员工信息表
    char sql[256] = "create table if not exists staff (num int PRIMARY KEY, type int, \
        name char, password char, age int, tel_num char, address char, \
        position char, date char, grade int, salary int)";
    char *errmsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
        return NULL;
    }
    // 添加一个管理员身份 admin admin
    sprintf(sql, "insert into staff values (0, 0, 'admin', 'admin', 0, 0, '0', '0', '0', 0, 0)");
    sqlite3_exec(db, sql, NULL, NULL, &errmsg); // 已经存在的错误不考虑

    // 创建历史信息表
    sprintf(sql, "create table if not exists history (history char)");
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
        return NULL;
    }

    return db;
}

// 定义分支线程函数
void *recv_cli(void *arg)
{
    // 分离当前线程，退出后自动回收资源
    pthread_detach(pthread_self());

    struct trans data;                       // 用于客户端发给服务器
    struct recvs recv_info;                  // 用于服务器回复客户端
    struct msg cliInfo = *(struct msg *)arg; // 接收客户端信息结构体参数
    int newfd = cliInfo.newfd;               // 从结构体中获取新的文件描述符
    struct sockaddr_in cin = cliInfo.cin;    // 从参数中获取客户信息结构体
    sqlite3 *db = cliInfo.db;                // 从参数中获取数据库地址

    ssize_t res = 0;            // 记录返回值
    time_t local_time = 0;;     // 记录当前时间
    struct tm* ltime = NULL;

    char name_pre[32] = {0};    // 当前登录的账户
    char sql[256] = {0};        // sqlite_exec语句
    char buf[256] = {0};        // 用于缓存

    char **result = NULL;
    char *errmsg = NULL;
    int row, column; // row行数（包括表头），column列数
    char brr[1] = "\t";
    int i = 0, j=0;
    int index = 0;

    while (1)
    {
        memset(&data, 0, sizeof(data));
        // 从文件描述符中读取数据
        res = recv(newfd, &data, sizeof(data), 0);
        if (res < 0)
        {
            ERR_MSG("recv");
            return (void*)-1;
        }
        else if (0 == res)
        {
            // 此时客户端关闭
            fprintf(stderr, "[%s:%d] 客户端关闭\n", \
                inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
            break;
        }
        // 准备数据包回复客户端
        memset(&recv_info, 0, sizeof(recv_info));
        switch (data.cmdtype)
        {
        case 1:
            // 登录
            if(0 == do_judge(db, 0, &data.st.name, &data.st.password, 2))
            {
                // 登录失败
                recv_info.cmd = 0;
                printf("[%s] login in failed\n", data.st.name);
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                break;
            }
            else
            {
                // 用户名密码身份都匹配成功
                printf("[%s] login in succeed\n", data.st.name);
                strncpy(name_pre, data.st.name, 32);
                recv_info.cmd = 1;
            }
            break;
        case 2:
            // 按人名查询
            sprintf(sql, "select * from staff where name='%s';", data.st.name);
            if (sqlite3_get_table(db, sql, &result, &row, &column, &errmsg) != SQLITE_OK)
            {
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                return NULL;
            }
            if (0 == row)
            {
                // 查找失败
                recv_info.cmd = 0;
            }
            else
            {
                // 查找成功
                recv_info.cmd = 1;
                sprintf(recv_info.buf, "%s   %s   %s   %s   %s   %s   %s   %s   %s   %s   %s", \
                result[column+0], result[column+1], result[column+2], result[column+3], result[column+4], \
                    result[column+5], result[column+6], result[column+7], result[column+8], \
                    result[column+9], result[column+10]);
            }

            /***********测试使用**************/
/*
            printf("row=%d column=%d\n", row, column);
            index = 0;
            for(i=0; i<(row+1); i++)
            {
                for(j=0; j<column; j++)
                {
                    printf("%10s", result[index++]);
                }
                putchar(10);
            }
*/
            // 释放内存
            sqlite3_free_table(result);
            break;
        case 3:
            // 查找全部
            sprintf(sql, "select * from staff;");
            if (sqlite3_get_table(db, sql, &result, &row, &column, &errmsg) != SQLITE_OK)
            {
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                return NULL;
            }
            if (0 == row)
            {
                // 查找失败
                recv_info.cmd = 0;
            }
            else
            {
                // 查找成功
                recv_info.cmd = 1;
                recv_info.row = row;
                // 发送数据给客户端
                if(send(newfd, &recv_info, sizeof(recv_info), 0) < 0)
                {
                    ERR_MSG("send");
                    return NULL;
                }
                // 循环收发查询到的数据
                for(i=0; i<row; i++)
                {
                    // 从文件描述符中读取数据
                    memset(&data, 0, sizeof(data));
                    res = recv(newfd, &data, sizeof(data), 0);
                    if (res < 0)
                    {
                        ERR_MSG("recv");
                        return (void*)-1;
                    }
                    else if (0 == res)
                    {
                        // 此时客户端关闭
                        fprintf(stderr, "[%s:%d] 客户端关闭\n", \
                            inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
                        return NULL;
                    }
                    // 判断数据包格式是否正确
                    if (3 == data.cmdtype)
                    {
                        recv_info.cmd = 1;
                        sprintf(recv_info.buf, "%s   %s   %s   %s   %s   %s   %s   %s   %s   %s   %s", \
                            result[(i+1)*column+0], result[(i+1)*column+1], result[(i+1)*column+2], result[(i+1)*column+3], result[(i+1)*column+4], \
                            result[(i+1)*column+5], result[(i+1)*column+6], result[(i+1)*column+7], result[(i+1)*column+8], \
                            result[(i+1)*column+9], result[(i+1)*column+10]);
                    }
                    // 发送数据给客户端
                    if(send(newfd, &recv_info, sizeof(recv_info), 0) < 0)
                    {
                        ERR_MSG("send");
                        return NULL;
                    }
                }
                recv_info.cmd = 2;      // 回复传输完毕
            }
            // 释放内存
            sqlite3_free_table(result);
            break;
        case 4:
            // 修改
            // 判断用户是否已经存在
            if(0 == do_judge(db, data.st.num, NULL, NULL, 1))
            {
                // 如果用户不存在则失败
                recv_info.cmd = 0;      // 回复命令码：失败
                break;
            }
            time(&local_time);      // 记录当前时间
            ltime = localtime(&local_time); // 转换格式
            switch (data.buf[0])
            {
                case 'N':
                    // 修改姓名
                    sprintf(sql, "update staff set name=\'%s\' where num=%d", data.st.name, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的姓名为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.name);
                    break;
                case 'A':
                    // 修改年龄
                    sprintf(sql, "update staff set age=%d where num = %d", data.st.age, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的年龄为%d", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.age);
                    break;
                case 'F':
                    // 修改家庭住址
                    sprintf(sql, "update staff set address=\'%s\' where num=%d", data.st.address, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的家庭住址为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.address);
                    
                    break;
                case 'T':
                    // 修改电话
                    sprintf(sql, "update staff set tel_num=\'%s\' where num=%d", data.st.tel_num, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的电话为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.tel_num);
                    break;
                case 'P':
                    // 修改职位
                    sprintf(sql, "update staff set position=\'%s\' where num=%d", data.st.position, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的职位为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.position);
                    break;
                case 'S':
                    // 修改工资
                    sprintf(sql, "update staff set salary=%d where num=%d", data.st.salary, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的工资为%d", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.salary);
                    break;
                case 'D':
                    // 修改入职日期
                    sprintf(sql, "update staff set date=\'%s\' where num=%d", data.st.date, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的入职日期为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.date);
                    break;
                case 'G':
                    // 修改评级
                    sprintf(sql, "update staff set grade=%d where num=%d", data.st.grade, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的评级为%d", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.grade);
                    break;
                case 'W':
                    // 修改密码
                    sprintf(sql, "update staff set password=\'%s\' where num=%d", data.st.password, data.st.num);
                    sprintf(buf, "%d-%d-%d %d:%d:%d ---%s---修改工号为%d的密码为%s", ltime->tm_year+1900, ltime->tm_mon+1, \
                        ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.num, data.st.password);
                    break;
            }
            if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
            {
                recv_info.cmd = 0;      // 回复命令码：失败
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                break;
            }
            else
            {
                recv_info.cmd = 1;      // 回复命令码：成功
                do_history(db, buf);
            }
            break;
        case 5:
            // 添加用户
            // 判断用户是否已经存在
            if(1 == do_judge(db, data.st.num, &data.st.name, NULL, 3))
            {
                // 说明存在
                recv_info.cmd = 0;      // 回复命令码：失败
                break;
            }
            /* (num int, type int, name char PRIMARY KEY, password char, \
                age int, tel_num int, address char, position char, \
                date char, grade int, salary int) */
            sprintf(sql, "insert into staff values (%d, %d, \"%s\", \"%s\", \
                %d, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d);", data.st.num, data.st.type, \
                data.st.name, data.st.password, data.st.age, data.st.tel_num, \
                data.st.address, data.st.position, data.st.date, data.st.grade, data.st.salary);
            if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
            {
                recv_info.cmd = 0;      // 回复命令码：失败
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                break;
            }
            else
            {
                printf("name = %s\n", data.st.name);
                // 添加成功
                recv_info.cmd = 1;
                time(&local_time);      // 记录当前时间
                ltime = localtime(&local_time); // 转换格式
                sprintf(sql, "%d-%d-%d %d:%d:%d---%s---添加了%s用户", ltime->tm_year+1900, ltime->tm_mon+1, \
                    ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.name);
                do_history(db, sql);
            }
            break;
        case 6:
            // 删除用户
            if(0 == do_judge(db, data.st.num, &data.st.name, NULL, 0))
            {
                recv_info.cmd = 0;      // 回复命令码：失败
                break;
            }
            sprintf(sql, "delete from staff where num=%d and name=\"%s\";", \
                data.st.num, data.st.name);
            if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
            {
                recv_info.cmd = 0;      // 回复命令码：失败
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                break;
            }
            else
            {
                // 删除成功
                recv_info.cmd = 1;
                time(&local_time);      // 记录当前时间
                ltime = localtime(&local_time); // 转换格式
                sprintf(sql, "%d-%d-%d %d:%d:%d---%s---删除了%s用户", ltime->tm_year+1900, ltime->tm_mon+1, \
                    ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, name_pre, data.st.name);
                do_history(db, sql);
            }
            break;
        case 7:
            // 查询历史记录
            sprintf(sql, "select * from history;");
            if (sqlite3_get_table(db, sql, &result, &row, &column, &errmsg) != SQLITE_OK)
            {
                fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
                return NULL;
            }
            /***********测试使用**************/
/*
            printf("row=%d column=%d\n", row, column);
            index = 0;
            for(i=0; i<(row+1); i++)
            {
                for(j=0; j<column; j++)
                {
                    printf("%10s", result[index++]);
                }
                putchar(10);
            }
*/
            if (0 == row)
            {
                // 查找失败
                recv_info.cmd = 0;
            }
            else
            {
                // 查找成功
                // 循环收发查询到的数据
                for(i=1; i<(row+1); i++)
                {
                    recv_info.cmd = 1;
                    sprintf(recv_info.buf, "%s", result[i]);
                    
                    // 发送数据给客户端
                    if(send(newfd, &recv_info, sizeof(recv_info), 0) < 0)
                    {
                        ERR_MSG("send");
                        return NULL;
                    }

                    // 从文件描述符中读取数据
                    memset(&data, 0, sizeof(data));
                    res = recv(newfd, &data, sizeof(data), 0);
                    if (res < 0)
                    {
                        ERR_MSG("recv");
                        return (void*)-1;
                    }
                    else if (0 == res)
                    {
                        // 此时客户端关闭
                        fprintf(stderr, "[%s:%d] 客户端关闭\n", \
                            inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
                        return NULL;
                    }
                } 
            }
            recv_info.cmd = 2;      // 回复传输完毕
            // 释放内存
            sqlite3_free_table(result);
            break;
        case 8:
            // 退出
            break;
        default:
            printf("输入的选项有误，请重新输入\n");
            break;
        }
        // 发送数据给客户端
        if(send(newfd, &recv_info, sizeof(recv_info), 0) < 0)
		{
			ERR_MSG("send");
			return NULL;
		}
    }
}

// 写入历史记录中 历史记录仅存放修改用户信息的记录
int do_history(sqlite3 *db, const char sql[])
{
    char *errmsg = NULL;
    char arr[512] = {0};
    sprintf(arr, "insert into history values ('%s')", sql);
    if (sqlite3_exec(db, arr, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
        return 0;
    }
    printf("%s\n", sql);
    return 1;
}

// 判断是否有该用户 0:根据工号和名字 1:仅根据工号   2:根据密码和姓名    3:工号或名字
int do_judge(sqlite3 *db, int num, char (* name)[32], char (* password)[6], int flag)
{
    char sql[256] = {0};
    char **result, *errmsg = NULL;
    int row, column;
    // 判断是什么需求
    if(0 == flag)
    {
        sprintf(sql, "select * from staff where num=%d and name=\'%s\';", num, name[0]);
    }
    else if (1 == flag)
    {
        sprintf(sql, "select * from staff where num=%d;", num);
    }
    else if(2 == flag)
    {
        sprintf(sql, "select * from staff where name=\'%s\' and password=\'%s\';", name[0], password[0]);
    }
    else if(3 == flag)
    {
        sprintf(sql, "select * from staff where num=%d or name=\'%s\';", num, name[0]);
    }
    // 搜索
    if (sqlite3_get_table(db, sql, &result, &row, &column, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
        return 0;
    }
/*
    printf("row=%d column=%d\n", row, column);
    index = 0;
    for(i=0; i<(row+1); i++)
    {
        for(j=0; j<column; j++)
        {
            printf("%10s", result[index++]);
        }
        putchar(10);
    }
*/
    // 释放内存
    sqlite3_free_table(result);
    // 返回搜索结果
    if(0 == row)
    {
        // 没有找到
        return 0;
    }
    else
    {
        // 成功找到
        return 1;
    }
}