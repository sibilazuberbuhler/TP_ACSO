
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || *pathname == '\0') {
        return -1;
    }

    int current_inumber = 1; 

    char path_copy[strlen(pathname) + 1];
    strcpy(path_copy, pathname);

    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        struct direntv6 entry;

        if (directory_findname(fs, token, current_inumber, &entry) == -1) {
            return -1;  
        }

        current_inumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return current_inumber; 
}
