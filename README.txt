pstsdk is a library for reading PST-format email archives.  It currently
supports little-endian systems with modern C++ compilers.

Windows
-------

You can build pstsdk on Windows by downloading and uncompressing a copy of
boost 1.42.0 and running the following command:

    cmake -G "Visual Studio 10" -D BOOST_ROOT=../boost_1_42_0 .

This will generate a pstsdk.sln file which may be opened in Visual C++ 10
Express.

Mac
---

pstsdk has been successfully compiled using MacPorts.  To try it out, first
make sure you have Boost 1.42, GCC 4.4, CMake 2.8 and iconv:

    sudo port install boost @1.42.0
    sudo port install gcc44 cmake libiconv

Then, build it using CMake:

    CC=gcc-mp-4.4 CXX=g++-mp-4.4 cmake .
    make

Linux
-----

The following build commands work on Ubuntu 10.04, on both i386 and amd64
architectures:

    sudo apt-get install cmake g++-4.4
    CC=gcc-4.4 CXX=g++-4.4 cmake -D BOOST_ROOT=../boost_1_42_0 .
    make

Running the unit tests
----------------------

You can run the unit tests as follows:

    CTEST_OUTPUT_ON_FAILURE=1 make test

The unit tests should pass on all supported platforms.
