#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10

char args[MAXARGS][50];
char jump[] = " \n";  //切分命令时跳过空格和\n

int getcmd(char *buf, int nbuf);
void cut(char *cmd, char* argv[],int* argc);
void runcmd(char*argv[],int argc);
void pipe_func(char*argv[],int argc);

//源于sh.c，用于取得命令，存放在buf中
int
getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

//将命令切分成多个字符串
void
cut(char *cmd, char *argv[], int *argc)
{
    int i, j;
    i = 0;
    for (j = 0; cmd[j] != '\n' && cmd[j] != '\0'; j++){
        //跳过前面的空格
        while(strchr(jump,cmd[j])){
            j++;
        }
        //存储字符串首地址
        argv[i]=cmd+j;
        i++;
        //找到字符串后面第一个空格或\n
        while(strchr(jump,cmd[j]) == 0){
            j++;
        }
        cmd[j]='\0';
    }
    argv[i]=0; //保证exec正确执行
    *argc=i;
}

void
runcmd(char *argv[], int argc)
{
    int i;
    for(i=1; i<argc; i++){
        if(!strcmp(argv[i],"|")){  //管道
            pipe_func(argv,argc);
        }
    }

    //不包含管道的命令执行
    for(i=1; i<argc; i++){
        if(!strcmp(argv[i], "<")){   //输入重定向
            close(0);
            open(argv[i+1], O_RDONLY);
            argv[i] = 0;
        }
        if(!strcmp(argv[i], ">")){   //输出重定向
            close(1);
            open(argv[i+1], O_CREATE|O_WRONLY);
            argv[i] = 0;
        }
    }
    exec(argv[0], argv);
}
 
void
pipe_func(char *argv[], int argc){
    int i;
    for(i=0; i<argc; i++){
        if(!strcmp(argv[i], "|")){  //去掉|，换成0，将命令分成左右两个
            argv[i] = 0;
            break;
        }
    }

    int p[2];  //管道
    pipe(p);

    if(fork() == 0){  //左命令
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        runcmd(argv, i);
    }
    else{             //右命令
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        runcmd(argv+i+1, argc-i-1);
    }
}

int 
main()
{
  static char buf[100];
  int fd;

  //源自sh.c，判断开始0,1,2是否打开
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }
    // Read and run input commands.
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        if (fork() == 0)
        {
            char* argv[MAXARGS];
            int argc=-1;
            cut(buf, argv,&argc);
            runcmd(argv,argc);
        }
        wait(0);
    }
    exit(0);
}