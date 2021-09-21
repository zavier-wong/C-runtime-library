#!/bin/bash

if [ $1 == "-m32" ];
then
    cf="-m32"
    lf="-m elf_i386"
fi
rm crt.a a.out
gcc -g -c  -fno-builtin -nostdlib -fno-stack-protector entry.c heap.c stdio.c string.c  $cf
g++ -g  -c -nostdinc++ -fno-rtti -fno-exceptions -fno-builtin -nostdlib -fno-stack-protector new_delete.cpp sysdep.cpp $cf
ar -rs crt.a  new_delete.o heap.o stdio.o string.o sysdep.o

g++ -g -c -nostdinc++ -fno-rtti -fno-exceptions -fno-builtin -nostdlib -fno-stack-protector main.cpp $cf

ld -static -e entry main.o entry.o crt.a  $lf

rm *.o

