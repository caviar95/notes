

```cpp
template <class T, class F>
void for_each_field(T&& value, F&& func) {
    constexpr std::size_t fields_count_val = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

    ::boost::pfr::detail::for_each_field_dispatcher(
        value,
        [f = std::forward<F>(func)](auto&& t) mutable {
            constexpr std::size_t fields_count_val_in_lambda
                = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

            ::boost::pfr::detail::for_each_field_impl(
                t,
                std::forward<F>(f),
                detail::make_index_sequence<fields_count_val_in_lambda>{},
                std::is_rvalue_reference<T&&>{}
            );
        },
        detail::make_index_sequence<fields_count_val>{}
    );
}
```

这是一个 **手动实现的类似 Boost.PFR 的字段遍历函数模板**，实现了对结构体所有成员的遍历。

---

## 🧠 背景知识：Boost.PFR 是如何实现“无侵入反射”的？

* Boost.PFR（Precise Function Reflection）使用的是 C++17 的结构化绑定（aggregate initialization）特性，把结构体“看作”一个 `tuple`，通过元编程进行成员展开。
* 它能在编译期获取一个结构体的成员数量，并用 `std::index_sequence` 展开成员。

---

## 🔍 逐段解析

### 👇 函数模板签名

```cpp
template <class T, class F>
void for_each_field(T&& value, F&& func)
```

* 接受任意类型 `T` 的对象和一个函数 `func`。
* `T&&` 是万能引用（perfect forwarding），允许传左值/右值。
* `F&&` 同理，支持函数对象的完美转发。

---

### 👇 编译期获取字段数量

```cpp
constexpr std::size_t fields_count_val = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();
```

* `boost::pfr::detail::fields_count<T>()` 是 Boost.PFR 的内部函数，用于**编译期获取结构体 T 的成员数量**。
* 必须先移除引用类型 `std::remove_reference_t<T>`。

---

### 👇 调用字段分发器

```cpp
::boost::pfr::detail::for_each_field_dispatcher(
    value,
    [f = std::forward<F>(func)](auto&& t) mutable {
        constexpr std::size_t fields_count_val_in_lambda
            = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

        ::boost::pfr::detail::for_each_field_impl(
            t,
            std::forward<F>(f),
            detail::make_index_sequence<fields_count_val_in_lambda>{},
            std::is_rvalue_reference<T&&>{}
        );
    },
    detail::make_index_sequence<fields_count_val>{}
);
```

### 拆解部分：

#### 🔹 `[f = std::forward<F>(func)](auto&& t)` 是 lambda，用于包装回调函数

* 这一步是**捕获用户的回调函数**，传给后面的实现代码。
* 使用 `mutable` 是因为 `f` 是右值引用，要允许在 lambda 中移动它。

#### 🔹 `fields_count_val_in_lambda`

* 再次在 lambda 中计算字段数量，是为了规避 MSVC（微软编译器）不能在 lambda 外使用 `constexpr` 捕获的问题。

#### 🔹 `make_index_sequence<...>{}`

* 用于在编译期生成字段索引 `[0, 1, ..., N-1]`，展开结构体成员。

#### 🔹 `for_each_field_impl(...)`

* 最终调用核心函数：**将结构体按字段展开成 `get<0>(...), get<1>(...)...`，然后逐个调用用户函数 `f`。**

#### 🔹 `std::is_rvalue_reference<T&&>{}`

* 判断结构是否是右值引用，用于控制成员访问的方式（拷贝 vs move）。

---

## 🧩 总体流程图解

```
               +---------------------+
  (user struct)│    struct MyType {  │
               │      int a;         │
               │      double b;      │
               │    };               │
               +---------------------+
                        │
                        ▼
      for_each_field(my, [](auto& field) { print(field); })
                        │
                        ▼
          boost::pfr::detail::fields_count<MyType>()  → 2
                        │
                        ▼
        boost::pfr::detail::make_index_sequence<2>
                        │
                        ▼
        boost::pfr::detail::for_each_field_impl:
            call func(get<0>(my))
            call func(get<1>(my))
```

---

## ✅ 总结：这个函数做了什么？

* **接收一个结构体实例**和一个回调函数；
* **编译期获取结构体字段数量**；
* 通过 `index_sequence` 展开字段；
* 逐字段调用用户提供的回调函数 `func(field)`；
* 完全**无需结构体定义任何额外方法**；
* 利用了 Boost.PFR 内部机制 `fields_count`, `for_each_field_impl`, `index_sequence`；
* 对用户是“无侵入的反射”。

---

## ❗注意

此函数使用了 Boost.PFR 的内部命名空间 `detail::`，不是官方推荐的方式。如果你想使用公开 API，可以直接用：

```cpp
boost::pfr::for_each_field(s, [](const auto& field) {
    std::cout << field << std::endl;
});
```

---

如你想完全自定义并实现这种反射机制，也可以通过模板 + 聚合初始化自己实现一个 PFR-like 模块，我可以帮你做一个“轻量无 Boost 依赖”的版本。是否需要？
