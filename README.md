# cmake dependency test

CMake is not smart about tracking inter-file dependencies through C++ `#include`
statements. This makes write a dual header-only/static library like
[libigl](https://github.com/libigl/libigl) difficult.

## Option A

In this option we consider the case where each header file includes its
corresponding implementation .cpp if the `STATIC_LIBRARY` macro is not defined.
For example, at the bottom of `A2.h` you'll see:

    #ifndef STATIC_LIBRARY
    #include "A2.cpp"
    #endif

We'll see that this works well for tracking changes in header-only mode (when
`STATIC_LIBRARY` is _not_ defined), but works poorly when in static-library mode
(when `STATIC_LIBRARY` is _not_ defined). Since `A1.h` seemingly includes
`A1.cpp` which includes `A2.h` and in turn `A2.cpp`, a change to `A2.cpp`
triggers cmake to think that `B2.cpp` needs to be recompiled. This is a
deficiency of cmake: the `include "A2.cpp"` line should never be reached because
`STATIC_LIBRARY` is defined.

## Option B

To try to trick cmake into not seeing this include when `STATIC_LIBRARY` _is_
defined, I tried constructing the include statement using a macro. For example,
at the bottom of `B2.h` you'll see:

    #ifndef STATIC_LIBRARY
    #define B2_CPP "B2.cpp"
    #include B2_CPP
    #endif

This works great for static-library mode: changing `B2.cpp` only triggers
recompiling `B2` and not `B1`, but it fails (for the same reason as option A) in
header-only mode: a change in `B2` does not trickle up to a registered change in
the main compilation.

## Test

### Header Only

In header-only mode (when `STATIC_LIBRARY` is _not_ defined), A works well.
If you issue:

    mkdir build-header-only
    cd build-header-only
    cmake ../
    make 

Both executables `Abin` and `Bbin` build correctly. Then suppose you made a
change to the implementation of `*2` in `*2.cpp`, simulated issuing

    touch ../A2.cpp ../B2.cpp

If you rebuild you'll see that **only Abin** is recompiled. The B option did not
register the change to `B2`:

    make 

we see

    [ 75%] Building CXX object CMakeFiles/Abin.dir/A.cpp.o


### Static Library Mode

In static-library-only mode (when `STATIC_LIBRARY` _is_ defined), B works well.
If you issue:

    mkdir build-static
    cd build-static
    cmake ../ -DSTATIC_LIBRARY
    make 

Both libraries `libAcore.a` and `libBcore.a` and both executables `Abin` and
`Bbin` build correctly. Then suppose you made a change to the implementation of
`*2` in `*2.cpp`, simulated issuing

    touch ../A2.cpp ../B2.cpp

Option B correctly registers that only `B2` needs to be recompiled, but Option
`A` recompiles everythign.

    [ 10%] Building CXX object CMakeFiles/Bcore.dir/B2.cpp.o
    [ 60%] Building CXX object CMakeFiles/Acore.dir/A1.cpp.o
    [ 70%] Building CXX object CMakeFiles/Acore.dir/A2.cpp.o
    [ 75%] Building CXX object CMakeFiles/Abin.dir/A.cpp.o

## Conclusion

Option A is more conservative and thus safer: there's no chance of running a
rebuild _not_ registering a change. But it scales horribly. A single change to a
fundamental _implementation_ whose header is included by all n functions of a
library will trigger an O(n) build, rather than a O(1) build.

Is there no way to convice cmake to use the correct dependencies? 
