#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAX_FILES 32
#define MAX_FILENAME 16

typedef struct {
    char name[MAX_FILENAME];
    int size;
    int start_block;
} file_entry;

void fs_init();
int create_file(const char *name);
int write_file(const char *name, const char *data, int size);
int read_file(const char *name, char *buffer);
void fs_list_files(void);

#endif