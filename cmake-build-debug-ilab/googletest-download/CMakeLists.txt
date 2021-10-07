cmake_minimum_required(VERSION 3.8)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG master
        SOURCE_DIR "/common/home/raw175/common/home/raw175/2021-10_DBSImpl_Proj1/cmake-build-debug-ilab/googletest-src"
        BINARY_DIR "/common/home/raw175/common/home/raw175/2021-10_DBSImpl_Proj1/cmake-build-debug-ilab/googletest-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        )
