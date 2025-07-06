拓扑排序（Topological Sort）是图论中的一种重要算法，用于对有向无环图（DAG）中的所有顶点进行线性排序，使得对于每一条有向边 `u → v`，顶点 `u` 出现在顶点 `v` 之前。

在 C++ 开发中，拓扑排序经常用于以下场景：

* 编译依赖管理：一个源文件依赖其他文件的编译顺序。
* 任务调度系统：某些任务必须先完成后才能执行其他任务。
* 构建系统（如 Make）：根据依赖关系决定构建顺序。
* 课程安排系统：先修课程约束。

---

## 1 基本概念

输入：一个有向无环图（DAG）

输出：一种顶点的线性序列，满足所有边的方向约束

前提：图必须是无环的，若存在环，则无法进行拓扑排序

---

## 2 常见实现方式

### 方法一：Kahn 算法（基于入度的广度优先）

#### 原理

1. 计算每个顶点的入度。
2. 将所有入度为 0 的顶点入队。
3. 每次从队列中取出一个顶点 `u`，加入拓扑序列。
4. 遍历 `u` 的邻接点 `v`，将 `v` 的入度减 1，若变为 0，则入队。
5. 重复直到队列为空。
6. 若最后拓扑序列大小小于顶点数，说明有环。

#### 示例代码

```cpp
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

bool topologicalSort(int n, const vector<vector<int>>& adj, vector<int>& result) {
    vector<int> inDegree(n, 0);
    for (const auto& edges : adj) {
        for (int v : edges) {
            inDegree[v]++;
        }
    }

    queue<int> q;
    for (int i = 0; i < n; ++i)
        if (inDegree[i] == 0)
            q.push(i);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.push_back(u);
        for (int v : adj[u]) {
            if (--inDegree[v] == 0)
                q.push(v);
        }
    }

    return result.size() == n; // 返回是否是一个合法拓扑排序
}
```

---

### 方法二：DFS（基于深度优先搜索）

#### 原理

1. 使用 DFS 从未访问过的点出发。
2. 当一个顶点的所有邻接点都被访问过，才能将其加入结果序列。
3. 最终结果需要 反转。

#### 示例代码

```cpp
#include <iostream>
#include <vector>
#include <stack>

using namespace std;

bool dfs(int u, const vector<vector<int>>& adj, vector<bool>& visited, vector<bool>& onPath, stack<int>& stk) {
    visited[u] = onPath[u] = true;
    for (int v : adj[u]) {
        if (!visited[v]) {
            if (!dfs(v, adj, visited, onPath, stk))
                return false;
        } else if (onPath[v]) {
            return false; // 有环
        }
    }
    onPath[u] = false;
    stk.push(u);
    return true;
}

bool topologicalSortDFS(int n, const vector<vector<int>>& adj, vector<int>& result) {
    vector<bool> visited(n, false), onPath(n, false);
    stack<int> stk;

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            if (!dfs(i, adj, visited, onPath, stk))
                return false;
        }
    }

    while (!stk.empty()) {
        result.push_back(stk.top());
        stk.pop();
    }
    return true;
}
```

---

## 3 拓扑排序在 C++ 实战中的典型场景

1. 模块初始化顺序
2. Makefile 编译依赖
3. 容器启动依赖管理（如 Kubernetes）
4. 项目构建系统中的任务图

---

## 4 检测环

* Kahn 算法中，如果最后输出的拓扑序列长度 < 顶点数，说明图中存在环。
* DFS 中，使用 `onPath` 数组标记路径栈，如果在递归中遇到还在当前路径中的节点，则存在环。

---

## 5 总结

| 算法        | 时间复杂度    | 空间复杂度 | 是否可检测环 | 适用场景          |
| --------- | -------- | ----- | ------ | ------------- |
| Kahn（BFS） | O(V + E) | O(V)  | 是      | 入度关系明显，适合任务调度 |
| DFS       | O(V + E) | O(V)  | 是      | 图结构复杂、依赖多层嵌套  |
