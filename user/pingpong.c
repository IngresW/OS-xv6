#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
    /* pipe(p) 会创建两个文件描述符p[0]和p[1]，分别用于读和写。如果管道创建失败，程序会报错并退出。*/
	int pid;
	int p[2];
	pipe(p);

    /* 使用 fork() 创建一个子进程。fork() 返回0表示当前是子进程。*/
    /* 获取子进程的 PID。 数组buf 用于存储消息。
    * 使用 read(p[0], buf, 1) 从管道 p 读取父进程发送的消息。如果读取失败，打印错误信息并退出。
    * 使用 write(p[1], buf, 1) 将消息写回管道 p，返回给父进程。如果写入失败，打印错误信息并退出。*/
	if (fork() == 0)
	{
		pid = getpid();
		char buf[2];
		if (read(p[0], buf, 1) != 1)
		{
			fprintf(2, "failed to read in child\n");
			exit(1);
		}
		printf("%d: received ping\n", pid);
		if(write(p[1], buf, 1) != 1)
		{
			fprintf(2, "failed to write in child\n");
			exit(1);
		}
	}
   /* 获取父进程的 PID。
    * 字符数组 info 包含要发送的消息 'a'。字符数组 buf 用于存储返回的消息
    * 使用 write(p[1], info, 1) 将消息写入管道 p，发送给子进程。
    * 使用 read(p[0], buf, 1) 从管道 p 读取子进程返回的消息。 */ 
    else{
		pid = getpid();
		char info[2] = "a";
		char buf[2];
		buf[1] = 0;
		if (write(p[1], info, 1) != 1)
		{
			fprintf(2, "failed to write in parent\n");
			exit(1);
		}
		// wait for child to receive ping
		if(read(p[0], buf, 1) != 1){
			fprintf(2, "failed to read in parent\n");
			exit(1);
		}
		printf("%d: received pong\n", pid);
	}
    close(p[0]);
    close(p[1]);
    exit(0);
}