# Memory Allocation Traker

Small API that keeps track of the of the allocated memory in the system <br />

### How it works

Include `debug_memory.h` (tested in Window 10 x64 under Visual C++ Build Tools 2015 - standalone C++ tools) <br />
`debug_malloc(size_t)` allocate memory, overrides _malloc_ in debug <br />
`debug_free(void *)` free memory, overrides _free_ in debug <br />
`debug_print()` print the allocated memory to stdout <br />

Each memory allocation generats a node in a linked list with file name, file line, size of the allocation and call stack. 
When memory is free the node coresponding to the allocation is searched and remove from the list. 
To achieve thread safety a spin lock is used making each thread accessing sequentially.

### Remarks

At the moment call stack and multithreading safety are only available for Windows platform, 
otherwise a warning will be shown at compilation time showing that those features aren't available.


### Output
```
======================== MEMORY ALLOCATIONS ========================
  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    44)      128 bytes
           0                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    46)
           1   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           2      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           3       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line     9)        4 bytes
           0          leak_function_1 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    10)
           1                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    44)
           2   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           3      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           4       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    41)      512 bytes
           0                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    43)
           1   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           2      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           3       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    14)      512 bytes
           0          leak_function_2 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    15)
           1                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    41)
           2   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           3      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           4       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    38)       32 bytes
           0                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    40)
           1   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           2      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           3       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    20)     4128 bytes
           0          leak_function_3 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    21)
           1          leak_function_4 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    26)
           2          leak_function_5 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    31)
           3                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    38)
           4   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           5      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           6       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

  >  f:\programing\debugmemory\sourcecode\src\src\main.cpp (Line    14)      512 bytes
           0          leak_function_2 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    15)
           1          leak_function_3 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    20)
           2          leak_function_4 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    26)
           3          leak_function_5 (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    31)
           4                     main (           f:\programing\debugmemory\sourcecode\src\src\main.cpp >    38)
           5   __scrt_common_main_seh (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           6      BaseThreadInitThunk (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)
           7       RtlUserThreadStart (          f:\dd\vctools\crt\vcstartup\src\startup\exe_common.inl >   253)

====================================================================
  >>>  5828 bytes in 7 allocations
```