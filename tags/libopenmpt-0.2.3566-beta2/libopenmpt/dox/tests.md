
Tests {#tests}
=====


libopenmpt provides some basic unit tests that check the platform for general
sanity and do some basic internal functionality testing. The test suite
requires a special libopenmpt build that includes file saving functionality
which is not included in normal builds.

### Running Tests

#### On Unix-like systems

Compile with

    make TEST=1 $YOURMAKEOPTIONS clean
    make TEST=1 $YOURMAKEOPTIONS
    make TEST=1 $YOURMAKEOPTIONS check

In order to build your normal binaries again, run

    make $YOURMAKEOPTIONS clean
    make $YOURMAKEOPTIONS

As the build system retains no state between make invocations, you have to
provide your make options on every make invocation.

#### On Windows

Using Visual Studio 2010, compile

    libopenmpt\libopenmpt_test.sln

and run

    bin\$ARCH\libopenmpt_test.exe

from the root of the source tree.
