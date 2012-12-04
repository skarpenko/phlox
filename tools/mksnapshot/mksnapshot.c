#include <stdio.h>
#include<string.h>
#include <time.h>

#define PHLOX_DIR "phlox" 
#define SNAP_DEST "snapshots/"
#define MAXSTR 300

time_t timer;
struct tm *lt;
char snapshot[MAXSTR];
char *command_fmt = "tar -v -cjf %s%s.tar.bz2 %s/";
char command[MAXSTR];


int main() {
  time(&timer);

  lt = localtime(&timer);

  sprintf(snapshot, "phlox-snapshot-%02d%02d%04d-%02d%02d%02d\0",
                                 lt->tm_mday,
                                 lt->tm_mon + 1,
				 lt->tm_year + 1900,
				 lt->tm_hour,
 				 lt->tm_min,
				 lt->tm_sec);
  sprintf(command, command_fmt, SNAP_DEST, snapshot, PHLOX_DIR);
						 
  printf("executing: %s\n", command);
  return system(command);
}
