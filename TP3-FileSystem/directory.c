#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define DIRNAMELEN 14

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
		int dirinumber, struct direntv6 *dirEnt) {
    struct inode in;

    if (inode_iget(fs, dirinumber, &in) == -1) {
        return -1;  
    }

    int filesize = inode_getsize(&in);
    int offset = 0;
    char block[DISKIMG_SECTOR_SIZE];

    while (offset < filesize) {
        int bytes = file_getblock(fs, dirinumber, offset / DISKIMG_SECTOR_SIZE, block);
        if (bytes <= 0) {
            return -1;  
        }

        int cantidad = bytes / sizeof(struct direntv6);
        struct direntv6 *entradas = (struct direntv6 *) block; 

        for (int i = 0; i < cantidad; i++) {
            if (strncmp(entradas[i].d_name, name, DIRNAMELEN) == 0) {
                *dirEnt = entradas[i];  
                return 0;
            }
        }

        offset += bytes; 
    }

    return -1;  
}
