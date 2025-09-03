# ver1

# C++ 掩码状态机（Mask-based State Machine）汇报

---

# 封面

**C++ 掩码状态机（Mask-based State Machine）**

* 报告人：
* 日期：
* 场景：技术分享 / 设计评审

> 备注：本 PPT 演示掩码状态机的背景、优势、实现思路与示例代码，并给出工程化建议。

---

# 目录

1. 背景与动机
2. 经典有限状态机回顾
3. 掩码状态机概念
4. 优势与适用场景
5. 核心数据结构与 API 设计
6. 实现思路与流程
7. C++ 示例实现（关键片段）
8. 性能优化与扩展
9. 测试、调试与注意事项
10. 总结与下一步建议

---

# 背景与动机

* 在复杂系统中，状态往往是多个子系统或多个属性的组合（并行状态），而不是单一离散值。
* 传统 FSM（单一枚举）在表达并行/位组合时会导致状态爆炸（状态数呈指数级）。
* 希望：用位掩码表示多个维度的状态，方便表达组合、快速检测条件、维护更少的状态定义。

**实际场景举例**：

* 网络协议解析（各标志位同时存在）
* 设备驱动（多个子模块状态并行）
* 工作流或审批引擎（多条件同时成立触发转换）

---

# 经典有限状态机回顾

* 状态表示：单一枚举（State）
* 事件驱动：在某个状态下接收事件后转到下一个状态
* 缺点：当存在并行子状态或大量组合时，状态数量膨胀；条件判断复杂，维护困难

**示意图**：

* 状态A --(e1)--> 状态B
* 状态B --(e2)--> 状态C

---

# 掩码状态机（Mask-based FSM）概念

* 使用\*\*位掩码（bitmask）\*\*表示状态：每一位代表一个独立子状态/属性。
* 状态机规则以掩码条件匹配（required\_mask、forbidden\_mask、changed\_bits 等）为单位来定义是否触发转换或完成。
* 通过位运算（AND/OR/XOR）实现高效匹配与切换。

**基本思想**：以布尔组合取代枚举组合，减少状态定义数量，提升匹配效率。

---

# 掩码状态机的优势

1. **表达力强**：自然表达并行状态与多属性组合。
2. **状态数减少**：避免列举所有组合状态，易维护。
3. **匹配高效**：位运算快（O(1)），适合高并发/高频事件路径。
4. **灵活规则**：支持按位要求/禁止/切换等多种匹配逻辑。
5. **易扩展**：新增子状态只需分配新位，不重构原有状态表。

**劣势与权衡**：

* 位数有限制（64位或128位可扩展）；
* 可读性：位含义需良好文档与枚举映射；
* 过度使用可能隐藏状态机的语义（需设计良好 API）。

---

# 适用场景（快速列举）

* 协议解析：多个标志位影响解析器决策
* 硬件/驱动：多个寄存器或模块的就绪/出错/断电等状态组合
* 复杂条件流控：按多个条件同时满足触发任务
* 大规模并发系统：用位向量做快速筛选、路由

---

# 核心数据结构与 API 设计（概览）

**基础定义**：

* `using Mask = uint64_t;`（或自定义宽度的位集）
* `enum class Flag : int { Ready=0, Error=1, Busy=2, ... }`
* `constexpr Mask flag_to_mask(Flag f) { return Mask(1) << static_cast<int>(f); }`

**状态注册与规则**：

* `register_state(name, mask)`
* `on_condition(required_mask, forbidden_mask, callback)` 或 `add_transition(from_mask, event_mask, to_mask)`

**运行时接口**：

* `update_mask(delta_mask)` — 修改当前位
* `set_mask(mask)` / `reset_mask(mask)`
* `evaluate()` — 基于当前 mask 触发匹配规则

---

# 实现思路（高层流程）

1. **位定义与映射**：通过 `enum` 定义位，提供到位掩码的 `constexpr` 映射。
2. **规则表达**：每条规则至少包含：required\_mask、forbidden\_mask、action（回调）和优先级。
3. **事件驱动**：当状态位发生变化时，触发匹配器去扫描规则表。
4. **匹配策略**：按优先级或按最严格匹配（required 位数多的先试）执行，支持一次触发多条规则或互斥规则。
5. **优化索引**：为常用 final 状态/规则建立倒排索引（按位或按子系统），减少线性扫描。
6. **线程与同步**：并发场景下，使用原子位操作 + 规则锁或无锁队列保证一致性。

---

# 简单 C++ 示例（关键片段）

```cpp
// 基础类型与工具
using Mask = uint64_t;
enum class Flag : int { Ready=0, Busy=1, Error=2, Configured=3 };
constexpr Mask flag_mask(Flag f) { return Mask(1) << static_cast<int>(f); }

struct Rule {
    Mask required;    // 必须全为1
    Mask forbidden;   // 必须全为0
    std::function<void(Mask)> action;
    int priority;
};

class MaskStateMachine {
public:
    void add_rule(Rule r) { rules_.push_back(std::move(r)); }

    // 原子更新位，并触发 evaluate
    void set_bits(Mask m) {
        mask_.fetch_or(m, std::memory_order_acq_rel);
        evaluate();
    }
    void reset_bits(Mask m) {
        mask_.fetch_and(~m, std::memory_order_acq_rel);
        evaluate();
    }

private:
    void evaluate() {
        Mask cur = mask_.load(std::memory_order_acquire);
        // 简单线性匹配，按优先级执行
        std::vector<Rule*> to_run;
        for (auto &r : rules_) {
            if ((cur & r.required) == r.required && (cur & r.forbidden) == 0) {
                to_run.push_back(const_cast<Rule*>(&r));
            }
        }
        std::sort(to_run.begin(), to_run.end(), [](Rule* a, Rule* b){ return a->priority > b->priority; });
        for (auto *rp : to_run) rp->action(cur);
    }

    std::atomic<Mask> mask_{0};
    std::vector<Rule> rules_;
};
```

> 备注：真实工程中应避免在 evaluate 内直接执行长时回调（可放入任务队列），并对规则表做索引优化。

---

# 优化思路（工程级）

1. **倒排索引（Inverted Index）**：对规则按关键位建立索引，改变时只查相关规则集合。
2. **位分组（subsystem）**：将位划分为子系统，维持子系统级别的规则集合。
3. **优先级与速率限制**：避免连续触发相同动作，支持去重与节流。
4. **宽位支持**：需要超过 64 位时使用 `std::bitset<N>` 或自定义位向量（或兼容 SIMD/AVX）。
5. **无锁/原子操作**：尽量用原子位操作和轻量级同步，减少锁竞争。
6. **可视化与日志**：打印位名与变化集，便于定位问题。

---

# 测试与调试策略

* **单元测试**：对每条规则的匹配逻辑做覆盖测试；模拟并发更新验证一致性。
* **集成测试**：在真实场景下验证规则触发顺序与期望一致。
* **断言与 invariant**：关键路径插入断言（例如某些互斥位不能同时为 1）。
* **Trace 与回放**：记录状态变化序列，支持回放来重现问题。

---

# 常见陷阱与最佳实践

* **滥用位逻辑**：不要把所有语义都压缩为位，保留清晰的高层语义。
* **位定义文档化**：每个位必须有名字、含义、所属子系统、是否可并行的说明。
* **回调时延管理**：回调不要在 evaluate 中做耗时操作，改为异步任务。
* **版本兼容**：新增位时要保证旧规则兼容，或提供迁移策略。

---

# 真实案例（短）

* **网络应用**：某网关用掩码表示连接的多个属性（TLS已就绪、认证已完成、客户端活跃、限速中），在同时满足 "TLS && Auth && Active" 时把会话送往后端池。
* **设备启动流程**：多个硬件模块上电/就绪标志位，各位组合满足后进入 `Operational` 模式。

---

# 总结与建议

* 掩码状态机是解决并行子状态、组合状态爆炸的有效工具；实现简单且高效。
* 推荐做法：严格定义位语义、构建规则索引、把执行从匹配路径里解耦（任务队列）。
* 下一步建议：

  * 在一个小模块中试点该方案（选择 8-16 个位测试）
  * 根据运行数据再做性能与索引优化
  * 补充可视化与回放工具，降低调试成本

---

# Q\&A

* 如果需要，我可以把该 PPT 导出为 **PPTX** 或 **PDF**，或根据你们的公司模板美化样式。

---

# 附：可选扩展（脚注）

* 支持规则优先级、互斥组、时间窗（例如：在 5s 内位未变则触发）
* 将掩码状态机与规则引擎（DSL）结合，做到非程序员可配置规则




# C++ 掩码状态机（Mask-based State Machine）汇报 — 图片主导版

---

# 封面（大图）

![封面：掩码与并行状态示意](images/cover_mask_big.png)

**C++ 掩码状态机（Mask-based State Machine）**

* 报告人：
* 日期：
* 场景：技术分享 / 设计评审

> 建议：封面使用高对比的矢量图（位掩码->位图矩阵），标题置中，副标题及作者信息置底。

---

# 目录（图示导航）

![目录图标导航](images/toc_icons.png)

1. 背景与动机
2. 经典 FSM 对比图
3. 掩码状态机概念图
4. 优势（图表）
5. 核心数据结构（可视化）
6. 实现流程图
7. 代码示例（图+文字）
8. 优化与扩展（架构图）
9. 测试/调试（流程+示例）
10. 总结与建议

---

# 背景与动机（图解）

![背景动机：状态爆炸示意](images/background_state_explosion.png)

* 左图：传统 FSM 面临的“状态爆炸”——每个子状态组合扩展出大量枚举。
* 右图：掩码表示把并行子状态压缩为位向量，表达更直观。

---

# 经典有限状态机回顾（对比图）

![FSM vs MaskFSM 比较矩阵](images/fsm_vs_maskfsm.png)

* 图中列出：节点数随子状态数量增长的对比曲线（折线图）。
* 右侧放一个小的状态转换示意动画（或序列图 GIF）。

---

# 掩码状态机概念（可交互位图）

![位掩码示意：每一位代表子状态](images/bitmask_concept.png)

* 可视化：把 64 位分为多组子系统（用颜色区分），每个子系统下列出位含义。
* 小提示框：位运算（AND/OR/XOR）的直观效果示意。

---

# 优势与适用场景（信息图）

![优势信息图：表达力、性能、可扩展性](images/benefits_infographic.png)

* 在一张图中用图标化的短语表示：表达力强、状态数减少、匹配高效、易扩展。
* 右侧列出适用场景的图标（网络、设备、工作流、并发）。

---

# 核心数据结构与 API（结构图 + 代码快照）

![数据结构图：Mask / Rule / Engine](images/data_structure_diagram.png)

* 图中展示 `Mask`、`Rule`、`MaskStateMachine` 三部分的关系箭头。
* 旁边放一张关键代码片段的图（code-screenshot.png），突出接口：`add_rule`、`set_bits`、`evaluate`。

---

# 实现思路（流程图）

![实现流程图：定义位 -> 注册规则 -> 更新位 -> 匹配触发 -> 执行动作](images/implementation_flowchart.png)

* 每一步用图标与简短说明；对 "匹配策略" 用分支图说明优先级处理/并发处理。

---

# C++ 示例（图 + 代码高亮截图）

![代码截图：关键片段](images/code_snippet_highlight.png)

* 左侧：代码截图（已高亮）；右侧：流程示意图（从 `set_bits` 到 `evaluate` 到 `action`）。
* 附注：避免在 `evaluate` 中直接执行长时回调 — 用任务队列图示说明。

---

# 性能优化（热力图与索引示意）

![优化示意：倒排索引与位子系统](images/optimization_inverted_index.png)

* 上图：规则线性扫描 vs 倒排索引的查找热力图（性能对比柱状图）。
* 下图：位分组与子系统索引示意。

---

# 并发与同步（时序图）

![并发更新时序图](images/concurrency_timeline.png)

* 图示：线程 A/B 对 `mask` 的原子更新，以及规则触发和任务入队的流程。
* 注明：使用 `std::atomic` + 任务队列以避免在 `evaluate` 中阻塞。

---

# 测试、调试与可视化（Trace 回放图）

![Trace 回放示意：状态时间轴与触发点](images/trace_replay.png)

* 一条时间轴表示 `mask` 的变化（高亮出每次规则触发点），旁边展示回放控制面板草图。

---

# 常见陷阱与最佳实践（图标列表）

![最佳实践图标集合](images/best_practices_icons.png)

* 每个图标下简短说明：文档化位、限时触发、避免滥用位语义、回调异步化。

---

# 真实案例（流程图 + 前后对比）

![案例：网关会话处理流程（位组合触发）](images/case_gateway_flow.png)

* 左图：采用传统方法的复杂流程图；右图：使用掩码后简洁的位组合触发图。

---

# 总结与下一步建议（图示路线图）

![路线图：试点 -> 优化 -> 工具化](images/roadmap_timeline.png)

* 推荐试点、收集 metrics、做索引优化、补充可视化工具。

---

# 附：需要我为每张占位图生成矢量图（PNG/SVG）吗？

* 我可以逐张生成：位布局图、实现流程图、倒排索引示意、并发时序图、Trace 回放图等。
* 请选择你优先需要生成的图（例如：实现流程图、位掩码示意、倒排索引），我会逐一生成并嵌入文档。

---

# 备注

文档中的 `images/*.png` 为占位文件名。若你需要我直接生成这些矢量图（SVG/PNG）并替换占位图，请在回复中列出**优先级前 3 张图**或直接说 “全部生成”。我将按顺序生成并更新文档。


# C++ 掩码状态机（Mask-based State Machine）汇报 — 文本+增强接口版

---

# 封面

C++ 掩码状态机（Mask-based State Machine） — 汇报PPT（增强版）

报告人：
日期：
场景：技术分享 / 设计评审

说明：本版本针对你的反馈做了三类改进：

1. 用更清晰的文本/框图（Unicode 箱线）替换原先简单 ASCII，便于阅读；
2. 增加并说明 `except` 类型接口与概念（用于排除/抑制规则触发与异常处理）；
3. 在示例代码中加入异常保护、on\_exception 回调、以及用于临时抑制规则触发的参数。

---

# 目录

1. 背景与动机
2. 经典 FSM 对比（清晰框图）
3. 掩码状态机概念（位图示 + 示例）
4. `except` 概念与 API 说明
5. 核心数据结构（文本 UML 风格）
6. 实现流程（更清晰的流程图）
7. C++ 示例（完整关键片段，含 except 与异常处理）
8. 优化与扩展（倒排索引、子系统）
9. 测试 / 调试（Trace 回放）
10. 总结与下一步建议

---

# 经典有限状态机回顾（清晰框图）

传统 FSM（单一枚举）示意：

╔════════╗    e1    ╔════════╗    e2    ╔════════╗
║ StateA ║ ───────> ║ StateB ║ ───────> ║ StateC ║
╚════════╝         ╚════════╝         ╚════════╝

问题：若每个 state 又有多个并行子状态（例如 A 有 {a0,a1}），传统 FSM 需要枚举合并组合，维护量急剧上升。

---

# 掩码状态机概念（位图示 + 示例）

位定义示例：

Bit index:   5 4 3 2 1 0
Flag name:  \[C2 C1 B1 B0 A1 A0]

当前 mask（举例）:

Mask: 0b  1 1 0 1 0 1   (从高位到低位)
┌────────────────┐
│ C2 C1 B1 B0 A1 A0 │
└────────────────┘

匹配逻辑（示意）：

* required\_mask: 0b000001 (A0 必须为1)
* forbidden\_mask:0b000010 (A1 必须为0)
* except\_mask(rule-level):0b001000 (若 B1=1 则此规则不触发 — 这是新增语义)

匹配条件（伪代码）：
matches = ((cur & required) == required) && ((cur & forbidden) == 0) && ((cur & except\_mask) == 0)

---

# `except` 概念与 API 说明

我们引入两类 `except` 概念：

1. **规则级排除（Rule.except\_mask）**

   * 含义：当当前 mask 的任一位与 `except_mask` 有交集（即这些位为1）时，该规则不会触发。
   * 用途：用于表达“在某些上下文（位为1时）不匹配此规则”。例如：处于维护模式（MAINT=1）时，禁止某些自动触发。

2. **调用级抑制（API except\_mask 参数）**

   * 在运行时调用 `set_bits` / `evaluate` 时，可传入 `except_mask` 来临时抑制那些依赖于这些位的规则被触发。
   * 用途：短期内禁止某类规则（例如：初始化阶段设置某些位，但不希望触发规则），或实现“批量设置后再触发”的语义。

3. **异常处理接口（on\_exception）**

   * 若 rule.action 抛出异常，StateMachine 捕获并将异常交给统一处理器：`on_exception(rule, exception_ptr)`。
   * 便于汇报/告警/回滚等。

示例 API：

* `set_bits(Mask m, Mask except_mask = 0)`
* `reset_bits(Mask m, Mask except_mask = 0)`
* `evaluate(Mask except_mask = 0)`
* `on_exception(std::function<void(const Rule&, std::exception_ptr)>)`

---

# 核心数据结构（文本 UML 风格）

+----------------------+        +-------------------------+
\| MaskStateMachine     | 1    \* | Rule                    |
+----------------------+ <----> +-------------------------+
\| - std::atomic<Mask>  |        | - name: string          |
\| - vector<Rule>       |        | - required: Mask        |
\| - on\_exception\_cb    |        | - forbidden: Mask       |
+----------------------+        | - except\_mask: Mask     |
\| - priority: int         |
\| - action: func          |
+-------------------------+

说明：`except_mask` 为规则级的抑制掩码。StateMachine 还维护倒排索引（可选）以加速 evaluate。

---

# 实现流程（更清晰的流程图）

╔════════════════════╗
║ 1) API: set\_bits() ║
╚════════════════════╝
│
▼
╔════════════════════╗      no          ╔════════════════════╗
║ 2) atomic OR mask  ║ ───────────────> ║ 3) evaluate()      ║
╚════════════════════╝                   ╚════════════════════╝
│
▼
compute candidate\_rules (using except\_mask)
│
▼
sort by priority / apply mutual-exclusion
│
▼
enqueue tasks for matched rules
│
▼
worker threads run actions with try/catch
│
on exception -> on\_exception\_cb

---

# C++ 示例（完整关键片段，含 except 与异常处理）

```cpp
using Mask = uint64_t;

struct Rule {
    std::string name;
    Mask required{0};
    Mask forbidden{0};
    Mask except_mask{0}; // 新增：若 (cur & except_mask) != 0，则跳过
    int priority{0};
    std::function<void(Mask)> action;
};

class MaskStateMachine {
public:
    // 注册规则
    void add_rule(Rule r) { rules_.push_back(std::move(r)); build_index_for(r); }

    // 设置回调：异常处理
    void set_on_exception(std::function<void(const Rule&, std::exception_ptr)> cb) {
        on_exception_ = std::move(cb);
    }

    // 支持 except_mask 来临时抑制触发
    void set_bits(Mask m, Mask except_mask = 0) {
        mask_.fetch_or(m, std::memory_order_acq_rel);
        evaluate(except_mask);
    }

    void reset_bits(Mask m, Mask except_mask = 0) {
        mask_.fetch_and(~m, std::memory_order_acq_rel);
        evaluate(except_mask);
    }

    void evaluate(Mask except_mask = 0) {
        Mask cur = mask_.load(std::memory_order_acquire);

        // 收集候选规则：基于倒排索引或全表扫描（此处为示例）
        std::vector<Rule*> matched;
        for (auto &r : rules_) {
            // 1) 排除规则级的 except
            if ((cur & r.except_mask) != 0) continue;
            // 2) API 级的 except 抑制
            if ((cur & except_mask) != 0 && (r.required & except_mask) != 0) continue;
            // 3) required / forbidden 匹配
            if ((cur & r.required) == r.required && (cur & r.forbidden) == 0) {
                matched.push_back(&r);
            }
        }

        // 按优先级排序并去重/互斥处理（按需要实现）
        std::sort(matched.begin(), matched.end(), [](Rule* a, Rule* b){ return a->priority > b->priority; });

        // 把 action 放入任务队列执行（示例为同步调用，但演示 try/catch）
        for (auto *rp : matched) {
            try {
                rp->action(cur);
            } catch(...) {
                if (on_exception_) on_exception_(*rp, std::current_exception());
            }
        }
    }

private:
    void build_index_for(const Rule& r) {
        // 示例：把 r 所关心的位加入倒排索引（略）
    }

    std::atomic<Mask> mask_{0};
    std::vector<Rule> rules_;
    std::function<void(const Rule&, std::exception_ptr)> on_exception_;
};
```

说明：上例为了演示清晰将 action 同步调用并捕获异常；生产环境建议将 action 入队由 worker 线程处理，并在 worker 内做异常捕获和回调。

---

# 示例：批量设置但延迟触发（借助 except\_mask）

场景：初始化时把若干位打开，但不希望触发规则，最后一次性触发。

// 批量设置但抑制触发
machine.set\_bits(mask\_part1, /*except\_mask=*/ALL\_RULES\_MASK);
machine.set\_bits(mask\_part2, /*except\_mask=*/ALL\_RULES\_MASK);

// 最后解除抑制并 evaluate（或调用 evaluate(0)）
machine.evaluate(/*except\_mask=*/0);

或者提供专门接口：
begin\_suppressed\_update(); set\_bits(...); end\_suppressed\_update();

---

# 优化与扩展（倒排索引、子系统）

倒排索引示意（文本）：

index:
bit0 -> \[R2, R5]
bit1 -> \[R1, R3, R4]

更新时：
changed\_bits = old ^ new
candidate\_rules = union(index\[changed\_bits])
仅对 candidate\_rules 进行匹配检查

---

# 测试 / 调试（Trace 回放）

增强 trace：记录每次 set\_bits/reset\_bits 的 (time, caller, mask\_delta, except\_mask)，并记录 evaluate 输出的 matched rule 列表以及 action 执行结果/异常。

回放时可以逐步复现并高亮由于 except\_mask 抑制导致未触发的规则。

---

# 常见陷阱与最佳实践（补充）

* 明确区分 `forbidden` 与 `except_mask`：

  * `forbidden` 表示规则要求这些位为 0 才能匹配（规则语义的一部分）；
  * `except_mask` 表示在这些上下文下 *整个规则被抑制*（通常为外部控制/上下文开关）。

* 使用 `begin_suppressed_update` / `end_suppressed_update` 或 `except_mask` 来实现批量原子设置并延迟触发。

* 始终在 worker 内捕获异常，并把失败信息通过 `on_exception` 上报，避免线程池或主循环被异常打断。

---

# 总结与下一步建议

本版本改进要点：

* 图示更清晰（Unicode 箱线式文本图）；
* 增加规则级 `except_mask` 与 API 级 `except_mask`；
* 增加统一异常回调 `on_exception` 并在示例中演示 try/catch；
* 给出批量设置/延迟触发的多种实现方案。

接下来我可以：

* 把这份增强版导出为 PPTX（图形化模板）或 PDF；
* 将关键图示（流程图 / 倒排索引 / 并发时序图）生成为向量 SVG，并替换文档中的文本图；
* 根据你团队的真实位定义（给我位列表）把示例规则与图示替换为真实内容。

请告诉我你要我执行哪一步（例如：“导出 PPTX”，“生成 SVG 流程图并替换”，或“把文档翻译成英文”）。


已根据你的反馈把 PPT 升级为“文本+增强接口版”并更新到画布上。主要改动（概览）：

* 用更清晰的 Unicode 箱线/流程图替换原先不够表达力的 ASCII 图示；
* 增加并详细说明 `except` 概念（规则级 `except_mask` 与 API 级 `except_mask`）以及相应用例；
* 在示例代码里加入异常保护（try/catch）、`on_exception` 回调，以及 `set_bits/reset_bits/evaluate` 的 `except_mask` 参数示例；
* 补充了批量设置/延迟触发的方案与最佳实践说明。

下一步我可以直接为你做下面其中一项（选一项或多项）：

1. 导出当前文档为 **PPTX**（公司模板可指定）或 **PDF**；
2. 把关键图示（实现流程、倒排索引、并发时序、Trace 回放）生成为 **SVG/PNG 向量图**并替换占位文本；
3. 根据你提供的 **具体位定义表**（flag 名称与索引），把示例规则替换为真实规则并生成示例 trace；
4. 将整个文档 **翻译成英文**。

你想先做哪一步？如果你指的 `except` 是别的含义（比如“异常（exception）”以外的某个接口），请直接说明，但如果你想让我按上面的解释继续，我会开始执行你选择的步骤。


https://sdmntpraustraliaeast.oaiusercontent.com/files/00000000-3038-61fa-8b46-96dc02ecf44c/raw?se=2025-09-03T01%3A20%3A08Z&sp=r&sv=2024-08-04&sr=b&scid=b07c03ef-73e9-5ed0-acce-b3bb4e03cd88&skoid=b7fc319f-b93c-4fac-ba5f-14fdc3f9209f&sktid=a48cca56-e6da-484e-a814-9c849652bcb3&skt=2025-09-02T21%3A01%3A40Z&ske=2025-09-03T21%3A01%3A40Z&sks=b&skv=2024-08-04&sig=YksW2dJn1UrYetglTSI3g%2BcH7izhCZSIU/Fp1EpRqoo%3D
