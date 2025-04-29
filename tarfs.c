#include "tarfs.h"
#include "string_utils.h"

#define TAR_BLOCK_SIZE 512
#define TAR_NAME_SIZE 100
#define TAR_MAX_FILES 128  // Increase from 32 to 128 or more
#define TARFS_DATA_POOL_SIZE (TAR_MAX_FILES * 65536)  // Increase to 64 KB per file

typedef struct {
    char name[TAR_NAME_SIZE];
    unsigned int size;
    const char* data;
} tar_file_entry;

static tar_file_entry files[TAR_MAX_FILES];
static int file_count = 0;
static char tarfs_data_pool[TARFS_DATA_POOL_SIZE];
static unsigned int tarfs_data_pool_used = 0;

extern unsigned char initfs_tar[];
extern unsigned int initfs_tar_len;

// Parse octal string
static unsigned int tar_octal(const char* s, int len) {
    unsigned int n = 0;
    for (int i = 0; i < len && s[i] >= '0' && s[i] <= '7'; i++)
        n = (n << 3) | (s[i] - '0');
    return n;
}

void tarfs_init(const char* tar_start) {
    file_count = 0;
    const char* p = tar_start;
    while (1) {
        if (p[0] == 0) break; // End of archive
        // Name
        for (int i = 0; i < TAR_NAME_SIZE; i++)
            files[file_count].name[i] = p[i];
        files[file_count].name[TAR_NAME_SIZE-1] = 0;
        // Size
        files[file_count].size = tar_octal(p + 124, 12);
        files[file_count].data = p + TAR_BLOCK_SIZE;
        if (files[file_count].name[0] == 0) break;
        file_count++;
        // Next header
        unsigned int size_blocks = (files[file_count-1].size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        p += TAR_BLOCK_SIZE + size_blocks * TAR_BLOCK_SIZE;
        if (file_count >= TAR_MAX_FILES) break;
    }
}

int tarfs_ls(const char*** names) {
    static const char* name_ptrs[TAR_MAX_FILES];
    for (int i = 0; i < file_count; i++) {
        name_ptrs[i] = files[i].name;
    }
    *names = name_ptrs;
    return file_count;
}

const char* tarfs_cat(const char* name, unsigned int* size) {
    for (int i = 0; i < file_count; i++) {
        // Match exact or with "./" prefix
        if (strcmp(files[i].name, name) == 0 ||
            (files[i].name[0] == '.' && files[i].name[1] == '/' && strcmp(files[i].name + 2, name) == 0)) {
            if (size) *size = files[i].size;
            return files[i].data;
        }
    }
    return 0;
}

// Dummy write: just a stub (RAM only, not persistent)
int tarfs_write(const char* name, const char* data, unsigned int size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, name) == 0 ||
            (files[i].name[0] == '.' && files[i].name[1] == '/' && strcmp(files[i].name + 2, name) == 0)) {
            unsigned int n = size < TARFS_MAX_FILE_SIZE ? size : TARFS_MAX_FILE_SIZE;
            for (unsigned int j = 0; j < n; j++)
                ((char*)files[i].data)[j] = data[j];
            files[i].size = n;
            return 0;
        }
    }
    // New file
    if (file_count < TAR_MAX_FILES && tarfs_data_pool_used + size <= TARFS_DATA_POOL_SIZE) {
        // Set file name
        int i = 0;
        for (; i < TAR_NAME_SIZE - 1 && name[i]; i++)
            files[file_count].name[i] = name[i];
        files[file_count].name[i] = 0;
        files[file_count].size = size < TARFS_MAX_FILE_SIZE ? size : TARFS_MAX_FILE_SIZE;
        files[file_count].data = &tarfs_data_pool[tarfs_data_pool_used];
        for (unsigned int j = 0; j < files[file_count].size; j++)
            ((char*)files[file_count].data)[j] = data[j];
        tarfs_data_pool_used += TARFS_MAX_FILE_SIZE;
        file_count++;
        return 0;
    }
    return -1;
}