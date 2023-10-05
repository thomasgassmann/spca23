#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRSIZE 100
#define NFIELDS 9

// use these functions to print so it will be in the expected formatting.
void print_header(void)
{
  printf("%-30s %7s\n", "STATE", "TOTAL");
  printf("----------------------------------------\n");
}

void print_state(char *state, int aggr)
{
  printf("%-30s %7d\n", state, aggr);
}

void print_footer(int total)
{
  printf("----------------------------------------\n");
  print_state("TOTAL", total);
}

typedef struct Entry
{
  char state_code_origin[3];
  char state_name[25];
  int aggr_agi;
} entry;

int main()
{
  char inputfile[] = "stateoutflow0708.txt";

  FILE *fp = fopen(inputfile, "r");
  // skip first line
  while (getc(fp) != '\n');
  print_header();
  int total = 0;
  while (1)
  {
    entry e;
    if (fscanf(fp, " \"%[^\"]\" ", e.state_code_origin) < 0) {
      break;
    }

    fscanf(fp, " \"%*[^\"]\" ");
    fscanf(fp, " \"%*[^\"]\" ");
    fscanf(fp, " \"%*[^\"]\" ");
    fscanf(fp, " \"%*[^\"]\" ");
    fscanf(fp, " %s ", e.state_name);
    fscanf(fp, " %*d ");
    fscanf(fp, " %*d ");
    fscanf(fp, " %d\n", &e.aggr_agi);

    if (strncmp(e.state_code_origin, "25", 2) == 0) {
      print_state(e.state_name, e.aggr_agi);
      total += e.aggr_agi;
    }
  }

  print_footer(total);

  return 0;
}
