#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1;
    }

    int inode_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode); //calculo cuantos inodes hay por sector
    int sector_num = INODE_START_SECTOR + (inumber - 1) / inode_per_sector; //Veo en que sector del disco esta
    int inode_index = (inumber - 1) % inode_per_sector; //Veo pos del inode adentro del sector 

    struct inode buffer[ inode_per_sector ]; //guardo espacio de la cantidad de inodes que hay por sector  

    int read_bytes = diskimg_readsector(fs->dfd, sector_num, buffer); // en buffer tengo los 16 inodes del sector
    if (read_bytes == -1) {
        return -1;
    }

    *inp = buffer[inode_index]; // copio el inodo llamandolo de buffer

    return 0; 
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        if ((inp->i_mode & IALLOC) == 0) {
        return -1; // inodo no asignado
    }

    // ARCHIVO NORMAL: bloques directos
    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 0 || blockNum >= 8) {
            return -1;
        }

        int sector = inp->i_addr[blockNum];

        if (sector == 0) { //Chequeo sector con valor 0
            return -1;
        } else {
            return sector;
        }
    }

    // ARCHIVO GRANDE
    const int PTRS_PER_BLOCK = DISKIMG_SECTOR_SIZE / 2; //Punteros a bloque entran por bloque del disco (512/2) 

    // BLOQUES INDIRECTOS (i_addr[0] a i_addr[6]) hay 7 entradas que apuntan a bloques indirectos y cada bloque indirecto puede almacenar 256 punteros
    if (blockNum < 7 * PTRS_PER_BLOCK) { // 7*256 = 1792 puede tener hasta 1792 bloques de datos usando bloques inderectos 
        int indirect_block_index = blockNum / PTRS_PER_BLOCK; //En que bloque indirecto esta el blockNum 
        int offset = blockNum % PTRS_PER_BLOCK;

        unsigned short indirect_block[PTRS_PER_BLOCK];
        int indirect_sector = inp->i_addr[indirect_block_index];
        if (indirect_sector == 0) return -1;

        if (diskimg_readsector(fs->dfd, indirect_sector, indirect_block) == -1) return -1; 

        int data_sector = indirect_block[offset];
        
        if (data_sector == 0) return -1;
        return data_sector;
        

    }

    // BLOQUE DOBLEMENTE INDIRECTO (i_addr[7])
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
