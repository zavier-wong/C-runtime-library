//----------------------------------    
//  some basic c string operation
//
//---------------------------------
#include "crt.h"

int strcmp(const char* src,const char* dest){
    for(;*src && *dest;++src,++dest){
        if(*src < *dest)
            return -1;
        else if(*src > *dest)
            return 1;
    }
    if(*src == '\0' && *dest == '\0')
        return 0;
    else if(*src=='\0')
        return -1;
    else
        return 1;
}

char* strcpy(char* dest,const char* src){
    char *ret=dest;
    while(*src){
        *dest=*src;
        dest++;
        src++;
    }
    *dest='\0';
    return ret;
}
char* strcat(char* dest,const char* src){
    if(!dest||!src)
        return dest;
    char* p=dest;
    while(*dest != '\0') dest++;
    strcpy(dest,src);
    return p;
}
unsigned strlen(const char *str){
    unsigned size=0;
    if(!str)
        return 0;
    while(*str != '\0'){
        size++;
        str++;
    }
    return size;
}

char* itoa(int n,char* str,int base){
    if(!str||base<2||base>36){
        return str;
    } 
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *p=str;
    int t=0,signed_flag=0,len=0;
    if(n<0){
        signed_flag=1;
        n*=-1;
    }else if(n==0){
        *p='0';
        *(p+1)=0;
        return p;
    }
    while(n){
        t=n%base;
        n/=base;
        *p++ = index[t];
    }
    if(signed_flag){
        *p++ = '-';
    }
    if(base==8){
        *p++='0';
    }else if(base==16){
        *p++='x';
        *p++='0';
    }
    *p='\0';
    len=p-str;
    
    for(int i=0;i<len/2;i++){
        char tmp;
        tmp=str[i];
        str[i]=str[len-i-1];
        str[len-i-1]=tmp;
    }
    return str;
}

void* memset(void* src,int ch,size_t n){
    char* p=src;
    for(int i=0;i<n;i++){
        *(p+i)=ch;
    }
    return src;
}

void* _memcpy(void* dest,void* src,size_t n){
    char* d=(char*)dest;
    char* s=(char*)src;
    while(n){
        n--;
        (*d++)=(*s++);
    }
    return dest;
}

