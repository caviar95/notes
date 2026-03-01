我明白了，你的意思是：

* 现在的 **mask → finalState** 映射依旧成立。
* 不允许回退的判断，不是简单地比较数值大小，而是**基于映射表**：

  * 如果某个设备的某个状态（比如 A=5）对应**多个不同的 finalState**（即该状态在多个 mask key 中出现过），那么一旦这个设备已经到达了该状态，就不能从该状态回到该设备的更低状态。
  * 换句话说，这个状态对系统是“多分支关键点”，一旦进入，回退可能导致状态机走到另一条不期望的路径，所以要禁止回退。

---

## 设计思路

我们可以在构建状态机映射表时，**额外记录每个设备的每个状态出现在哪些 finalState 中**。

* 用一个结构，比如：

  ```cpp
  std::unordered_map<size_t /*设备编号*/, std::unordered_map<uint32_t /*状态值*/, std::unordered_set<std::string> /*finalState集合*/>> deviceStateToFinals;
  ```
* 在 `addMapping()` 时，同时更新这个统计表。
* 在 `getNextMask()` 时，如果当前设备状态值的 `finalState` 集合大小 > 1，就触发禁止回退逻辑（只针对降级情况）。

---

## 代码示例

### state\_machine.h

```cpp
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <vector>

template <typename MaskT = uint32_t, size_t DEVICES = 4, size_t BITS_PER_DEVICE = 8>
class StateMachine {
public:
    using Mask = MaskT;
    using FinalState = std::string;

    StateMachine();

    static constexpr Mask fieldMask() {
        return (Mask(1) << BITS_PER_DEVICE) - 1;
    }

    static constexpr size_t bitOffset(size_t deviceIndex) {
        return deviceIndex * BITS_PER_DEVICE;
    }

    static constexpr uint32_t getDeviceState(Mask m, size_t deviceIndex) {
        return (m >> bitOffset(deviceIndex)) & fieldMask();
    }

    static constexpr Mask setDeviceState(Mask m, size_t deviceIndex, uint32_t stateVal) {
        Mask fm = fieldMask();
        Mask shift = bitOffset(deviceIndex);
        m &= ~(fm << shift);
        m |= (stateVal & fm) << shift;
        return m;
    }

    static constexpr Mask makeMask(const std::array<uint32_t, DEVICES>& states) {
        Mask m = 0;
        for (size_t i = 0; i < DEVICES; ++i)
            m = setDeviceState(m, i, states[i]);
        return m;
    }

    static std::array<uint32_t, DEVICES> decodeMask(Mask m) {
        std::array<uint32_t, DEVICES> arr{};
        for (size_t i = 0; i < DEVICES; ++i)
            arr[i] = getDeviceState(m, i);
        return arr;
    }

    void addMapping(Mask key, const FinalState& finalState);

    Mask getNextMask(Mask current, Mask desired) const;

    FinalState findFinalState(Mask key) const;

    void printStateTable() const;

private:
    std::unordered_map<Mask, FinalState> table_;

    // 新增：记录每个设备的每个状态对应哪些finalState
    std::unordered_map<size_t, std::unordered_map<uint32_t, std::unordered_set<FinalState>>> deviceStateToFinals_;
};

#endif
```

---

### state\_machine.cpp

```cpp
#include "state_machine.h"

template <typename MaskT, size_t DEVICES, size_t BITS_PER_DEVICE>
StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::StateMachine() {}

template <typename MaskT, size_t DEVICES, size_t BITS_PER_DEVICE>
void StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::addMapping(Mask key, const FinalState& finalState) {
    table_[key] = finalState;

    auto states = decodeMask(key);
    for (size_t dev = 0; dev < DEVICES; ++dev) {
        deviceStateToFinals_[dev][states[dev]].insert(finalState);
    }
}

template <typename MaskT, size_t DEVICES, size_t BITS_PER_DEVICE>
typename StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::Mask
StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::getNextMask(Mask current, Mask desired) const {
    Mask result = current;
    auto curStates = decodeMask(current);
    auto desStates = decodeMask(desired);

    for (size_t dev = 0; dev < DEVICES; ++dev) {
        uint32_t cur = curStates[dev];
        uint32_t des = desStates[dev];

        if (des < cur) {
            // 如果当前设备状态在多个finalState中出现，则禁止回退
            auto itState = deviceStateToFinals_.find(dev);
            if (itState != deviceStateToFinals_.end()) {
                auto itVal = itState->second.find(cur);
                if (itVal != itState->second.end() && itVal->second.size() > 1) {
                    std::cout << "[BLOCK] Device " << dev << " state " << cur
                              << " maps to multiple finalStates, downgrade blocked.\n";
                    continue; // 保持当前状态
                }
            }
        }
        result = setDeviceState(result, dev, des);
    }
    return result;
}

template <typename MaskT, size_t DEVICES, size_t BITS_PER_DEVICE>
typename StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::FinalState
StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::findFinalState(Mask key) const {
    auto it = table_.find(key);
    return (it != table_.end()) ? it->second : "UNKNOWN";
}

template <typename MaskT, size_t DEVICES, size_t BITS_PER_DEVICE>
void StateMachine<MaskT, DEVICES, BITS_PER_DEVICE>::printStateTable() const {
    std::cout << "State Table:\n";
    for (const auto& kv : table_) {
        auto arr = decodeMask(kv.first);
        std::cout << "Mask 0x" << std::hex << +kv.first << std::dec << " -> " << kv.second << " [";
        for (size_t i = 0; i < DEVICES; ++i) {
            std::cout << arr[i] << (i + 1 == DEVICES ? "" : ", ");
        }
        std::cout << "]\n";
    }
}

// 显式实例化
template class StateMachine<uint32_t, 4, 8>;
```

---

### main.cpp

```cpp
#include "state_machine.h"
#include <iostream>

int main() {
    using SM = StateMachine<uint32_t, 4, 8>;
    SM sm;

    // 添加映射（mask -> finalState）
    auto m1 = SM::makeMask({5, 1, 0, 0});
    auto m2 = SM::makeMask({5, 2, 0, 0}); // 同设备A=5，但finalState不同
    auto m3 = SM::makeMask({4, 1, 0, 0});

    sm.addMapping(m1, "FINAL_X");
    sm.addMapping(m2, "FINAL_Y");
    sm.addMapping(m3, "FINAL_Z");

    sm.printStateTable();

    // 当前状态（A=5）
    auto cur = m1;
    // 目标状态想回退到 A=4
    auto desired = m3;

    auto next = sm.getNextMask(cur, desired);

    auto arrCur = SM::decodeMask(cur);
    auto arrDes = SM::decodeMask(desired);
    auto arrNext = SM::decodeMask(next);

    std::cout << "\nTransition attempt:\n";
    for (size_t i = 0; i < arrCur.size(); ++i) {
        std::cout << "Device " << i << ": " << arrCur[i]
                  << " -> desired " << arrDes[i]
                  << " -> next " << arrNext[i] << "\n";
    }

    return 0;
}
```

---

这样就满足你的需求：

* **mask → finalState** 保留
* 每个设备的状态是数值编码（可扩展）
* 判断回退时，**只有在该设备当前状态映射多个 finalState 的情况下**，才禁止回退
* 其他情况（只有一个 finalState 对应当前状态）则允许回退

---

你要我帮你加一个**打印“设备状态 -> 对应finalState数量”的调试工具**吗？这样可以在运行前快速看出哪些状态会触发禁止回退。这样在配置状态表时就能一眼看出风险点。
