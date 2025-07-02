在 C++ 项目中，如果要调用那些由宏（`#define`）暴露的外部接口，又希望对它们进行单元测试（UT）时能够方便地 stub/mock，就需要在“不改宏定义”的前提下，为这些宏调用“留一个替身”或“插入一层适配／抽象”。下面给出几种常见方案，并对它们的优缺点进行对比。

---

## 方法一：**函数封装（Wrapper Function）**

### 思路

把宏调用包裹到一个正常的 C++ 函数（通常写成 `inline` 或 `static`），所有应用代码都改为调用这个封装函数。

```cpp
// 原始宏（来自外部头文件，不能改）
#define EXTERNAL_SEND(data)    ext_send(data)

// 新增 wrapper.h
inline int ExternalSend(const Data& d) {
    return EXTERNAL_SEND(d);
}

// 应用代码只调用 ExternalSend()
```

### UT 实现

在测试中，使用链接替换（link seam）或在测试专用的 translation unit 中，提供同名 `ExternalSend` 的替身实现（mock），并确保测试时链接到这个替身即可。

### 优点

* **类型安全**：函数比宏更有类型检查。
* **可定位 stub**：只需 mock 一处函数接口即可。
* **维护成本低**：应用层代码改动量小（一次性替换调用）。

### 缺点

* **额外一层间接**：可能有微乎其微的性能损失（通常可忽略）。
* **需要集中替换**：所有原来直接调用宏的地方都必须改为调用 wrapper。

---

## 方法二：**抽象接口＋依赖注入（Adapter + DI）**

### 思路

定义一个纯虚基类接口，把所有宏调用封装为接口方法；在生产代码中提供具体实现，在测试时注入 Mock 对象。

```cpp
struct IExternal {
    virtual ~IExternal() = default;
    virtual int send(const Data&) = 0;
};

class ExternalImpl : public IExternal {
public:
    int send(const Data& d) override {
        return EXTERNAL_SEND(d);
    }
};

// 应用层通过 IExternal& 注入
void foo(IExternal& ext) {
    ext.send(...);
}
```

### UT 实现

测试时注入继承自 `IExternal` 的 Mock（可用 GoogleMock 等），完全隔离真实宏。

### 优点

* **高内聚低耦合**：遵循依赖倒置原则，业务代码与具体实现解耦。
* **灵活可扩展**：不仅可 mock，还能替换成其它实现（如日志版、缓存版等）。
* **可结合容器/工厂管理**：集成更复杂的依赖管理。

### 缺点

* **初期工作量大**：要设计接口、管理生命周期（所有需要注入的位置都要改）。
* **运行时开销**：虚函数调用相比 inline 函数稍有性能损失（通常无关紧要）。

---

## 方法三：**宏重定义（Preprocessor Redefinition）**

### 思路

在测试编译单元里，先 `#undef` 原宏，再 `#define` 成调用 stub 函数或直接返回固定值。

```cpp
// test.cpp
#undef EXTERNAL_SEND
#define EXTERNAL_SEND(data)    TestStub_send(data)
```

### UT 实现

只要在测试文件中包含对宏的重定义，就能让原来所有直接调用该宏的代码在测试时走 stub。

### 优点

* **无需改业务代码**：生产代码不用动，测试时局部生效。
* **实现简单**：只需一两行预处理指令。

### 缺点

* **易错易污染**：宏范围全局，可能误改不想改的地方；且难以保证 `#undef`／`#define` 的顺序。
* **可读性差**：预处理逻辑隐蔽，不易追踪。
* **宏本身风险**：宏重定义可能带来难以预料的副作用。

---

## 方法四：**模板策略（Policy-based Design）**

### 思路

把外部接口调用封装到一个模板策略类，业务代码通过模板参数“注入”具体策略；测试时传入 stub 策略。

```cpp
template <typename Policy>
struct ExternalCaller {
    static int send(const Data& d) {
        return Policy::send(d);
    }
};

// 生产策略
struct RealPolicy {
    static int send(const Data& d) { return EXTERNAL_SEND(d); }
};

// 测试策略
struct MockPolicy {
    static int send(const Data& d) { return /* stub */; }
};

// 应用
using MyCaller = ExternalCaller<RealPolicy>;
```

### UT 实现

在测试时，用 `ExternalCaller<MockPolicy>` 代替 `ExternalCaller<RealPolicy>` 即可。

### 优点

* **零运行时开销**：全部在编译期决策，可内联优化。
* **编译时灵活**：无需虚函数，类型安全。

### 缺点

* **调用点需为模板上下文**：如果业务代码已经写死，改动较大。
* **对非模板代码支撑差**：需要将业务入口改为模板或模板参数传入。

---

## 方法五：**链接层隔离（Linker Interposition）**

### 思路

利用链接器特性（在 Linux 下用 `--wrap`，或在 Windows 下用导入库重定向），在链接阶段把对某符号的引用重定向到测试版符号。

### UT 实现

编译测试版时，加入链接选项：

```
g++ -Wl,--wrap=ext_send test.o ... -o test
```

此时对 `ext_send` 的所有调用会跳转到你自定义的 `__wrap_ext_send`。

### 优点

* **不改源代码**：业务代码无需改也能重定向所有调用。
* **强大**：对 `.o` 级别调用全局生效。

### 缺点

* **平台依赖**：不同编译器/链接器实现差异大，不够可移植。
* **调试复杂**：栈回溯时符号名被 “wrap” 前缀弄乱，不直观。

---

## 方法对比小结

| 方法                 | 对业务代码改动 | 类型安全 | 可读性 | 测试隔离度 | 性能开销   | 适用场景           |
| ------------------ | ------- | ---- | --- | ----- | ------ | -------------- |
| 函数封装（Wrapper）      | 中等      | 高    | 较好  | 较高    | 低      | 既想少改调用，又想高类型安全 |
| 接口＋依赖注入（DI）        | 较大      | 高    | 很好  | 最高    | 中      | 需要长远维护／复杂依赖管理  |
| 宏重定义（Preprocessor） | 无       | 低    | 差   | 中     | 无      | 只为快速做少量测试隔离    |
| 模板策略（Policy）       | 中       | 极高   | 较好  | 高     | 0      | 性能敏感、偏静态、愿意改模板 |
| 链接层隔离（Linker）      | 无       | 中    | 差   | 高     | 无（可忽略） | 对链接阶段有控制、不改源码  |

---

### 推荐实践

1. **新项目**：优先选用「接口＋依赖注入」或「模板策略」，代码可维护性最高。
2. **已有项目**：若业务调用宏非常分散、难以一次性改完，可先用「函数封装」平滑过渡，再逐步引入 DI。
3. **临时／一次性测试**：可以用「宏重定义」或「链接隔离」快速出结果，但不建议长期保留。

通过以上多种方法的比较和取舍，你可以根据团队的规模、项目的性质和目标测试覆盖度，选择最合适的方案。

既然“原有宏的调用”都不能改动，我们就要在编译／链接／运行时“悄悄”把对这些宏展开后实际调用的符号（如 `ext_send`）导向我们的测试替身。下面给出四种常见做法，并分析它们的优劣。

---

## 方法一：测试单元里宏重定义（Preprocessor Redefinition）

### 实现

在测试源码（或测试专用的头）中，先把原宏 `#undef`，再 `#define` 成调用你自己实现的 stub：

```cpp
// test_stub.h（只在测试编译里包含）
#undef EXTERNAL_SEND
#define EXTERNAL_SEND(data)    TestStub_send(data)

// stub 实现
int TestStub_send(const Data& d) {
    // 记录参数、返回可控值等
    return 42;
}
```

#### 优点

* **零侵入主代码**：生产代码里所有 `EXTERNAL_SEND(x)` 不用动。
* **实现简单**：只需几行预处理指令。

#### 缺点

* **易污染**：宏重定义可能意外影响其它测试／模块，且依赖包含顺序。
* **可读性差**：真正调用哪个实现不直观，维护困难。

---

## 方法二：链接器层 Wrap（Link‑time Function Wrapping）

### 实现

利用 GNU LD 的 `--wrap` 选项（或类似功能）把对底层符号的所有调用重定向：

```bash
# 编译测试时加：
g++ test.o foo.o … -Wl,--wrap=ext_send -o test
```

此时，任何对 `ext_send` 的引用都会跳到你定义的 `__wrap_ext_send`，而在需要调用真实时再用 `__real_ext_send`。

```cpp
// 在测试链接时提供：
extern "C" int __real_ext_send(const Data&);
extern "C" int __wrap_ext_send(const Data& d) {
    // 可以记录、模拟返回、或条件调用 __real_ext_send(d)
    return /* stub */;
}
```

#### 优点

* **不改源代码**：完全在链接层面接管。
* **范围可控**：只影响加入 `--wrap` 的测试可执行。

#### 缺点

* **平台依赖**：非 GNU 链接器或 Windows/MSVC 下难以复用。
* **调试不便**：符号被 `__wrap_`／`__real_` 隐藏，阅读调用栈不直观。

---

## 方法三：运行时符号替换（LD\_PRELOAD／DLL 注入）

### 实现

把目标函数（如 `ext_send`）所在的库编译成可被替换的动态库（或直接在测试时构造一个替身库），并通过环境变量让它“优先加载”：

```bash
# 先编译一个 libstub.so，其中定义：
extern "C" int ext_send(const Data& d) {
    return /* stub */;
}

# 运行测试时：
LD_PRELOAD=./libstub.so ./test
```

#### 优点

* **零改动**：既不用改宏，也不用改链接规则（除了 preload）。
* **可测试第三方库**：只要符号名匹配，就能拦截。

#### 缺点

* **仅限动态链接**：静态链接时无效。
* **平台差异**：Windows 下用 `DLL` 注入或 `Detours` 等更复杂。
* **执行环境依赖**：需要通过环境变量或特殊启动脚本才能生效。

---

## 方法四：弱符号／强符号覆盖（Weak‑Symbol Override）

### 实现

在编译主程序时，把原 `ext_send` 定义为弱符号（`__attribute__((weak))` 或在链接脚本中标记），然后在测试目标中提供一个同名的“强符号”替身：

```cpp
// 在外部库里（或在头里）标记弱符号
extern "C" int ext_send(const Data&) __attribute__((weak));

// 在测试用例中提供强符号
extern "C" int ext_send(const Data& d) {
    return /* stub */;
}
```

链接时，强符号会优先绑定。

#### 优点

* **不改宏调用**：调用点不动。
* **不需要链接器 wrap**：比 `--wrap` 更直接、跨平台性稍好（只要支持弱／强符号）。

#### 缺点

* **需重编译库／可执行**：要把原实现标注为弱符号。
* **符号冲突风险**：如果多个翻译单元都定义，需要注意 ODR。

---

## 小结对比

| 方法                  | 侵入度   | 可控范围            | 平台依赖      | 调试友好度 | 动态／静态 |
| ------------------- | ----- | --------------- | --------- | ----- | ----- |
| 宏重定义                | 零     | 宏展开所在翻译单元       | 低         | 差     | 任意    |
| 链接器 Wrap (`--wrap`) | 零     | 加 `--wrap` 的可执行 | 高（仅 GNU）  | 较差    | 任意    |
| LD\_PRELOAD／DLL 注入  | 零     | 环境变量控制          | 中（类 Unix） | 中     | 动态    |
| 弱符号覆盖               | 需改库标记 | 全链接范围           | 低–中       | 较好    | 任意    |

* **最快速试验**：用「宏重定义」或「LD\_PRELOAD」，成本最低，适合临时／探索性测试。
* **更稳定可维护**：若能改库或构建流程，推荐「弱符号覆盖」或「链接器 wrap」，不改业务调用也能长期使用。

任选其一，即可在「不动任何原有宏调用」的前提下，为单元测试提供你所需的 stub/mock 支持。


在现代 x86\_64/Linux 下，静态链接（static linking）和动态链接（dynamic linking）在**启动时间**和**函数调用开销**上会有一些差异，但对于大多数“实际业务”场景，这些差异都非常小。下面给出一些典型的量化数据和分析。

---

## 1. 启动时间开销

| 链接方式 | 可执行文件体积   | 首次启动（cold start）时间 |
| ---- | --------- | ------------------ |
| 静态链接 | 较大（几 MB）  | 较快（约 2–5 ms）       |
| 动态链接 | 较小（几十 KB） | 略慢（约 5–15 ms）      |

* **原因**：

  * 动态链接的可执行文件在首次加载时，动态链接器（ld‑linux.so）要对所有未定义符号做重定位（relocation），并初始化 GOT/PLT 表。
  * 静态链接在编译时已完成所有符号绑定，加载时操作更少。

> **量化示例**
> 一份简单的 Hello World 程序，
>
> * **静态**编译后体积 \~1.8 MB，启动约 3 ms；
> * **动态**编译后体积 \~100 KB，启动约 8 ms。
>   （具体数字会因硬盘类型、缓存状态等有所波动，但同类机器上通常差异在 **5–10 ms** 量级。）

---

## 2. 运行时函数调用开销

### 直接函数调用（static / 内部函数）

```cpp
inline int foo(int x) { return x + 1; }
int main() { return foo(42); }
```

* 直接编译后会被内联或直接调用，机器码中就是一条 `add` 指令，开销 **1–2 个时钟周期**。

### PLT／GOT 间接调用（dynamic）

```cpp
// foo 来自共享库
extern "C" int foo(int);
int main() { return foo(42); }
```

* 第一次调用 foo 时，通过 PLT 跳转到动态链接器，完成符号解析；
* 随后调用则走到已经被“写死”在 GOT 表中的地址。
* 每次调用都要经过一次内存加载＋一次间接跳转，开销约 **4–6 个时钟周期**。

| 调用类型        | 典型开销         |
| ----------- | ------------ |
| 静态／内联调用     | \~1–2 cycles |
| 动态（PLT→GOT） | \~4–6 cycles |

> **结论**：
> 动态链接带来的额外开销大约 **3–4 cycles**（约 1 ns）左右。
> 即使一个函数被调用 1 亿次，总额外成本也只有 **0.1 s** 量级。

---

## 3. 整体性能影响

1. **CPU‑密集型代码**

   * 多数时间都耗在算法运算上，PLT 调用的微小差异（≈1 ns／次）可以忽略不计。

2. **IO‑密集型或启动型应用**

   * 如果非常看重首次启动延迟（如微服务 cold start、CLI 工具启动体验），动态链接可能多耗 **数毫秒**。
   * 对于大型可执行文件（成百上千个符号重定位），动态链接启动时间差异可达 **20–50 ms**。

3. **内存占用**

   * 动态链接可以共享同一份库代码到多个进程，**节省物理内存**；静态链接则每个进程都拷贝一份，反而在多进程场景下更耗内存。

---

## 4. 综合建议

* **一般场景**：对于服务器后端、算法库、绝大部分业务服务，二者性能差异可忽略，优先选动态链接以节省磁盘和内存。
* **极限性能／超低延迟**：若对“每次调用”或“每次进程启动”都在纳秒／毫秒级严格苛求，可考虑静态链接＋函数内联＋内存布局优化。
* **多进程／容器场景**：动态链接因共享库页性能更优，推荐优先使用。

---

**总结**：

* **启动时间**：差异在 **5–15 ms**；
* **函数调用**：额外 **3–4 个 CPU 周期**（≈1 ns）。
  对于绝大多数 C++ 应用，这些开销远小于算法和 IO 花费，对业务性能几乎没有影响。

当原有宏不仅传入业务参数，还借助 `__FILE__`/`__LINE__`/`__func__`（或 `__FUNCTION__`）自动注入调用上下文时，我们依旧要「不改动宏调用」地把它们导向测试替身。核心思想是：模拟或拦截那个实际被宏展开成的底层函数（或符号），并让你的 Stub／Mock 实现拥有完全相同的签名（含文件名、行号、函数名参数）。下面基于前面四种“零改调用点”方法，说明如何适配这三项额外信息。

---

## 方法一：测试单元里宏重定义

如果原宏是：

```cpp
// external.h（不可改）
#define EXTERNAL_SEND(data) \
    ext_send(data, __FILE__, __LINE__, __func__)
```

那么在测试专用头里，把它重定义为调用你的 Stub：

```cpp
// test_stub.h（仅在测试编译里包含）
#undef EXTERNAL_SEND
#define EXTERNAL_SEND(data) \
    TestStub_send(data, __FILE__, __LINE__, __func__)

// Stub 实现，签名必须与原 ext_send 匹配
int TestStub_send(const Data& d,
                  const char* file,
                  int line,
                  const char* func)
{
    // 你可以记录这些上下文：
    record_call(file, line, func, d);
    return /* 可控返回值 */;
}
```

> **要点**：
>
> * `TestStub_send` 的参数顺序与原 `ext_send` 保持一致；
> * 所有生产代码里的宏调用不变，仍会隐式传入 `__FILE__`/`__LINE__`/`__func__`。

---

## 方法二：链接器 Wrap (`--wrap`)

原来宏展开后的符号是 `ext_send(Data, const char*, int, const char*)`。借助 GNU ld：

1. **编写 Wrap 函数**

   ```cpp
   // test_wrap.cpp
   extern "C" int __real_ext_send(const Data&,
                                  const char*,
                                  int,
                                  const char*);
   extern "C" int __wrap_ext_send(const Data& d,
                                  const char* file,
                                  int line,
                                  const char* func)
   {
       // 这里拿到 file/line/func
       record_call(file, line, func, d);
       // 如需调用真实实现：
       // return __real_ext_send(d, file, line, func);
       return /* mock 值 */;
   }
   ```
2. **编译并链接测试时加入**

   ```bash
   g++ test_wrap.o foo.o ... \
       -Wl,--wrap=ext_send \
       -o test
   ```

> **要点**：
>
> * `--wrap=ext_send` 会让所有对 `ext_send` 的调用转到 `__wrap_ext_send`；
> * `__wrap_ext_send` 拿到所有四个参数，完全透明。

---

## 方法三：运行时符号替换（`LD_PRELOAD`）

假设生产代码链接时，`ext_send` 从某动态库导出。你只需提供同名的替身库：

```cpp
// libstub.cpp
extern "C" int ext_send(const Data& d,
                        const char* file,
                        int line,
                        const char* func)
{
    record_call(file, line, func, d);
    return /* stub */;
}
```

编译成 `libstub.so`，然后：

```bash
LD_PRELOAD=./libstub.so ./test
```

> **要点**：
>
> * 动态链接下，加载时 `ext_send` 会被你的 `libstub.so` 覆盖；
> * 参数 `file/line/func` 完整传递。

---

## 方法四：弱符号／强符号覆盖

如果你能在原始库里把 `ext_send` 定义为弱符号：

```cpp
// in original library
extern "C"
int ext_send(const Data&, const char*, int, const char*) __attribute__((weak));
```

然后在测试模块中提供一个“强符号”：

```cpp
// test_override.cpp
extern "C"
int ext_send(const Data& d,
             const char* file,
             int line,
             const char* func)
{
    record_call(file, line, func, d);
    return /* mock */;
}
```

链接时，测试目标中的强符号优先绑定。

> **要点**：
>
> * 无需 `--wrap`、无须改宏；
> * 你的覆盖函数会拿到文件、行号、函数名。

---

### 小结

无论是哪种方式，适配带 `__FILE__`/`__LINE__`/`__func__` 的宏，只要你：

1. **让 Stub／Mock 的函数签名** 完全匹配宏展开后的真实符号；
2. **在测试范围内**，以「重定义」「链接拦截」「弱／强符号覆盖」「预加载」等手段，把对符号的调用劫持到你的替身实现；

就能在不改任何业务调用的前提下，拿到调用发生的源文件、行号、函数名，并对其进行记录或断言。选择哪一种方法，取决于你的构建环境、平台兼容性要求，以及对测试隔离和维护成本的偏好。

是的，你理解得很对：
**链接器层隔离（`--wrap`）的原理，本质上就是一种“打补丁”（patch）机制**，它在**链接阶段**对函数符号进行“替换”和“劫持”，从而实现对原有行为的注入、修改或完全绕过。

---

## 🔧 `--wrap=foo` 是怎么打补丁的？

当你在链接时加入：

```bash
-Wl,--wrap=foo
```

链接器（如 `ld.bfd` 或 `ld.gold`）会将所有对 `foo` 的调用：

```
call foo
```

替换成对你提供的：

```
call __wrap_foo
```

同时，你如果想在 `__wrap_foo` 里**间接调用原函数**，可以调用：

```cpp
__real_foo(...)
```

这其实是将程序中对原始 `foo` 的调用“重定向”到了 `__wrap_foo`，然后你是否调用 `__real_foo` 由你决定。

---

## ✅ 类似“打补丁”的特征：

| 特征                     | `--wrap` 是否符合                   |
| ---------------------- | ------------------------------- |
| **在原函数前插入逻辑**          | ✅ 是，`__wrap_foo` 可加记录、验证逻辑等     |
| **控制是否调用原函数**          | ✅ 是，可决定是否调用 `__real_foo`        |
| **无需改动原代码或调用点**        | ✅ 是，调用点仍然是 `foo()`，只是被重定向       |
| **作用范围可控（只测试目标）**      | ✅ 是，影响范围仅限加了 `--wrap` 的可执行或测试目标 |
| **底层靠符号操作（符号级 patch）** | ✅ 本质是符号表级的符号“打补丁”               |

---

## 🧠 小结

> `--wrap` 是一种 **链接器层级的“函数打补丁”机制**，就像热修复或 hook 一样插入你自己的实现，控制是否继续调用原函数。
> 你可以理解为是一种“**编译期的函数 hook**”，比运行时 patch（如 `LD_PRELOAD` 或 Windows 的 Detours）更早、更稳、更可控，但需要你掌握链接器的使用。

---

如果你对“打补丁”的理解是：“**不改原逻辑代码，但改它的执行路径**”，那么 `--wrap` 就是**精确匹配这个定义的手段**。

是的，除了 `--wrap` 这种 **链接时符号替换（Link-time patch）**，还有不少“打补丁”机制，分布在 **编译期、链接期、加载期和运行期** 不同阶段。它们的核心目标都是一样的：

> **不改调用代码的前提下，插入、替换或拦截已有接口的行为**。

下面我按照“打补丁的生效时机”分类，并分别说明原理、实现方式和使用场景。

---

## ✅ 一、**链接时打补丁**

### 1. `--wrap`（符号重定向）✔️

详见前述，是最稳定、静态、可控的补丁方式。

---

## ✅ 二、**加载时打补丁（Load-time patch）**

### 2. `LD_PRELOAD`（动态库注入）

* **平台**：Linux/Unix
* **机制**：通过环境变量 `LD_PRELOAD` 提前加载一个动态库（.so），只要其中包含目标符号（如 `foo()`），系统加载器就优先使用这个版本。

```bash
LD_PRELOAD=./liboverride.so ./app
```

```cpp
extern "C" int foo(int x) {
    return 42;  // 替代真实 foo
}
```

* **用途**：测试、监控、日志注入、模拟网络/文件等。
* **缺点**：仅对动态链接有效，不能覆盖静态链接。

---

### 3. Windows `DLL` 注入（如 Detours、EasyHook）

* **平台**：Windows
* **机制**：将自定义的 DLL 注入到目标进程地址空间，然后通过修改 IAT（导入地址表）或代码 patch 替换函数指针。

```cpp
// 原来的地址: MessageBoxA -> user32.dll
// 注入后的地址: MessageBoxA -> my_hook.dll
```

* **用途**：调试、反作弊、日志系统、API 模拟。
* **代表工具**：Microsoft Detours、EasyHook。

---

## ✅ 三、**运行时打补丁（Runtime patch）**

### 4. Hook 框架（如 PLT/GOT 重定向）

* **原理**：在运行时，通过覆盖 GOT 表中的地址，将原函数指针替换为你自己的实现。

```cpp
// GOT[foo] = &foo_impl;  -->  替换为 GOT[foo] = &my_foo
```

* **方式**：使用 `dlsym(RTLD_NEXT, "foo")` 获取原始实现，然后自己写 hook 函数再调原函数。

```cpp
int foo(...) {
    static auto real_foo = (decltype(&foo)) dlsym(RTLD_NEXT, "foo");
    ... // hook logic
    return real_foo(...);
}
```

* **用途**：插件框架、调试工具（如 `libhook`、`libtrace`）、性能分析工具（如 `perf`、`gprof`）。

---

### 5. `LD_AUDIT`（符号解析审计）

* **平台**：Linux
* **机制**：LD audit 接口允许你“监听”所有符号解析和库加载过程，可实现按需替换符号。
* **使用**：需要实现一个 audit 库，并设置环境变量 `LD_AUDIT=./libaudit.so`。

```cpp
la_symbind64(...)  // 回调函数，在绑定符号时调用
```

* **优点**：比 `LD_PRELOAD` 更底层、更精细。
* **用途**：调试、检测 ABI 使用、性能优化分析。

---

### 6. 运行时代码 patch（Inline Patch / Trampoline）

* **原理**：直接修改目标函数开头机器码，比如前几个字节替换成跳转指令（`jmp`）跳到你的实现上。

```asm
foo:
    jmp my_hook
```

* **工具**：Linux 下 `mprotect + memcpy`，Windows 下使用 `VirtualProtect`。

* **用途**：游戏 Mod、反作弊工具、热补丁系统（如 Android 热更新）、高级性能监控工具。

* **风险**：容易踩坏栈、破坏 ABI，不安全；需严格控制指令对齐和平台兼容性。

---

## ✅ 四、**编译期打补丁（Compile-time patch）**

### 7. 宏封装 / 包裹函数（静态代理）

```cpp
#define LOG_CALL(x) log_call(__FILE__, __LINE__, __func__, x)
```

* 只能在编译时插入，适合你控制所有调用点。
* 属于“源代码级别打补丁”，不是“黑盒”。

---

### 8. LLVM 插桩 / 静态分析插桩（编译器级 patch）

* **工具**：Clang 插件、LLVM Pass、Sanitizer
* **用途**：插入运行时检查（UBSan、ASan、TSan）、插入追踪点、变换函数调用。

```cpp
void foo() {
    __sanitizer_cov_trace_pc();  // 自动插入
}
```

* **典型框架**：Clang 插件、Sanitizer、LLVM IR 修改工具链。

---

## ✅ 五、**平台级热补丁机制**

### 9. Kernel Patch（如 kpatch、livepatch）

* Linux 下对内核函数直接替换：

```bash
kpatch load update.patch
```

* 通过“函数替换 + 内核跳转机制”在线替换某些内核模块实现。

---

### 10. Android / Java 热更新（如 AndFix、Tinker）

* 替换 Dex 文件、插入补丁类，或替换方法元信息（ART VM hook）。

---

## 🧠 总结：不同“打补丁”机制一览

| 类型   | 技术               | 生效时机  | 控制粒度 | 安全性 | 跨平台性        | 适用场景        |
| ---- | ---------------- | ----- | ---- | --- | ----------- | ----------- |
| 链接期  | `--wrap`         | 链接时   | 精确   | 高   | 中（GNU only） | 单元测试、Mock替换 |
| 加载期  | `LD_PRELOAD`     | 加载动态库 | 中    | 高   | Unix-only   | 外部库替换、调试    |
| 运行期  | GOT/PLT hook     | 运行中   | 精确   | 中   | 中           | 插件、分析工具     |
| 运行期  | Inline Patch     | 运行中   | 指令级  | 低   | 差           | 游戏 Mod、调试工具 |
| 编译期  | 宏、LLVM 插桩        | 编译时   | 静态   | 高   | 高           | 日志、覆盖率分析    |
| 内核级别 | kpatch、livepatch | 内核运行中 | 函数级  | 高   | Linux-only  | 在线修复内核      |

---

## ✅ 推荐（针对你场景）

* **C++ 用户态测试/替换**，又不能改代码、要拿到 `__FILE__` 等信息：

  * ✔️ `--wrap`（最佳，控制力强）
  * ✔️ `LD_PRELOAD`（次选，限制较少）
  * ✔️ GOT hook / `dlsym(RTLD_NEXT)`（精细 hook）

---

如果你愿意，我可以继续为你演示这些方法的实际 C++ 代码示例，帮助你选择最佳方式在项目中落地。
