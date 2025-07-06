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


