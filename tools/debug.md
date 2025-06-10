在 Linux 环境下调试 C/C++ 代码是开发中非常常见且重要的环节，以下是一个完整的调试工具链和方法体系，适合从新手到进阶使用。

---

## 🛠 常见调试工具总览

| 工具                    | 作用           | 特点                   |
| --------------------- | ------------ | -------------------- |
| `gdb`                 | 断点调试器        | 功能强大，CLI 操作，适合深度调试   |
| `lldb`                | LLVM 的调试器    | 类似 gdb，Clang 用户推荐    |
| `valgrind`            | 内存错误检测工具     | 检测内存泄漏、越界、未初始化使用     |
| `strace`              | 系统调用跟踪工具     | 查看程序调用了哪些系统调用        |
| `ltrace`              | 库函数调用跟踪工具    | 跟踪动态库函数              |
| `perf` / `gprof`      | 性能分析工具       | 性能瓶颈、函数耗时            |
| `addr2line`           | 地址转源码        | 结合 core dump 使用      |
| `ASan`/`UBSan`/`TSan` | Sanitizer 系列 | 编译期开启，运行时报错，快速发现 bug |
| `core dump` + `gdb`   | 崩溃时保存调试信息    | 调试 release 程序常用      |
| `gdb-dashboard`       | gdb 增强 UI    | 视觉更友好，支持 TUI         |

---

## 🧪 最常用调试方式详解

### 1. 使用 `gdb` 进行断点调试

#### 编译时开启调试信息：

```bash
g++ -g main.cpp -o main
```

#### 启动调试器：

```bash
gdb ./main
```

#### 常用命令：

```gdb
break main       # 设置断点
run              # 启动程序
next             # 单步跳过
step             # 单步进入函数
print var        # 打印变量
backtrace        # 查看函数调用栈
info locals      # 查看当前函数的局部变量
continue         # 继续运行
quit             # 退出
```

---

### 2. 使用 `valgrind` 检查内存问题

```bash
valgrind --leak-check=full ./main
```

* 检查内存泄漏
* 检查访问未初始化内存
* 检查数组越界（配合 `-g`）

---

### 3. 使用 Sanitizer（推荐）

#### 开启 AddressSanitizer：

```bash
g++ -fsanitize=address -g main.cpp -o main
./main
```

* 运行时自动提示：野指针、内存越界、double free 等

#### 其他 Sanitizer：

| Sanitizer | 编译参数                   | 作用    |
| --------- | ---------------------- | ----- |
| ASan      | `-fsanitize=address`   | 内存错误  |
| TSan      | `-fsanitize=thread`    | 数据竞争  |
| UBSan     | `-fsanitize=undefined` | 未定义行为 |

---

### 4. 打开 core dump 文件调试

#### 启用 core 文件生成：

```bash
ulimit -c unlimited
```

#### 程序崩溃后会生成 `core` 文件，使用如下命令调试：

```bash
gdb ./main core
```

然后查看崩溃位置：

```gdb
bt           # backtrace
frame 0      # 查看第0帧
list         # 查看源码
```

---

### 5. 使用 `strace` 跟踪系统调用

```bash
strace ./main
```

* 跟踪 `open`, `read`, `write`, `mmap`, `brk`, `futex` 等调用，定位 crash 或卡顿

---

## ✅ 实战建议与技巧

1. **debug 构建单独一套 build 目录：**

   ```bash
   mkdir build-debug && cd build-debug
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make
   ```

2. **函数输出日志配合调试器双保险。**

3. **编译开启 `-Wall -Wextra -Werror`。**

4. **善用 `assert()` 和日志模块进行调试定位。**

5. **多线程程序建议开启 `-fsanitize=thread` 检测数据竞争。**

---

## 🔧 GUI 工具（可选）

| 工具             | 简介                  |
| -------------- | ------------------- |
| `vscode + gdb` | 配置 launch.json 支持断点 |
| `CLion`        | 自带调试器，体验最佳          |
| `gdbgui`       | Web UI for gdb      |
| `ddd` / `cgdb` | TUI 界面              |

---

## 📦 示例代码及调试演示（可选提供）

如果你有一段崩溃/行为异常的 C/C++ 代码，我可以现场给出：

* 编译选项
* gdb/valgrind/asan 调试示例
* 如何查看原因
