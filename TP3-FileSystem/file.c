#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;

    if (inode_iget(fs, inumber, &in) == -1) return -1;
    
    int sector = inode_indexlookup(fs, &in, blockNum); 
    if (sector == -1) return -1;
    
    int result = diskimg_readsector(fs->dfd, sector, buf); 
    if (result == -1) return -1;
    
    int filesize = inode_getsize(&in);
    int start_byte = blockNum * DISKIMG_SECTOR_SIZE; 

    if (filesize <= start_byte) return -1;

    int remaining_bytes = filesize - start_byte;
    if (remaining_bytes < DISKIMG_SECTOR_SIZE) return remaining_bytes;
 
    return DISKIMG_SECTOR_SIZE;

}

