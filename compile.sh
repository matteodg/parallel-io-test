#!/bin/sh

dd if=/dev/zero of=file0 bs=$((1024*1024*1024)) count=1
cp file0 file1
cp file0 file2
cp file0 file3

mkdir -p x86_64
mkdir -p i686

echo "Linux x86_64:"
gcc                    main.c -o x86_64/main64     -lpthread -static

echo "Linux i686:"
gcc -m32               main.c -o i686/main32       -lpthread -static

echo "Windows x86_64:"
x86_64-w64-mingw32-gcc main.c -o x86_64/main64.exe -lpthread -static

echo "Windows i686:"
i686-w64-mingw32-gcc   main.c -o i686/main32.exe   -lpthread -static
