#ifndef EXTS_FILE_EXT_H
#define EXTS_FILE_EXT_H

#include <sys/stat.h>

int file_exists(char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

#endif 