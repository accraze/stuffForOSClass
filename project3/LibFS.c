#include "LibDisk.h"
#include "LibFS.h"
#include <cstring>
#include <libgen.h>
#include <errno.h>
#include <iterator>
#include <iostream>

using namespace std;

#define MAGIC_NUMBER 1337
#define DIR_SIZE 32
#define INODE_OFFSET 5
#define DATA_OFFSET 255
#define MAX_OPEN_FILES 256
#define MAX_FILE_SIZE 30

/////////////////////////////
//      GLOBALS
////////////////////////////

// global errno value here
int osErrno;

char* name;

bool isEmpty = false; 

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
char* openList [MAX_OPEN_FILES];
int openFileIndex = 0;

char dirInodeSector[4];
int dirInodeSectorIndex = 0;
int pointerIndex = 0;

char dirSector[16];
int dirSectorIndex = 0;

char fileInodeSector[4];
int fileInodeSectorIndex = 0;
int filePointerIndex = 0;

char inodeBitmap[SECTOR_SIZE];
char dataBitmap[SECTOR_SIZE];

int dirInodeCounter = 0;
int dataBlockCounter = 0;
int dirDataCounter = 0;

int fileInodeCounter = 0;
int fileDataCounter = 0;

char* diskName;

char* createdFileList[SECTOR_SIZE];
int fileCreatePointer = 0;
char* createdDirList[SECTOR_SIZE];
int dirCreatePointer = 0;

iNode inodeTemp;
Dir dirTemp;


//Private Declarations
int _check(char* file);
int _check2(char* file);
bool _isOpen(char* file);
int _addToRootDir(char* path);
int _checkRootDir(char* path);
void _updateDirCounters(char* path);
void _updateFileCounters(char* path);
bool _checkIfFileExists(char* file);
bool _checkIfDirExists(char* dir);
int _makeInode(char* path, char type);
int _makeDataBlock(char* path, char type);
int _getInodeNumber(int dataBlockNum, char *dirName);
int _getFileNumFromDir(char* fileName, int inodeNum);
void _persistLoad();
void _assistFile();
void _assistDir();

int _makeInode(char* path, char type){
/*
    Handles inode creation process.
*/
    if(type == 'd'){
        //printf("inode for dir %s\n", path);
        inodeBitmap[dirInodeCounter] = (char)1;

        //write inodeBitmap
        if(Disk_Write(1, inodeBitmap) == -1){
                osErrno = E_CREATE;
                return -1;
        }
        
        //initialize inode
        inodeTemp.fileSize = DIR_SIZE;
        inodeTemp.fileType = 1; 
        inodeTemp.pointers[0] = path; 

        
        //add inode to sector buffer
        char* const buf = reinterpret_cast<char*>(&inodeTemp);
        dirInodeSector[dirDataCounter] = *buf;
        
        //write inode sector to disk
        if(Disk_Write(dirInodeCounter+INODE_OFFSET, dirInodeSector) == -1){
                osErrno = E_CREATE;
                return -1;
        }
    }

    if(type == 'f'){
        //printf("inode for file\n");
        //get inode bitmap num
        while (inodeBitmap[fileInodeCounter] == '1'){
            fileInodeCounter++;
        }

        //if new.... allocate?
        inodeBitmap[fileInodeCounter] = (char)1;
        
        //intialize inode for file
        inodeTemp.fileSize = 0; // files initialize to size zero
        inodeTemp.fileType = 0; // file 
        inodeTemp.pointers[0] =  path;

        //write inodeBitmap
        if(Disk_Write(1, inodeBitmap) == -1){
                osErrno = E_CREATE;
                return -1;
        }
        
        //add inode to sector
        char* const buf = reinterpret_cast<char*>(&inodeTemp);
        fileInodeSector[fileInodeSectorIndex] = *buf;
        
        //write inode sector
        if(Disk_Write(fileInodeCounter+INODE_OFFSET, fileInodeSector) == -1){
                osErrno = E_CREATE;
                return -1;
        }
    }

    return 0;
}

int _makeDataBlock(char* path, char type) {
/*
    Handles data block creation process.
*/
    //printf("data block!\n");
    if(type == 'd'){
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
    }

    if(type == 'f'){
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

        
    }

    return 0;    
}

int 
FS_Boot(char *path)
{
    /* 
        must be called exactly once before any other LibFS functions
        are called. It takes a single argument, the path, which either 
        points to a real file where your “disk image” is stored, or to a 
        file that does not yet exist and which must be created to hold a 
        new disk image. Upon success, return 0. Upon failure, return -1 
        and set osErrno to E GENERAL.
    */
    printf("FS_Boot %s\n", path);

    diskName = path;

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
        _persistLoad();
    }

    isEmpty = false; 
    
    return 0;
}

int
FS_Sync()
{
 /*
    FS Sync() ensures the contents of the file system are stored persistently on disk. More details on how
    this is accomplished using LibDisk are described in §3.1. Upon success, return 0. Upon failure, return
    -1 and set osErrno to E GENERAL.
 */
    printf("FS_Sync\n");
    

    //save to disk
    if(Disk_Save(diskName) == -1){
            osErrno = E_GENERAL;
            return -1;
    }

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

    bool exists = _checkIfFileExists(file);
    
    if (exists){
        osErrno = E_CREATE;
        return -1;
    }

    if(_makeDataBlock(file, 'f') == -1){
                osErrno = E_CREATE;
                return -1;
    }

    if(_makeInode(file, 'f') == -1){
            osErrno = E_CREATE;
            return -1;
    }

    _updateFileCounters(file);
    
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
    
    char temp = dirInodeSector[0];
    memcpy(&inodeTemp, &temp, sizeof(iNode));

    int dataIndex = -1;

    //first check for directory match
    for(int i = 0; i < 30; i++){
        if(inodeTemp.pointers[i] == dirname(file)){
            dataIndex = i;
        }
    }

    // no such directory...then look for file
    if(dataIndex == -1){
        for(int i = 0; i < 30; i++){
            if(inodeTemp.pointers[i] == basename(file)){
                dataIndex = i;
            }
        }

        //no such file
        if(dataIndex == -1){ 
            osErrno = E_NO_SUCH_FILE;
            return -1;
        }
    } else {
        //now get directory
        int inodeNum = _getInodeNumber(dataIndex, dirname(file));

        int fd = _getFileNumFromDir(basename(file), inodeNum);

        //no file found in dir
        if(fd == -1){
            osErrno = E_NO_SUCH_FILE;
            return -1; 
        }

        dataIndex = fd;

    }

    //get the integer file descriptor and return it!
    openFileTable[openFileIndex] = dataIndex + DATA_OFFSET;
    openList[openFileIndex] = file;
    openFileIndex++;

    return openFileIndex - 1;
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

    // bool exist = _checkIfDirExists(path);
    // if(exist){
    //     osErrno = E_CREATE;
    //     return -1;
    // }

    //do we have a new image?
    if(isEmpty == true){
        printf("!first dir entry\n");
        isEmpty = false;

        if(_makeDataBlock(path, 'd') == -1){
                osErrno = E_CREATE;
                return -1;
        }

        if(_makeInode(path, 'd') == -1){
                osErrno = E_CREATE;
                return -1;
        }

        _updateDirCounters(path);
    } else {
        //printf("has root persist\n");
        // image persists
        //char rootdir = dirname(path);
        //printf("!!path root %s\n", rootdir);
        

        if(_makeDataBlock(path, 'd') == -1){
                    osErrno = E_CREATE;
                    return -1;
        }

        if(_makeInode(path, 'd' ) == -1){
                osErrno = E_CREATE;
                return -1;
        }
        
        _updateDirCounters(path);
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

void _assistFile(){
    //Debug
    Disk_Write(9999, (char*) createdFileList);
    FS_Sync();
}

void _assistDir(){
    //Debug
    Disk_Write(9998, (char*) createdDirList);
}

void _persistLoad(){
    //Debug
    char* buff[SECTOR_SIZE];
    Disk_Read(9999, (char*)buff);
    //std::copy(std::begin(buff), std::end(buff), std::begin(createdFileList));
    memcpy ( createdFileList, buff, SECTOR_SIZE);
    Disk_Read(9998, (char*)buff);
    //std::copy(std::begin(buff), std::end(buff), std::begin(createdDirList));
    memcpy ( createdDirList, buff, SECTOR_SIZE);


    //cout <<"HEY" << createdFileList[1];
}

bool _checkIfDirExists(char* dir) {
/*
    check if dir exists
*/
    //printf("Check If Fle Exists\n" );
    // char* dir = dirname(file);

    // if(dir == "/"){
    //     if(_checkRootDir(dir) != -1){
    //         return true;
    //     }
    // } else {
    //     int dataNum = _checkRootDir(dir);
        
        // if(dataNum != -1){
        //     //now go check new inode
        //     int inodeNum = _getInodeNumber(dataNum, dir);
        //     Disk_Read(inodeNum, dirInodeSector);

            //iterate thru buffer
            // for(int i = 0; i < 4; i++){
            //     char temp = dirInodeSector[i];
            //     memcpy(&inodeTemp, &temp, sizeof(iNode));

            //     //check pointers for name match
            //     for(int j = 0; j < 30; i++){
            //         if(inodeTemp.pointers[j] == file){
            //             return true;
            //         }
            //     }
            // }
        //}

    int c = _check2(dir);
    
    if(c != -1){
        return true;
    }
    //}

    return false;
}

void _updateDirCounters(char* path){
/*
    Updates all counter variables for directory mgmt
*/
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
    createdDirList[dirCreatePointer] = path;
    _assistDir();
    dirCreatePointer++;
}

void _updateFileCounters(char*path) {
/*
    Updates all counter variables for file mgmt
*/
    //full file inode data sector then advance index
    if(fileInodeSectorIndex == 3){
        fileInodeCounter++; 

        //reset directory counter
        fileInodeSectorIndex = 0;
    }
    else {
        //advance directory index in sector
        fileInodeSectorIndex++;
    }

    createdFileList[fileCreatePointer] = path;
    _assistFile();
    fileCreatePointer++;
}

int _addToRootDir(char* path){
/*
    Adds an element to the root directory
*/
    printf("add to root %s\n", path);
    //read in root directory
    Disk_Read(5, dirInodeSector);

    char temp = dirInodeSector[0];
    memcpy(&inodeTemp, &temp, sizeof(iNode));

    //add to root
    int dataIndex = -1;

    //add name to root
    for(int i = 0; i < 30; i++){
        //check for dupes
        if(inodeTemp.pointers[i] == basename(path)){
            osErrno = E_CREATE;
            return -1;
        }

        if(inodeTemp.pointers[i] == NULL){
            inodeTemp.pointers[i] = basename(path);
            dataIndex = i;
            break;
        }
    }

    return 0;
}

int _getInodeNumber(int dataBlockNum, char *dirName){

    char buffer[ SECTOR_SIZE ];

    if(Disk_Read(dataBlockNum, buffer)== -1){
            return -1;
    }
    
    
    for(int i = 0; i < 16; i++){
        //
        struct dir* dirTemp = (dir* )buffer[i];

        if(dirTemp->name == dirName){
            return dirTemp->inodeNumber;
        }
    }

    return -1;
}

int _checkRootDir(char* path) {

    //printf("Check Root Dir\n" );
    
    Disk_Read(5, dirInodeSector);

    char temp = dirInodeSector[0];
    memcpy(&inodeTemp, &temp, sizeof(iNode));


    //check to see if 
    for(int i = 0; i < 30; i++){
        //printf("%c /n", inodeTemp.pointers[i] );
        if(inodeTemp.pointers[i] == path){
            //printf("yeppp\n" );
            return i + DATA_OFFSET;
        }
    }

    return -1;
}

int _check(char* file){
    //debug function
    //printf("Checkingggg\n" );
    int i = 0;

    char* buff[SECTOR_SIZE];
    Disk_Read(9999, (char*)buff);
    
    for(i = 0; i < 10; i++){
        if (buff[i] == file){
            return 0;
        }
    }

    return -1;
}

int _check2(char* file){
    int i = 0;
    
    char* buff[SECTOR_SIZE];
    Disk_Read(9998, (char*)buff);
    
    for(i = 0; i < 10; i++){
        if (buff[i] == file){
            return 0;
        }
    }

    return -1;
}

bool isOpen(int index){
    if(openFileTable[index] == NULL){
        return false;
    }

    return true;
}

bool _checkIfFileExists(char* file) {
/*
    check if file exists
*/
    //printf("Check If Fle Exists\n" );
    // char* dir = dirname(file);

    // if(dir == "/"){
    //     if(_checkRootDir(dir) != -1){
    //         return true;
    //     }
    // } else {
    //     int dataNum = _checkRootDir(dir);
        
        // if(dataNum != -1){
        //     //now go check new inode
        //     int inodeNum = _getInodeNumber(dataNum, dir);
        //     Disk_Read(inodeNum, dirInodeSector);

            //iterate thru buffer
            // for(int i = 0; i < 4; i++){
            //     char temp = dirInodeSector[i];
            //     memcpy(&inodeTemp, &temp, sizeof(iNode));

            //     //check pointers for name match
            //     for(int j = 0; j < 30; i++){
            //         if(inodeTemp.pointers[j] == file){
            //             return true;
            //         }
            //     }
            // }
        //}

    int c = _check(file);
    
    if(c != -1){
        return true;
    }
    //}

    return false;
}

int _getFileNumFromDir(char* fileName, int inodeNum){
    char buffer[ SECTOR_SIZE ];

    if(Disk_Read(inodeNum, buffer)== -1){
            return -1;
    }

    for(int i = 0; i < 4; i++){
        struct inode* tmp = (inode* )buffer[i];
        
        if(tmp->fileType == 1){
            for(int j = 0; j < 30; j++){
                  if(inodeTemp.pointers[j] == basename(fileName)){
                    return j+DATA_OFFSET; // return the datablock number
                }  
            }
        }
    }

    return -1;
}


