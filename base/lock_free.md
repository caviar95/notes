C++ 中的\*\*无锁队列（lock-free queue）\*\*常用于高性能场景，如日志系统、网络通信、任务调度、游戏引擎等，能极大减少线程间同步开销。以下是一些成熟、值得参考和使用的高质量开源项目：

---

## ✅ 1. **[Moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue)**（业界最常用）

### 📌 特点：

* 高性能无锁 MPMC 队列（multi-producer, multi-consumer）
* 支持线程本地缓存池、显式内存管理等
* 性能极佳，广泛用于日志、游戏引擎等

### ✅ 使用示例：

```cpp
moodycamel::ConcurrentQueue<int> queue;
queue.enqueue(42);
int item;
if (queue.try_dequeue(item)) {
    // use item
}
```

---

## ✅ 2. **[Facebook Folly: ProducerConsumerQueue / MPMCQueue](https://github.com/facebook/folly)**

### 📌 特点：

* Folly 是 Facebook 的高性能 C++ 库集合，包含多个 lock-free 队列实现。
* 支持：

  * SPSC（单生产者单消费者）
  * MPMC（多生产者多消费者）
* C++14/17 支持良好，性能非常强劲。

### ✅ 地址：

[https://github.com/facebook/folly](https://github.com/facebook/folly)

---

## ✅ 3. **[Intel TBB: concurrent\_bounded\_queue / concurrent\_queue](https://github.com/oneapi-src/oneTBB)**

### 📌 特点：

* Intel OneTBB（前身 Threading Building Blocks）
* `concurrent_queue` 是高性能线程安全队列（不是纯 lock-free，但并发非常优秀）
* `concurrent_bounded_queue` 支持容量控制，适合生产者消费者模型

### ✅ 地址：

[https://github.com/oneapi-src/oneTBB](https://github.com/oneapi-src/oneTBB)

---

## ✅ 4. **[boost::lockfree::queue](https://www.boost.org/doc/libs/release/doc/html/boost/lockfree/queue.html)**

### 📌 特点：

* Boost 官方库，C++ 标准兼容性极强
* 使用模板和静态分配（预分配 buffer）实现 lock-free 队列
* 支持 SPSC / MPMC 场景
* 实现基于 `boost::atomic` 或 `std::atomic`

### ✅ 使用示例：

```cpp
boost::lockfree::queue<int> q(128);
q.push(1);
int v;
q.pop(v);
```

---

## ✅ 5. **[libcds](https://github.com/khizmax/libcds)**（Concurrent Data Structures）

### 📌 特点：

* 包含几十种 lock-free/relaxed-lock 数据结构，包括：

  * Michael-Scott queue（MPMC）
  * SPSC/MPMC Ring Buffer
  * Hazard Pointer 支持
* 非常适合做 lock-free 系统结构研究

---

## ✅ 6. **[Disruptor C++](https://github.com/cloudwall/Disruptor-cpp)**

### 📌 特点：

* 移植自 LMAX Disruptor 模式（极低延迟）
* 适合高频交易、低延迟日志等场景
* 无锁环形队列 + 内存屏障 + sequence ID 机制

---

## 🧠 如何选择？

| 项目                   | 适合场景        | 是否 lock-free | 是否支持 MPMC | 难度   | 备注                 |
| -------------------- | ----------- | ------------ | --------- | ---- | ------------------ |
| Moodycamel           | 高性能通用场景     | ✅            | ✅         | ⭐⭐   | 推荐使用               |
| Boost Lockfree Queue | 依赖 Boost 项目 | ✅            | ✅         | ⭐    | 工程通用               |
| Facebook Folly       | FB 大型系统使用   | ✅            | ✅         | ⭐⭐⭐  | 比较复杂               |
| Intel TBB            | 并行库整合使用     | ❓部分lock-free | ✅         | ⭐⭐   | 工业级                |
| libcds               | 学术/中间件研究    | ✅            | ✅         | ⭐⭐⭐⭐ | 需了解 hazard pointer |
| Disruptor            | 低延迟高吞吐      | ✅（特殊机制）      | SPSC      | ⭐⭐⭐  | 非常高性能              |

---

## 🎯 推荐实践路径

* 初学者建议从 **Boost Lockfree** 或 **Moodycamel** 入手；
* 构建日志系统、线程池时可直接集成 `moodycamel::ConcurrentQueue`；
* 学术/中间件研究建议看 `libcds`，深入 Hazard Pointers/RCU；
* 高频交易/低延迟场景可研究 `Disruptor` 模式。

---

如果你希望我：

* 帮你构建一个基于某个库的 **无锁队列使用示例工程**（含 CMake）；
* 或者帮你手写一个 **简易版 lock-free queue（比如 SPSC）**；


