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


