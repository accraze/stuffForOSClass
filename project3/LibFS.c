#include "LibDisk.h"
#include "LibFS.h"

// global errno value here
int osErrno;

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
    if(Disk_Load(file) == -1){
        //if not found
        if(File_Create(path) == -1){
            osErrno = E_GENERAL;
            return -1;
        }
    } else {
        //check to make sure correct size.
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