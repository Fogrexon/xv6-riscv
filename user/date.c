#include "kernel/types.h"
#include "user/user.h"
#include "kernel/date.h"

int
main(int argc, char *argv[])
{
  struct rtcdate ds;
  if (getdate(&ds) < 0) {
    fprintf(2, "date: failed to get date\n");
    exit(1);
  }

  printf("%d/%d/%d %d:%d:%d\n",
         ds.year, ds.month, ds.day,
         ds.hour, ds.minute, ds.second);

  exit(0);
}
