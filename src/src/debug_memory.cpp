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

typedef struct mutex_ticket
{
    unsigned long long volatile Ticket;
    unsigned long long volatile Serving;
} mutex_ticket;

///// Global variables
static debug_memory_record *GlobalLinkedListHead = NULL;
static mutex_ticket GlobalMutex = { 0 };

///// Platform specific functions
static inline unsigned long long AtomicAddU64(unsigned long long volatile *Value, unsigned long long Addend);
static void SetCallStack(debug_memory_record *DebugRecord);

static inline void BeginMutex(mutex_ticket *Mutex);
static inline void EndMutex(mutex_ticket *Mutex);

///// General functions
static void bzero(void *s1, size_t n)
{
    char *t = (char *)s1;
    while (n-- != 0) *t++ = 0;
}

void *debug_malloc_imp(size_t size, char *FileName, int FileLine)
{
    void *Memory = malloc(size);
    if (!Memory) return Memory;

    debug_memory_record *NewRecord = (debug_memory_record *) malloc(sizeof(debug_memory_record));
    if(!NewRecord) return Memory;

    bzero(NewRecord, sizeof(debug_memory_record));
    NewRecord->Memory = Memory;

    NewRecord->FileName = FileName;
    NewRecord->FileLine = FileLine;

    NewRecord->MemorySize = size;
    NewRecord->Prev = GlobalLinkedListHead;

    BeginMutex(&GlobalMutex);
        GlobalLinkedListHead = NewRecord;
        SetCallStack(NewRecord);
    EndMutex(&GlobalMutex);

    return Memory;
}

void debug_free_imp(void *Mem)
{
    free(Mem);
    debug_memory_record **SearchNode = &GlobalLinkedListHead;

    while(*SearchNode != NULL && (*SearchNode)->Memory != Mem)
    {
        SearchNode = &(*SearchNode)->Prev;
    }

    if(*SearchNode == NULL)
    {
        return;
    }

    BeginMutex(&GlobalMutex);
        debug_memory_record *DeleteNode = *SearchNode;
        *SearchNode = DeleteNode->Prev;
    EndMutex(&GlobalMutex);

    free(DeleteNode);
}

void debug_print_imp(void)
{
    debug_memory_record *Node = GlobalLinkedListHead;
    size_t TotalBytes = 0, RemainingAllocations = 0;

    printf("======================== MEMORY ALLOCATIONS ========================\n");

    while(Node != NULL)
    {
        printf("  >  %s (Line % 5zd)    % 5zd bytes\n", Node->FileName, Node->FileLine, Node->MemorySize);
        for(unsigned int i = 0; i < Node->CallStackSize; ++i) printf("          % 2d %s\n", i, Node->CallStack[i]);
        if(Node->CallStackSize) printf("\n");

        TotalBytes += Node->MemorySize; 
        ++RemainingAllocations;

        Node = Node->Prev;
    }


    printf("====================================================================\n");
    printf("  >>>  %zd bytes in %zd allocations\n", TotalBytes, RemainingAllocations);
}


///////////////////////////////////////////////////////////////////////////////////////////////

#if defined(WIN64) || defined(WIN32)
    #include <windows.h>
    #include <immintrin.h>

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

        for (int i = 0; i < nCaptured; i++)
        {
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = { 0 };
            SYMBOL_INFO *Symbol = (PSYMBOL_INFO)buffer;

            Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            Symbol->MaxNameLen = MAX_SYM_NAME;

            SymFromAddr(GetCurrentProcess(), (DWORD64)callers[i], 0, Symbol);
            SimbolLine.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            unsigned long Displacement = 0;
            SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)callers[i], &Displacement, &SimbolLine);

            sprintf_s(DebugRecord->CallStack[i], 1024, "%24s (%64s > % 5d)", Symbol->Name, SimbolLine.FileName, SimbolLine.LineNumber);
        }

        SymCleanup(GetCurrentProcess());
    }

    static inline unsigned long long AtomicAddU64(unsigned long long volatile *Value, unsigned long long Addend)
    {
        return _InterlockedExchangeAdd64((__int64 volatile *)Value, Addend);
    }

    static inline void BeginMutex(mutex_ticket *Mutex)
    {
        unsigned long long Ticket = AtomicAddU64(&Mutex->Ticket, 1);
        while (Ticket != Mutex->Serving) { _mm_pause(); }
    }

    static inline void EndMutex(mutex_ticket *Mutex)
    {
        AtomicAddU64(&Mutex->Serving, 1);
    }
#else
    #ifndef UNUSED
        #define UNUSED(x) (void)x
    #endif

    #ifndef STRINGIFY
        #define STRINGIFY_(x) #x
        #define STRINGIFY(x) STRINGIFY_(x)
    #endif

    #pragma message (__FILE__ "(" STRINGIFY(__LINE__) "): warning: No multithreading and advanced callstack support")

    static void SetCallStack(debug_memory_record *DebugRecord)
    {
        UNUSED(DebugRecord);
    }

    static inline unsigned long long AtomicAddU64(unsigned long long volatile *Value, unsigned long long Addend)
    {
        UNUSED(Value); UNUSED(Addend);
    }

    static inline void BeginMutex(mutex_ticket *Mutex)
    {
        UNUSED(Mutex);
    }

    static inline void EndMutex(mutex_ticket *Mutex)
    {
        UNUSED(Mutex);
    }
#endif
///////////////////////////////////////////////////////////////////////////////////////////////