#ifndef EXTS_FILE_EXT_H
#define EXTS_FILE_EXT_H

#ifndef _SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>

int get_size_file(FILE* f)
{
    fseek(f, 0, SEEK_END);
    int sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    return sz;
}

/*int file_exists(char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}*/

#endif 