#include "../include/filesystem.h"
#include "../include/fat.h"
#include "../include/printf.h"

static file_entry files[MAX_FILES]; // FAT table
static char data_blocks[MAX_BLOCKS][BLOCK_SIZE]; // actual storage

void fs_init() { // initializing the file system
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].name[0] = '\0'; // empty file at start
        files[i].size = 0; 
        files[i].start_block = -1; // no data yet
    }
    fat_init(); // calling the fat_initi()
}

static int find_file(const char* name) // private function 
{
    for (int i = 0; i < MAX_FILES; i++) {
        int j = 0;
        // check file name one character by one character
        while (name[j] && files[i].name[j] && name[j] == files[i].name[j]) {
            j++;
        }
        if (name[j] == '\0' && files[i].name[j] == '\0') {
            return i;
        }
    }
    return -1;
}

int create_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].name[0] == '\0') { // find empty slot
            int j = 0;
            while (name[j] && j < MAX_FILENAME - 1) { // copy filenames manually
                files[i].name[j] = name[j];
                j++;
            }
            // setting the metadata
            files[i].name[j] = '\0';
            files[i].size = 0;
            files[i].start_block = -1;
            return 0;
        }
    }
    return -1; // no space
}

int write_file(const char* name, const char* data, int size) {
    int file_index = find_file(name); // finidng the file
    if (file_index == -1) return -1;

    int prev_block = -1; // last block
    int first_block = -1; // start block

    int bytes_written = 0; // progress

    while (bytes_written < size) { // allocate and fill blocks
        int block = allocate_block();
        if (block == -1) return -1;

        if (first_block == -1) {
            first_block = block;
        }

        if (prev_block != -1) {
            set_next_block(prev_block, block);
        }

        for (int i = 0; i < BLOCK_SIZE && bytes_written < size; i++) {
            data_blocks[block][i] = data[bytes_written++];
        }

        prev_block = block;
    }

    files[file_index].start_block = first_block;
    files[file_index].size = size;

    return 0;
}

int read_file(const char* name, char* buffer) {
    int file_index = find_file(name);
    if (file_index == -1) return -1;

    int block = files[file_index].start_block;
    int bytes_read = 0;

    while (block != -2 && block != -1) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            buffer[bytes_read++] = data_blocks[block][i];
        }
        block = get_next_block(block);
    }

    return bytes_read;
}

void fs_list_files(void) {
    int any = 0;
    printf("\033[1;36mName            Size  StartBlk\033[0m\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].name[0] == '\0') {
            continue;
        }
        any = 1;
        printf("  \033[1;37m%s\033[0m  size=%d  start_block=%d\n",
            files[i].name, files[i].size, files[i].start_block);
    }
    if (!any) {
        printf("\033[1;33m(no files yet - use create / write)\033[0m\n");
    }
}