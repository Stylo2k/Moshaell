#include <stdio.h>
#include <string.h>

int main () {
   FILE *fp;
   int c;
   char buffer [256];

   fp = fopen("file.txt", "r");
   if( fp == NULL ) {
      perror("Error in opening file");
      return(-1);
   }
   while(!feof(fp)) {
      c = getc (fp);
      /* replace ! with + */
      if( c == '!' ) {
        ungetc ('+', fp);
      }
   }
   
   return(0);
}