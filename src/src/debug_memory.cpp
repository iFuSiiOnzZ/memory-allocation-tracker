#include <debug_memory.h>
#include <stdio.h>

typedef struct debug_memory_record
{
    void *Memory;
    char *FileName;

    size_t FileLine;
    size_t MemorySize;

    char CallStack[48][1024];
    size_t CallStackSize;

    struct debug_memory_record *Prev;
} debug_memory_record;

static debug_memory_record *LinkedListHead = NULL;


///////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

static void SetCallStack(debug_memory_record *DebugRecord)
{
    void *callers[48] = { 0 };
    SymInitialize(GetCurrentProcess(), NULL, TRUE);

    int nCaptured = CaptureStackBackTrace(2, 48, callers, NULL);
    DebugRecord->CallStackSize = nCaptured;

    IMAGEHLP_LINE64 SimbolLine = { 0 };
    SymSetOptions(SYMOPT_LOAD_LINES);

    for(int i = 0; i < nCaptured; i++)
    {
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {  0 };
        SYMBOL_INFO *Symbol = (PSYMBOL_INFO)buffer;

        Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        Symbol->MaxNameLen = MAX_SYM_NAME;

        SymFromAddr(GetCurrentProcess(), (DWORD64) callers[i], 0, Symbol);
        SimbolLine.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        unsigned long Displacement = 0;
        SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64) callers[i], &Displacement, &SimbolLine);

        sprintf_s(DebugRecord->CallStack[i], 1024, "%24s (%64s > % 5d)", Symbol->Name, SimbolLine.FileName, SimbolLine.LineNumber);
    }

    SymCleanup(GetCurrentProcess());
}
///////////////////////////////////////////////////////////////////////////////////////////////

static void bzero(void *s1, size_t n)
{
    char *t = (char *)s1;
    while (n-- != 0) *t++ = 0;
    
}

void *debug_malloc_imp(size_t size, char *FileName, int FileLine)
{
    debug_memory_record *NewRecord = (debug_memory_record *) malloc(sizeof(debug_memory_record));
    bzero(NewRecord, sizeof(debug_memory_record));
    SetCallStack(NewRecord);

    NewRecord->FileName = FileName;
    NewRecord->FileLine = FileLine;

    NewRecord->MemorySize = size;
    NewRecord->Prev = LinkedListHead;

    NewRecord->Memory = malloc(size);
    LinkedListHead = NewRecord;

    return NewRecord->Memory;
}

void debug_free_imp(void *Mem)
{
    debug_memory_record **SearchNode = &LinkedListHead;
    free(Mem);

    while(*SearchNode != NULL && (*SearchNode)->Memory != Mem)
    {
        SearchNode = &(*SearchNode)->Prev;
    }

    if(*SearchNode == NULL)
    {
        return;
    }

    debug_memory_record *DeleteNode = *SearchNode;
    *SearchNode = DeleteNode->Prev;

    free(DeleteNode);
}

void debug_print_imp(void)
{
    debug_memory_record *Node = LinkedListHead;
    size_t TotalBytes = 0, RemainingAllocations = 0;

    printf("======================== MEMORY ALLOCATIONS ========================\n");

    while(Node != NULL)
    {
        printf("  >  %s (Line % 5zd)    % 5zd bytes\n", Node->FileName, Node->FileLine, Node->MemorySize);
        for(int i = 0; i < Node->CallStackSize; ++i) printf("          % 2d %s\n", i, Node->CallStack[i]);
        if(Node->CallStackSize) printf("\n");

        TotalBytes += Node->MemorySize; 
        ++RemainingAllocations;

        Node = Node->Prev;
    }


    printf("====================================================================\n");
    printf("  >>>  %zd bytes in %zd allocations\n", TotalBytes, RemainingAllocations);
}