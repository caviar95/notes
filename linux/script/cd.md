

# 🐚 Shell 脚本中使用 `cd` 无效？你可能忽略了这些细节！


在使用 macOS 或 Linux 编写 shell 脚本时，很多人都遇到过这样一个令人困惑的现象：

> 🧩 *“我在脚本中使用了 `cd` 命令切换目录，脚本也执行了，但终端当前的目录并没有改变？”*

如果你也被这个问题困扰过，那么本文将为你揭开背后的原理，并提供几种实用解决方案。

---

## ✅ 背景：Shell 脚本中的 `cd` 命令为什么“无效”？

首先我们来看一个典型的示例脚本：

```bash
#!/bin/zsh
cd /Volumes/ExtDisk/code
```

执行它：

```bash
./goto_disk.sh
```

预期：当前终端切换到 `/Volumes/ExtDisk/code`
现实：终端路径保持不变 😵

这是为什么？

---

## 🔍 本质：Shell 脚本在“子进程”中执行

关键点是：

> **Shell 脚本默认在一个子进程（新的 shell 实例）中运行，当前 shell 不受影响。**

### 举例说明：

```bash
# 假设你当前在 ~/Desktop
./goto_disk.sh
pwd  # 输出仍是 ~/Desktop
```

当你执行脚本，系统会启动一个新的进程执行里面的命令，`cd` 虽然执行了，但**它只改变了子进程的工作目录**。当脚本结束时，子进程退出，目录的改变也随之消失。

---

## 🚧 常见问题场景

| 问题表现                     | 原因            |
| ------------------------ | ------------- |
| 脚本内 `cd` 没反应             | 使用了子进程执行脚本    |
| `cd` 到 `/Volumes/...` 失败 | 可能硬盘未挂载或名字不一致 |
| 脚本执行成功但“没进入目录”           | 当前终端环境未改变     |

---

## ✅ 正确做法：使用 `source` 或 `.` 执行脚本

要让脚本影响当前终端的工作目录，你必须使用：

```bash
source ./goto_disk.sh
# 或者
. ./goto_disk.sh
```

这两个命令会在“当前 shell 环境”中执行脚本中的命令，而不是新建子进程。因此，`cd` 就能正常生效。

### 示例：

```bash
# 脚本内容 goto_disk.sh
#!/bin/zsh
cd /Volumes/ExtDisk/code || echo "路径不存在"
```

```bash
# 执行脚本
source ./goto_disk.sh
pwd
# ✅ 输出应为 /Volumes/ExtDisk/code
```

---

## 💡 进阶：将跳转封装为函数

更推荐的方式是将跳转逻辑封装为 shell 函数，并添加到你的 `~/.zshrc` 或 `~/.bash_profile` 中：

```zsh
function gotocode() {
    cd /Volumes/ExtDisk/code || echo "扩展硬盘未挂载或路径不存在"
}
```

保存后执行：

```bash
source ~/.zshrc
gotocode
```

这种方式方便、快捷，并且不需要记忆 `source` 的细节。

---

## 🛠️ 其他可能遇到的问题及排查方式

### 1. 🔍 硬盘名称不一致

macOS 默认会将外接硬盘挂载到 `/Volumes/<硬盘名称>` 下，确保名称拼写正确：

```bash
ls /Volumes
```

### 2. 💢 权限问题

如果脚本无法进入某个目录，可能是权限不足。可尝试加 `sudo` 或检查磁盘挂载状态。

### 3. 🚫 执行权限问题

脚本首次执行前记得赋予可执行权限：

```bash
chmod +x goto_disk.sh
```

---

## 🧠 总结

| 目标         | 正确做法                  |
| ---------- | --------------------- |
| 在当前终端中切换目录 | 使用 `source` 或定义函数     |
| 快速访问磁盘目录   | 写函数并加入 `.zshrc`       |
| 保证路径正确     | 用 `ls /Volumes` 查看挂载名 |

---

## 📌 附：推荐脚本模板

```bash
#!/bin/zsh

disk="ExtDisk"
target_dir="/Volumes/$disk/code"

if [ -d "$target_dir" ]; then
    cd "$target_dir"
else
    echo "⚠️ 目标路径不存在：$target_dir"
fi
```

使用方式：

```bash
source ./goto_disk.sh
```

---

## 📚 相关链接

* [zsh 手册](https://zsh.sourceforge.io/Doc/)
* [macOS 下的挂载路径 `/Volumes`](https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html)

