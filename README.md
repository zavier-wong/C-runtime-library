# C-runtime-library　Ｃ运行库
A cross platform library support for C runtime. 为C提供运行时环境的迷你运行库。

代码具备良好的跨平台能力，可在64位,32位的 `Windows` 以及 `Linux` 上编译成运行库。

I/O部分采用了缓冲，实现了高效的输入输出。

本项目已经编写好了编译脚本，在 `Windows` 平台运行 `windows.bat` 编译，在 `Linux` 平台运行 `linux.sh` 编译。

在main.cpp里测试运行时环境。

下面是整个库的接口

``` cpp
//----------------------------------------------------------
//  C Run Time Library Head
//
//      It contains all constant definitions, 
//  macro definitions and function declarations 
//  required to use the CRT.
//
//-----------------------------------------------------------
#ifndef CRT_H
#define CRT_H

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32
#include <Windows.h>
#endif

//stddef.h
#ifndef NULL
#define NULL (0)
#endif

#ifdef __linux
#if (__SIZEOF_POINTER__ == 4)
typedef unsigned int size_t;
#else
typedef unsigned long int size_t;
#endif
#endif

#if defined(WIN32) || (__SIZEOF_POINTER__==4)
typedef int ssize_t;
#else
typedef long long int ssize_t;
#endif

#define EOF (-1)

//entry.h
void exit(int);


//string.h
int strcmp(const char*,const char*);
char* strcpy(char*,const char*);
char* strcat(char*,const char*);
unsigned strlen(const char *);
char* itoa(int,char*,int);
int ftoa(char*,float,int);
void* memset(void*,int,size_t );

void* _memcpy(void*,void*,size_t);

//heap.h
void free(void*);
void* malloc(size_t);
int crt_heap_init();


//stdio.h
#define BUFFSIZE 512

//FILE MODE
#define FILE_READ       0
#define FILE_WRITE      1
#define FILE_RDWR       2
//BUFFER MODE
#define BUFF_LINE       0
#define BUFF_FULL       1
#define BUFF_NO         2

typedef struct io_file{
    int mode;               //file mode: read,write,all
    int buffmode;           //buffer mode;
    int flags;              //flie flags
    int access;             //file access
#ifdef WIN32
    HANDLE file;            //file description or handle
#else
    int file;               //file description or handle
#endif
    struct io_file* next;   //next FILE
    struct io_file* prev;   //prev FILE
    size_t buffsize;        //buffer size
    char* read_base;        //read buffer base pointer
    char* read_ptr;         //reading pointer
    int read_cnt;           //read buffer left 
    char* write_base;       //write buffer base pointer
    char* write_ptr;        //writing pointer
    int write_cnt;          //write buffer left
} FILE;

FILE* _buitin_file_func(int);

#define stdin   (_buitin_file_func(0))
#define stdout  (_buitin_file_func(1))
#define stderr  (_buitin_file_func(2))

#ifdef __linux
#define O_RDONLY        00
#define O_WRONLY        01
#define O_RDWR          02
#define O_CREAT         0100
#define O_TRUNC         01000
#define O_APPEND        02000

#if __SIZEOF_POINTER__ == 4
//32bit variable argmuent
typedef char* va_list;
#define _INTSIZEOF(n) ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1)  )
#define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v)  )
#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t))  )
#define va_end(ap) ( ap = (va_list)0  )

#elif __SIZEOF_POINTER__ == 8

//64bit variable argmuent
typedef struct __va_list_tag{
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list;

#define va_start(ap,v) do{              \
    void *rbp=0;                        \
    asm("movq %%rbp,%0  \n":"=r"(rbp)); \
    ap.overflow_arg_area=(char*)rbp;    \
    ap.reg_save_area=(char*)&v+40;      \
    ap.fp_offset=0;                     \
    ap.gp_offset=0;                     \
}while(0)

#define va_arg(ap,t)                    \
    ((ap.gp_offset<48)?                 \
    (ap.gp_offset+=8,                   \
    *(t*)((char*)ap.reg_save_area+ap.gp_offset))\
    :(((char*)ap.overflow_arg_area),    \
    *(t*)ap.overflow_arg_area))

#define va_end(ap)                      \
    (ap.gp_offset=0,                    \
    ap.fp_offset=0,                     \
    ap.overflow_arg_area=0,             \
    ap.reg_save_area=0 )        

#endif

#endif
int crt_io_init();
int crt_io_fini();
FILE* fopen(const char*,const char*);
size_t fwrite(const void*,size_t,size_t,FILE*);
size_t fread(void *,size_t,size_t,FILE*);
int fclose(FILE*);
int fseek(FILE*,int,int);
int fflush(FILE*);
int fputc(int,FILE*);
int fputs(const char *,FILE*);
int fgetc(FILE*);
char* fgets(char*,int,FILE*);
int putchar(int);
int puts(const char *); 
int getchar();
char* gets(char*);
int vprintf(FILE*,const char*,va_list);
int printf(const char*, ...);
int fprintf(FILE*,const char*, ...);


#ifdef __cplusplus
}
#endif

#endif  //CRT_H
```
