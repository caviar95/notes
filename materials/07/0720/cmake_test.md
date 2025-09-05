在 C++ 项目中使用 CMake 自动生成 **单元测试覆盖率报告** 是一种常见的做法，尤其是在持续集成（CI）场景中。下面我将从 **完整流程** 的角度，详细介绍如何用 CMake 集成测试覆盖率收集和报告功能。

---

## ✅ 1. 背景要求

* 操作系统：建议 Linux/macOS（因为主要依赖 `gcov`, `lcov`, `llvm-cov` 等工具）
* 编译器：`gcc` 或 `clang`
* CMake：建议版本 >= 3.10
* 单元测试框架：推荐 `GoogleTest`（但不限于）

---

## 🛠️ 2. 编译器和 CMake 编译选项配置

### 使用 GCC + gcov + lcov 方式：

```cmake
# CMakeLists.txt
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Building with code coverage enabled")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()
```

### ⚠️ 注意事项：

* 优化（`-O2`, `-O3`）会导致覆盖率不准确，所以务必使用 `-O0`
* 需要加 `-g` 保留调试信息

---

## ✅ 3. 单元测试编写与注册

使用 GoogleTest 示例：

```cpp
#include <gtest/gtest.h>

int add(int a, int b) {
    return a + b;
}

TEST(AdditionTest, Basic) {
    EXPECT_EQ(add(2, 3), 5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

## 🧪 4. CMake 添加测试与目标

```cmake
enable_testing()

add_executable(test_mycode test.cpp)
target_link_libraries(test_mycode gtest gtest_main pthread)

add_test(NAME my_test COMMAND test_mycode)
```

---

## 📊 5. 添加生成覆盖率报告的目标

你可以通过 `lcov + genhtml` 工具生成 HTML 报告。

```cmake
if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    add_custom_target(coverage
        COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
        COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info.cleaned
        COMMAND ${GENHTML_EXECUTABLE} coverage.info.cleaned --output-directory coverage_html
        COMMENT "Generating code coverage report"
        VERBATIM
    )
endif()
```

执行：

```bash
cmake -DENABLE_COVERAGE=ON -B build
cmake --build build
cd build
ctest    # 或者直接运行测试可执行文件
make coverage
```

输出结果位于 `build/coverage_html/index.html`，可以用浏览器打开查看覆盖率图。

---

## 🧬 6. clang + llvm-cov（可选）

如果你使用 `clang` + `llvm-cov`，推荐使用 `llvm-cov show` 和 `llvm-profdata`。

```cmake
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()
```

执行测试时：

```bash
LLVM_PROFILE_FILE="test.profraw" ./test_mycode
llvm-profdata merge -sparse test.profraw -o test.profdata
llvm-cov show ./test_mycode -instr-profile=test.profdata > coverage.txt
```

也可以生成 HTML：

```bash
llvm-cov show ./test_mycode -instr-profile=test.profdata -format=html -output-dir=coverage_html
```

---

## 📦 7. 其他工具推荐

| 工具            | 描述                               |
| ------------- | -------------------------------- |
| **gcovr**     | Python 编写的轻量级覆盖率生成工具，支持 XML/HTML |
| **Codecov**   | 与 CI/CD 平台集成上传覆盖率报告              |
| **Coveralls** | 与 GitHub Actions 配合上传覆盖率数据       |

### 使用 gcovr 的 CMake 目标：

```cmake
find_program(GCOVR gcovr)

add_custom_target(coverage_report
    COMMAND ${GCOVR} -r .. --html --html-details -o coverage.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

---

## 🧩 小贴士

* 推荐使用 `CTest` 管理测试，然后统一生成覆盖率
* CI 环境下建议通过 `lcov --rc lcov_branch_coverage=1` 开启分支覆盖率
* 可以通过 `genhtml --legend` 生成详细图示

---

## ✅ 示例项目结构

```
project/
├── CMakeLists.txt
├── src/
│   └── add.cpp
├── include/
│   └── add.h
├── test/
│   └── test_add.cpp
└── build/
```

---

## 总结

| 步骤                      | 说明                                         |
| ----------------------- | ------------------------------------------ |
| ✅ 添加编译器覆盖率参数            | `--coverage` or `-fprofile-instr-generate` |
| ✅ 写好测试代码                | 推荐 GoogleTest                              |
| ✅ 添加 `enable_testing()` | 使用 `add_test()` 注册                         |
| ✅ 添加 `coverage` 目标      | 调用 `lcov` 或 `gcovr` 自动生成报告                 |
| ✅ 使用 CI 平台上传            | 可集成 Codecov/Coveralls 等服务                  |

---

如你需要一个完整的 CMake 示例工程用于复制粘贴，我可以为你创建一个完整目录和脚本。是否需要？

你说得没错，**传统的覆盖率流程**确实往往是：

1. 用 CMake 编译并安装测试程序；
2. 写 shell 脚本（或者手动）运行测试；
3. 执行 `lcov` / `gcovr` / `llvm-cov` 等工具收集数据；
4. 再用 `genhtml` 生成 HTML；
5. 手动打开浏览器查看。

这个流程确实有些“重”。

---

## ✅ 更加简单直接的方式：**在 CMake 中一键执行并生成 HTML**

是的，我们可以让 **CMake 构建测试程序后，自动执行测试并生成 HTML 报告**，你只需要：

```bash
cmake -DENABLE_COVERAGE=ON -B build && cmake --build build && cmake --build build --target coverage_html && xdg-open build/coverage_html/index.html
```

### ✅ 推荐做法：CMake + `add_custom_target`

下面给出一个完整的可复用 **最简 CMake 方案**，支持以下功能：

* 一次构建测试目标
* 自动运行测试
* 自动收集覆盖率（lcov）
* 自动生成 HTML 报告
* 一条命令 `make coverage_html` 全部完成

---

## 🧪 示例项目结构

```
your_project/
├── CMakeLists.txt
├── add.cpp
├── add.h
├── test_add.cpp
```

---

## 📄 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

option(ENABLE_COVERAGE "Enable coverage" ON)

# 1. 设置编译选项
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Coverage enabled")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# 2. 源码和测试
add_library(mycode add.cpp)
target_include_directories(mycode PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(test_add test_add.cpp)
target_link_libraries(test_add mycode gcov)

enable_testing()
add_test(NAME test_add COMMAND test_add)

# 3. 生成 coverage_html 目标
find_program(LCOV_EXEC lcov)
find_program(GENHTML_EXEC genhtml)

add_custom_target(coverage_html
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND ${LCOV_EXEC} --directory . --capture --output-file coverage.info
    COMMAND ${LCOV_EXEC} --remove coverage.info '/usr/*' '*test*' --output-file coverage_filtered.info
    COMMAND ${GENHTML_EXEC} coverage_filtered.info --output-directory coverage_html
    COMMENT "Generating coverage HTML report..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

---

## ✅ 源码示例

### add.h

```cpp
#pragma once
int add(int a, int b);
```

### add.cpp

```cpp
#include "add.h"
int add(int a, int b) {
    return a + b;
}
```

### test\_add.cpp

```cpp
#include <cassert>
#include "add.h"

int main() {
    assert(add(2, 3) == 5);
    return 0;
}
```

---

## ✅ 执行一条命令搞定

```bash
cmake -DENABLE_COVERAGE=ON -B build
cmake --build build --target coverage_html
xdg-open build/coverage_html/index.html  # Linux
open build/coverage_html/index.html      # macOS
```

---

## 🧩 补充优化建议

| 场景       | 优化方法                                         |
| -------- | -------------------------------------------- |
| 更复杂的测试框架 | 使用 GoogleTest / Catch2，也一样能收集                |
| 多个测试文件   | 加入多个 `add_test()`，仍然自动收集                     |
| CI 平台使用  | 只需上传 `coverage.info` 或 HTML                  |
| 忽略某些路径   | `--remove` 加入 `'*_mock.cpp'`、`third_party/*` |

---

## ✅ 结论

你想要的「**CMake 构建后自动运行测试并生成覆盖率 HTML**」完全可以做到，**不需要 shell 脚本**，只需要：

1. 使用 `add_custom_target` 注册 `coverage_html`
2. 在构建目录执行：`make coverage_html`
3. 自动完成构建、测试、覆盖率收集、HTML 生成

如果你需要我生成完整 demo 项目或写成模板工程，随时告诉我，我可以给你打包输出。


明白了。你希望的效果是：

> 单元测试工程嵌入主工程中，在正常 `cmake` + `make` 后就能自动编译测试 + 执行测试 + 生成 HTML 覆盖率报告，无需手动加 `-DENABLE_COVERAGE=ON` 等选项。

我们可以做到以下目标：

---

## ✅ 你的需求总结为：

1. **不手动加参数**，即默认启用覆盖率；
2. **内嵌到主工程**，即一旦构建主项目，就能自动编译测试；
3. **编译后自动执行测试**，并立即生成 HTML 覆盖率；
4. **一条命令：`make` 即生成结果并可打开 HTML 文件查看**。

---

## ✅ 实现方式：全自动嵌入式覆盖率生成方案

### 🧩 第一步：测试覆盖率在 Debug 模式自动开启

```cmake
# 主工程的 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MainProject)

# 默认在 Debug 模式开启覆盖率（只对 gcc/clang）
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling coverage instrumentation (Debug mode + GCC/Clang)")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
    set(ENABLE_COVERAGE TRUE)
else()
    set(ENABLE_COVERAGE FALSE)
endif()
```

---

### 🧩 第二步：添加测试并内嵌覆盖率生成目标

```cmake
enable_testing()

add_library(mycode src/add.cpp)
target_include_directories(mycode PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(test_add test/test_add.cpp)
target_link_libraries(test_add mycode gcov) # gcov for coverage
add_test(NAME test_add COMMAND test_add)

# 自动运行测试 + 生成覆盖率 HTML（嵌入默认构建流程）
if(ENABLE_COVERAGE)
    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)

    add_custom_command(TARGET test_add POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${LCOV_EXEC} --directory . --capture --output-file coverage.info
        COMMAND ${LCOV_EXEC} --remove coverage.info '/usr/*' '*test*' --output-file coverage_filtered.info
        COMMAND ${GENHTML_EXEC} coverage_filtered.info --output-directory coverage_html
        COMMENT "Running test and generating coverage HTML"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
```

---

## ✅ 效果

你现在只需运行以下命令，不需要加任何额外参数：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

然后：

* `test_add` 会自动执行
* `coverage_html/index.html` 会自动生成

你可以直接浏览器打开或自动打开：

```cmake
# 可选：在 HTML 生成后自动用浏览器打开
add_custom_command(TARGET test_add POST_BUILD
    ...
    COMMAND xdg-open coverage_html/index.html || true
)
```

---

## 🔁 更进一步（CI友好）

你也可以让主工程构建后，在构建树根目录生成一个符号链接：

```bash
ln -s build/coverage_html/index.html coverage_report.html
```

方便在 CI 工具中找到并展示。

---

## ✅ 总结

| 项目需求      | 实现方式                             |
| --------- | -------------------------------- |
| 不添加命令行参数  | 默认开启覆盖率（在 Debug 模式 + GCC/Clang）  |
| 测试自动运行    | 在 `test_add` 编译完成后自动执行           |
| 覆盖率自动收集   | `add_custom_command` POST\_BUILD |
| HTML 自动生成 | 使用 `lcov + genhtml`              |
| 可选自动打开    | 加 `xdg-open` 或 `open`            |

---

如你需要，我可以提供一个 zip 包或完整 CMake 模板目录，供你快速嵌入项目。是否需要？


✅ 已为你创建完整的 CMake 工程，支持构建后自动执行单元测试并生成 HTML 覆盖率报告。

你可以在下面链接下载项目：

👉 [点击下载 cpp\_coverage\_demo.zip](sandbox:/mnt/data/cpp_coverage_demo.zip)

---

### 📦 包含内容：

```
cpp_coverage_demo/
├── CMakeLists.txt
├── include/
│   └── add.h
├── src/
│   └── add.cpp
└── test/
    └── test_add.cpp
```

---

### 🧪 使用方式：

```bash
# 解压
unzip cpp_coverage_demo.zip
cd cpp_coverage_demo

# 构建并自动运行测试、生成覆盖率
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

然后 HTML 报告将生成在：

```
build/coverage_html/index.html
```

可以用浏览器打开它查看测试覆盖情况。

---

如你希望支持 GoogleTest、Catch2、CI 上上传到 Codecov 等，也可以继续帮你扩展这个项目。


非常好的问题。

如果你通过 `CMakeLists.txt` 使用了 `install(TARGETS ...)` 将测试可执行文件安装到特定目录（如 `/usr/local/bin`、`${CMAKE_INSTALL_PREFIX}/bin` 等），那么为了在**安装路径下运行并收集覆盖率数据**，你需要做几件关键的事：

---

## ✅ 场景总结：

> **执行文件被 `install()` 到某目录，如何在这个目录执行时生成覆盖率文件，并支持 HTML 报告？**

---

## 🧩 步骤详解

### ✅ 1. 正确安装测试目标

```cmake
# 安装测试可执行文件到 bin 目录
install(TARGETS test_add RUNTIME DESTINATION bin)
```

如果你用 `CMAKE_INSTALL_PREFIX=/home/you/test_install`，它会被安装到 `/home/you/test_install/bin/test_add`

---

### ✅ 2. 设置 `--coverage` 编译链接选项（你应该已设置过）

```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()
```

这会让 `gcda`、`gcno` 文件在**程序执行时**自动生成。

---

### ✅ 3. 运行测试：确保执行路径正确 & 有写权限

例如：

```bash
cd /home/you/test_install/bin
./test_add
```

这时会在当前目录或源文件同目录下生成：

* `src/CMakeFiles/mycode.dir/add.cpp.gcda`
* `test/test_add.gcda`
* `*.gcno` 之前就已经在构建时生成

---

### ✅ 4. 使用 `lcov` 从安装目录收集覆盖率信息

例如你构建目录为 `build/`，你可以在构建目录中执行以下命令（设置 `--directory` 指向 `.gcda/.gcno` 所在目录）：

```bash
# 从构建/源目录收集覆盖率信息
lcov --directory /home/you/test_install/bin --capture --output-file coverage.info
```

但更稳妥的方式是将 `--directory` 指向**构建目录**（`.o`、`.gcno` 所在位置），并确保 `.gcda` 被写入那里（靠 `build` 目录运行测试时实现）。

---

### ✅ 5. 使用 `genhtml` 生成 HTML 报告

```bash
genhtml coverage.info --output-directory coverage_html
xdg-open coverage_html/index.html
```

---

## ✅ 建议：不要在 install 后运行测试收集覆盖率（推荐方式如下）

> ✅ **最推荐的方式仍然是：**
>
> * 在构建目录内运行测试（不是安装后再运行）
> * 因为 `.gcno` 文件只在构建目录内存在，安装目录不会自动包含它

---

## 🧩 如果你必须在安装目录收集覆盖率……

### 可以这样做：

#### ✅ 修改 `install()` 时也复制 `.gcno` 文件（非标准操作）

在 CMake 中强行安装 `.gcno` 文件：

```cmake
install(DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles/
        DESTINATION share/gcno
        FILES_MATCHING PATTERN "*.gcno")
```

然后运行时指定 `--directory` 到 `${prefix}/share/gcno`，手动拼回源文件目录结构 —— 比较繁琐，**不推荐作为主流程**。

---

## ✅ 最佳方案总结

| 方案                              | 是否推荐   | 原因                       |
| ------------------------------- | ------ | ------------------------ |
| 在构建目录中运行测试、收集覆盖率                | ✅ 推荐   | `.gcno` 文件完整在构建目录中       |
| 安装后在安装目录运行测试并收集覆盖率              | ❌ 不推荐  | 安装不会带 `.gcno`，会丢失文件或路径错误 |
| 安装后收集 coverage，依赖 `.gcno` 被一同安装 | ⚠️ 有风险 | 需要额外 install 指令并保持路径一致性  |

---

## ✅ 最简自动方案（推荐）：

你可以保留 install，但主构建目录中添加如下自动测试 + 覆盖率命令：

```cmake
add_custom_target(run_coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND lcov --directory . --capture --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.cleaned.info
    COMMAND genhtml coverage.cleaned.info --output-directory coverage_html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

执行：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build --target run_coverage
xdg-open build/coverage_html/index.html
```

---

如你确实希望在 install 后部署测试系统并自动上传覆盖率，我们可以再进一步封装脚本逻辑。是否需要我帮你生成一个 install 后支持收集 coverage 的完整 demo？
