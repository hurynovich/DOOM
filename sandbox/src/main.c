//
// Created by pahan on 10/11/25.
//
#include <stdio.h>

int main(int argc, char* argv[])
{
#ifdef __BIG_ENDIAN__
    printf("Big endian");
#else
    printf("Little endian"); //this one is executed
#endif
}
