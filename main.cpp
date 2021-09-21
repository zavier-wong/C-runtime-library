//
//	main function
//
#include "crt.h"

int main(int argc,char *argv[])
{
    char *str=new char[100];
    strcpy(str,"Hello,World");
    printf("%s \n",str);
    return 0;
}
