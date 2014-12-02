#include "LibDisk.h"
#include "LibFS.h"
#include <cstring>
#include <libgen.h>
#include <errno.h>


using namespace std;

#define MAGIC_NUMBER 1337
#define DIR_SIZE 32
#define INODE_OFFSET 5
#define DATA_OFFSET 255
#define MAX_OPEN_FILES 256


// global errno value here
int osErrno;

char* name; 

typedef struct inode {
  int fileSize;
  int fileType;
  char* pointers[30];
} iNode;

typedef struct dir {
  char* name;
  int inodeNumber;
} Dir;

int openFileTable [MAX_OPEN_FILES];
int openFileIndex = 0;

char dirInodeSector[4];
int dirInodeSectorIndex = 0;
int pointerIndex = 0;

char dirSector[16];
int dirSectorIndex = 0;

char inodeBitmap[SECTOR_SIZE];
char dataBitmap[SECTOR_SIZE];

int dirInodeCounter = 0;
int dataBlockCounter = 0;
int dirDataCounter = 0;

int fileInodeCounter = 0;
int fileDataCounter = 0;


iNode inodeTemp;
Dir dirTemp;


int 
FS_Boot(char *path)
{
    printf("FS_Boot %s\n", path);

    // oops, check for errors
    if (Disk_Init() == -1) {
    	printf("Disk_Init() failed\n");
    	osErrno = E_GENERAL;
    	return -1;
    }

    //now search for image
    if(Disk_Load(path) == -1){
        //if not found...then init
        // create buffer and write superblock
        char buff[ SECTOR_SIZE ];
        buff[0] = (char) MAGIC_NUMBER;
        if(Disk_Write(0, buff) == -1){
            osErrno = E_GENERAL;
            return -1;
        }

        //init inode bitmap
        for(int i = 0; i < 1000; i++){
            inodeBitmap[i] = (char)0;
        }

        //write inode bitmap
        if(Disk_Write(1,inodeBitmap) == -1){
            osErrno = E_GENERAL;
            return -1;
        }

        //create empty directory
        int rc;
        if(( rc = Dir_Create( "/" ) ) == -1 ){
            //corrupted
            printf("ERROR...Corrupted Image\n");
            osErrno = E_GENERAL;
            return -1;
        }

    } else {
        //verify image
        char buff[ SECTOR_SIZE ];
        if(Disk_Read(0,buff) == -1){
            osErrno = E_GENERAL;
            return -1;
        }

        //check super block has magic number
        if(buff[0]!= (char) MAGIC_NUMBER){
            //corrupted
            printf("ERROR...Corrupted Image\n");
            osErrno = E_GENERAL;
            return -1;
        }

    }
    
    return 0;
}

int
FS_Sync()
{
    printf("FS_Sync\n");
    //Disk_Save();
    return 0;
}


int
File_Create(char *file)
{
    /*  File Create() creates a new file of the name pointed to by file. Upon success, return 0. If the file
        already exists, you should return -1 and set osErrno to E CREATE. Note: the file should not be “open”
        after the create call. Rather, File Create() simply creates a new file on disk of size 0.
    */
    printf("FS_Create\n");

    //TODO: do not use FILE_OPEN!! look for name exist already
    //check to see if already exists
    if(File_Open(file) != -1){
            osErrno = E_CREATE;
            return -1;
    }

    //get inode bitmap num
    while (inodeBitmap[fileInodeCounter] == '1'){
        fileInodeCounter++;
    }

    //if new.... allocate?
    inodeBitmap[fileInodeCounter] = (char)1;
    
    //intialize inode for file
    inodeTemp.fileSize = 0; // files initialize to size zero
    inodeTemp.fileType = 0; // file 
    inodeTemp.pointers[0] =  file;

    //write inodeBitmap
    if(Disk_Write(1, inodeBitmap) == -1){
            osErrno = E_CREATE;
            return -1;
    }
    
    //add inode to sector
    char* const buf = reinterpret_cast<char*>(&inodeTemp);
    dirInodeSector[dirInodeSectorIndex] = *buf;
    
    //write inode sector
    if(Disk_Write(dirInodeCounter+INODE_OFFSET, dirInodeSector) == -1){
            osErrno = E_CREATE;
            return -1;
    }

     //get inode bitmap num
    while (dataBitmap[fileDataCounter] == '1'){
        fileDataCounter++;
    }

    //if new.... allocate?
    dataBitmap[fileDataCounter] = (char)1;

    if(Disk_Write(2, dataBitmap) == -1){
            osErrno = E_CREATE;
            return -1;
    }
    
    return 0;
}

int
File_Open(char *file)
{
    /*
        File Open() opens a file (whose name is pointed to by file) and returns an integer file descriptor (a
        number greater than or equal to 0), which can be used to read or write data to that file. If the file
        doesn’t exist, return -1 and set osErrno to E NO SUCH FILE. If there are already a maximum number
        of files open, return -1 and set osErrno to E TOO MANY OPEN FILES.
    */
    printf("FS_Open\n");


    //read in root directory
    Disk_Read(5, dirInodeSector);
    memcpy(&inodeTemp, dirInodeSector, sizeof(iNode));

    int dataIndex = -1;

    for(int i = 0; i < 30; i++){
        if(inodeTemp.pointers[i] == dirname(file)){
            dataIndex = i;
        }
    }

    // no such file
    if(dataIndex == -1){
        osErrno = E_NO_SUCH_FILE;
        return -1;
    } else {
        //get the integer file descriptor and return it!
        openFileTable[openFileIndex] = dataIndex + DATA_OFFSET;
        openFileIndex++;

        return openFileIndex - 1;

    }

    return 0;
}

int
File_Read(int fd, void *buffer, int size)
{
    printf("FS_Read\n");
    return 0;
}

int
File_Write(int fd, void *buffer, int size)
{
    /*
        should write size bytes from buffer and write them into the file referenced by fd. All
        writes should begin at the current location of the file pointer and the file pointer should be updated
        after the write to its current location plus size. Note that writes are the only way to extend the size
        of a file. If the file is not open, return -1 and set osErrno to E BAD. Upon success of the write, all of
        the data should be written out to disk and the value of size should be returned. If the write cannot
        complete (due to a lack of space), return -1 and set osErrno to E NO SPACE. Finally, if the file exceeds
        the maximum file size, you should return -1 and set osErrno to E FILE TOO BIG.
    */
    printf("FS_Write\n");

    //use fd as index for open file table
    int datablock = openFileTable[fd];

    //if no value, then bad fd
    if (datablock == NULL){
        osErrno = E_BAD_FD;
        return -1;
    }

    //handle incorrect size
    if (size > SECTOR_SIZE){
        osErrno = E_FILE_TOO_BIG;
        return -1;
    }

    if(Disk_Write(datablock, (char*)buffer) == 0){
        return size;
    }


    return 0;
}

int
File_Seek(int fd, int offset)
{
    printf("FS_Seek\n");
    return 0;
}

int
File_Close(int fd)
{
    /*
        File Close() closes the file referred to by file descriptor fd. If the file is not currently open, return
        -1 and set osErrno to E BAD FD. Upon success, return 0.
    */
    printf("FS_Close\n");

    //use fd as index for open file table
    int datablock = openFileTable[fd];

    //if no value, then bad fd
    if (datablock == NULL){
        osErrno = E_BAD_FD;
        return -1;
    }

    openFileTable[fd] = NULL;

    return 0;
}

int
File_Unlink(char *file)
{
    printf("FS_Unlink\n");
    return 0;
}


// directory ops
int
Dir_Create(char *path)
{
    /*
        You need to call Disk Read and
        Disk Write for every File Read, File Write, 
        and other file systems operations that interact with the disk.

    */
    printf("Dir_Create %s\n", path);
    
    //set inode bitmap
    inodeBitmap[dirInodeCounter] = (char)1;

    //write inodeBitmap
    if(Disk_Write(1, inodeBitmap) == -1){
            osErrno = E_CREATE;
            return -1;
    }
    
    //intialize inode
    inodeTemp.fileSize = DIR_SIZE;
    inodeTemp.fileType = 1; 
    inodeTemp.pointers[0] = dirname(path); 

    
    //add inode to sector
    char* const buf = reinterpret_cast<char*>(&inodeTemp);
    dirInodeSector[dirDataCounter] = *buf;
    
    //write inode sector
    if(Disk_Write(dirInodeCounter+INODE_OFFSET, dirInodeSector) == -1){
            osErrno = E_CREATE;
            return -1;
    }

    //full dir inode data sector then advance index
    if(dirInodeSectorIndex == 3){
        dirInodeCounter++; 

        //reset directory counter
        dirInodeSectorIndex = 0;
    }
    else {
        //advance directory index in sector
        dirInodeSectorIndex++;
    }
///////////////////////////////////////////
    //set data bitmap
    dataBitmap[dirDataCounter] = (char)1;

    //write dataBitmap
    if(Disk_Write(2, dataBitmap) == -1){
            osErrno = E_CREATE;
            return -1;
    }

    //initialize dir 
    dirTemp.name = path;
    dirTemp.inodeNumber = dirInodeCounter;

    //add dir to sector
    char* const dirBuf = reinterpret_cast<char*>(&dirTemp);
    dirSector[dirSectorIndex] = *dirBuf;

    //write dir sector
    if(Disk_Write(dirDataCounter+DATA_OFFSET, dirSector) == -1){
            osErrno = E_CREATE;
            return -1;
    }

    //full dir data sector then advance data bitmap index
    if(dirSectorIndex == 15){
        dirDataCounter++; 

        //reset directory counter
        dirSectorIndex = 0;
    }
    else {
        //advance directory index in sector
        dirSectorIndex++;
    }
    
    return 0;

}

int
Dir_Size(char *path)
{
    printf("Dir_Size\n");
    return 0;
}

int
Dir_Read(char *path, void *buffer, int size)
{

    printf("Dir_Read\n");


    return 0;
}

int
Dir_Unlink(char *path)
{
    printf("Dir_Unlink\n");
    return 0;
}