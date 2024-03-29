#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
  FILE *fp = NULL;
  int nfiles = --argc; /*ignore the name of the program itself*/
  int argidx = 1;      /*ignore the name of the program itself*/
  char *currfile = "";
  char c;
  /*count of words,lines,characters*/
  unsigned long nw = 0, nl = 0, nc = 0;

  /*following used to count words*/
  enum state
  {
    INSIDE,
    OUTSIDE
  };
  enum state currstate = OUTSIDE;

  if (nfiles == 0)
  {
    fp = stdin; /*standard input*/
    nfiles++;
  }
  else /*set to first*/
  {
    currfile = argv[argidx++];
    fp = fopen(currfile, "r");
  }
  while (nfiles > 0) /*files left >0*/
  {
    if (fp == NULL)
    {
      fprintf(stderr, "Unable to open input\n");
      exit(-1);
    }
    nc = nw = nl = 0;
    currstate = OUTSIDE;
    while ((c = getc(fp)) != EOF)
    {
      /*TODO:FILL HERE
       process the file using getc(fp)
       */
      nc++;
      if (c == '\n') {
        nl++;
      }

      if (isspace(c)) {
        currstate = OUTSIDE;
      } else if (currstate == OUTSIDE) {
        nw++;
        currstate = INSIDE;
      }
    }

    /*print totals*/
    printf("%ld %ld %ld %s\n", nl, nw - 1, nc, currfile);
    /*next file if exists*/
    nfiles--;
    if (nfiles > 0)
    {
      currfile = argv[argidx++];
      fp = fopen(currfile, "r");
    }
  }
  return 0;
}
