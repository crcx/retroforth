You will need to compile the C source to a shared object. Pass
the same compiler flags as used when building the VM.

E.g.,

    cc -DFAST   -DENABLE_FLOATS -DENABLE_FILES -DENABLE_UNIX \
       -DENABLE_RNG -DENABLE_CLOCK -DENABLE_SCRIPTING \
       -DENABLE_SIGNALS -DENABLE_MULTICORE -DENABLE_FFI \
       -DENABLE_UNSIGNED -O2 \
       -shared test.c -o test.so

Then run the test sample:

    ../bin/retro ffi.retro
