#include "LibDisk.h"
#include "LibFS.h"
#include "LibFS.c"
#include <cstring>
#include <libgen.h>
#include <string.h>

using namespace std;


int getInodeNumber(int dataBlockNum, char *dirName){
	char buffer[ SECTOR_SIZE ];

	if(Disk_Read(dataBlockNum, buffer)== -1){
            return -1;
    }
    
    
    for(int i = 0; i < 16; i++){
    	struct dir* dirTemp = (dir* )buffer[i];
    	//memcpy(&dirTemp, buf, sizeof(dirTemp));
    	if(dirTemp->name == dirName){
    		return dirTemp->inodeNumber;
    	}
    }

    return -1;
}

