# Rutgers CS437 - Database System Implementation


## OVERVIEW

All the programming projects this semester will be written in C++. To ensure that you have the necessary C++ background, we are requiring everyone to complete a simple programming assignment to assess your knowledge of basic C++ features. You will not be given a grade for this project, but you are required to complete the project with a perfect result before being allowed to proceed in the course.

All of the code in this programming assignment must be written in C++ (specifically C++17). It is generally sufficient to know C++11. If you have not used C++ before, here is a [short tutorial](https://www.thegeekstuff.com/2016/02/c-plus-plus-11/) on the language. More detailed documentation of language internals is available on [cppreference](https://en.cppreference.com/w/). `A Tour of C++` and `Effective Modern C++` are also recommended books for leanring C++.

There are many tutorials available to teach you how to use `gdb` effectively. Here are some that we have found useful:

- [Debugging Under Unix: gdb Tutorial](https://15445.courses.cs.cmu.edu/fall2020/project0/#:~:text=Debugging%20Under%20Unix%3A%20gdb%20Tutorial)
- [GDB Tutorial: Advanced Debugging Tips For C/C++ Programmers](http://www.techbeamers.com/how-to-use-gdb-top-debugging-tips/)
- [Give me 15 minutes & I'll change your view of GDB](https://www.youtube.com/watch?v=PorfLSr3DDI) [VIDEO]

## SETTING UP YOUR DEVELOPMENT ENVIRONMENT

### Linux / Mac
To ensure that you have the proper packages on your machine, run the following script to automatically install them:

```
$ sudo build_support/packages.sh
```

Then run the following commands to build the system:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If you want to compile the system in debug mode, pass in the following flag to cmake:
Debug mode:

```
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```
This enables [AddressSanitizer](https://github.com/google/sanitizers), which can generate false positives for overflow on STL containers. If you encounter this, define the environment variable `ASAN_OPTIONS=detect_container_overflow=0`.

### Windows
If you are using Windows 10, you can use the Windows Subsystem for Linux (WSL) to develop, build, and test project. All you need is to [Install WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10). You can just choose "Ubuntu" (no specific version) in Microsoft Store. Then, enter WSL and follow the above instructions.

If you are using CLion, it also [works with WSL](https://blog.jetbrains.com/clion/2018/01/clion-and-linux-toolchain-on-windows-are-now-friends).

## DEVELOPMENT HINTS

Instead of using `printf` statements for debugging, use the `LOG_*` macros for logging information like this:

```
LOG_INFO("# Pages: %d", num_pages);
LOG_DEBUG("Fetching page %d", page_id);
```

To enable logging in your project, you will need to reconfigure it like this:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=DEBUG ..
$ make
```

The different logging levels are defined in `src/include/common/logger.h`. After enabling logging, the logging level defaults to `LOG_LEVEL_INFO`. Any logging method with a level that is equal to or higher than `LOG_LEVEL_INFO` (e.g., `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`) will emit logging information. Note that you will need to add `#include "common/logger.h"` to any file that you want to use the logging infrastructure.

We encourage you to use **gdb** to debug your project if you are having problems.

## GRADING RUBRIC

In order to pass this project, you must ensure your code follows the following guidelines:

1. Does the submission successfully execute all of the test cases and produce the correct answer?
1. Does the submission execute without any memory leaks?
1. Does the submission follow the code formatting and style policies?

Note that we will use additional test cases to grade your submission that are more complex than the sample test cases that we provide you.

## COLLABORATION POLICY

Every student has to work individually on this assignment.

Students are allowed to discuss high-level details about the project with others.

Students are not allowed to copy the contents of a white-board after a group meeting with other students.

Students are not allowed to copy the solutions from another colleague.