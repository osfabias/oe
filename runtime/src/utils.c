#include <sys/time.h>

extern double get_time(void) {
  struct timeval t;
  gettimeofday(&t, NULL);

  return t.tv_sec * 1000.0 + // sec to ms
         t.tv_usec / 1000.0; // us to ms
}
