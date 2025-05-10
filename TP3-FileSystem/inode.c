#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) return -1;

    int inode_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode); 
    int sector_num = INODE_START_SECTOR + (inumber - 1) / inode_per_sector; 
    int inode_index = (inumber - 1) % inode_per_sector; 

    struct inode buffer[ inode_per_sector ]; 
    if (diskimg_readsector(fs->dfd, sector_num, buffer) == -1) return -1;

    *inp = buffer[inode_index];

    return 0; 
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        if ((inp->i_mode & IALLOC) == 0) {
        return -1; 
    }

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 0 || blockNum >= 8) return -1;

        int sector = inp->i_addr[blockNum];

        if (sector == 0) return -1;
    
        return sector;
    }

    const int PTRS_PER_BLOCK = DISKIMG_SECTOR_SIZE / 2;

    if (blockNum < 7 * PTRS_PER_BLOCK) {
        int indirect_block_index = blockNum / PTRS_PER_BLOCK; 
        int offset = blockNum % PTRS_PER_BLOCK;

        unsigned short indirect_block[PTRS_PER_BLOCK];
        int indirect_sector = inp->i_addr[indirect_block_index];
        if (indirect_sector == 0) return -1;

        if (diskimg_readsector(fs->dfd, indirect_sector, indirect_block) == -1) return -1; 

        int data_sector = indirect_block[offset];
        
        if (data_sector == 0) return -1;
        return data_sector;
        

    }

    int remaining_block = blockNum - 7 * PTRS_PER_BLOCK;
    int first_level_index = remaining_block / PTRS_PER_BLOCK;
    int second_level_index = remaining_block % PTRS_PER_BLOCK;

    unsigned short double_indirect_block[PTRS_PER_BLOCK];
    int dbl_indirect_sector = inp->i_addr[7];
    if (dbl_indirect_sector == 0) return -1;

    if (diskimg_readsector(fs->dfd, dbl_indirect_sector, double_indirect_block) == -1) return -1;

    int indirect_sector = double_indirect_block[first_level_index];
    if (indirect_sector == 0) return -1;

    unsigned short indirect_block[PTRS_PER_BLOCK];
    if (diskimg_readsector(fs->dfd, indirect_sector, indirect_block) == -1) return -1;

    int data_sector = indirect_block[second_level_index];

    if (data_sector == 0) return -1; 
    return data_sector;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
