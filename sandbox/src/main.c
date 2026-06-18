#include <stdio.h>

typedef enum { foo } test_enum;

void print_line() {
  printf("---\n");
}

void print_endianes() {
#ifdef __BIG_ENDIAN__
  printf("Big endian\n");
#else
  printf("Little endian\n"); // this one is executed
#endif
}

void print_native_types_sizes() {
  printf("sizeof(char):               %zu\n", sizeof(char));
  printf("sizeof(signed char):        %zu\n", sizeof(signed char));
  printf("sizeof(unsigned char):      %zu\n", sizeof(unsigned char));
  printf("sizeof(short):              %zu\n", sizeof(short));
  printf("sizeof(unsigned short):     %zu\n", sizeof(unsigned short));
  printf("sizeof(int):                %zu\n", sizeof(int));
  printf("sizeof(unsigned int):       %zu\n", sizeof(unsigned int));
  printf("sizeof(long):               %zu\n", sizeof(long));
  printf("sizeof(unsigned long):      %zu\n", sizeof(unsigned long));
  printf("sizeof(long long):          %zu\n", sizeof(long long));
  printf("sizeof(unsigned long long): %zu\n", sizeof(unsigned long long));
  printf("sizeof(float):              %zu\n", sizeof(float));
  printf("sizeof(double):             %zu\n", sizeof(double));
  printf("sizeof(long double):        %zu\n", sizeof(long double));
  printf("sizeof(void*):              %zu\n", sizeof(void *));
  printf("sizeof(enum):               %zu\n", sizeof(test_enum));
}

int main() {
  print_endianes();
  print_line();
  print_native_types_sizes();
  return 0;
}
