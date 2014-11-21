#include "LibDisk.h"
#include "LibFS.h"

#define MAGIC_NUMBER 1337
#define DIR_SIZE 32
#define INODE_OFFSET 5
#define DATA_OFFSET 250


// global errno value here
int osErrno;

typedef struct inode {
  int fileSize;
  int fileType;
  int pointers[30];
} iNode;

typedef struct dir {
  char* name;
  int inodeNumber;
  int pointers[30];
} Dir;

struct dir dirSector[4];
int dirSectorIndex = 0;

int inodeBitmap[SECTOR_SIZE];
int dataBlockTemplate[SECTOR_SIZE];

int inodeCounter = 0;
int dataBlockCounter = 0;
int dirCounter = 5;


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
        char buff[ SECTOR_SIZE ];
        buff[0] = (char) MAGIC_NUMBER;
        if(Disk_Write(0, buff) == -1){
            osErrno = E_GENERAL;
            return -1;
        }
        // create buffer
        buff[0] = (char) MAGIC_NUMBER;
        if(Disk_Write(0, buff) == -1){
            osErrno = E_GENERAL;
            return -1;
        }

        //init inode bitmap
        for(int i = 0; i < 1000; i++){
            inodeBitmap[i] = 0;
        }

        if(Disk_Read(1,inodeBitmap) == -1){
            osErrno = E_GENERAL;
            return -1;
        }

        //create empty directory
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
    printf("FS_Create\n");

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
    
    int loadInfo = Disk_Load(file);

    if(loadInfo == -1){
        return -1;
    } else {
        //get the integer file descriptor and return it!
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
    printf("FS_Write\n");
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
    printf("FS_Close\n");
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
    printf("Dir_Create %s\n", path);
    
    //set bitmap
    inodeBitmap[inodeCounter] = 1;
    
    //intialize inode
    iNode->fileSize = DIR_SIZE;
    iNode->fileType = 1; 
    iNode->pointers[dataBlockCounter] =  dataBlockCounter + DATA_OFFSET; 

    //write inodeBitmap
    if(Disk_Write(1, inodeBitmap) == -1){
            osErrno = E_CREATE;
            return -1;
    }
    
    //add inode to sector
    dirSector[dirSectorIndex] = iNode;
    
    //write inode sector
    if(Disk_Write(inodeCounter+INODE_OFFSET, dirSector) == -1){
            osErrno = E_CREATE;
            return -1;
    }

    //advance directory index in sector
    dirSectorIndex++;

    //fill up each sector and then advance index
    if(dirSectorIndex == 3){
        inodeCounter++; 

        //reset directory counter
        dirSectorIndex = 0;
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