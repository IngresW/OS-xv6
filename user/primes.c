#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int pipeAry[12][2]; // 最多支持 12 层筛选, 管道包含读和写两端
/* prime 函数用于处理第 layer 层的素数筛选。如果到达第 11 层，函数退出。
 * 定义 EOF 和 num，分别用于表示结束标志和存放读出来的数。
 * 从当前层的管道读取数据。如果读取到 EOF，则表示没有更多数据，函数退出。
 * 如果读取到的不是 EOF，则将其视为当前层的最小素数，并打印。创建下一层的管道。
 * 子进程关闭当前层的写管道，递归调用 prime 处理下一层。
 * 父进程关闭当前层的写管道和下一层的读管道，然后读取数据。
 * 如果数据不是当前层的最小素数的倍数，则写入下一层的管道。最后写入 EOF，关闭管道并等待子进程结束，释放内存。
*/
void prime(int layer){
    if(layer==11){
        exit(0);
    }
    int *EOF=(int*)malloc(sizeof(int));
    *EOF=-1;
    int *num=(int*) malloc(sizeof(int));//存放读出来的数
    int min_num=0;
    read(pipeAry[layer][0],num,sizeof(int));//读数据
    if(*num==*EOF){
        fprintf(1,"over!");
        exit(0);
    }
    else{
        min_num=*num;
        fprintf(1,"prime %d\n",min_num);
        if(pipe(pipeAry[layer+1])==-1){
           //若创建右侧管道失败
           exit(2);
        }
    }
    int pid=fork();
    if(pid<0){
        exit(3);
    }
    if(pid==0){
        close(pipeAry[layer+1][1]);
        prime(layer+1);
        return;
    }
    else{
        close(pipeAry[layer][1]);
        close(pipeAry[layer+1][0]);
        while(read(pipeAry[layer][0],num,sizeof(int))&&*num!=*EOF){            
            if((*num)%min_num!=0){
                //加入右侧管道
                write(pipeAry[layer+1][1],num,sizeof(int));
            }
        }
        write(pipeAry[layer+1][1],EOF,sizeof(int));
        close(pipeAry[layer+1][1]);
        close(pipeAry[layer][0]);
        wait(0);
        free(num);
    }
    free(EOF);
}

int main(int argc,char *argv[])
{
    if(argc!=1){
        exit(1);
    }
    if(pipe(pipeAry[0])==-1){
        exit(2);
    }
    int pid=fork();
    if(pid<0){
        exit(3);
    }
    int *EOF=(int*)malloc(sizeof(int));
    *EOF=-1;
    /* 子进程关闭初始管道的写端，调用 prime 函数开始处理素数筛选 */
    if(pid==0){
        close(pipeAry[0][1]);
        prime(0);
        close(pipeAry[1][1]);
    }
    /* 父进程关闭初始管道的读端，将2到35的数写入管道，然后写入 EOF，关闭管道并等待子进程结束。 */
    else{
        close(pipeAry[0][0]);
        int status=0;
        //放入2-35
        for(int i=2;i<=35;++i){
            write(pipeAry[0][1],&i,sizeof(i));
        }
        write(pipeAry[0][1],EOF,sizeof(int));
        close(pipeAry[0][1]);
        wait(&status);
    }
    free(EOF);
    exit(0);
}