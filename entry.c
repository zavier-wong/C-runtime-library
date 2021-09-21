//-------------------------------------------------------------
//  Process Entry
//
//  the basic c run time library of process entry 
//
//  It is mainly responsible for:
//
//  1.Prepare the runtime environment and initialize the runtime library
//  2.Call the "main" function
//  3.Clean up the resources after the program runs and exit
//
//-------------------------------------------------------------
#include "crt.h"

#ifdef WIN32
#include <Windows.h>
#endif

extern int main(int argc,char *argv[]);
void exit(int);
static void crt_error(const char *);

//---------------------------
//  Entry Function
//  the first called function
//---------------------------
void entry(){
    int ret=0;
    int argc=64;

//parse Windows commond line argument
#if defined(WIN32)
    int flag=0;
    char *argv[16];
	char *i, *j;
	char *cl = GetCommandLineA();
    char* line=GetCommandLineA();
	for (i = cl; *i; i++) {
		if (*i == ' ') {
			continue;
		}
		if (*i == '\"') {
			if (*(i + 1) == '\"') {
				i++;
				continue;
			}
			flag = 1;
		}
		j = i + 1;
		if (flag) {
			flag = 0;
			while (*j != '\"' && *j != '\0') {
				j++;
			}
			if (*j == '\0') {
				crt_error("argument error");
			}
			*j = '\0';
			argv[argc++] = i+1;
		}
		else
		{
			while (*j != ' ' && *j != '\0') {
				j++;
			}
			argv[argc++] = i;
			if (*j == '\0') {
				break;
			}
			*j = '\0';
		}
		i = j;
	}

//parse Linux commond line argument
#elif defined(__linux)
    char **argv;
//32bit Linux
#if __SIZEOF_POINTER__ == 8
    char *rbp;
    asm("movq %%rbp,%0  \n":"=r"(rbp));
    argc=*(int*)(rbp+8);
    argv=(char**)(rbp+16);

//64bit Linux
#elif __SIZEOF_POINTER__ == 4
    char *ebp;
    asm("movl %%ebp,%0  \n":"=r"(ebp));
    argc=*(int*)(ebp+4);
    argv=(char**)(ebp+8);

#endif
#endif
    if(crt_heap_init()){
        crt_error("heap init fail");
    }
    
    if(crt_io_init()){
        crt_error("io init fail");
    }
    //call the main function    
    ret=main(argc,argv);    
    exit(ret);
}

//exit and clean up resources
void exit(int code){
    if(crt_io_fini()){
        crt_error("io fini fail");
    }
#if defined(WIN32)
    ExitProcess(code);
#elif defined(__linux)
#if __SIZEOF_POINTER__ == 8
    asm("movq %0,%%rdi  \n"
        "movq $60,%%rax \n"
        "syscall        \n"
        "hlt            \n"
        ::"m"(code));
#elif __SIZEOF_POINTER__ == 4
    asm("movl %0,%%ebx  \n"
        "movl $1,%%eax  \n"
        "int $0x80      \n"
        "hlt            \n"
        ::"m"(code));
#endif
#endif
}

void crt_error(const char *msg){
    //printf("CRT: %s",msg);
    exit(1);
}
