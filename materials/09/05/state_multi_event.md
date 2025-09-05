下面给你一个“**多事件同时满足才转移**”的 C++ 示例，用**位掩码**收集事件；当收集到的事件集合覆盖了转移规则中要求的事件集合（AND 条件）时，执行状态迁移。示例还支持**时间窗口**（例如要求这些事件在 200ms 内全部发生，否则重置重新收集）。

```cpp
#include <bits/stdc++.h>
using namespace std;

// 业务状态
enum class State { Idle, Arming, Active };

// 事件枚举（用位掩码表示）
enum EventBits : uint32_t {
    EVT_NONE = 0,
    EVT_A    = 1u << 0, // 比如：安全门关闭
    EVT_B    = 1u << 1, // 比如：温度达标
    EVT_C    = 1u << 2, // 比如：电压达标
};
inline EventBits operator|(EventBits a, EventBits b){ return EventBits(uint32_t(a)|uint32_t(b)); }
inline EventBits operator&(EventBits a, EventBits b){ return EventBits(uint32_t(a)&uint32_t(b)); }
inline EventBits& operator|=(EventBits& a, EventBits b){ a = a | b; return a; }

// 状态转移规则：from->to 需要 required_mask 中的所有事件在 window 内满足
struct TransitionRule {
    State from;
    State to;
    uint32_t required_mask;                  // 需要的事件集合（AND）
    chrono::milliseconds window{0};          // 0 表示不限时间
    bool clear_events_on_enter = true;       // 进入目标态是否清空已收集事件
};

// 简单的掩码式状态机
class MaskAndStateMachine {
public:
    explicit MaskAndStateMachine(State s0 = State::Idle) : state_(s0) {}

    void add_rule(const TransitionRule& r){ rules_.push_back(r); }

    State state() const { return state_; }

    // 注入事件（可多次调用，事件会累积）
    void post(EventBits e){
        auto now = chrono::steady_clock::now();

        // 第一次收集，启动计时
        if (collected_mask_ == 0) {
            first_event_ts_ = now;
        }
        collected_mask_ |= uint32_t(e);

        try_transition(now);
    }

    // 周期性调用（可选）：用于检查时间窗口是否超时并重置
    void tick(){
        if (collected_mask_ == 0) return;
        auto now = chrono::steady_clock::now();

        // 如果当前状态下有带窗口的规则，检查是否超时（任意一条带窗口且需事件的规则即可触发重置）
        for (const auto& r : rules_) {
            if (r.from != state_) continue;
            if (r.window.count() <= 0) continue;
            if (chrono::duration_cast<chrono::milliseconds>(now - first_event_ts_) > r.window) {
                // 超时：清空，重新收集
                collected_mask_ = 0;
                break;
            }
        }
    }

private:
    void try_transition(chrono::steady_clock::time_point now){
        for (const auto& r : rules_) {
            if (r.from != state_) continue;

            // 时间窗口检查（如果配置了）
            if (r.window.count() > 0 && collected_mask_ != 0) {
                if (now - first_event_ts_ > r.window) {
                    // 超时则丢弃本轮收集
                    collected_mask_ = 0;
                    first_event_ts_ = now; // 下一次事件作为新的起点
                    continue;
                }
            }

            // AND 条件：已收集事件必须包含全部 required_mask 位
            bool all_met = ( (collected_mask_ & r.required_mask) == r.required_mask );
            if (all_met) {
                // 执行转移
                cout << "Transition: " << to_string(state_) << " -> " << to_string(r.to)
                     << " (mask=0x" << hex << collected_mask_ << dec << ")\n";

                state_ = r.to;

                if (r.clear_events_on_enter) {
                    collected_mask_ = 0;
                } else {
                    // 若不清空，仅移除已经用掉的位（可选策略）
                    collected_mask_ &= ~r.required_mask;
                }
                // 进入新状态后，起点时间重置
                first_event_ts_ = now;
                // 一次只触发一条规则，若需链式触发可以继续 for 循环前不 return
                return;
            }
        }
    }

    static const char* to_string(State s){
        switch(s){
            case State::Idle:   return "Idle";
            case State::Arming: return "Arming";
            case State::Active: return "Active";
            default:            return "?";
        }
    }

private:
    State state_;
    uint32_t collected_mask_ = 0; // 已收集事件位
    chrono::steady_clock::time_point first_event_ts_{}; // 本轮收集的起点时间
    vector<TransitionRule> rules_;
};

int main(){
    MaskAndStateMachine sm(State::Idle);

    // 规则1：Idle -> Arming，要求 A 和 B 都满足（AND），500ms 内
    sm.add_rule(TransitionRule{
        .from = State::Idle,
        .to = State::Arming,
        .required_mask = EVT_A | EVT_B,
        .window = chrono::milliseconds(500),
        .clear_events_on_enter = true
    });

    // 规则2：Arming -> Active，要求 B 和 C（比如再次校验 B 与新增 C），不限制时间
    sm.add_rule(TransitionRule{
        .from = State::Arming,
        .to = State::Active,
        .required_mask = EVT_B | EVT_C,
        .window = chrono::milliseconds(0),
        .clear_events_on_enter = true
    });

    // --- 演示：事件乱序且分次到达，但只要在窗口内就能转移 ---
    sm.post(EVT_A);                    // 收到 A
    this_thread::sleep_for(chrono::milliseconds(120));
    sm.tick();                         // 可选：周期检查窗口
    sm.post(EVT_B);                    // 500ms 窗口内收到 B -> 触发 Idle->Arming

    // 进入 Arming 后，收集下一跳的事件
    sm.post(EVT_B);                    // 再次确认 B
    sm.post(EVT_C);                    // 收到 C -> 触发 Arming->Active

    cout << "Final state = Active? " << (sm.state() == State::Active) << "\n";
    return 0;
}
```

### 说明与可改造点

* **AND 条件**：用 `required_mask` 指定本次转移必须“全部具备”的事件集合；判断条件是
  `all_met = ((collected_mask & required_mask) == required_mask)`.
* **时间窗口**：`window=0` 表示不限时间；>0 则要求这些事件在窗口内到齐，否则在 `tick()` 中重置。
* **事件复用**：`clear_events_on_enter` 控制进入新状态后是否清空已收集事件位；如果想“部分复用”，示例里也演示了只清掉用过位的做法。
* **扩展**：

  * 可把规则放到表里用数据驱动（方便单测与切环境）。
  * 可加入“**必须禁止的事件**”掩码（某些位出现则立即失败或回退）。
  * 加锁即可变成线程安全版本（在 `post/tick/try_transition` 外围加 `std::mutex`）。

如果你要把它接到你之前的“掩码状态机基类/多子系统”框架，我可以直接按你的接口改成可复用的组件版本。
