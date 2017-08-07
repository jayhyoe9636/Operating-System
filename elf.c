# Operating-System
OS project
/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>  /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/elf.h>


/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat)
{
    elfHeader *elfh = (elfHeader *)exeFileData;
    programHeader *prgh;
    int i;

    if (exeFileData == 0)
    {
        Print("Executable file error\n");
        return -1;
    }

    if (elfh->ident[0] != 0x7f || elfh->ident[1] != 'E' || elfh->ident[2] != 'L' || elfh->ident[3] != 'F')
    {
        Print("Not ELF file\n");
        return -1;
    }

    exeFormat->numSegments = elfh->phnum;
    exeFormat->entryAddr = elfh->entry;
    prgh = (programHeader *)(exeFileData + elfh->phoff);

    for (i = 0; i<elfh->phnum; i++)
    {
        exeFormat->segmentList[i].offsetInFile = prgh->offset;
        exeFormat->segmentList[i].lengthInFile = prgh->fileSize;
        exeFormat->segmentList[i].startAddress = prgh->vaddr;
        exeFormat->segmentList[i].sizeInMemory = prgh->memSize;
        exeFormat->segmentList[i].protFlags = prgh->flags;
        prgh++;
    }

    return 0;
}
