#ifndef TARFS_H
#define TARFS_H

#define TARFS_MAX_FILE_SIZE 4096

typedef struct {
    const char* name;
    unsigned int size;
    const char* data;
} tarfs_file_t;

void tarfs_init(const char* tar_start);
int tarfs_ls(const char*** names);
const char* tarfs_cat(const char* name, unsigned int* size);
int tarfs_write(const char* name, const char* data, unsigned int size);

#endif