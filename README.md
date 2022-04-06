# MemBench

## 1. 介绍

[`English`]: A memory benchmark routine, written by C++.

[`Chinese`]: 一个用 C++ 写的内存性能测试程序。

## 2. 安装方法

### 2.1 安装 Google/benchmark

更新子模块 `Google/benchmark` ：

```shell
git submodule update --init --recursive
```

编译和安装 `Google/benchmark` ：

```shell
cd ./third/benchmark

# 创建 build 目录
sudo mkdir build

# 切换到 build 目录, 用 cmake 生成 Makefile 配置文件, 并且下载依赖文件
sudo cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ../

# 编译和生成 benchmark 库, 并安装到系统文件夹里
sudo cmake --build "build" --config Release --target install
```

### 2.2 编译 MemBench

```shell
# 用 cmake 生成 Makefile 配置文件
cmake -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release .

# 开始编译
make
```

实际上，"`-DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release`" 也不是必须的，只使用 "`cmake .`" 即可，这里只是为了展示可能需要哪些参数。

## 3. 使用说明

1. xxxx
2. xxxx
3. xxxx

## 4. 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request

## 5. 其他特性

1. 使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2. Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3. 你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4. [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5. Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6. Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
