在 C++ 里，如果你给一个函数加上 `noexcept`（或者编译器推断它是 `noexcept`），意味着你**承诺**这个函数不会抛出异常。

如果它**真的**抛出了异常，标准的处理方式是：

**直接调用 `std::terminate()` 结束程序**，而不会像普通函数一样去寻找 `catch` 块。

---

### 详细过程

1. **函数被标记 `noexcept`**

   ```cpp
   void f() noexcept {
       throw std::runtime_error("oops");
   }
   ```

2. **执行时抛出异常**

   * 编译器在生成代码时，会在函数尾或异常表中标记它为“不能抛出异常”。
   * 如果发生了异常，C++ 运行时检测到该异常**无法传播**（因为 `noexcept` 禁止传播）。

3. **调用 `std::terminate()`**

   * 运行时不会去寻找 `try...catch`，而是立即调用 `std::terminate()`。
   * 默认 `std::terminate()` 会调用 `std::abort()`，导致程序异常退出（通常会产生 core dump）。

---

### 示例

```cpp
#include <iostream>
#include <stdexcept>

void test() noexcept {
    throw std::runtime_error("error in noexcept");
}

int main() {
    try {
        test(); // 无法被 catch 到
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << '\n';
    }
}
```

**运行结果**（不会进入 `catch`）：

```
terminate called after throwing an instance of 'std::runtime_error'
  what():  error in noexcept
Aborted (core dumped)
```

---

### 额外细节

* `noexcept` 并不阻止你在函数里写 `throw`，只是规定了**抛出异常的后果**。

* 你可以在 `noexcept` 函数内部捕获并处理异常，只要它不传播到外面，就不会触发 `std::terminate()`：

  ```cpp
  void safe_noexcept() noexcept {
      try {
          risky_function();
      } catch (...) {
          // 异常被吃掉，没事
      }
  }
  ```

* `noexcept(true)` 和 `noexcept` 等价，`noexcept(false)` 相当于没标记。

---
好——下面把 **C++ 中最常见会抛出异常的操作** 按类别列清楚，每项给出为什么会抛、常见异常类型和简短示例 / 应对建议。需要我把某一类展开成可运行的小例子也可以继续说。

# 常见会抛异常的操作（按类）

**1. 内存分配 / new**

* 为什么：`new` 在分配失败时会抛 `std::bad_alloc`（默认行为）。
* 示例：`int *p = new int[100000000000];`
* 应对：在性能/异常敏感代码用 `new (std::nothrow)` 或预分配，或在接口边界捕获 `bad_alloc`。

**2. 容器操作（会导致分配或拷贝/移动构造）**

* 为什么：`push_back`/`emplace_back`、`insert`、`resize`、`reserve` 等可能分配内存或调用元素的拷贝/移动/构造函数，这些都可能抛异常（`bad_alloc` 或元素构造抛出）。
* 示例：`v.push_back(x);`（如果 reallocate 或 x 的拷贝构造抛出）
* 应对：用 `reserve` 预留、确保移动构造被标记 `noexcept`（可以避免某些容器在重分配时使用拷贝而不是移动）。

**3. 字符串及文本转换**

* 为什么：`std::string` 的操作可能分配；`std::stoi`/`std::stod` 在非法或越界时抛 `std::invalid_argument` / `std::out_of_range`。
* 示例：`int n = std::stoi(s);`
* 应对：先验证字符串或使用 `std::from_chars`（这是无异常接口）。

**4. 流（iostream）与文件系统**

* 为什么：默认 iostream 不一定抛，但如果你启用了异常掩码会抛 `std::ios_base::failure`；`std::filesystem` 的函数常抛 `std::filesystem::filesystem_error`。
* 示例：`fs::create_directories(path);` 可能抛 `filesystem_error`。
* 应对：捕获异常或使用返回码/检查状态位。

**5. RTTI / 类型转换 / 类型访问**

* `dynamic_cast<T&>(obj)` — 如果失败会抛 `std::bad_cast`（对引用）；对指针返回 `nullptr` 不抛。
* `typeid` 通常不抛。
* `std::any_cast`、`std::variant::get`、`std::optional::value` 等在类型不匹配或无值时会抛 `bad_any_cast`、`bad_variant_access`、`bad_optional_access`。
* 示例：`std::optional<int> o; o.value();` 抛 `bad_optional_access`。

**6. 标准库工具 / 算法依赖用户代码**

* 为什么：`std::sort`、`std::for_each` 等会调用用户提供的比较器/函数，如果这些函数抛，会传播异常。
* 示例：`std::sort(v.begin(), v.end(), cmp)`，如果 `cmp` 抛异常，`sort` 会中断并向上抛。
* 应对：确保回调/比较器稳定、不抛或在边界捕获异常。

**7. 异常类型 / 标准异常类**

* 常见：`std::bad_alloc`, `std::out_of_range`, `std::invalid_argument`, `std::ios_base::failure`, `std::bad_cast`, `std::system_error`（线程/同步/系统调用失败时）。
* 示例：线程或互斥操作失败可能抛 `std::system_error`。

**8. 线程 / 同步 / 系统资源**

* 为什么：`std::thread` 构造、`std::mutex`/`std::condition_variable` 的某些操作在系统资源不足或错误时可抛 `std::system_error`。
* 示例：`std::thread t(func);`（创建线程失败在某些实现中抛）

**9. 正则 / 文本解析 / 库函数**

* `std::regex` 构造或匹配在语法错误时抛 `std::regex_error`。其他库（网络、数据库、第三方）很多 API 也通过异常报告错误。

**10. 用户代码中的构造/析构/赋值等**

* 为什么：类的构造函数、赋值操作、析构（不应抛）中若有异常操作，会把异常向上传播。默认或自定义拷贝/移动构造也可能抛。
* 应对：把可能抛的操作放在构造体外，或使用资源获取即初始化（RAII）并设计强异常安全性。

# 简短示例（几行）

```cpp
#include <vector>
#include <string>
#include <any>

void f() {
    std::vector<std::string> v;
    v.reserve(1);
    v.push_back(std::string(10000000000, 'x')); // 可能抛 std::bad_alloc

    int x = std::stoi("not-a-number"); // std::invalid_argument

    std::any a = 1;
    double d = std::any_cast<double>(a); // std::bad_any_cast
}
```

# 如何在 `noexcept`/异常敏感代码中降低风险（实用建议）

* 尽量**不要**在 `noexcept` 函数里调用可能抛的代码；若必须调用，内部 `try { ... } catch(...) { /* 处理或记录 */ }`。
* 预分配（`reserve`、`reserve` 容器容量、`make_shared` 前处理）以减少运行时分配。
* 使用 `std::from_chars` 替代 `std::stoi`（无异常且高效）。
* 对第三方或系统 API，查看是否提供非抛出版本或返回错误码。
* 设计移动构造/移动赋值为 `noexcept`（如果安全），这能减少容器重分配时的异常开销。
* 在程序边界（例如线程入口、main、或公共 API 层）统一捕获异常并进行清理/记录。

---

好——分两部分说明：**为什么把移动构造/移动赋值标为 `noexcept` 会减少容器重分配的开销**，以及**在外层捕获异常是否会影响原有的正常功能**（以及如何保证安全）。

---

# 1) 为什么 `noexcept` 的移动操作能降低容器重分配开销（要点）

* 当 `std::vector`（或其它顺序容器）需要**扩容/搬移已有元素**到新内存时，它要把旧元素从旧内存构造到新内存。实现上有两个选择：**移动构造**（move）或**拷贝构造**（copy）。
* 如果移动构造有可能抛出（即不是 `noexcept`），为了**保证异常安全性**（通常是强异常保证：要么操作成功，要么容器保持原样），标准库实现**通常会选择用拷贝构造**，因为拷贝在许多类型上要么存在且更安全、要么实现者可以用拷贝来保证在异常发生时回退到原来状态。
* 相反，当移动构造是 `noexcept`（或实现判断为不抛出）时，库实现能放心使用**移动**来搬迁元素——移动通常比拷贝更便宜（比如交换指针、窃取内部缓冲而非复制数据），因此重分配成本显著降低。
* 结果：把类型的移动构造/移动赋值声明为 `noexcept` 会让容器在扩容时更可能采用高效的移动而不是昂贵的拷贝，从而提升性能并避免不必要的内存/CPU 开销。

(注意：不同实现细节上可能有差异，但这是主流实现 —— 也可用 `std::is_nothrow_move_constructible<T>::value` 来检测库/编译器的决策依据。)

---

# 2) 例子（如何声明 + 检查）

```cpp
#include <type_traits>
#include <vector>
#include <iostream>

struct A {
    A() = default;
    A(const A&) { /* 可能昂贵 */ }            // copy ctor
    A(A&&) noexcept = default;                 // 标记为 noexcept
};

struct B {
    B() = default;
    B(const B&) { }
    B(B&&) /* not noexcept */ { /* 可能抛 */ } // 非 noexcept
};

int main() {
    std::cout << std::boolalpha;
    std::cout << "A nothrow move? "
              << std::is_nothrow_move_constructible<A>::value << '\n';
    std::cout << "B nothrow move? "
              << std::is_nothrow_move_constructible<B>::value << '\n';
}
```

当 `A` 是 `nothrow` 时，容器在重分配时更愿意（或能够）使用 `A` 的移动构造；若 `B` 的移动可能抛，则容器可能退回到拷贝或采取更保守的策略。

---

# 3) 如何安全地实现 `noexcept` 的移动构造/赋值（实践建议）

* **不要在移动构造/赋值里做可能抛出的操作**（例如分配内存、抛异常的库调用、抛的 I/O 操作等）。移动应尽量只是交换/窃取资源指针、把旧对象置为“有效的空状态”。
* 常见写法：

  * `MyType(MyType&&) noexcept = default;`（如果成员的移动也不抛）
  * 或者用条件 noexcept：`MyType(MyType&&) noexcept(noexcept(member_move(...)))`，适用于模板/组合成员。
* 如果移动会分配或可能抛（例如移动内部持有复杂资源时会触发分配），就不要硬标 `noexcept`，除非你能在移动中捕获并保证不向外抛出。

---

# 4) 如果在外层捕获异常，会不会影响原本的正常功能？

短答：**可能会，也可能不会，取决于被捕获操作的“异常安全性”保证。**

解释与要点：

* 捕获异常会阻止程序 `std::terminate()`，并让你有机会继续执行后续代码——这并不自动“回滚”被中断操作的副作用。
* **关键是：被调用函数在抛出异常前/抛出后对对象的状态保证是什么**。常见的异常安全等级：

  * **无异常保证（no guarantee）**：如果操作中抛出，相关对象可能处于破坏/未定义状态（危险）。
  * **基本保证（basic guarantee）**：即使抛出，程序的对象仍然处于合法但未指定的状态（不会崩溃，但语义可能不完整）。
  * **强保证（strong guarantee）**：要么操作完全成功，要么对程序状态没有任何改变（原子/事务式）。
  * **不抛出保证（nothrow guarantee）**：操作绝不抛异常。
* 举例：`std::vector::push_back` 在标准实现中通常给出**强或基本保证**（实现细节不同），但很多容器在 reallocation 时若元素复制/移动抛出，会保持原容器不变（strong）。如果操作确实提供强保证，那么你在外层捕获异常后，容器仍是原来状态，后续正常功能可以继续。
* 反过来，如果所调用的操作只提供基本保证（例如一个自定义类在部分步骤修改了内部状态而没有回滚），即使你在外层捕获异常，相关对象也可能进入某个合法但不期望的中间状态；这会影响后续逻辑，需要你显式恢复或重置状态。

因此：**外层捕获异常本身不会“修复”内部留下的副作用**——是否安全继续使用取决于那次操作所提供的异常安全语义。你需要依据该语义来决定是否可以继续，或者在 catch 中做补救（回滚、清理、重新初始化、记录错误、重试等）。

---

# 5) 实战建议（综合）

1. **尽量把移动构造/移动赋值写成 `noexcept`（当且仅当确实不会抛）**。这样能让容器利用高效的移动语义。
2. **对于库类型或关键数据结构，尽量实现强异常保证**（尤其是接口边界处），这样调用者在 catch 后更容易安全恢复。
3. **在 `noexcept` 的函数里不要调用可能抛的代码**；否则一旦抛出会 `terminate()`。
4. **在外层捕获异常时，依据被捕操作的异常安全级别决定处理策略**：如果只提供基本保证，考虑在 catch 中清理或重新初始化受影响对象。
5. 使用 `static_assert(std::is_nothrow_move_constructible<T>::value)` 在调试/测试时验证假设，特别是模板库中。

---

需要的话我可以：

* 给你一段完整的 demo，演示标 `noexcept` 与否在 `std::vector` 重分配时的性能/行为差异（包含 `is_nothrow_move_constructible` 检查），或者
* 帮你审查一类自定义类型的移动实现，告诉你能否安全地标 `noexcept` 并给出改写建议。


好 — 我给你两部分：一张**清晰的流程图（ASCII 版）**说明 `noexcept` 下异常的传播与 `std::terminate()` 的触发条件，随后给**两个可运行的 C++ 演示程序**：

* 演示 A：`noexcept` 函数抛出异常会导致 `terminate()`（以及在该函数内部捕获则不会）。
* 演示 B：演示标记 `noexcept` 的移动构造对 `std::vector` 重分配时是如何让容器选择 *move* 而非 *copy*（通过输出拷贝/移动构造被调用次数来观察差别）。

你可以先看流程图理解原理，再跑代码验证行为。

---

# `noexcept` 异常传播流程图（ASCII）

```
Caller
  |
  |  调用 noexcept 函数 f() noexcept
  v
+---------------------------+
|  f() noexcept {           |
|    ...                    |
|    throw SomeException;   |  <-- 抛出异常
|    ...                    |
|  }                        |
+---------------------------+
       |
       | 异常被 f() 内部捕获？ (try/catch 在 f 内)
       |-- Yes --> 异常被处理，返回/继续，调用者可继续正常执行
       |
       |-- No  --> 异常将向上传播，但 f() 被标记 noexcept，运行时发现“不允许传播”
                    |
                    v
               std::terminate() 被调用
                    |
                    v
               程序立即终止（默认调用 abort()，可由
               set_terminate 设定自定义行为）
```

要点总结：

* 如果 **在 `noexcept` 函数内部捕获并处理**了异常，**不会触发 `terminate()`**。
* **只有当异常逸出 `noexcept` 函数（即没有在该函数内被捕获）** 时，运行时会调用 `std::terminate()`（立即结束程序）。
* `noexcept` 的目的是对接口使用者作“不抛出”保证；若违反该保证，后果是终止程序（而不是正常的异常传播与捕获流程）。

---

# 演示 A：`noexcept` 抛出导致 `terminate()`（以及内部捕获不会）

将下列代码保存为 `noexcept_terminate_demo.cpp` 并编译运行。

```cpp
// noexcept_terminate_demo.cpp
#include <iostream>
#include <stdexcept>

void throws_in_noexcept() noexcept {
    std::cout << "[throws_in_noexcept] about to throw\n";
    throw std::runtime_error("error from noexcept");
    // unreachable
}

void catches_inside_noexcept() noexcept {
    std::cout << "[catches_inside_noexcept] about to throw then catch\n";
    try {
        throw std::runtime_error("inner error");
    } catch (const std::exception& e) {
        std::cout << "[catches_inside_noexcept] caught: " << e.what() << '\n';
    }
    std::cout << "[catches_inside_noexcept] continuing normally\n";
}

int main() {
    std::cout << "1) call catches_inside_noexcept (should handle internally)\n";
    catches_inside_noexcept();

    std::cout << "\n2) call throws_in_noexcept inside try/catch in main\n";
    try {
        throws_in_noexcept(); // this will cause terminate; won't reach catch
    } catch (const std::exception& e) {
        std::cout << "Caught in main: " << e.what() << '\n';
    }

    std::cout << "This line will NOT be printed if terminate() was called.\n";
    return 0;
}
```

编译并运行：

```
g++ -std=c++17 noexcept_terminate_demo.cpp -O0 -pthread -o demoA
./demoA
```

你会看到 `catches_inside_noexcept` 的输出并继续；但在调用 `throws_in_noexcept()` 时，程序不会进入 `main` 的 `catch` —— 而是直接 `terminate`（通常输出 `terminate called after throwing ...` 并退出）。

---

# 演示 B：观察 `noexcept` 移动构造对 `std::vector` 重分配策略的影响

下面示例定义两种类型 `ElemNoexceptMove`（移动构造 `noexcept`）和 `ElemMayThrowMove`（移动构造**非** `noexcept`），在大量 push\_back 导致重分配时统计拷贝与移动构造被调用的次数。

保存为 `vector_move_noexcept_demo.cpp`。

```cpp
// vector_move_noexcept_demo.cpp
#include <iostream>
#include <vector>
#include <string>
#include <type_traits>

struct ElemNoexceptMove {
    int id;
    static int copies, moves;
    ElemNoexceptMove(int i=0): id(i) {}
    ElemNoexceptMove(const ElemNoexceptMove& o) : id(o.id) { ++copies; }
    ElemNoexceptMove(ElemNoexceptMove&& o) noexcept : id(o.id) { ++moves; o.id = -1; }
};
int ElemNoexceptMove::copies = 0;
int ElemNoexceptMove::moves = 0;

struct ElemMayThrowMove {
    int id;
    static int copies, moves;
    ElemMayThrowMove(int i=0): id(i) {}
    ElemMayThrowMove(const ElemMayThrowMove& o) : id(o.id) { ++copies; }
    // move is NOT noexcept
    ElemMayThrowMove(ElemMayThrowMove&& o) /* not noexcept */ : id(o.id) { ++moves; o.id = -1; }
};
int ElemMayThrowMove::copies = 0;
int ElemMayThrowMove::moves = 0;

template<typename T>
void test_moves(const std::string& name) {
    T::copies = T::moves = 0;
    std::vector<T> v;
    v.reserve(1); // start small to force reallocations
    const int N = 10000; // push many to trigger multiple reallocations
    for (int i = 0; i < N; ++i) {
        v.emplace_back(i);
    }
    std::cout << name << " copies = " << T::copies << ", moves = " << T::moves << '\n';
}

int main() {
    static_assert(std::is_nothrow_move_constructible<ElemNoexceptMove>::value,
                  "ElemNoexceptMove should be nothrow move constructible");
    std::cout << "is_nothrow_move_constructible<ElemNoexceptMove> = "
              << std::is_nothrow_move_constructible<ElemNoexceptMove>::value << '\n';
    std::cout << "is_nothrow_move_constructible<ElemMayThrowMove> = "
              << std::is_nothrow_move_constructible<ElemMayThrowMove>::value << '\n';

    test_moves<ElemNoexceptMove>("ElemNoexceptMove");
    test_moves<ElemMayThrowMove>("ElemMayThrowMove");
    return 0;
}
```

编译并运行：

```
g++ -std=c++17 vector_move_noexcept_demo.cpp -O0 -pthread -o demoB
./demoB
```

预期观察（典型）：

* `ElemNoexceptMove` 的 `moves` 会远大于 `copies`（容器在扩容时使用移动构造）。
* `ElemMayThrowMove` 的 `copies` 会远大于 `moves`（容器为安全起见退回使用拷贝构造）。

注意事项：

* 不同的标准库实现（libstdc++, libc++）在策略上可能有细微不同，但主流实现会按 `std::is_nothrow_move_constructible<T>` 的结果来决定是否使用 move。
* 在极端优化或编译器优化下，一些构造调用可能被优化，但统计拷贝/移动的打印/计数通常能反映策略差异。用 `-O0` 更能直观观察构造调用次数。

---

# 总结（你应该带走的要点）

* `noexcept` 是对调用者的“不抛出”承诺：异常逸出将导致 `std::terminate()`。
* 在 `noexcept` 函数内部**捕获并处理**异常可避免 `terminate()`。
* 将类型的移动构造/赋值标为 `noexcept`（只在确实不会抛时）会让标准容器在重分配时选择高效的 *move*，显著降低开销。
* 在外层捕获异常不会自动“回滚”中间状态——是否能安全继续取决于被调用操作提供的异常安全级别（basic/strong/no-throw）。

---
