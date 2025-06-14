# C/C++ Grammar

## 1 宏

pragma
用途1：关闭告警
```c
#pragma warning(push) // 保存当前警告状态

#pragma warning(disable:4512) // 关闭特定告警

#include <boost/lambda/lambda.hpp>

#pragma warning(pop) // 恢复之前警告状态
```