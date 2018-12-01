#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "helper.h"
#include "csapp.h"

unsigned int convert(char *st) {
  char *x;
  for (x = st ; *x ; x++) {
    if (!isdigit(*x))
      return 0L;
  }
  return (strtoul(st, 0L, 10));
}

int validPortNumber(unsigned int num){
    if(num==0 || num>= MAXPORTNUMBER)
        return 0;
    return 1;
}
