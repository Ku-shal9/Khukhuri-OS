#ifndef FAT_H
#define FAT_H

// virtual disk structure
#define MAX_BLOCKS 128 // total blocks
#define BLOCK_SIZE 64  // size of each block

void fat_init();                            // setting up FAT table
int allocate_block();                       // allocating when writing a file
void free_block(int block);                 // marking when the block is freed
void set_next_block(int current, int next); // connecting the blocks
int get_next_block(int current);            // next block
int fat_count_free(void);

#endif