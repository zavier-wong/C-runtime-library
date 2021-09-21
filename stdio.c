//-----------------------------------
//  Standard Input and Output
//
//  the basic c run time library of I/O
//
//  note: The I/O are buffered;
//------------------------------------
#include "crt.h"

static FILE* file_head=NULL;
FILE* _buitin_file[3];

FILE* _buitin_file_func(int n){
    return _buitin_file[n];
}
//init FILE
static FILE* file_init(int* file,int buffmode,size_t buffsize,int mode,int flags,int access){
    FILE* newfile=(FILE*)malloc(sizeof(FILE));    
    memset(newfile,0,sizeof(FILE));
    newfile->buffmode=buffmode;
    newfile->mode=mode;
    newfile->flags=flags;
    newfile->access=access;
    newfile->buffsize=buffsize;
    newfile->read_cnt=0;
    newfile->write_cnt=0;
#ifdef WIN32
    newfile->file=(HANDLE)file;
#else
    newfile->file=(int)file;
#endif
    if(mode == FILE_READ){
        newfile->read_base=(char*)malloc(newfile->buffsize);
        newfile->read_ptr=newfile->read_base;
    }else if(mode == FILE_WRITE){
        newfile->write_base=(char*)malloc(newfile->buffsize);
        newfile->write_ptr=newfile->write_base;
    }else if(mode == FILE_RDWR){
        newfile->read_base=(char*)malloc(newfile->buffsize);
        newfile->read_ptr=newfile->read_base;
        newfile->write_base=(char*)malloc(newfile->buffsize);
        newfile->write_ptr=newfile->write_base;
    }
    newfile->next=file_head->next;
    newfile->prev=file_head;
    file_head->next=newfile;
    
    return newfile;
}

#ifdef WIN32
#include <Windows.h>

FILE* fopen(const char* filename,const char* mode){
    FILE* ret;
    HANDLE hFile=0;
    int access=0;
    int creation=0;
    int fmode=0;
    if(strcmp(mode,"w")==0){
        fmode=FILE_WRITE;
        access |= GENERIC_WRITE;
        creation=CREATE_ALWAYS;
    }
    if(strcmp(mode,"w+")==0){
        fmode=FILE_RDWR;
        access |= GENERIC_WRITE | GENERIC_READ;
        creation=CREATE_ALWAYS;
    }
    if(strcmp(mode,"r")==0){
        fmode=FILE_READ;
        access |= GENERIC_READ;
        creation|=OPEN_EXISTING;
    }
    if(strcmp(mode,"r+")==0){
        fmode=FILE_RDWR;
        access |= GENERIC_WRITE|GENERIC_READ;
        creation|=TRUNCATE_EXISTING;
    }
    if(strcmp(mode,"a")==0){
        fmode=FILE_RDWR;
        access |= GENERIC_WRITE|GENERIC_READ;
        creation=OPEN_ALWAYS ;
    }
    hFile=CreateFileA(filename,access,0,0,creation,0,0);
    if(hFile==INVALID_HANDLE_VALUE)
        return 0;
    ret=file_init((int*)hFile,BUFF_FULL,BUFFSIZE,fmode,creation,access);
    if(strcmp(mode,"a")==0)
         SetFilePointer(hFile,0,0,FILE_END);
    return ret;
}
//no buffer write
static ssize_t writen(FILE* stream,const void* buff,size_t n){
    if(n==0)
        return 0;
    if(stream->mode==FILE_READ)
        return EOF;
    ssize_t written=0;
    if(!WriteFile(stream->file,buff,n,&written,0))
        return EOF;
    return written;
}
//no buffer write
static ssize_t readn(FILE* stream,void* buff,size_t n){
    if(n==0)
        return 0;
    if(stream->mode==FILE_WRITE)
        return EOF;
    ssize_t read=0;
    if(!ReadFile(stream->file,buff,n,&read,0))
        return EOF;
    return read;
}

int fseek(FILE* stream,int offset,int mode){
    fflush(stream);
    return SetFilePointer((HANDLE)stream,offset,0,mode);
}

int crt_io_init(){
    file_head=(FILE*)malloc(sizeof(FILE));
    memset(file_head,0,sizeof file_head);
    
    HANDLE hFILE;
    hFILE=GetStdHandle(STD_INPUT_HANDLE);
    _buitin_file[0]=file_init((int*)hFILE,BUFF_LINE,BUFFSIZE,FILE_READ,OPEN_EXISTING,GENERIC_READ);
    hFILE=GetStdHandle(STD_OUTPUT_HANDLE);
    _buitin_file[1]=file_init((int*)hFILE,BUFF_LINE,BUFFSIZE,FILE_WRITE,CREATE_ALWAYS,GENERIC_WRITE);
    hFILE=GetStdHandle(STD_ERROR_HANDLE);
    _buitin_file[2]=file_init((int*)hFILE,BUFF_NO,0,FILE_WRITE,CREATE_ALWAYS,GENERIC_WRITE);
    return 0;
}

#else
//32bit System Call
#if __SIZEOF_POINTER__ == 4

static int open(const char *filename,int flag,int mode){
    int fd=0;
    asm("push %eax      \n"
        "push %ebx      \n"
        "push %ecx      \n"
        "push %edx      \n");
    asm("movl $5,%%eax  \n"
        "movl %1,%%ebx  \n"
        "movl %2,%%ecx  \n"
        "movl %3,%%edx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(fd)
        :"m"(filename),"m"(flag),"m"(mode));
    asm("pop %edx      \n"
        "pop %ecx      \n"
        "pop %ebx      \n"
        "pop %eax      \n");
    return fd;
}
static int write(int fd,const void *buff,size_t size){
    int ret=0;
    asm("push %eax      \n"
        "push %ebx      \n"
        "push %ecx      \n"
        "push %edx      \n");
    asm("movl $4,%%eax  \n"
        "movl %1,%%ebx  \n"
        "movl %2,%%ecx  \n"
        "movl %3,%%edx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(buff),"m"(size));
    asm("pop %edx      \n"
        "pop %ecx      \n"
        "pop %ebx      \n"
        "pop %eax      \n");
    return ret;
}
static int read(int fd,void *buff,size_t size){
    int ret=0;
    asm("push %eax      \n"
        "push %ebx      \n"
        "push %ecx      \n"
        "push %edx      \n");
    asm("movl $3,%%eax  \n"
        "movl %1,%%ebx  \n"
        "movl %2,%%ecx  \n"
        "movl %3,%%edx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(buff),"m"(size));
    asm("pop %edx      \n"
        "pop %ecx      \n"
        "pop %ebx      \n"
        "pop %eax      \n");
    return ret;
}
static int close(int fd){
    int ret=0;
    asm("push %eax      \n"
        "push %ebx      \n");
    asm("movl $6,%%eax  \n"
        "movl %1,%%ebx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(ret)
        :"m"(fd));
    asm("pop %ebx      \n"
        "pop %eax      \n");
    return ret;
}
static int lseek(int fd,int offset,int mode){
    int ret=0;
    asm("push %eax      \n"
        "push %ebx      \n"
        "push %ecx      \n"
        "push %edx      \n");
    asm("movl $19,%%eax  \n"
        "movl %1,%%ebx  \n"
        "movl %2,%%ecx  \n"
        "movl %3,%%edx  \n"
        "int $0x80      \n"
        "movl %%eax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(offset),"m"(mode));
    asm("pop %edx      \n"
        "pop %ecx      \n"
        "pop %ebx      \n"
        "pop %eax      \n");
    return ret;
}
//64bit System Call
#elif __SIZEOF_POINTER__ == 8

static long open(const char *filename,int flag,int mode){
    long fd=0;
    asm("movq $2,%%rax  \n"
        "movq %1,%%rdi  \n"
        "movq %2,%%rsi  \n"
        "movq %3,%%rdx  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(fd)
        :"m"(filename),"m"(flag),"m"(mode));
    return fd;
}
static long write(int fd,const void *buff,size_t size){
    long ret=0;
    asm("movq $1,%%rax  \n"
        "movq %1,%%rdi  \n"
        "movq %2,%%rsi  \n"
        "movq %3,%%rdx  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(buff),"m"(size));
    return ret;
}
static long read(int fd,void *buff,size_t size){
    long ret=0;
    asm("movq $0,%%rax  \n"
        "movq %1,%%rdi  \n"
        "movq %2,%%rsi  \n"
        "movq %3,%%rdx  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(buff),"m"(size));
    return ret;
}
static long close(int fd){
    long ret=0;
    asm("movq $3,%%rax  \n"
        "movq %1,%%rdi  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(ret)
        :"m"(fd));
    return ret;
}
static long lseek(int fd,int offset,int mode){
    long ret=0;
    asm("movq $8,%%rax  \n"
        "movq %1,%%rdi  \n"
        "movq %2,%%rsi  \n"
        "movq %3,%%rdx  \n"
        "syscall        \n"
        "movq %%rax,%0  \n"
        :"=m"(ret)
        :"m"(fd),"m"(offset),"m"(mode));
    return ret;
}

#endif

FILE* fopen(const char* filename,const char* mode){
    FILE* ret;
    int fd      =   -1;
    int flags   =   0;
    int access  =   0700;
    int fmode    =   0;
    if(strcmp(mode,"w")==0){
        fmode=FILE_WRITE;
        flags|=O_WRONLY | O_CREAT | O_TRUNC;
    }
    if(strcmp(mode,"w+")==0){
        fmode=FILE_RDWR;
        flags|=O_RDWR | O_CREAT | O_TRUNC;
    }
    if(strcmp(mode,"r")==0){
        fmode=FILE_READ;
        flags|=O_RDONLY;
    }
    if(strcmp(mode,"r+")==0){
        fmode=FILE_RDWR;
        flags|=O_RDWR | O_CREAT;
    }
    if(strcmp(mode,"a")==0){
        fmode=FILE_RDWR;
        flags|=O_RDWR | O_CREAT | O_APPEND; 
    }
    fd=open(filename,flags,access);
    ret=file_init((int*)fd,BUFF_FULL,BUFFSIZE,fmode,flags,access);
    return ret;
}
//no buffer write
static ssize_t writen(FILE* stream,const void* buff,size_t n){
    if(n==0)
        return 0;
    if(stream->mode==FILE_READ)
        return EOF;
    return write(stream->file,buff,n);
}
//no buffer read
static ssize_t readn(FILE* stream,void* buff,size_t n){
    if(n==0)
        return 0;
    if(stream->mode==FILE_WRITE)
        return EOF;
    return read(stream->file,buff,n);
}

int fseek(FILE* stream,int offset,int mode){
    fflush(stream);
    return lseek((int)stream,offset,mode);
}

int crt_io_init(){
    file_head=(FILE*)malloc(sizeof(FILE));
    memset(file_head,0,sizeof file_head);
    
    _buitin_file[0]=file_init((int*)0,BUFF_LINE,BUFFSIZE,FILE_READ,O_RDONLY,0700);
    _buitin_file[1]=file_init((int*)1,BUFF_LINE,BUFFSIZE,FILE_WRITE,O_WRONLY,0700);
    _buitin_file[2]=file_init((int*)2,BUFF_NO,0,FILE_WRITE,O_WRONLY,0700);
    return 0;
}
#endif

int crt_io_fini(){
    if(file_head==NULL)
        return -1;
    
    while(file_head->next){
        fclose(file_head->next);
    }
    free(file_head);
    return 0;
}
//read from FILE buffer
static ssize_t read_buff(FILE* stream,void* buff,size_t n){
    ssize_t nread=n;
    while(stream->read_cnt <= 0){
        if((stream->read_cnt=readn(stream,stream->read_base,stream->buffsize))<0){
            return EOF;
        }
        if(stream->read_cnt==0){
            break;
        }else{
            stream->read_ptr=stream->read_base;
        }
    }
    if(stream->read_cnt < n)
        nread=stream->read_cnt;
    _memcpy(buff,stream->read_ptr,(size_t)nread);
    stream->read_ptr+=nread;
    stream->read_cnt-=nread;
    return nread;
}
//full buffer read
static ssize_t readnb(FILE* stream,void* buff,size_t n){
    size_t nleft=n;
    ssize_t nread=0;
    char* userbuff=(char*)buff;
    while(nleft>0){
        if((nread=read_buff(stream,userbuff,nleft))<0){
            return EOF;
        }
        if(nread==0){
            break;
        }
        nleft-=nread;
        userbuff+=nread;
    }
    return n-nleft;
}
//line buffer read
static ssize_t readlineb(FILE* stream,void* buff,size_t n){
    size_t i=0;
    ssize_t nread;
    char* userbuff=(char*)buff;
    char c=0;
    for(i=0;i<n;i++){
        if((nread=read_buff(stream,&c,1))<0){
            return EOF;
        }
        if(nread==0){
            return i;
        }
        *userbuff++=c;
        if(c=='\n'){
            return i+1;
        }
    }
    return i;
}
//write to FILE buffer
static ssize_t write_buff(FILE* stream,const void* buff,size_t n){
    ssize_t nwritten;
    size_t nleft=n;
    char* bp=(char*)buff;
    int copysize;
    while(nleft>0){
        if(stream->write_cnt+nleft > stream->buffsize){
            copysize=stream->buffsize - stream->write_cnt;
        }else{
            copysize=nleft;
        }
        _memcpy(stream->write_base + stream->write_cnt,bp,copysize);
        nleft-=copysize;
        bp+=copysize;
        stream->write_cnt+=copysize;
        if(stream->write_cnt==stream->buffsize){
            if((nwritten=writen(stream,stream->write_base,stream->write_cnt))!=stream->write_cnt){
                return EOF;
            }
            stream->write_cnt=0;
            stream->write_ptr=stream->write_base;
        }
    }
    return n-nleft;
}
//full buffer write
static ssize_t writenb(FILE* stream,const void* buff,size_t n){
    return write_buff(stream,buff,n);    
}
//line buffer write
static ssize_t writelineb(FILE* stream,const void* buff,size_t n){
    if(!n)
        return 0;
    size_t i=0;
    char* bp=(char*)buff;
    for(i=0;i<n;i++){
        write_buff(stream,bp,1);
        if(*bp=='\n'){
            fflush(stream);
            return i+1;
        }
        bp++;
    }
    return i;
}

size_t fread(void* buff,size_t size,size_t count,FILE* stream){
    size_t n = size * count;
    if(stream == stdin)
        fflush(stdout);
    if(stream->mode == FILE_RDWR){
        writen(stream,stream->write_ptr,stream->write_cnt);
    }
    switch(stream->buffmode){
    case BUFF_NO:
        return readn(stream,buff,n);
    case BUFF_LINE:
        return readlineb(stream,buff,n);
    case BUFF_FULL:
        return readnb(stream,buff,n);
    default:
        return 0;
    }
}
size_t fwrite(const void* buff,size_t size,size_t count,FILE* stream){
    size_t n = size * count;
    if(stream->mode == FILE_RDWR){
        readn(stream,stream->read_ptr,stream->read_cnt);
    }
    switch(stream->buffmode){
    case BUFF_NO:
        return writen(stream,buff,n);
    case BUFF_LINE:
        return writelineb(stream,buff,n);
    case BUFF_FULL:
        return writenb(stream,buff,n);
    default:
        return 0;
    }
}

int fclose(FILE* stream){
    int ret=0;
    if(stream==NULL)
        return EOF;
    fflush(stream);
    if(stream->read_base)
        free(stream->read_base);
    if(stream->write_base)
        free(stream->write_base);
    
    //remove FILE from FILE list
    stream->prev->next=stream->next;
    if(stream->next){
        stream->next->prev=stream->prev;
    }

#ifdef WIN32
    ret=CloseHandle(stream->file);
#else
    ret=close(stream->file);
#endif
    free(stream);
    return ret;
}

int fflush(FILE* stream){
    if(stream==NULL){
        for(FILE* p=file_head->next;p;p=p->next){
            fflush(p);
        }
    }
    if(stream==stdin){
        stdin->read_cnt=0;
        return 0;
    }
    switch(stream->mode){
    case FILE_READ:
        readn(stream,stream->read_ptr,stream->read_cnt);
        stream->read_cnt=0;
        return 0;
    case FILE_WRITE:
        writen(stream,stream->write_ptr,stream->write_cnt);
        stream->write_cnt=0;
        return 0;
    case FILE_RDWR:
        readn(stream,stream->read_ptr,stream->read_cnt);
        writen(stream,stream->write_ptr,stream->write_cnt);
        stream->read_cnt=0;
        stream->write_cnt=0;
        return 0;
    default:
        return EOF;
    }
    return 0;
}

int fputc(int c,FILE* stream){
    if(fwrite(&c,1,1,stream)!=1)
        return EOF;
    return c;
}

int fputs(const char* str,FILE* stream){
    int len=strlen(str);
    if(fwrite(str,1,len,stream)!=len)
        return EOF;
    return len;
}

int fgetc(FILE* stream){
    int ret=0;
    if(fread(&ret,1,1,stream)!=1)
        return EOF;
    return ret;
}

char* fgets(char* str,int n,FILE* stream){
    if(fread(str,1,n,stream)!=n)
        return EOF;
    return n;

    if(!str)
        return NULL;
    char *p=str;
    char c;
    while((c=fgetc(stream))!=EOF){
        *p++ = c;
        if(c=='\n'){
            break;
        }
    }
    *p='\0';
    return str;
}

int putchar(int c){
    return fputc(c,stdout);
}
int puts(const char *str){
    if(fputs(str,stdout)==EOF)
        return EOF;
    if(fputc('\n',stdout)==EOF)
        return EOF;
    return 0;
}
int getchar(){
    return fgetc(stdin);
}
char* gets(char* str){
    if(!str)
        return NULL;
    char *p=str;
    char c;
    while((c=getchar())!=EOF&&c!='\n'){
        *p++ = c;
    }
    *p='\0';
    return str;
}

int vprintf(FILE *stream,const char* format,va_list varg){
    int escape=0;
    int ret=0;
    for(const char* p=format;*p!='\0';p++){
        switch(*p){
        case '%':
            if(!escape){
                escape=1;
            }else{
                escape=0;
                if(fputc('%',stream)<0)
                    return EOF;
                ret++;
            }
            break;
        case 'd':
            if(escape){
                escape=0;
                int d=va_arg(varg,int);
                char buff[15];
                itoa(d,buff,10);
                if(fputs(buff,stream)<0)
                    return EOF;
                ret+=strlen(buff);
            }else if(fputc('d',stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;

        case 'c':
            if(escape){
                escape=0;
                char c=va_arg(varg,char);
                if(fputc(c,stream)<0)
                    return EOF;
                ret++;
            }else if(fputc('c',stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;

        case 's':
            if(escape){
                escape=0;
                const char* buff=va_arg(varg,const char*);
                if(fputs(buff,stream)<0)
                    return EOF;
                ret+=strlen(buff);
            }else if(fputc('s',stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;
        
        case 'x':
            if(escape){
                escape=0;
                int x=va_arg(varg,int);
                char buff[15];
                itoa(x,buff,16);
                if(fputs(buff,stream)<0)
                    return EOF;
                ret+=strlen(buff);
            }else if(fputc('x',stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;
       
        case 'o':
            if(escape){
                escape=0;
                int o=va_arg(varg,int);
                char buff[15];
                itoa(o,buff,8);
                if(fputs(buff,stream)<0)
                    return EOF;
                ret+=strlen(buff);
            }else if(fputc('o',stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;
        default:
            if(escape)
                escape=0;
            if(fputc(*p,stream)<0){
                return EOF;
            }else{
                ret++;
            }
            break;
        }
    }
    return ret;
}
int printf(const char* format, ...){
    va_list varg;
    va_start(varg,format);
    return vprintf(stdout,format,varg);
}
int fprintf(FILE* stream,const char* format, ...){
    va_list varg;
    va_start(varg,format);
    return vprintf(stream,format,varg);
}


