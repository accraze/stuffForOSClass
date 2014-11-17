#include "LibDisk.h"
#include "LibFS.h"

// global errno value here
int osErrno;

int 
FS_Boot(char *path)
{
    printf("FS_Boot %sn", path);

    // oops, check for errors
    if (Disk_Init() == -1) {
	printf("Disk_Init() failedn");
	osErrno = E_GENERAL;
	return -1;
    }

    //now search for path
    if(File_Open(path) == -1){
        //if not found
        if(File_Create(path) == -1){
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
    printf("FS_Openn");
    return 0;
}

int
File_Read(int fd, void *buffer, int size)
{
    printf("FS_Readn");
    return 0;
}

int
File_Write(int fd, void *buffer, int size)
{
    printf("FS_Writen");
    return 0;
}

int
File_Seek(int fd, int offset)
{
    printf("FS_Seekn");
    return 0;
}

int
File_Close(int fd)
{
    printf("FS_Closen");
    return 0;
}

int
File_Unlink(char *file)
{
    printf("FS_Unlinkn");
    return 0;
}


// directory ops
int
Dir_Create(char *path)
{
    printf("Dir_Create %sn", path);
    return 0;
}

int
Dir_Size(char *path)
{
    printf("Dir_Sizen");
    return 0;
}

int
Dir_Read(char *path, void *buffer, int size)
{
    printf("Dir_Readn");
    return 0;
}

int
Dir_Unlink(char *path)
{
    printf("Dir_Unlinkn");
    return 0;
}