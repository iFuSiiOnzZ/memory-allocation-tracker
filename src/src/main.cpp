
#include <debug_memory.h>


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

int main(int argc, char *argv[])
{
    UNUSED(argc); UNUSED(argv);

    leak_function_5();
    debug_malloc(32);

    leak_function_2();
    debug_malloc(512);

    leak_function_1();
    debug_malloc(128);

    debug_print();
    return EXIT_SUCCESS;
}