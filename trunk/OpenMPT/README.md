
README
======

OpenMPT and libopenmpt
======================

This repository contains OpenMPT, a free Windows/Wine-based
[tracker](https://en.wikipedia.org/wiki/Music_tracker) and libopenmpt,
a library to render tracker music (MOD, XM, S3M, IT MPTM and dozens of other
legacy formats) to a PCM audio stream. libopenmpt is directly based on OpenMPT,
offering the same playback quality and format support, and development of the
two happens in parallel.


How to compile
--------------


### OpenMPT

 -  Supported Visual Studio versions:

     -  Visual Studio 2015 Update 3 Community/Professional/Enterprise

        To compile the project, open `build/vs2015/OpenMPT.sln` and hit the
        compile button.

     -  Visual Studio 2017 Community/Professional/Enterprise

        To compile the project, open `build/vs2017/OpenMPT.sln` and hit the
        compile button.

 -  The Windows 8.1 SDK and Microsoft Foundation Classes (MFC) are required to
    build OpenMPT (both are included with Visual Studio, however may need to be
    selected explicitly during setup). In order to build OpenMPT for Windows XP,
    the XP targetting toolset also needs to be installed.

 -  The VST and ASIO SDKs are needed for compiling with VST and ASIO support.

    If you don't want this, uncomment `#define NO_VST` and comment out
    `#define MPT_WITH_ASIO` in the file `common/BuildSettings.h`.

    The ASIO and VST SDKs can be downloaded automatically on Windows 7 or later
    with 7-Zip installed by just running the `build/download_externals.cmd`
    script.

    If you do not want to or cannot use this script, you may follow these manual
    steps instead:

     -  ASIO:

        If you use `#define MPT_WITH_ASIO`, you will need to put the ASIO SDK in
        the `include/ASIOSDK2` folder. The top level directory of the SDK is
        already named `ASIOSDK2`, so simply move that directory in the include
        folder.

        Please visit
        [steinberg.net](https://www.steinberg.net/en/company/developers.html) to
        download the SDK.

     -  VST:

        If you don't use `#define NO_VST`, you will need to put the VST SDK in
        the `include/vstsdk2.4` folder.
        
        Simply copy all files from the `VST3 SDK` folder in the SDK .zip file to
        `include/vstsdk2.4/`.

        Note: OpenMPT makes use of the VST 2.4 specification only. The VST3 SDK
        still contains all necessary files in the right locations. If you still
        have the old VST 2.4 SDK laying around, this should also work fine.

        Please visit
        [steinberg.net](https://www.steinberg.net/en/company/developers.html) to
        download the SDK.

    If you need further help with the VST and ASIO SDKs, get in touch with the
    main OpenMPT developers. 

 -  7-Zip is required to be installed in the default path in order to build the
    required files for OpenMPT Wine integration.

    Please visit [7-zip.org](https://www.7-zip.org/) to download 7-Zip.


### libopenmpt and openmpt123

For detailed requirements, see `libopenmpt/dox/quickstart.md`.

 -  Autotools

    Grab a `libopenmpt-VERSION-autotools.tar.gz` tarball.

        ./configure
        make
        make check
        sudo make install

    Cross-compilation is generally supported (although only tested for
    targetting MinGW-w64).

    Note that some MinGW-w64 distributions come with the `win32` threading model
    enabled by default instead of the `posix` threading model. The `win32`
    threading model lacks proper support for C++11 `<thread>` and `<mutex>` as
    well as thread-safe magic statics. It is recommended to use the `posix`
    threading model for libopenmpt for this reason. On Debian, the appropriate
    configure command is
    `./configure --host=x86_64-w64-mingw32 CC=x86_64-w64-mingw32-gcc-posix CXX=x86_64-w64-mingw32-g++-posix`
    for 64bit, or
    `./configure --host=i686-w64-mingw32 CC=i686-w64-mingw32-gcc-posix CXX=i686-w64-mingw32-g++-posix`
    for 32bit. Other MinGW-w64 distributions may differ.

 -  Visual Studio:

     -  You will find solutions for Visual Studio 2015 to 2017 in the
        corresponding `build/vsVERSION/` folder.
        Projects that target Windows versions before Windows 7 are available in
        `build/vsVERSIONxp/`
        Most projects are supported with any of the mentioned Visual Studio
        verions, with the following exceptions:

         -  in_openmpt: Requires Visual Studio with MFC.

         -  xmp-openmpt: Requires Visual Studio with MFC.

     -  You will need the Winamp 5 SDK and the XMPlay SDK if you want to
        compile the plugins for these 2 players. They can be downloaded
        automatically on Windows 7 or later with 7-Zip installed by just running
        the `build/download_externals.cmd` script.

        If you do not want to or cannot use this script, you may follow these
        manual steps instead:

         -  Winamp 5 SDK:

            To build libopenmpt as a winamp input plugin, copy the contents of
            `WA5.55_SDK.exe` to include/winamp/.

            Please visit
            [winamp.com](http://wiki.winamp.com/wiki/Plug-in_Developer) to
            download the SDK.
            You can disable in_openmpt in the solution configuration.

         -  XMPlay SDK:

            To build libopenmpt with XMPlay input plugin support, copy the
            contents of xmp-sdk.zip into include/xmplay/.

            Please visit [un4seen.com](https://www.un4seen.com/xmplay.html) to
            download the SDK.
            You can disable xmp-openmpt in the solution configuration.

 -  Makefile

    The makefile supports different build environments and targets via the
    `CONFIG=` parameter directly to the make invocation.
    Use `make CONFIG=$newconfig clean` when switching between different configs
    because the makefile cleans only intermediates and target that are active
    for the current config and no configuration state is kept around across
    invocations.

     -  mingw-w64:

        The required version is at least 4.8.

            make CONFIG=mingw64-win32    # for win32

            make CONFIG=mingw64-win64    # for win64

     -  gcc or clang (on Unix-like systems, including Mac OS X with MacPorts,
        and Haiku (32-bit Hybrid and 64-bit)):

        The minimum required compiler versions are:

         -  gcc 4.8

         -  clang 3.6

        The Makefile requires pkg-config for native builds.
        For sound output in openmpt123, PortAudio or SDL is required.
        openmpt123 can optionally use libflac and libsndfile to render PCM
        files to disk.

        When using gcc, run:

            make CONFIG=gcc

        When using clang, it is recommended to do:

            make CONFIG=clang

        Otherwise, simply run

            make

        which will try to guess the compiler based on your operating system.

     -  emscripten (on Unix-like systems):

        libopenmpt has been tested and verified to work with emscripten 1.31 or
        later (earlier versions might or might not work).

        Run:

            make CONFIG=emscripten

        Running the test suite on the command line is also supported by using
        node.js. Version 0.10.25 or greater has been tested. Earlier versions
        might or might not work. Depending on how your distribution calls the
        `node.js` binary, you might have to edit
        `build/make/config-emscripten.mk`.

     -  American Fuzzy Lop:

        To compile libopenmpt with fuzzing instrumentation for afl-fuzz, run:
        
            make CONFIG=afl
        
        For more detailed instructions, read contrib/fuzzing/readme.md

     -  other compilers:

        To compiler libopenmpt with other C++11 compliant compilers, run:
        
            make CONFIG=generic
        
    
    The `Makefile` supports some customizations. You might want to read the top
    which should get you some possible make settings, like e.g.
    `make DYNLINK=0` or similar. Cross compiling or different compiler would
    best be implemented via new `config-*.mk` files.

    The `Makefile` also supports building doxygen documentation by using

        make doc

    Binaries and documentation can be installed systen-wide with

        make PREFIX=/yourprefix install
        make PREFIX=/yourprefix install-doc

    Some systems (i.e. Linux) require running

        sudo ldconfig

    in order for the system linker to be able to pick up newly installed
    libraries.

    `PREFIX` defaults to `/usr/local`. A `DESTDIR=` parameter is also
    supported.

 -  Android NDK

    See `build/android_ndk/README.AndroidNDK.txt`.



Contributing to OpenMPT/libopenmpt
----------------------------------


See [contributing](doc/contributing.md).

