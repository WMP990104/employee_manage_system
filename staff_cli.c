#include "staff_cli.h"

char name_pre[32] = {0};    // 当前登录的账户

int main(int argc, const char *argv[])
{
	// 判断 如果传进来的参数没有三个，则打印参数不够
	if (argc < 3)
	{
		printf("请检查传入的参数:IP and PORT\n");
		return -1;
	}

	//创建套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (EOF == sfd)
	{
		ERR_MSG("socket");
		return -1;
	}
	//设置结构体，填充服务器IP和端口
	int port = atoi(argv[2]); 	// 转换一下参数的类型
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(argv[1]);
	char name[32] = {0};		// 临时变量，用户缓存输入的数据
	int num = 0;				// 临时变量，用户缓存输入的数据

	//连接服务器
	if (EOF == connect(sfd, (struct sockaddr *)&sin, sizeof(sin)))
	{
		ERR_MSG("connect");
		return -1;
	}

	int res = 0;				// 记录返回值
	char sql[256] = {0}; 		// sqlite_exec语句
	int choice = 0;				// 选择

	// 登录环节循环
	while (1)
	{
	NLOGIN:
		menu1();
		scanf("%d", &choice);
		switch (choice)
		{
		case 1:
			// 管理员
			res = do_login(sfd, ADMIN);
			if (res > 0) // 成功
			{
				goto NADMIN;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			else
			{
				printf("登陆失败请重试\n");
				printf("input '#' to continue\n");
				while (getchar() != '#'); // 吸收垃圾字符
				goto NLOGIN;
			}
			break;
		case 2:
			// 普通用户
			res = do_login(sfd, STAFF);
			if (res > 0) // 成功
			{
				goto NSTAFF;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			else
			{
				printf("登陆失败请重试\n");
				printf("input '#' to continue\n");
				while (getchar() != '#'); // 吸收垃圾字符
				goto NLOGIN;
			}
			break;
		case 3:
			// 退出
			printf("正在退出员工原理系统......\n");
			goto END;
		default:
			printf("unknow choice, please try again\n");
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 吸收垃圾字符
			break;
		}
	}
	// 管理员选项菜单循环
	while (1)
	{
	NADMIN:
		menu_admin();
		scanf("%d", &choice);
		while (getchar() != '\n'); // 吸收垃圾字符
		switch (choice)
		{
		case 1:
			// 查询
			res = do_research(sfd, ADMIN);
			if (res > 0) // 成功
			{
				continue;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			else
			{
				printf("查询失败，没有此用户\n");
				printf("input '#' to continue\n");
				while (getchar() != '#'); // 吸收垃圾字符
				continue;
			}
			break;
		case 2:
			// 修改
			printf("请输入您要修改的员工工号:");
			scanf("%d", &num);
			while (getchar() != '\n'); // 吸收垃圾字符

			res = do_update(sfd, num);
			if (res > 0) // 成功
			{
				goto NADMIN;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败\n");
				goto END;
			}
			break;
		case 3:
			// 添加用户
			printf("***************热烈欢迎新员工***************\n");
			res = do_insert(sfd);
			if (res > 0) // 成功
			{
				goto NADMIN;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败\n");
				goto END;
			}
			break;
		case 4:
			// 删除用户
			res = do_delete(sfd);
			if (res > 0) // 成功
			{
				goto NADMIN;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			break;
		case 5:
			// 查询历史记录
			res = do_review(sfd);
			if (res > 0) // 成功
			{
				goto NADMIN;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			break;
		case 6:
			// 退出
			goto NLOGIN;
			break;
		default:
			printf("unknow choice, please try again\n");
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 吸收垃圾字符
			continue;
		}
	}
	// 员工选项菜单循环
	while (1)
	{
	NSTAFF:
		menu_staff();
		scanf("%d", &choice);
		while (getchar() != '\n'); // 吸收垃圾字符
		switch (choice)
		{
		case 1:
			// 查询
			res = do_research(sfd, STAFF);
			if (res > 0) // 成功
			{
				continue;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败");
				goto END;
			}
			else
			{
				printf("查询失败，没有此用户\n");
				printf("input '#' to continue\n");
				while (getchar() != '#'); // 吸收垃圾字符
				continue;
			}
			break;
		case 2:
			// 修改
			printf("请输入您要修改的员工工号:");
			scanf("%d", &num);
			while (getchar() != '\n'); // 吸收垃圾字符
			res = do_update(sfd, num);
			if (res > 0) // 成功
			{
				goto NSTAFF;
			}
			else if (res == 0)
			{
				printf("服务器已关闭\n");
				goto END;
			}
			else if (res == -1)
			{
				printf("与服务器通信失败\n");
				goto END;
			}
			break;
		case 3:
			// 退出
			goto NLOGIN;
		default:
			printf("unknow choice, please try again\n");
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 输入'#'继续
			continue;
		}
	}
END:
	close(sfd);
	return 0;
}

// 登录函数
int do_login(const int sfd, const int type)
{
	char name[32] = {0};
	char password[6] = {0};
	struct trans dataToSer;
	struct recvs recFromSer;
	memset(&dataToSer, 0, sizeof(dataToSer));
	memset(&recFromSer, 0, sizeof(recFromSer));

	dataToSer.cmdtype = 1;	   // 设为登录命令
	dataToSer.st.type = type; // 设置身份

	printf("请输入用户名：");
	scanf("%s", name);
	strncpy(dataToSer.st.name, name, 32);

	printf("请输入密码(6位)：");
	scanf("%s", password);
	strncpy(dataToSer.st.password, password, 6);
	// 发送数据包
	if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
	{
		printf("send data to ser failed\n");
		return -1; // 发送数据失败
	}
	// 接收数据包
	memset(&recFromSer, 0, sizeof(recFromSer));
	int ret = 0; // 用于记录recv到的字节数
	ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
	if (ret <= 0)
	{
		return ret;
	}
	// 判断是否登陆成功
	if (recFromSer.cmd == 0)
		return -2; // 登陆失败
	else
	{
		strncpy(name_pre, dataToSer.st.name, 32);
		// printf("name_pre = %s\n", name_pre);
		return 1;
	}
	return 1;
}

// 查询函数
int do_research(const int sfd, const int type)
{
	int choice = 0, flag = 1;
	int i =0;				// 循环变量
	int ret = 0; // 用于记录recv到的字节数
	struct trans dataToSer;
	struct recvs recFromSer;

	dataToSer.cmdtype = 1;	  // 设为登录命令
	dataToSer.st.type = type; // 设置身份
	while (1)
	{
		if (type == STAFF)
		{
			dataToSer.cmdtype = 2; // 设置命令码
			strncpy(dataToSer.st.name, name_pre, 32);
			printf("name = %s\n", dataToSer.st.name);
			// 发送数据包
			if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
			{
				printf("send data to ser failed\n");
				return -1; // 发送数据失败
			}
			// 接收数据包
			ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
			if (ret <= 0)
			{
				return ret;
			}
			// 判断是否查询成功
			if (recFromSer.cmd == 0)
				return -2; // 查找失败
			else
			{
				printf("工号 用户类型 姓名 密码 年龄 电话 地址 职位 入职年月 等级 工资\n");
				printf("===========================================================\n");
				printf("%s\n", recFromSer.buf);
			}
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 输入'#'继续
			return 1;
		}
		memset(&dataToSer, 0, sizeof(dataToSer));
		menu_search(); // 打印查询菜单
		/* 1：按人名查找  	2：查找所有 	3：退出 */
		scanf("%d", &choice);
		while (getchar() != '\n'); // 吸收垃圾字符
		
		switch (choice)
		{
		case 1:
			// 按人名查找
			// 封装数据包
			dataToSer.cmdtype = 2; // 设置命令码
			printf("请输入您要查找的用户名：");
			scanf("%s", dataToSer.st.name);
			while (getchar() != '\n'); // 吸收垃圾字符

			// 发送数据包
			if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
			{
				printf("send data to ser failed\n");
				return -1; // 发送数据失败
			}
			// 接收数据包
			ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
			if (ret <= 0)
			{
				return ret;
			}
			// 判断是否查询成功
			if (recFromSer.cmd == 0)
				return -2; // 查找失败
			else
			{
				printf("工号 用户类型 姓名 密码 年龄 电话 地址 职位 入职年月 等级 工资\n");
				printf("===========================================================\n");
				printf("%s\n", recFromSer.buf);
			}
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 输入'#'继续
			break;
		case 2:
			// 查找所有
			// 封装数据包
			dataToSer.cmdtype = 3; // 设置命令码

			// 发送数据包
			if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
			{
				printf("send data to ser failed\n");
				return -1; // 发送数据失败
			}
			// 接收数据包
			ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
			if (ret <= 0)
			{
				printf("ret = %d\n", ret);
				return ret;
			}
			// 判断是否查询成功
			if (recFromSer.cmd == 0)
				return -2; // 查找失败
			else
			{
				// 循环接收发送接收数据包
				for (i=0; i<recFromSer.row; i++)
				{
					// 发送数据包
					if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
					{
						printf("send data to ser failed\n");
						return -1; // 发送数据失败
					}
					// 接收数据包
					ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
					if (ret <= 0)
					{
						printf("ret = %d\n", ret);
						return ret;
					}
					if (flag)
					{
						printf("工号 用户类型 姓名 密码 年龄 电话 地址 职位 入职年月 等级 工资\n");
						printf("===========================================================\n");
						flag--;
					}
					printf("%s\n", recFromSer.buf);
					printf("===========================================================\n");
					
				}
				flag = 1;
				ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
				if (ret <= 0)
				{
					printf("ret = %d\n", ret);
					return ret;
				}
				// 判断是否查询成功
				if (2 == recFromSer.cmd)
				{
					// 读取完毕
					printf("input '#' to continue\n");
					while (getchar() != '#'); // 输入'#'继续
					break;
				}
			}
			continue;
		case 3:
			// 退出
			return 1;
		default:
			printf("unknow choice, please try again\n");
			printf("input '#' to continue\n");
			while (getchar() != '#'); // 输入'#'继续
			continue;
		}
	}
	return 1;
}

// 添加用户函数
int do_insert(const int sfd)
{
	char arr[32] = {0};
	int ret = 0;
	struct trans dataToSer;
	struct recvs recFromSer;
	memset(&dataToSer, 0, sizeof(dataToSer));
	memset(&recFromSer, 0, sizeof(recFromSer));
	while(1)
	{
		dataToSer.cmdtype = 5;	  // 设为添加成员命令
		printf("请输入工号：");
		scanf("%d", &dataToSer.st.num);
		printf("工号信息一旦录入无法更改，请确认您所输入的是否正确！(Y/N)");
		scanf("%s", arr);
		if (strncasecmp(arr, "y", 1))
			continue;
		else
		{
			printf("请输入用户名：");
			scanf("%s", dataToSer.st.name);

			printf("请输入用户密码：(6位)");
			scanf("%s", arr);
			strncpy(dataToSer.st.password, arr, 6);

			printf("请输入年龄");
			scanf("%d", &dataToSer.st.age);

			printf("请输入电话：");
			scanf("%s", dataToSer.st.tel_num);

			printf("请输入家庭住址：");
			scanf("%s", dataToSer.st.address);

			printf("请输入职位：");
			scanf("%s", dataToSer.st.position);

			printf("请收入入职日期(格式：0000.00.00)：");
			scanf("%s", dataToSer.st.date);
			do
			{
				printf("请输入评级(1~5,5为最高，新员工为 1)：");
				scanf("%d", &dataToSer.st.grade);
			}while(dataToSer.st.grade>5 || dataToSer.st.grade<1);
			printf("请输入工资：");
			scanf("%d", &dataToSer.st.salary);
			printf("是否为管理员：(Y/N)");
			scanf("%s", arr);
			if ( 0 == strncasecmp(arr, "y", 1))
				dataToSer.st.type = 0;
			else
				dataToSer.st.type = 1;
		}
		if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
		{
			printf("send data to ser failed\n");
			return -1; // 发送数据失败
		}
		// 接收数据包
		ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
		if (ret <= 0)
		{
			printf("ret = %d\n", ret);
			return ret;
		}
		// 判断是否添加成功
		if (1 == recFromSer.cmd)
		{
			// 成功
			printf("数据库修改成功!是否继续添加员工:(Y/N)");
			scanf("%s", arr);
			if ( 0 == strncasecmp(arr, "y", 1))
				continue;
			else
				break;
		}
		else
		{
			printf("数据库修改失败!工号已存在\n");
			printf("input '#' to exit\n");
			while (getchar() != '#'); // 吸收垃圾字符
			break;
		}
	}
	return 1;
}

// 删除用户函数
int do_delete(const int sfd)
{
	char arr[32] = {0};
	int ret = 0;
	struct trans dataToSer;
	struct recvs recFromSer;
	while(1)
	{
		dataToSer.cmdtype = 6;	  // 设为删除成员命令
		printf("请输入要删除的用户工号：");
		scanf("%d", &dataToSer.st.num);
		printf("请输入要删除的用户名：");
		scanf("%s", arr);
		strncpy(dataToSer.st.name, arr, 32);

		// 发送数据包
		if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
		{
			printf("send data to ser failed\n");
			return -1; // 发送数据失败
		}

		// 接收数据包
		ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
		if (ret <= 0)
		{
			printf("ret = %d\n", ret);
			return ret;
		}
		// 判断是否删除成功
		if (1 == recFromSer.cmd)
		{
			// 成功
			printf("删除用户\"%s\"删除成功!工号为%d\n", dataToSer.st.name, dataToSer.st.num);
			printf("input '#' to exit\n");
			while (getchar() != '#'); // 吸收垃圾字符
			break;
		}
		else
		{
			printf("数据库修改失败!请检查用户数据\n");
			printf("input '#' to exit\n");
			while (getchar() != '#'); // 吸收垃圾字符
			break;
		}
		
	}
	return 1;
}

// 修改信息函数
int do_update(const int sfd, int num)
{
	char arr[64] = {0};
	int ch = 0;
	int ret = 0;
	struct trans dataToSer;
	struct recvs recFromSer;
	memset(&dataToSer, 0, sizeof(dataToSer));
	memset(&recFromSer, 0, sizeof(recFromSer));
	dataToSer.st.num = num;
	
AGAIN:
	menu_update();
	dataToSer.cmdtype = 4;		// 设为修改命令
	printf("请输入您的选择（数字）>>");
	scanf("%d", &ch);
	while (getchar() != '\n'); // 吸收垃圾字符
	switch (ch)
	{
	case 1:
		// 修改姓名
		dataToSer.buf[0] = 'N';		// 修改姓名
		printf("请输入姓名：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.name, arr, 32);
		break;
	case 2:
		// 修改年龄
		dataToSer.buf[0] = 'A';		// 修改年龄
		printf("请输入年龄：");
		scanf("%d", &dataToSer.st.age);
		while (getchar() != '\n'); // 吸收垃圾字符
		break;
	case 3:
		// 修改家庭住址
		dataToSer.buf[0] = 'F';		// 修改家庭住址
		printf("请输入家庭住址：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.address, arr, 64);
		break;
	case 4:
		// 修改电话
		dataToSer.buf[0] = 'T';		// 修改电话
		printf("请输入电话：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.tel_num, arr, 12);
		break;
	case 5:
		// 修改职位
		dataToSer.buf[0] = 'P';		// 修改职位
		printf("请输入职位：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.position, arr, 32);
		break;
	case 6:
		// 修改工资
		dataToSer.buf[0] = 'S';		// 修改工资
		printf("请输入工资：");
		while (getchar() != '\n'); // 吸收垃圾字符
		scanf("%d", &dataToSer.st.salary);
		break;
	case 7:
		// 修改入职日期
		dataToSer.buf[0] = 'D';		// 修改入职日期
		printf("请输入入职日期：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.date, arr, 32);
		break;
	case 8:
		// 修改评级
		dataToSer.buf[0] = 'G';		// 修改评级
		printf("请输入评级：");
		scanf("%d", &dataToSer.st.grade);
		while (getchar() != '\n'); // 吸收垃圾字符
		break;
	case 9:
		// 修改密码
		dataToSer.buf[0] = 'W';		// 修改密码
		printf("请输入密码：");
		scanf("%s", arr);
		while (getchar() != '\n'); // 吸收垃圾字符
		strncpy(dataToSer.st.password, arr, 32);
		break;
	case 10:
		// 退出
		return 1;
	default:
		printf("unknow choice, please try again\n");
		printf("input '#' to continue\n");
		while (getchar() != '#'); // 吸收垃圾字符
		goto AGAIN;
	}
	// 收发数据
	if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
	{
		printf("send data to ser failed\n");
		return -1; // 发送数据失败
	}
	// 接收数据包
	ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
	if (ret <= 0)
	{
		printf("ret = %d\n", ret);
		return ret;
	}
	// 判断是否添加成功
	if (1 == recFromSer.cmd)
	{
		// 成功
		printf("数据库修改成功!修改结束.\n");
	}
	else
	{
		printf("数据库修改失败!\n");
	}
	printf("input '#' to exit\n");
	while (getchar() != '#'); // 吸收垃圾字符
	return 1;
}

// 查询历史记录
int do_review(const int sfd)
{
	int flag = 1;			//	标志位
	int ret = 0;			// 用于记录recv到的字节数
	struct trans dataToSer;
	struct recvs recFromSer;
	memset(&dataToSer, 0, sizeof(dataToSer));
	memset(&recFromSer, 0, sizeof(recFromSer));

	dataToSer.cmdtype = 7;	// 设为查询历史记录命令
	while(1)
	{
		// 发送数据包
		if (send(sfd, &dataToSer, sizeof(dataToSer), 0) < 0)
		{
			printf("send data to ser failed\n");
			return -1; // 发送数据失败
		}
		// 接收数据包
		ret = recv(sfd, &recFromSer, sizeof(recFromSer), 0);
		if (ret <= 0)
		{
			printf("ret = %d\n", ret);
			return ret;
		}
		// 判断是否查询成功
		if (recFromSer.cmd == 0)
			return -2; // 查找失败
		else if (2 == recFromSer.cmd)
		{

			printf("successfully reviewed, please input '#' to continue\n");
			while (getchar() != '#'); // 输入'#'继续
			break;
		}
		else
		{
			if (flag)
			{
				printf("历史记录：\n");
				flag--;
			}
			printf("%s\n", recFromSer.buf);
		}
	}
	flag = 1;
	return 1;
}