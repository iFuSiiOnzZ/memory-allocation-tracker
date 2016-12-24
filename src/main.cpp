
#include "debug_memory\debug_memory.h"

#include <windows.h>    /* HANDLE, CreateThread */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


#define UNUSED(x) (void)(x)

void leak_function_1()
{
    debug_malloc(4);
}

void leak_function_2()
{
    debug_malloc(512);
}

void leak_function_3()
{
    leak_function_2();
    debug_malloc(4128);
}

void leak_function_4()
{
    leak_function_3();
}

void leak_function_5()
{
    leak_function_4();   
}


static void *t1[10] = { 0 };
static void *t2[10] = { 0 };

unsigned long __stdcall thread_alloc(void *p)
{
    for(int i = 0; i < 10; ++i)  (p != NULL ? t1[i] : t2[i]) = debug_malloc(rand() % (1024 * 1024) + 1024);
    return 0;
}


unsigned long __stdcall thread_free(void *p)
{
    for(int i = 0; i < 10; ++i) debug_free(p != NULL ? t1[i] : t2[i]);
    return 0;
}


int main(int argc, char *argv[])
{
    UNUSED(argc); UNUSED(argv);
    srand ((int) time(NULL));

    leak_function_5();
    debug_malloc(32);

    leak_function_2();
    debug_malloc(512);

    leak_function_1();
    debug_malloc(128);

    debug_print();

    HANDLE h1[2] = {
      CreateThread(NULL, 0, thread_alloc, (void *) 0x1, 0, NULL),
      CreateThread(NULL, 0, thread_alloc, NULL, 0, NULL)
    };

    WaitForMultipleObjects(2, h1, TRUE, INFINITE);
    debug_print();

    HANDLE h2[2] = {
      CreateThread(NULL, 0, thread_free, (void *) 0x1, 0, NULL),
      CreateThread(NULL, 0, thread_free, NULL, 0, NULL)
    };

    WaitForMultipleObjects(2, h2, TRUE, INFINITE);
    debug_print();

    return EXIT_SUCCESS;
}