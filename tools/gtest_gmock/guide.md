以下是一篇面向 C++ 开发者、以 Google Test/GMock 为核心工具，系统讲解如何在工程中编写高质量单元测试（UT）的指导文章。文章分为七大部分：环境准备、测试基础、测试夹具、参数化测试、Mock 对象、最佳实践与进阶技巧、总结。

---

## 一、为什么要做单元测试

1. **提高代码质量**：通过对函数、类的边界条件和异常路径进行覆盖，能及早发现逻辑缺陷。
2. **降低维护成本**：重构时，有信心现有功能不被破坏，快速定位回归问题。
3. **文档即测试**：测试代码本身也是最准确的使用示例，便于新人理解接口设计。
4. **持续集成**：与 CI/CD 流水线结合，实现每次提交自动验证，为团队协作保驾护航。

---

## 二、环境准备

1. **获取源码**

   * GitHub 地址：[https://github.com/google/googletest](https://github.com/google/googletest)
2. **在 CMake 工程中集成**

   ```cmake
   # 在根 CMakeLists.txt 中添加
   include(FetchContent)
   FetchContent_Declare(
     googletest
     GIT_REPOSITORY https://github.com/google/googletest.git
     GIT_TAG        release-1.13.0
   )
   FetchContent_MakeAvailable(googletest)

   # 为测试添加子目录
   enable_testing()
   add_subdirectory(test)
   ```
3. **测试目标**

   ```cmake
   # test/CMakeLists.txt
   add_executable(mytests
     example_test.cpp
     mock_service_test.cpp
   )
   target_link_libraries(mytests
     gtest_main
     gmock_main
     your_project_lib
   )
   include(GoogleTest)
   gtest_discover_tests(mytests)
   ```
4. **编译运行**

   ```bash
   mkdir build && cd build
   cmake ..
   make -j
   ctest --output-on-failure
   ```

---

## 三、Google Test 基础

### 1. TEST 宏

* **语法**

  ```cpp
  TEST(TestSuiteName, TestName) {
    EXPECT_EQ(func(1), 2);
    ASSERT_TRUE(isValid(...));
  }
  ```
* **EXPECT\_ vs ASSERT\_**

  * `EXPECT_*`：失败后继续执行当前测试
  * `ASSERT_*`：失败后立即退出当前测试，用于“必须成立”的前提

### 2. 常用断言

* **相等/近似**

  ```cpp
  EXPECT_EQ(a, b);
  EXPECT_NE(a, b);
  EXPECT_NEAR(val, expect, abs_error);
  ```
* **布尔**

  ```cpp
  EXPECT_TRUE(cond);
  EXPECT_FALSE(cond);
  ```
* **指针**

  ```cpp
  EXPECT_NULL(ptr);
  EXPECT_NOTNULL(ptr);
  ```
* **异常**

  ```cpp
  EXPECT_THROW(func(), std::runtime_error);
  EXPECT_NO_THROW(func());
  ```

---

## 四、测试夹具（Test Fixture）

当多个测试需要重复创建/销毁测试对象时，使用测试夹具能让代码更简洁可维护。

```cpp
class MyClassTest : public ::testing::Test {
protected:
  void SetUp() override {
    svc = std::make_unique<MyService>();
    client = std::make_unique<Client>(*svc);
  }
  void TearDown() override {
    // 清理资源（一般无需手动，智能指针自动释放）
  }
  std::unique_ptr<MyService> svc;
  std::unique_ptr<Client> client;
};

TEST_F(MyClassTest, DefaultBehavior) {
  EXPECT_EQ(client->doWork(5), 25);
}

TEST_F(MyClassTest, ErrorOnNegative) {
  EXPECT_THROW(client->doWork(-1), std::invalid_argument);
}
```

* `TEST_F`：绑定到夹具类
* `SetUp`/`TearDown`：在每个 `TEST_F` 运行前后自动调用
* 构造/析构与 `SetUp`/`TearDown` 的区别：

  * 构造析构用于初始化固定成员；
  * `SetUp` 可处理更复杂准备，抛错会使测试失败并停止。

---

## 五、参数化测试

当同一逻辑需要在多组输入下重复验证，使用参数化测试可减少代码冗余。

### 1. 简单值参数化

```cpp
class FactorialTest : public ::testing::TestWithParam<std::pair<int,int>> {};

TEST_P(FactorialTest, HandlesVariousInputs) {
  auto [input, expect] = GetParam();
  EXPECT_EQ(Factorial(input), expect);
}

INSTANTIATE_TEST_SUITE_P(
  ValidInputs,
  FactorialTest,
  ::testing::Values(
    std::make_pair(0,1),
    std::make_pair(1,1),
    std::make_pair(5,120),
    std::make_pair(10,3628800)
  )
);
```

### 2. 结合打印

```cpp
struct Case {
  int x;
  bool valid;
};

class ParseTest : public ::testing::TestWithParam<Case> {};

TEST_P(ParseTest, HandlesJson) {
  const Case &c = GetParam();
  if (c.valid) {
    EXPECT_NO_THROW(parseJson(x));
  } else {
    EXPECT_THROW(parseJson(x), ParseError);
  }
}

INSTANTIATE_TEST_SUITE_P(
  AllCases,
  ParseTest,
  ::testing::Values(
    Case{1,true}, Case{9999,false}
  ),
  [](const ::testing::TestParamInfo<ParseTest::ParamType>& info) {
    // 自定义子测试名
    return "Case" + std::to_string(info.param.x);
  }
);
```

---

## 六、Google Mock 使用指南

### 1. Mock 类的定义

假设有接口：

```cpp
class IRepository {
public:
  virtual ~IRepository() = default;
  virtual bool save(const std::string &data) = 0;
  virtual std::string load(int id) = 0;
};
```

创建 Mock：

```cpp
#include <gmock/gmock.h>

class MockRepository : public IRepository {
public:
  MOCK_METHOD(bool, save, (const std::string& data), (override));
  MOCK_METHOD(std::string, load, (int id), (override));
};
```

### 2. 设置期望（Expectations）

* **简单期望**

  ```cpp
  MockRepository mock;
  EXPECT_CALL(mock, save("hello"))
      .Times(1)
      .WillOnce(::testing::Return(true));
  ```
* **返回值序列**

  ```cpp
  EXPECT_CALL(mock, load(::testing::_))
      .WillOnce(::testing::Return("one"))
      .WillRepeatedly(::testing::Return("default"));
  ```
* **参数匹配器**

  * `_`：任意参数
  * `Eq(val)`：等于
  * `Gt(val)`/`Lt(val)`：大于/小于
  * `Contains(substr)`：包含

### 3. 与被测对象集成

```cpp
class Service {
public:
  Service(IRepository &repo) : repo_(repo) {}
  bool process(const std::string &d) {
    if (!repo_.save(d)) return false;
    auto r = repo_.load(1);
    return !r.empty();
  }
private:
  IRepository &repo_;
};

TEST(ServiceTest, ProcessSuccess) {
  MockRepository mock_repo;
  EXPECT_CALL(mock_repo, save("data")).WillOnce(Return(true));
  EXPECT_CALL(mock_repo, load(1)).WillOnce(Return("ok"));

  Service svc(mock_repo);
  EXPECT_TRUE(svc.process("data"));
}
```

---

## 七、最佳实践与进阶技巧

1. **保持测试独立**：不同测试间不依赖共享状态，方便并行执行。
2. **快速执行**：避免在测试中做 I/O、sleep 或调用外部进程。
3. **合适的覆盖率**：关注业务逻辑边界、异常流程；不要盲目追求 100%。
4. **命名规范**：`TEST(Suite, ShouldDoXWhenY)` 或 `TEST_F(ClassTest, Behavior_Z)`，弱化实现细节。
5. **使用 Death Tests**：测试断言失败流程，例如：

   ```cpp
   EXPECT_DEATH({ crash(); }, "segfault");
   ```
6. **组合 Fixture 与 Mock**：通过继承或聚合，将常用 Mock 对象与 SetUp 结合，减少重复。
7. **捕获日志输出**：配合 `testing::internal::CaptureStdout()` 验证函数印出的信息。
8. **自动化生成测试报告**：配合 `--gtest_output=xml:report.xml`，在 CI 中生成 JUnit/XML 报告。

---

## 八、总结

本文系统介绍了如何在 C++ 项目中引入 Google Test / Google Mock，涵盖：

* 环境搭建与 CMake 集成
* 基本断言与测试用例编写
* Test Fixture 与参数化测试
* Mock 类的定义与期望设置
* 与业务代码的集成示例
* 常见最佳实践与进阶技巧

通过合理地组织测试、使用 Mock 对象隔离外部依赖、并将单元测试纳入持续集成流程，能大幅提升代码质量与开发效率。希望本文能帮助你入门并精通 gtest/gmock，让你的 C++ 应用更加健壮可靠。
