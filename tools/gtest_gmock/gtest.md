`gtest`（Google Test）是 Google 开源的 C++ 单元测试框架，具有简洁的语法和强大的断言功能，常与 `gmock` 搭配使用进行完整的单元测试。

---

## 1 gtest 简介

### 主要功能

* 编写结构化的测试用例和测试集；
* 提供丰富的断言（ASSERT/EXPECT 系列）；
* 支持测试夹具（Test Fixtures）；
* 支持参数化测试、类型参数测试；
* 易于集成到 CMake 项目或 CI/CD 系统。

---

## 2 gtest 基本结构

```cpp
#include <gtest/gtest.h>

TEST(TestCaseName, TestName) {
    // 测试内容
    EXPECT_EQ(1 + 1, 2);
}
```

### 示例说明

* `TEST(TestCaseName, TestName)`：定义一个测试函数。
* 每个 `TEST()` 宏定义的函数会被自动注册并运行。

---

## 3 常用断言（断言失败后行为不同）

| 类型     | 非致命断言（继续）                                    | 致命断言（中断）             |
| ------ | -------------------------------------------- | -------------------- |
| 相等     | `EXPECT_EQ(a, b)`                            | `ASSERT_EQ(a, b)`    |
| 不等     | `EXPECT_NE(a, b)`                            | `ASSERT_NE(a, b)`    |
| 大于     | `EXPECT_GT(a, b)`                            | `ASSERT_GT(a, b)`    |
| 小于     | `EXPECT_LT(a, b)`                            | `ASSERT_LT(a, b)`    |
| 真/假判断  | `EXPECT_TRUE(expr)`                          | `ASSERT_TRUE(expr)`  |
| 空/非空指针 | `EXPECT_EQ(ptr, nullptr)` / `EXPECT_NE(...)` | 同上                   |
| 字符串比较  | `EXPECT_STREQ(a, b)`                         | `ASSERT_STRNE(a, b)` |
| 浮点比较   | `EXPECT_NEAR(a, b, abs_error)`               | `ASSERT_NEAR(...)`   |

---

## 4 测试夹具（Test Fixture）

用于为一组相关测试共享初始化和清理代码。

```cpp
class MyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化资源
    }
    void TearDown() override {
        // 清理资源
    }

    int value;
};

TEST_F(MyTest, CheckInit) {
    value = 42;
    EXPECT_EQ(value, 42);
}
```

---

## 5 参数化测试（进阶）

适用于测试函数在不同输入下的行为。

```cpp
class ParamTest : public ::testing::TestWithParam<int> {};

TEST_P(ParamTest, IsEven) {
    int n = GetParam();
    EXPECT_EQ(n % 2, 0);
}

INSTANTIATE_TEST_SUITE_P(EvenValues, ParamTest, ::testing::Values(2, 4, 6));
```

---

## 6 运行测试

### 使用g++

```bash
g++ my_test.cpp -lgtest -lgtest_main -pthread -o my_test
./my_test
```

### 使用CMake（推荐）

```cmake
# CMakeLists.txt
enable_testing()
add_executable(my_test my_test.cpp)
target_link_libraries(my_test gtest gtest_main pthread)
add_test(NAME MyTest COMMAND my_test)
```

---

## 7 其他常用功能

### 多组断言并列输出

```cpp
EXPECT_EQ(f(1), 1);
EXPECT_EQ(f(2), 2);  // 所有失败会一起报告
```

### 自定义消息

```cpp
EXPECT_EQ(a, b) << "Optional message: a = " << a << ", b = " << b;
```

### 禁用测试

```cpp
TEST(DISABLED_MyTest, DoesNotRun) {
    FAIL();  // 不会执行
}
```

### 全局/每组 SetUp/TearDown

使用 `::testing::Environment` 或 `SetUpTestSuite()` 静态函数。

---

## 8 常用命令行选项

```bash
./my_test --gtest_filter=MyTestSuite.MyTestName     # 只运行某个测试
./my_test --gtest_repeat=10                         # 重复运行
./my_test --gtest_break_on_failure                  # 断点调试用
./my_test --gtest_output=xml:report.xml             # 输出测试报告
```

---

## 9 快速对比 gtest 与 gmock

| 特性   | gtest                  | gmock                        |
| ---- | ---------------------- | ---------------------------- |
| 作用   | 编写和断言测试用例              | 模拟对象/依赖交互                    |
| 示例用途 | 验证函数返回值                | 验证函数是否被调用几次等                 |
| 宏前缀  | `EXPECT_*`, `ASSERT_*` | `EXPECT_CALL`, `MOCK_METHOD` |



# GTest 参数化测试指南：深入理解 `TEST_P` 与 `INSTANTIATE_TEST_SUITE_P`

在编写 C++ 单元测试时，我们常会遇到“相同逻辑、不同数据”的测试场景。Google Test（GTest）提供了强大的参数化测试机制，其中 `TEST_P` 是核心利器，能让我们优雅地测试多个输入输出组合，而不重复代码。

本文将系统介绍 GTest 中参数化测试的用法，尤其是 `TEST_P` 的使用流程，并配合实例代码和常见坑点，帮助你轻松掌握这一工具。

---

## ✨ 什么是参数化测试（Parameterized Test）？

参数化测试的核心思想是：

> **将测试逻辑和测试数据解耦，让一份测试逻辑在多组输入下重复执行。**

在 GTest 中，参数化测试基于四个关键组件：

1. **测试夹具类**：继承自 `::testing::TestWithParam<T>`，其中 `T` 是参数类型；
2. **测试用例**：使用 `TEST_P` 宏定义；
3. **参数实例化**：通过 `INSTANTIATE_TEST_SUITE_P` 将参数注入；
4. **参数获取**：在测试体内通过 `GetParam()` 获取当前参数。

---

## 🧱 基础使用步骤

我们以一个例子说明：测试一个 `IsEven(int n)` 函数是否正确判断偶数。

### 1. 定义被测函数

```cpp
bool IsEven(int n) {
    return n % 2 == 0;
}
```

### 2. 定义测试夹具

```cpp
#include <gtest/gtest.h>

class IsEvenTest : public ::testing::TestWithParam<int> {
    // 无需额外成员
};
```

### 3. 定义参数化测试用例

```cpp
TEST_P(IsEvenTest, HandlesEvenNumbers) {
    int value = GetParam();
    EXPECT_TRUE(IsEven(value));
}
```

### 4. 实例化参数集

```cpp
INSTANTIATE_TEST_SUITE_P(EvenValues, IsEvenTest,
                         ::testing::Values(2, 4, 6, 8, 10));
```

运行后，这个测试会被自动展开为：

* `EvenValues/IsEvenTest.HandlesEvenNumbers/0` with param 2
* `EvenValues/IsEvenTest.HandlesEvenNumbers/1` with param 4
  ……以此类推。

---

## 🧪 使用结构体作为参数类型

你可以使用结构体（或 `std::tuple`）传入多维参数。

```cpp
struct TestParam {
    int input;
    bool expected;
};

class IsEvenBoolTest : public ::testing::TestWithParam<TestParam> {};

bool IsEven(int n) {
    return n % 2 == 0;
}

TEST_P(IsEvenBoolTest, Correctness) {
    TestParam param = GetParam();
    EXPECT_EQ(IsEven(param.input), param.expected);
}

INSTANTIATE_TEST_SUITE_P(MixedCases, IsEvenBoolTest,
    ::testing::Values(
        TestParam{2, true},
        TestParam{3, false},
        TestParam{0, true},
        TestParam{-1, false}
    ));
```

---

## 🎯 使用 `std::tuple` 传递多参数

另一种常见方式是 `std::tuple<T1, T2, ..., Tn>`：

```cpp
class TupleTest : public ::testing::TestWithParam<std::tuple<int, bool>> {};

TEST_P(TupleTest, Test) {
    int input = std::get<0>(GetParam());
    bool expected = std::get<1>(GetParam());

    EXPECT_EQ(IsEven(input), expected);
}

INSTANTIATE_TEST_SUITE_P(WithTuples, TupleTest,
    ::testing::Values(
        std::make_tuple(1, false),
        std::make_tuple(2, true),
        std::make_tuple(5, false)
    ));
```

---

## 🧩 高级技巧

### 1. 参数生成器 `::testing::Range`

你可以使用 GTest 内置的生成器快速创建参数范围：

```cpp
INSTANTIATE_TEST_SUITE_P(RangeTest, IsEvenTest,
    ::testing::Range(0, 10, 2)); // 0, 2, 4, 6, 8
```

### 2. 自定义测试名称

你可以通过 `INSTANTIATE_TEST_SUITE_P` 的第四个参数自定义每个测试的名称：

```cpp
INSTANTIATE_TEST_SUITE_P(
    NamedParams,
    IsEvenTest,
    ::testing::Values(1, 2, 3, 4),
    [](const testing::TestParamInfo<int>& info) {
        return "Input" + std::to_string(info.param);
    });
```

生成的测试名字将是 `NamedParams/Input1`, `Input2` 等，增强可读性和调试能力。

---

## 🧱 典型用例场景

* **算法多组输入验证**：如排序、查找、图遍历等算法。
* **边界值测试**：测试函数在各种极端输入下是否稳定。
* **接口兼容性测试**：多种数据结构/配置组合下，接口是否稳定。
* **测试数据来自文件或 DB**：可将外部数据加载为参数。

---

## ⚠️ 常见坑点与排查建议

| 问题                                  | 原因                     | 解决建议                        |
| ----------------------------------- | ---------------------- | --------------------------- |
| `undefined reference to GetParam()` | 未继承 `TestWithParam<T>` | 检查基类是否正确                    |
| `INSTANTIATE_TEST_SUITE_P` 无效       | 参数类型不匹配或未注册            | 确保参数类型一致                    |
| 参数测试无输出                             | 测试未被运行                 | 检查测试是否编译并链接进主测试             |
| 想传多个参数却只用了 `int`                    | `TEST_P` 只能接收一个 `T`    | 使用 `std::tuple` 或自定义 struct |

---

## ✅ 总结

`TEST_P` 是 GTest 提供的重要工具，适用于测试逻辑固定、数据多样的场景。它能有效减少测试代码重复，并提升可维护性。掌握 `TestWithParam<T>`、`TEST_P`、`INSTANTIATE_TEST_SUITE_P` 三件套，就能在实际工程中发挥巨大作用。

---

## 📎 附录：最小可运行示例（main.cpp）

```cpp
#include <gtest/gtest.h>

bool IsEven(int n) {
    return n % 2 == 0;
}

class IsEvenTest : public ::testing::TestWithParam<int> {};

TEST_P(IsEvenTest, HandlesEvenNumbers) {
    int value = GetParam();
    EXPECT_TRUE(IsEven(value));
}

INSTANTIATE_TEST_SUITE_P(EvenValues, IsEvenTest,
                         ::testing::Values(2, 4, 6, 8, 10));

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

编译指令：

```bash
g++ -std=c++17 main.cpp -lgtest -lgtest_main -pthread -o test
./test
```

---
