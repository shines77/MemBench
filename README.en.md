# MemBench

## Description

A memory benchmark routine, written by C++.

## Software Architecture

Software architecture description

## Installation

Update submodule `Google/benchmark` :

```shell
git submodule update --init --recursive
```

Compile and install `Google/benchmark` :

```shell
cd ./third/benchmark

# Make a build directory to place the build output.
sudo mkdir build

# Switch to build folder, generate build system files with cmake,
# and download any dependencies.
sudo cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ../

# Build "benchmark" library, and install the library globally.
sudo cmake --build "build" --config Release --target install
```

### 2.2 编译 MemBench

```shell
# Gnerate Makefile files with cmake
cmake -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release .

# Starting to compile
make
```

In fact, "`-DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release`" it's not necessary，you can use "`cmake .`" only. This is just to show what parameters may be required.

## 3. Instructions

1. xxxx
2. xxxx
3. xxxx

## 4. Contribution

1. Fork the repository
2. Create Feat_xxx branch
3. Commit your code
4. Create Pull Request

## 5. Gitee Feature

1. You can use Readme\_XXX.md to support different languages, such as Readme\_en.md, Readme\_zh.md
2. Gitee blog [blog.gitee.com](https://blog.gitee.com)
3. Explore open source project [https://gitee.com/explore](https://gitee.com/explore)
4. The most valuable open source project [GVP](https://gitee.com/gvp)
5. The manual of Gitee [https://gitee.com/help](https://gitee.com/help)
6. The most popular members  [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
