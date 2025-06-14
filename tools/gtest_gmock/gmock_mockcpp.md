`gmock` 和 `mockcpp` 是 C++ 中常用的两个 Mock 框架，用于单元测试时模拟对象或函数的行为。

它们的核心目标相似：使测试更加隔离、可控与可验证，但两者在设计理念、使用方式、语法风格、兼容性等方面存在明显差异。


---

## 1 基本介绍

| 项目   | gmock（Google Mock）  | mockcpp             |
| ---- | ------------------- | ------------------- |
| 作者   | Google              | 东南大学（何海涛等）          |
| 语言   | C++（与 gtest 紧密集成）   | C++                 |
| 发布时间 | 2008 年左右            | 2006 年左右            |
| 依赖   | Google Test (gtest) | 无需 gtest，可与其它测试框架搭配 |

---

## 2 语法风格与使用方式

### gmock 特点（基于接口 Mock）：

* 基于接口/抽象类设计（需要提前设计好虚函数接口）
* 使用宏定义 `MOCK_METHOD()` 快速生成 Mock 函数
* 测试语句直观如：`EXPECT_CALL(mockObj, Foo()).Times(1);`
* 支持匹配器（`_`, `Eq(x)`, `AnyOf`, `AllOf` 等）
* 语法现代、类型安全，支持 C++11+

```cpp
class ICalc {
public:
    virtual int Add(int a, int b) = 0;
    virtual ~ICalc() = default;
};

class MockCalc : public ICalc {
public:
    MOCK_METHOD(int, Add, (int a, int b), (override));
};

TEST(MyTest, Addition) {
    MockCalc calc;
    EXPECT_CALL(calc, Add(1, 2)).WillOnce(Return(3));
    ASSERT_EQ(calc.Add(1, 2), 3);
}
```

---

### mockcpp 特点（基于重写/拦截原函数）：

* 不依赖接口或虚函数，可以对 非虚函数/全局函数/静态函数 进行 Mock
* 使用 `MOCKER(methodName).stubs().will(returnValue());` 风格
* 内部使用函数地址重写、hook 技术等实现原函数替换
* 支持打桩（Stub）与调用验证
* 适合 legacy code（老代码）测试，不易侵入原设计

```cpp
class Calc {
public:
    int Add(int a, int b) { return a + b; }
};

TEST_F(MyTest, AddMock) {
    MOCK_METHOD(Calc, Add).stubs().will(returnValue(42));
    Calc calc;
    ASSERT_EQ(calc.Add(1, 2), 42);
}
```

---

## 3 对比总结

| 特性      | gmock                       | mockcpp                   |
| ------- | --------------------------- | ------------------------- |
| Mock 方式 | 基于虚函数接口                     | 函数地址拦截，可 Mock 普通函数        |
| 对老代码支持  | 差，需重构为虚接口                   | 强，无需修改原代码                 |
| 语法风格    | 现代，强类型安全，表达式链式调用            | 类似脚本语言的 DSL，易用但类型不安全      |
| 调用验证    | 支持 `EXPECT_CALL`            | 支持 `.expects().with(...)` |
| 可测试对象   | 虚函数、成员函数                    | 所有函数（包括静态函数、全局函数）         |
| 依赖关系    | 依赖 gtest                    | 独立，兼容多种测试框架               |
| 编译兼容性   | 高度依赖编译器特性，现代 GCC/Clang 更好支持 | 低层 hook，有些编译器和系统下不稳定      |
| 适合应用    | 新代码、接口良好的模块                 | 老代码、未封装接口的遗留项目            |

---

## 4 实际使用建议

* 如果你在做现代 C++ 项目（C++11+）并使用 gtest 作为测试框架，推荐使用 gmock，配合 gtest 效果更佳；
* 如果你在维护遗留项目、无法改写为虚函数接口，或者需要对 静态函数、全局函数、第三方库函数进行打桩，推荐使用 mockcpp；
* 也可以在一个项目中同时使用 gmock 和 mockcpp，根据不同的模块情况选择合适的 Mock 方式。


