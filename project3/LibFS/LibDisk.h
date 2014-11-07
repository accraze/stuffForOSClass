{\rtf1\ansi\ansicpg1252\cocoartf1038\cocoasubrtf360
{\fonttbl\f0\fmodern\fcharset0 Courier;}
{\colortbl;\red255\green255\blue255;}
\margl1440\margr1440\vieww9000\viewh8400\viewkind0
\deftab720
\pard\pardeftab720\ql\qnatural

\f0\fs24 \cf0 //\
// Disk.h\
//\
// Emulates a very simple disk (no timing issues). Allows user to\
// read and write to the disk just as if it was dealing with sectors\
//\
//\
\
#ifndef __Disk_H__\
#define __Disk_H__\
\
#include <stdio.h>\
#include <stdlib.h>\
#include <unistd.h>\
\
// a few disk parameters\
#define SECTOR_SIZE  512\
#define NUM_SECTORS  10000 \
\
// disk errors\
typedef enum \{\
  E_MEM_OP,\
  E_INVALID_PARAM,\
  E_OPENING_FILE,\
  E_WRITING_FILE,\
  E_READING_FILE,\
\} Disk_Error_t;\
\
typedef struct sector \{\
  char data[SECTOR_SIZE];\
\} Sector;\
\
extern Disk_Error_t diskErrno; // used to see what happened w/ disk ops\
\
int Disk_Init();\
int Disk_Save(char* file);\
int Disk_Load(char* file);\
int Disk_Write(int sector, char* buffer);\
int Disk_Read(int sector, char* buffer);\
\
#endif // __Disk_H__\
}