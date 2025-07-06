
最大子段和
使用动态规划（Kadane's Algorithm)求解

```cpp
#include <bits/stdc++.h>

using namespace std;

int main() {
    ios::sync_with_stdio(false);

    cin.tie(nullptr);

    int n;
    cin >> n;

    long long mx = LLONG_MIN;
    long long dp = 0;
    long long x;

    for (int i = 0; i < n; ++i) {
        cin >> x;
        dp = x + max(0, dp);
        mx = max(mx, dp);
    }

    cout << mx << '\n';

    return 0;
}
```