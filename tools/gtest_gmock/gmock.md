`gmock`（Google Mock）是 Google 提供的 C++ Mock 测试框架，通常与 `gtest`（Google Test）配合使用，用于编写单元测试中的模拟对象（Mock Objects），以便隔离依赖、验证交互等。

---

## 1 gmock 作用和使用场景

### 用途：

* 隔离依赖模块（如数据库、网络、外部服务等）。
* 验证函数调用顺序、次数、参数。
* 模拟特定行为（如返回值、异常）。
* 用于接口驱动开发（TDD/BDD）。

### 使用场景：

* 测试逻辑模块 A，依赖接口 I，你不想调用 I 的真实实现。
* 验证模块 A 是否“正确调用了接口 I”。

---

## 2 gmock 基本结构

### 2.1 定义接口类（纯虚类）

```cpp
class Database {
public:
    virtual ~Database() = default;
    virtual bool Connect(const std::string& url) = 0;
    virtual int Query(const std::string& sql) = 0;
};
```

### 2.2 定义 Mock 类（使用宏 `MOCK_METHOD`）

```cpp
#include <gmock/gmock.h>

class MockDatabase : public Database {
public:
    MOCK_METHOD(bool, Connect, (const std::string& url), (override));
    MOCK_METHOD(int, Query, (const std::string& sql), (override));
};
```

---

## 3 常用操作

### 设置返回值

```cpp
MockDatabase mock;
EXPECT_CALL(mock, Connect).WillOnce(::testing::Return(true));
```

### 设置多个调用的行为

```cpp
EXPECT_CALL(mock, Connect)
    .Times(2)
    .WillRepeatedly(::testing::Return(true));
```

### 按参数返回不同值

```cpp
EXPECT_CALL(mock, Query(::testing::StrEq("SELECT * FROM users")))
    .WillOnce(::testing::Return(42));
```

### 验证调用次数

```cpp
EXPECT_CALL(mock, Query).Times(3);
```

### 模拟调用顺序

```cpp
::testing::InSequence s;
EXPECT_CALL(mock, Connect);
EXPECT_CALL(mock, Query);
```

### 调用自定义行为（回调）

```cpp
EXPECT_CALL(mock, Query)
    .WillOnce([](const std::string& sql) {
        std::cout << "Intercepted query: " << sql << std::endl;
        return 100;
    });
```

---

## 4 示例

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Database {
public:
    virtual ~Database() = default;
    virtual bool Connect(const std::string& url) = 0;
    virtual int Query(const std::string& sql) = 0;
};

class MockDatabase : public Database {
public:
    MOCK_METHOD(bool, Connect, (const std::string& url), (override));
    MOCK_METHOD(int, Query, (const std::string& sql), (override));
};

class Service {
public:
    Service(Database* db) : db_(db) {}
    bool InitAndQuery() {
        if (!db_->Connect("localhost")) return false;
        return db_->Query("SELECT * FROM table") > 0;
    }
private:
    Database* db_;
};

TEST(ServiceTest, InitAndQuerySuccess) {
    MockDatabase mock;
    EXPECT_CALL(mock, Connect("localhost")).WillOnce(::testing::Return(true));
    EXPECT_CALL(mock, Query("SELECT * FROM table")).WillOnce(::testing::Return(42));

    Service svc(&mock);
    EXPECT_TRUE(svc.InitAndQuery());
}
```

---

## 5 gmock 常用宏速查表

| 宏名                                                | 说明                                     |
| ------------------------------------------------- | -------------------------------------- |
| `MOCK_METHOD(ret_type, name, (args...), (specs))` | 定义 Mock 方法（specs 可包含 override、const 等） |
| `EXPECT_CALL(mock_obj, method(args_matcher))`     | 设置期望调用                                 |
| `WillOnce(Return(val))`                           | 设置单次返回值                                |
| `WillRepeatedly(Return(val))`                     | 设置默认返回值                                |
| `Times(n)`                                        | 设置期望调用次数                               |
| `InSequence seq;`                                 | 设置调用顺序                                 |
| `Invoke(lambda)` / `WillOnce([]{})`               | 自定义行为                                  |
| `DoAll(...)`                                      | 组合多个操作                                 |
| `SaveArg<N>(&var)`                                | 保存调用参数                                 |
| `WithArg<N>(lambda)`                              | 对指定参数执行操作                              |



# 🧪 GMock 使用指南：原理、用法与实战示例

在 C++ 单元测试中，我们常常希望\*\*“隔离依赖”\*\*，即只测试当前模块的行为而不被其它模块干扰。这时候，Google 提供的 Mock 框架 —— **Google Mock（GMock）** 就能派上大用场。

本文将系统介绍 GMock 的使用原理、常用语法和示例实践，帮助你快速掌握它的精髓。

---

## 🎯 为什么需要 Mock？

现实开发中一个函数往往会依赖外部模块（如数据库、网络、文件系统等），而这些模块通常：

* 响应慢（影响测试速度）
* 难以构造（如网络异常）
* 不可控（如读取时间）

Mock 的作用是：

> **用一个“假的对象”来代替真实依赖，用于控制、记录、验证行为。**

Mock 让我们可以这样做：

* 替换复杂依赖
* 精准验证调用次数、顺序、参数
* 更快的测试执行

---

## ⚙️ GMock 的原理简析

GMock 基于 C++ 虚函数机制工作，核心思想是：

1. 你定义一个接口类（纯虚函数）
2. GMock 自动生成该接口的 mock 实现（宏方式）
3. 在测试中注入 mock 对象，通过设置“期望值”来断言行为

简单来说：**Mock 对象是一个带行为验证的虚类实现**。

---

## 🔧 常见用法汇总

| 用法                                 | 说明                   |
| ---------------------------------- | -------------------- |
| `MOCK_METHOD()` / `MOCK_METHODn()` | 定义 mock 方法           |
| `EXPECT_CALL(mock, Method(args))`  | 设置期望调用（可包含次数、顺序、返回值） |
| `.WillOnce(Return(val))`           | 设置返回值                |
| `.Times(n)`                        | 限定调用次数               |
| `.With(...)`                       | 指定参数匹配器              |
| `.InSequence(...)`                 | 限定调用顺序               |

---

## 📚 示例：一个简单的数据库场景

### 1. 定义接口类

```cpp
// IDatabase.h
class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool Connect(const std::string& url) = 0;
    virtual int Query(const std::string& sql) = 0;
};
```

### 2. 使用 GMock 定义 Mock 类

```cpp
#include <gmock/gmock.h>
#include "IDatabase.h"

class MockDatabase : public IDatabase {
public:
    MOCK_METHOD(bool, Connect, (const std::string&), (override));
    MOCK_METHOD(int, Query, (const std::string&), (override));
};
```

### 3. 被测业务逻辑

```cpp
class DataFetcher {
public:
    DataFetcher(IDatabase* db) : db_(db) {}
    int GetUserCount() {
        if (!db_->Connect("db://remote")) return -1;
        return db_->Query("SELECT COUNT(*) FROM users");
    }
private:
    IDatabase* db_;
};
```

### 4. 测试用例编写

```cpp
#include <gtest/gtest.h>

TEST(DataFetcherTest, FetchSuccess) {
    MockDatabase mockDb;

    EXPECT_CALL(mockDb, Connect(::testing::StrEq("db://remote")))
        .Times(1)
        .WillOnce(::testing::Return(true));

    EXPECT_CALL(mockDb, Query(::testing::HasSubstr("users")))
        .Times(1)
        .WillOnce(::testing::Return(42));

    DataFetcher fetcher(&mockDb);
    EXPECT_EQ(fetcher.GetUserCount(), 42);
}
```

---

## 🔍 参数匹配器（Matchers）

GMock 提供丰富的参数匹配方式：

```cpp
EXPECT_CALL(mock, Method(_, _));                     // 任意参数
EXPECT_CALL(mock, Method(42, "hello"));              // 精确匹配
EXPECT_CALL(mock, Method(_, ::testing::Gt(10)));     // 大于匹配
EXPECT_CALL(mock, Method(::testing::StartsWith("db"))); // 字符串前缀
```

常用匹配器包括：

* `Eq(val)` / `Ne(val)`
* `Gt(val)` / `Lt(val)` / `Ge(val)` / `Le(val)`
* `Contains(substring)` / `HasSubstr(substr)`
* `_`：通配符，匹配任意参数

---

## 🧩 设置调用行为（Actions）

```cpp
.WillOnce(Return(42))
.WillRepeatedly(Return(-1)) // 多次调用使用此值
.WillOnce(Invoke([](const std::string& sql) {
    return sql == "SELECT *" ? 100 : 0;
}))
```

你也可以模拟抛出异常、记录调用顺序，甚至在调用中断言。

---

## 🔁 调用次数控制

```cpp
EXPECT_CALL(mock, Query(_)).Times(1);
EXPECT_CALL(mock, Query(_)).Times(::testing::AtLeast(1));
EXPECT_CALL(mock, Query(_)).Times(::testing::Between(1, 3));
```

---

## ⛓ 顺序验证

GMock 允许验证函数调用顺序：

```cpp
::testing::Sequence s;

EXPECT_CALL(mock, Connect(_)).InSequence(s);
EXPECT_CALL(mock, Query(_)).InSequence(s);
```

---

## 🧨 常见错误与排查

| 现象               | 原因                       | 解决                                |
| ---------------- | ------------------------ | --------------------------------- |
| `EXPECT_CALL` 无效 | Mock 类未正确继承/声明           | 使用 `MOCK_METHOD(..., (override))` |
| 报错说找不到符号         | 缺少链接 `-lgmock`           | 编译时添加 `-lgmock -lgtest -pthread`  |
| 调用顺序断言失败         | 忘记用 `InSequence` 或调用顺序写反 | 使用 `Sequence` 明确顺序                |

---

## ✅ 小结

| 关键词                        | 作用                       |
| -------------------------- | ------------------------ |
| `MOCK_METHOD()`            | 定义模拟接口                   |
| `EXPECT_CALL`              | 设置调用期望与行为                |
| `Matchers`                 | 参数匹配（Eq、Gt、HasSubstr、\_） |
| `Actions`                  | 返回值、Lambda、自定义行为         |
| `Times()` / `InSequence()` | 次数/顺序控制                  |

GMock 是 Google Test 强大的“虚拟依赖替身系统”，适合在中大型工程中用于隔离、验证和控制行为。只需理解“接口驱动 + 模拟替换”的理念，就能在各类场景中快速使用。

---

## 📦 编译示例

```bash
g++ -std=c++17 test.cpp -lgtest -lgmock -pthread -o test
./test
```

确保安装了 GTest/GMock，可通过包管理器安装：

```bash
sudo apt install libgtest-dev libgmock-dev
```

---

## 📌 附加建议

* 推荐搭配接口注入模式（依赖注入）使用
* 可用于模拟系统调用、RPC、网络、数据库、线程、文件等模块
* 与 `gtest` 无缝结合，可以嵌入 `TEST_F`、`TEST_P` 等高级场景

