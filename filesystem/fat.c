#include "../include/fat.h"

/* 
Special Value:
-1 --> free block
-2 --> end of file
*/

static int fat_table[MAX_BLOCKS]; // static -> private (encapsulation)

void fat_init(){ // initialize the file system
    for(int i=0; i<MAX_BLOCKS; i++){ // running through all the block
        fat_table[i] = -1; // marking that its freee at the start of everything
    }
} 

int allocate_block(){ // allocating the freed block
    for(int i=0; i<MAX_BLOCKS; i++){
        if(fat_table[i] == -1){
            fat_table[i] = -2; // denotes the EOF
            return i;
        }
    }
    return -1;
}

void free_block(int block){ // freeing the block
    fat_table[block] = -1; // -1 denotes the freed block
}

void set_next_block(int current, int next){ // choosing the next block
    fat_table[current] = next; // replacing current with the next block
}

int get_next_block(int current){ // getting the next block
    return fat_table[current];
}