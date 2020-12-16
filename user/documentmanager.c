#include "stdio.h"

typedef struct super_block {
    int32_t magic_num;                  // 幻数
    int32_t free_block_count;           // 空闲数据块数
    int32_t free_inode_count;           // 空闲inode数
    int32_t dir_inode_count;            // 目录inode数
    uint32_t block_map[128];            // 数据块占用位图
    uint32_t inode_map[32];             // inode占用位图
} sp_block;

struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
};

struct dir_item {               // 目录项一个更常见的叫法是 dirent(directory entry)
    uint32_t inode_id;          // 当前目录项表示的文件/目录的对应inode
    uint16_t valid;             // 当前目录项是否有效 
    uint8_t type;               // 当前目录项类型（文件/目录）
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
};

//函数声明
void create_file();  //创建文件
void create_dir();   //创建文件夹
void read_dir();     //读取文件夹内容
void copy_file();    //复制文件
void open();         //打开系统
void shutdown();     //关闭系统
void init();         //每次打开系统后初始化某些变量

int main(){
    int 

    return 0;
}




void create_file(){
    if(open_disk() == -1){
        printf("Open disk failed!\n");
    }
}

void create_dir(){

}

void read_dir(){

}

void copy_file(){

}

void open(){

}

void shutdown(){

}

void init(){

}
