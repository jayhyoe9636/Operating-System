/*
 * Segmentation-based user mode implementation
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.23 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/defs.h>
#include <geekos/mem.h>
#include <geekos/string.h>
#include <geekos/malloc.h>
#include <geekos/int.h>
#include <geekos/gdt.h>
#include <geekos/segment.h>
#include <geekos/tss.h>
#include <geekos/kthread.h>
#include <geekos/argblock.h>
#include <geekos/user.h>

/* ----------------------------------------------------------------------
 * Variables
 * ---------------------------------------------------------------------- */

#define DEFAULT_USER_STACK_SIZE 8192


/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */


/*
 * Create a new user context of given size
 */

/* TODO: Implement*/
static struct User_Context* Create_User_Context(ulong_t size)
{
    struct User_Context *uc = NULL;
    int i;

    uc = (struct User_Context *)Malloc(sizeof(*uc));
    if (uc == NULL)
        return 0;
    else
    {
        size = Round_Up_To_Page(size);
        uc->size = size;
        uc->memory = Malloc(size);
    }

    if (uc->memory == 0)
        return 0;
    else
        memset(uc->memory, '\0', uc->size);

    uc->ldtDescriptor = Allocate_Segment_Descriptor();
    if (uc->ldtDescriptor == 0)
        return 0;

    Init_LDT_Descriptor(uc->ldtDescriptor, uc->ldt, NUM_USER_LDT_ENTRIES);
    i = Get_Descriptor_Index(uc->ldtDescriptor);
    uc->ldtSelector = Selector(KERNEL_PRIVILEGE, true, i);

    Init_Code_Segment_Descriptor(&uc->ldt[0], (ulong_t)uc->memory, size/PAGE_SIZE, USER_PRIVILEGE);
    Init_Data_Segment_Descriptor(&uc->ldt[1], (ulong_t)uc->memory, size/PAGE_SIZE, USER_PRIVILEGE);
    uc->csSelector = Selector(USER_PRIVILEGE, false, 0);
    uc->dsSelector = Selector(USER_PRIVILEGE, false, 1);

    uc->refCount = 0;

    return uc;
}


static bool Validate_User_Memory(struct User_Context* userContext,
    ulong_t userAddr, ulong_t bufSize)
{
    ulong_t avail;

    if (userAddr >= userContext->size)
        return false;

    avail = userContext->size - userAddr;
    if (bufSize > avail)
        return false;

    return true;
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Destroy a User_Context object, including all memory
 * and other resources allocated within it.
 */
void Destroy_User_Context(struct User_Context* userContext)
{
    /*
     * Hints:
     * - you need to free the memory allocated for the user process
     * - don't forget to free the segment descriptor allocated
     *   for the process's LDT
     */
    // TODO("Destroy a User_Context");
    KASSERT(userContext->refCount == 0);

    Free_Segment_Descriptor(userContext->ldtDescriptor);

    Disable_Interrupts();
    Free(userContext->memory);
    Free(userContext);
    Enable_Interrupts();
}

/*
 * Load a user executable into memory by creating a User_Context
 * data structure.
 * Params:
 * exeFileData - a buffer containing the executable to load
 * exeFileLength - number of bytes in exeFileData
 * exeFormat - parsed ELF segment information describing how to
 *   load the executable's text and data segments, and the
 *   code entry point address
 * command - string containing the complete command to be executed:
 *   this should be used to create the argument block for the
 *   process
 * pUserContext - reference to the pointer where the User_Context
 *   should be stored
 *
 * Returns:
 *   0 if successful, or an error code (< 0) if unsuccessful
 */
int Load_User_Program(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat, const char *command,
    struct User_Context **pUserContext)
{
    /*
     * Hints:
     * - Determine where in memory each executable segment will be placed
     * - Determine size of argument block and where it memory it will
     *   be placed
     * - Copy each executable segment into memory
     * - Format argument block in memory
     * - In the created User_Context object, set code entry point
     *   address, argument block address, and initial kernel stack pointer
     *   address
     */
    // TODO("Load a user executable into a user memory space using segmentation");
    struct User_Context *uc = NULL;
    struct Exe_Segment *es = NULL;
    ulong_t abSize, stSize, abAddr;
    unsigned aNum;
    ulong_t top, max = 0;
    int i;

    for (i=0; i<exeFormat->numSegments; i++) {
        es = &exeFormat->segmentList[i];
        top = es->startAddress + es->sizeInMemory;

        if (top > max)
            max = top;
    }

    Get_Argument_Block_Size(command, &aNum, &abSize);

    stSize = Round_Up_To_Page(max) + DEFAULT_USER_STACK_SIZE;
    abAddr = stSize;
    stSize += abSize;

    uc = Create_User_Context(stSize);
    if (uc == NULL)
        return -1;

    for (i=0; i<exeFormat->numSegments; i++) {
        es = &exeFormat->segmentList[i];
        memcpy(uc->memory + es->startAddress, exeFileData + es->offsetInFile, es->lengthInFile);
    }

    Format_Argument_Block(uc->memory + abAddr, aNum, abAddr, command);
    uc->entryAddr = exeFormat->entryAddr;
    uc->argBlockAddr = abAddr;
    uc->stackPointerAddr = abAddr;

    *pUserContext = uc;

    return 0;
}

/*
 * Copy data from user memory into a kernel buffer.
 * Params:
 * destInKernel - address of kernel buffer
 * srcInUser - address of user buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_From_User(void* destInKernel, ulong_t srcInUser, ulong_t bufSize)
{
    /*
     * Hints:
     * - the User_Context of the current process can be found
     *   from g_currentThread->userContext
     * - the user address is an index relative to the chunk
     *   of memory you allocated for it
     * - make sure the user buffer lies entirely in memory belonging
     *   to the process
     */
    // TODO("Copy memory from user buffer to kernel buffer");
    // Validate_User_Memory(NULL,0,0); /* delete this; keeps gcc happy */
    struct User_Context *uc = g_currentThread->userContext;
    uchar_t *mem = (uchar_t *)uc->memory;

    if (!Validate_User_Memory(uc, srcInUser, bufSize))
        return false;
    memcpy(destInKernel, (void *)(mem + srcInUser), bufSize);

    return true;
}

/*
 * Copy data from kernel memory into a user buffer.
 * Params:
 * destInUser - address of user buffer
 * srcInKernel - address of kernel buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_To_User(ulong_t destInUser, void* srcInKernel, ulong_t bufSize)
{
    /*
     * Hints: same as for Copy_From_User()
     */
    // TODO("Copy memory from kernel buffer to user buffer");
    struct User_Context *uc = g_currentThread->userContext;
    uchar_t *mem = (uchar_t *)uc->memory;

    if (!Validate_User_Memory(uc, destInUser, bufSize))
        return false;
    memcpy((void *)(mem + destInUser), srcInKernel, bufSize);

    return true;
}

/*
 * Switch to user address space belonging to given
 * User_Context object.
 * Params:
 * userContext - the User_Context
 */
void Switch_To_Address_Space(struct User_Context *userContext)
{
    /*
     * Hint: you will need to use the lldt assembly language instruction
     * to load the process's LDT by specifying its LDT selector.
     */
    // TODO("Switch to user address space using segmentation/LDT");
    __asm__ __volatile__("lldt %0"::"a"(userContext->ldtSelector));
}
