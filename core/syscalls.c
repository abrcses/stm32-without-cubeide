#include <sys/stat.h>
#include <sys/times.h>
#include "main.h"
#include <errno.h>
#undef errno
extern int errno;

void _exit(int exit_code)
{
    while (1)
    {

    }
}

int _close(int file) {
  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int _execve(char *name, char **argv, char **env) {
  errno = ENOMEM;
  return -1;
}

int _fork(void) {
  errno = EAGAIN;
  return -1;
}


int _fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _getpid(void) {
  return 1;
}

int _isatty(int file) {
  return 1;
}

int _kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

int _link(char *old, char *new) {
  errno = EMLINK;
  return -1;
}

int _lseek(int file, int ptr, int dir) {
  return 0;
}

int _open(const char *name, int flags, int mode) {
  return -1;
}

int _read(int file, char *ptr, int len) {
  (void) file;
  uint8_t c;
  if (HAL_UART_Receive(&uart2, &c, 1, HAL_MAX_DELAY) != HAL_OK) {
    return 0;
  }
  *ptr = (char) c;
  return 1;
}

caddr_t _sbrk(int incr) {
  extern char __heap_start__;
  extern char __heap_end__;

  static char *heap_ptr = NULL;
  char *prev_heap_ptr;

  if (heap_ptr == NULL) {
    heap_ptr = &__heap_start__;
  }

  prev_heap_ptr = heap_ptr;

  if ((heap_ptr + incr) > &__heap_end__) {
    errno = ENOMEM;
    return (caddr_t)-1;
  }

  heap_ptr += incr;
  return (caddr_t)prev_heap_ptr;
}

int _stat(char *file, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _times(struct tms *buf) {
  return -1;
}

int _unlink(char *name) {
  errno = ENOENT;
  return -1; 
}

int _wait(int *status) {
  errno = ECHILD;
  return -1;
}

int _write(int file, char *ptr, int len) {
  (void) file;
  HAL_UART_Transmit(&uart2, ptr, len, HAL_MAX_DELAY); 
  return len;
}