CMake 构建系统重点内容

├── 4. CMake 构建系统（重点）
│   ├── 基本语法与结构
│   │   ├── cmake_minimum_required / project()
│   │   ├── add_executable / add_library
│   │   ├── target_include_directories / target_link_libraries
│   │   └── target_compile_options / target_compile_definitions
│   ├── 构建类型：Debug / Release / RelWithDebInfo
│   ├── 编译器检测与选项控制
│   ├── 子模块与子项目：add_subdirectory / FetchContent
│   ├── 外部项目支持：ExternalProject_Add
│   └── 安装与导出：install(), export(), config file 生成

---

## 1 基本语法与结构

### `cmake_minimum_required()`

指定使用的最低 CMake 版本，防止不同版本行为不一致：

```cmake
cmake_minimum_required(VERSION 3.16)
```

### `project()`

定义项目名称、语言和版本：

```cmake
project(MyApp VERSION 1.0 LANGUAGES C CXX)
```

---

### `add_executable()`

添加一个可执行文件目标：

```cmake
add_executable(myapp main.cpp)
```

### `add_library()`

添加一个静态或动态库：

```cmake
add_library(mylib STATIC lib.cpp)
# 或
add_library(mylib SHARED lib.cpp)
```

---

### `target_include_directories()`

指定目标使用的头文件搜索路径：

```cmake
target_include_directories(myapp
  PRIVATE ${CMAKE_SOURCE_DIR}/include
)
```

* `PRIVATE`: 仅本目标使用
* `PUBLIC`: 本目标及依赖它的目标都使用
* `INTERFACE`: 只导出给依赖者，不用于本身

---

### `target_link_libraries()`

链接库或其他目标：

```cmake
target_link_libraries(myapp
  PRIVATE mylib
  PRIVATE pthread
)
```

---

### `target_compile_options()`

添加编译选项：

```cmake
target_compile_options(myapp PRIVATE -Wall -O2)
```

---

### `target_compile_definitions()`

添加宏定义（等价于 `-D`）：

```cmake
target_compile_definitions(myapp PRIVATE VERSION="1.0")
```

---

## 2 构建类型（Build Type）

通过 `CMAKE_BUILD_TYPE` 控制不同构建模式：

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

| 构建类型             | 描述                   |
| ---------------- | -------------------- |
| `Debug`          | 含调试信息，关闭优化，默认编译 `-g` |
| `Release`        | 启用优化 `-O3`，无调试信息     |
| `RelWithDebInfo` | 含调试信息 + 优化（适合性能调试）   |
| `MinSizeRel`     | 最小二进制文件（嵌入式）         |

---

## 3 编译器检测与选项控制

CMake 可检测编译器与平台差异，并根据平台设置不同选项：

```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(myapp PRIVATE -Wall -Wextra)
endif()

if (MSVC)
  target_compile_options(myapp PRIVATE /W4)
endif()
```

---

## 4 子模块与子项目支持

### `add_subdirectory()`

添加子模块项目：

```cmake
add_subdirectory(thirdparty/mylib)
target_link_libraries(myapp PRIVATE mylib)
```

* 要求子目录下有 `CMakeLists.txt`

---

### `FetchContent`（现代推荐）

自动下载第三方依赖并引入构建流程：

```cmake
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)
target_link_libraries(myapp PRIVATE nlohmann_json::nlohmann_json)
```

适合构建阶段临时获取依赖而不污染系统环境。

---

## 5 外部项目支持：`ExternalProject_Add`

用于构建不能直接作为子目录添加的外部项目（适用于非 CMake 项目）：

```cmake
include(ExternalProject)
ExternalProject_Add(extlib
  URL https://example.com/extlib.tar.gz
  CONFIGURE_COMMAND ./configure
  BUILD_COMMAND make
  INSTALL_COMMAND make install
)
```

可用于引入 Autotools / Makefile 项目。

---

## 6 安装与导出

### `install()`

设置安装路径规则：

```cmake
install(TARGETS myapp DESTINATION bin)
install(FILES include/mylib.h DESTINATION include)
```

安装到 `CMAKE_INSTALL_PREFIX`，默认是 `/usr/local`

执行安装命令：

```bash
cmake --install build
```

---

### `export()` 与配置导出

为了支持 **目标被其他 CMake 项目 find\_package 使用**，需要导出配置：

```cmake
install(TARGETS mylib EXPORT mylibTargets DESTINATION lib)
install(EXPORT mylibTargets
  FILE mylibTargets.cmake
  NAMESPACE mylib::
  DESTINATION lib/cmake/mylib
)
```

配合生成配置文件 `mylibConfig.cmake`：

```cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mylibConfigVersion.cmake"
  VERSION 1.0
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/mylibConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/mylibConfigVersion.cmake"
  DESTINATION lib/cmake/mylib
)
```

这样就能在其他项目中使用：

```cmake
find_package(mylib REQUIRED)
target_link_libraries(app PRIVATE mylib::mylib)
```

---

## 总结

```text
CMake 总体结构
├── 项目定义与语法
│   ├── cmake_minimum_required()
│   ├── project()
│   ├── add_executable / add_library
│   ├── target_* APIs
├── 构建类型与选项
│   ├── CMAKE_BUILD_TYPE
│   ├── target_compile_options / definitions
├── 多模块与依赖
│   ├── add_subdirectory
│   ├── FetchContent_Declare
│   └── ExternalProject_Add
├── 安装与包导出
│   ├── install()
│   ├── export()
│   └── CMakePackageConfigHelpers
```

---

如果你有一个具体项目结构，我可以帮你写出完整的 `CMakeLists.txt`，包括：

* 编译选项、头文件路径
* 多模块依赖组织
* 安装与版本导出支持

