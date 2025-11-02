# 状态映射实现方案

* 面向技术委员会评审 / 专家汇报
* 包含伪代码 & C++ 实现示例
* 三种方案（枚举、掩码、表驱动）全对比
* 含“合法性判断 / 获取合法状态”实现页

## 背景与问题

* 系统由多个子系统状态组成，总状态组合爆炸
* 合法状态需人工枚举或硬编码映射，容易遗漏
* 目标：找到**可扩展、易维护、易验证**的状态映射机制

## 方案

### 方案1 枚举合法状态组合

* 描述方式：`enum class` + 查表
* 判断逻辑：遍历合法组合表
* C++ 示例：

  ```cpp
  enum class CommState { Idle, Active };
  enum class PowerState { Off, On };
  struct SystemState { CommState comm; PowerState power; };
  static const std::vector<SystemState> validStates = {
      {CommState::Idle, PowerState::Off},
      {CommState::Active, PowerState::On}
  };
  bool isLegal(const SystemState& s) {
      return std::find(validStates.begin(), validStates.end(), s) != validStates.end();
  }
  ```
* 优点：逻辑直观
* 缺点：组合爆炸、维护繁琐、所有状态组合放在一起，不利于可读性

### 方案2 掩码表示

* 描述方式：每个子系统状态使用若干 bit 表示
* 判断逻辑：按位匹配合法掩码
* C++ 示例：

  ```cpp
  using StateMask = uint32_t;
  constexpr StateMask COMM_ACTIVE = 1 << 0;
  constexpr StateMask POWER_ON = 1 << 1;
  constexpr StateMask LEGAL = COMM_ACTIVE | POWER_ON;
  bool isLegal(StateMask s) { return (s & LEGAL) == LEGAL; }
  ```
* 优点：效率高、存储紧凑
* 缺点：可读性差、跨模块扩展困难

虽然掩码高效，但会隐含产生非预期状态组合（非法组合）
表面上看，这种组合既简洁又高效（按位或组合、按位与判断），但问题是： 掩码本身是**自由组合**的，不具备“合法性约束”，因此理论上可以组合出无意义的状态。

问题原因

1. **掩码天然是扁平空间**
   它无法表达“每个子系统内只能选一个状态”的约束。

2. **缺乏合法性验证机制**
   掩码逻辑层只做按位判断，不关心状态间逻辑。

3. **多子系统之间缺乏约束表达能力**
   无法直接表示跨子系统组合是否允许（例如：Power=Off 时 Comm 不能 Active）。



### 方案3 表驱动 / 层级状态映射

* 描述方式：状态节点 + 转移表
* 支持层级继承与动态扩展
* C++ 示例：

  ```cpp
  struct StateNode {
      std::string name;
      std::vector<std::string> allowedTransitions;
      bool canTransitTo(const std::string& next) const {
          return std::find(allowedTransitions.begin(), allowedTransitions.end(), next)
                 != allowedTransitions.end();
      }
  };
  std::map<std::string, StateNode> stateTable = {
      {"Idle", {"Idle", {"Active"}}},
      {"Active", {"Active", {"Idle", "Error"}}}
  };
  bool isLegalTransition(const std::string& cur, const std::string& next) {
      return stateTable[cur].canTransitTo(next);
  }
  ```
* 可通过 JSON 配置驱动状态表，无需改代码即可扩展
* 可集成掩码用于运行时高效判断

### 对比

| 功能     | 枚举方案    | 掩码方案     | 表驱动方案     |
| ------ | ------- | -------- | --------- |
| 获取合法状态 | 遍历静态表   | 计算掩码组合   | 动态配置或注册   |
| 判断合法性  | O(n) 查表 | O(1) 位运算 | O(1) 哈希查找 |

## 总结

* 推荐采用 **表驱动 + 掩码混合模式**：

  * 配置层：表驱动合法组合
  * 运行层：掩码快速判断
* 支持插件式扩展与热更新
* 结合 JSON/Proto 可实现跨语言共享状态描述

