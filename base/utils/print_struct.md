你说的“打印结构体的方法，不用自己一个一个写打印函数，自动打印所有成员”，其实是个挺经典的需求。用纯C++（尤其是C++11/14/17）原生是不太好直接做到的，因为C++没有反射机制，无法自动遍历结构体成员。

不过，基于现代C++有几种常用的“变通方案”可以实现类似效果：

---

## 方法1：宏+元编程（类似“结构体元信息”）

核心思路：
通过宏定义结构体成员列表，然后用宏展开自动生成打印代码。

示例代码：

```cpp
#include <iostream>

#define STRUCT_MEMBERS \
    X(int, a)          \
    X(double, b)       \
    X(std::string, c)

struct MyStruct {
#define X(type, name) type name;
    STRUCT_MEMBERS
#undef X
};

std::ostream& operator<<(std::ostream& os, const MyStruct& s) {
#define X(type, name) os << #name << ": " << s.name << ", ";
    STRUCT_MEMBERS
#undef X
    return os;
}

int main() {
    MyStruct s{1, 2.5, "hello"};
    std::cout << s << std::endl;
}
```

优点：只写一次成员宏列表，打印代码自动生成。
缺点：写结构体时必须用宏方式定义成员。

---

## 方法2：Boost.PFR (Precise Function Reflection)

Boost库中的`Boost.PFR`支持“无侵入”结构体成员遍历，前提是结构体是“聚合类型”（简单的public成员结构体）。

示例：

```cpp
#include <boost/pfr.hpp>
#include <iostream>
#include <string>

struct MyStruct {
    int a;
    double b;
    std::string c;
};

int main() {
    MyStruct s{1, 2.5, "hello"};

    boost::pfr::for_each_field(s, [](const auto& field, std::size_t idx) {
        std::cout << "Field " << idx << ": " << field << std::endl;
    });
}
```

优点：不需要改结构体定义，无需宏。
缺点：依赖Boost库，且只能打印成员值，成员名需要自己维护。

---

## 方法3：C++20的`std::tuple` + `std::apply`模拟反射

如果你愿意把结构体用`std::tuple`表示，或者写一个转换函数，将结构体成员包装成元组，然后用`std::apply`展开。

示例：

```cpp
#include <iostream>
#include <tuple>
#include <string>

struct MyStruct {
    int a;
    double b;
    std::string c;
};

auto as_tuple(const MyStruct& s) {
    return std::tie(s.a, s.b, s.c);
}

int main() {
    MyStruct s{1, 2.5, "hello"};
    std::apply([](auto&&... args) {
        ((std::cout << args << " "), ...);
    }, as_tuple(s));
}
```

缺点：依然需要手写`as_tuple`函数。

---

## 总结

| 方法          | 是否自动 | 需要改结构体定义            | 依赖库   | 打印成员名 | 备注           |
| ----------- | ---- | ------------------- | ----- | ----- | ------------ |
| 宏定义成员列表     | 半自动  | 需要                  | 无     | 是     | 灵活，但写宏不方便    |
| Boost.PFR   | 自动   | 不需要                 | Boost | 否     | 只能打印值，不能打印名字 |
| tuple+apply | 手动   | 不需要，但需要手写`as_tuple` | 无     | 否     | 比较简洁，支持任意成员数 |


