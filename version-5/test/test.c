#include <stdio.h>

int main(int argc, char **argv) {
int j=0;
  printf("1 2 3 4 5 6 7 8 9 10\n");
  for(int i=0;i<1000000000;i++)
  {
  	j =j+1;
  }
  //sleep(10);
  perror("testing");
  return 0;
}
