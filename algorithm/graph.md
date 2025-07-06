
# 图论基础入门

图论（Graph Theory）是计算机科学中的核心基础之一，被广泛应用于算法设计、工程系统、人工智能、编译器、网络通信、地图导航等多个方向。

---

## 一、什么是图（Graph）？

图是一种由顶点（Vertex）和边（Edge）组成的数据结构，用于表示元素之间的关系。

### 分类：

* 有向图（Directed Graph）：边具有方向，如 A → B。
* 无向图（Undirected Graph）：边无方向，A 与 B 相互连接。
* 带权图（Weighted Graph）：边携带权重信息。
* 稀疏图 / 稠密图：根据边的数量与顶点的比例判断。

---

## 二、图的存储结构（C++ 实现）

### 1. 邻接表（Adjacency List）

适用于稀疏图，节省空间。

```cpp
int n; // 顶点数量
vector<vector<int>> adj(n); // 无权图的邻接表

// 添加一条有向边 u → v
adj[u].push_back(v);
```

带权图：

```cpp
vector<vector<pair<int, int>>> adj(n); // pair<邻接点, 权重>

adj[u].emplace_back(v, weight);
```

### 2. 邻接矩阵（Adjacency Matrix）

适用于稠密图，占用空间大但查询更快。

```cpp
vector<vector<int>> matrix(n, vector<int>(n, 0));
matrix[u][v] = 1; // 表示 u → v 有边
```

---

## 三、图的遍历

### 1. 深度优先搜索（DFS）

```cpp
void dfs(int u, vector<vector<int>>& adj, vector<bool>& visited) {
    visited[u] = true;
    for (int v : adj[u]) {
        if (!visited[v]) {
            dfs(v, adj, visited);
        }
    }
}
```

### 2. 广度优先搜索（BFS）

```cpp
void bfs(int start, vector<vector<int>>& adj, vector<bool>& visited) {
    queue<int> q;
    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
}
```

---

## 四、经典图算法（C++ 实现）

### 1. 拓扑排序（Topological Sort）——仅适用于有向无环图（DAG）

#### Kahn 算法（基于入度）

```cpp
bool topoSort(int n, const vector<vector<int>>& adj, vector<int>& result) {
    vector<int> inDegree(n, 0);
    for (const auto& neighbors : adj)
        for (int v : neighbors) inDegree[v]++;

    queue<int> q;
    for (int i = 0; i < n; ++i)
        if (inDegree[i] == 0) q.push(i);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.push_back(u);
        for (int v : adj[u])
            if (--inDegree[v] == 0) q.push(v);
    }

    return result.size() == n; // false 表示有环
}
```

### 2. 最短路径算法

#### Dijkstra（适用于正权图）

```cpp
vector<int> dijkstra(int n, int start, const vector<vector<pair<int, int>>>& adj) {
    vector<int> dist(n, INT_MAX);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

    dist[start] = 0;
    pq.emplace(0, start);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.emplace(dist[v], v);
            }
        }
    }
    return dist;
}
```

### 3. 并查集（Union-Find）——用于连通性判断、Kruskal 算法

```cpp
struct UnionFind {
    vector<int> parent;
    UnionFind(int n) : parent(n) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        return parent[x] == x ? x : (parent[x] = find(parent[x]));
    }

    bool unite(int a, int b) {
        int pa = find(a), pb = find(b);
        if (pa == pb) return false;
        parent[pa] = pb;
        return true;
    }
};
```

---

## 五、应用场景

* 编译依赖分析：如 Makefile 中的文件依赖。
* 项目构建系统：如 CMake 中的模块拓扑。
* 图数据库遍历：如 Neo4j 查询路径。
* 地图导航系统：如最短路径搜索（Dijkstra、A\*）。
* 社交网络分析：好友推荐、社群划分。
* 多线程任务调度系统：拓扑排序 + 优先级控制。


