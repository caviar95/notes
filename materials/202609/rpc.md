核心要先区分一个概念：

> **RPC 的 callback 通常表示“RPC 调用完成/响应收到”，不一定表示“业务动作完成”。**

如果服务端提供的是**异步业务处理**，那么你真正想要的是：

```text
Client                         Server
  |                              |
  |---- StartAction RPC -------->|
  |                              | 接收请求
  |<--- RPC Response ------------| 仅表示“已接受”
  |                              |
  |                              | 后台执行 Action
  |                              | ...
  |                              |
  |<========= Event =============| ActionCompleted
  |                              |
  |       执行最终 callback       |
```

所以不能简单地认为：

```cpp
stub->StartAction(..., callback);
```

然后 callback 就会在业务动作完成后执行。

---

# 1. gRPC 的典型情况

假设服务端：

```protobuf
service DeviceService {
    rpc StartAction(StartRequest) returns (StartResponse);
}
```

服务端内部实际上是：

```cpp
StartResponse DeviceServiceImpl::StartAction(...)
{
    action_executor_.StartAsync();

    return StartResponse{
        .accepted = true
    };
}
```

那么客户端 gRPC callback：

```cpp
stub_->async()->StartAction(
    &context,
    request,
    &response,
    [](grpc::Status status) {
        // callback
    });
```

这个 callback 表示：

```text
StartAction RPC 已经完成
```

而不是：

```text
设备动作已经完成
```

因此：

```text
RPC callback
    ↓
服务端已经接受/处理 RPC
    ↓
不等于
    ↓
业务动作完成
```

---

# 2. 如果服务端本身就是异步处理

例如：

```cpp
StartAction()
{
    executor_.post([this] {
        do_real_action();
    });

    return OK;
}
```

那么时间关系是：

```text
T0 Client 发起 RPC
 |
T1 Server 收到 RPC
 |
T2 Server 把 Action 放入任务队列
 |
T3 Server 返回 RPC response
 |
T4 Client RPC callback
 |
 |      <------ 真正业务执行 ------>
 |
T5 Action 完成
```

所以：

```cpp
callback();
```

最多发生在：

```text
T4
```

而你真正想要的是：

```text
T5
```

这两个时间点必须区分。

---

# 3. 最常见的正确设计：RPC + Event

如果你的业务是你之前一直讨论的这种：

> 上位机 → 下位机
> RPC 下发动作
> 下位机执行动作
> Pub/Sub 上报最终状态

那么我非常建议采用：

```text
                RPC
Client ----------------------> Server
       StartAction
             |
             | RPC callback
             ↓
        Accepted
             |
             |
             |       Action
             |      execution
             |          ↓
             |     ActionCompleted
             |          |
             |          ↓
Client <------- Pub/Sub/Event
             |
             ↓
       Business callback
```

也就是说：

### RPC callback

负责：

```text
“命令是否成功送达/是否被接受”
```

### Event callback

负责：

```text
“动作是否真正完成”
```

这是两个不同的 callback。

---

# 4. 一个 C++17 的完整模型

例如定义：

```cpp
enum class ActionResult {
    Success,
    Failed,
    Timeout,
    Cancelled
};

using ActionCallback =
    std::function<void(ActionResult)>;
```

客户端：

```cpp
class DeviceController {
public:
    void start(ActionCallback callback)
    {
        auto request_id = generateRequestId();

        pending_actions_[request_id] = std::move(callback);

        rpc_client_.StartAction(
            request_id,
            [this, request_id](RpcResult result) {

                if (!result.ok()) {
                    complete(
                        request_id,
                        ActionResult::Failed);
                    return;
                }

                // 注意：
                // 这里不能调用真正的业务 callback
                //
                // 因为 RPC 成功 != Action 完成
            });
    }

private:
    void onActionCompleted(
        const std::string& request_id,
        ActionResult result)
    {
        complete(request_id, result);
    }

    void complete(
        const std::string& request_id,
        ActionResult result)
    {
        auto it = pending_actions_.find(request_id);

        if (it == pending_actions_.end()) {
            return;
        }

        auto callback = std::move(it->second);
        pending_actions_.erase(it);

        if (callback) {
            callback(result);
        }
    }

private:
    std::unordered_map<
        std::string,
        ActionCallback> pending_actions_;

    RpcClient rpc_client_;
};
```

然后 Pub/Sub：

```cpp
void DeviceController::onEvent(const Event& event)
{
    if (event.type != EventType::ActionCompleted) {
        return;
    }

    onActionCompleted(
        event.request_id,
        event.success
            ? ActionResult::Success
            : ActionResult::Failed);
}
```

最终：

```cpp
controller.start(
    [](ActionResult result) {

        if (result == ActionResult::Success) {
            std::cout << "动作真正完成\n";
        }
    });
```

整个生命周期：

```text
start()
   |
   +-- request_id = 123
   |
   +-- 保存 callback
   |
   +-- RPC StartAction
          |
          +---- RPC callback
                    |
                    +---- accepted
                              |
                              | 等待
                              ↓
                       Pub/Sub Event
                              |
                              ↓
                    ActionCompleted
                              |
                              ↓
                     查 request_id
                              |
                              ↓
                       取出 callback
                              |
                              ↓
                         callback()
```

---

# 5. brpc 的情况也是类似

Baidu RPC (brpc) 的 RPC done/callback 本质上也是围绕：

```text
RPC 是否完成
```

而不是自动知道：

```text
你的业务 Action 是否完成
```

例如：

```cpp
Controller cntl;
Request req;
Response resp;

service_stub_.StartAction(
    &cntl,
    &req,
    &resp,
    google::protobuf::NewCallback(
        &callback));
```

`callback` 被调用时，通常意味着：

```text
RPC 生命周期结束
```

如果服务端：

```cpp
void StartAction(...)
{
    executor_.submit([] {
        do_action();
    });

    return;
}
```

那么：

```text
brpc callback
```

仍然只能说明：

```text
StartAction RPC 完成
```

不能说明：

```text
do_action() 完成
```

---

# 6. 还有一种设计：让服务端一直保持 RPC

如果你非常希望：

```cpp
StartAction(..., callback);
```

这个 callback **天然就代表业务动作完成**，那么可以改变 RPC 语义。

例如：

```protobuf
rpc StartAction(StartRequest)
    returns (StartResponse);
```

服务端不要立即返回：

```cpp
StartResponse StartAction(...)
{
    start_action();

    return response;
}
```

而是：

```text
RPC Request
    ↓
Server
    ↓
启动异步动作
    ↓
等待动作完成
    ↓
构造 Response
    ↓
RPC Response
    ↓
Client callback
```

于是：

```text
RPC callback == Action completed
```

这其实就是一种：

> **异步业务执行 + RPC 长生命周期**

模型。

---

# 7. gRPC 特别适合的方式：Server Streaming

如果一个 Action 会产生多个阶段：

```text
Starting
    ↓
Running
    ↓
Stopping
    ↓
Completed
```

可以考虑：

```protobuf
rpc StartAction(StartRequest)
    returns (stream ActionEvent);
```

客户端：

```text
StartAction
    |
    +---- Starting
    |
    +---- Running
    |
    +---- Stopping
    |
    +---- Completed
```

这比单纯：

```text
RPC + callback
```

更适合长生命周期任务。

但是你的场景如果已经有 Pub/Sub，那么没有必要为了这个问题强行改成 streaming。

---

# 8. 对你目前这种“上位机 + 下位机”的架构，我更推荐第三种语义

结合你之前的状态机设计，我会把一次动作拆成三个阶段：

```text
                Command
                   │
                   ▼
             RPC Accepted
                   │
                   ▼
             Action Running
                   │
                   │ Pub/Sub
                   ▼
             Action Completed
```

对应 API：

```cpp
using AcceptCallback =
    std::function<void(RpcResult)>;

using CompleteCallback =
    std::function<void(ActionResult)>;
```

甚至可以统一成：

```cpp
class AsyncAction {
public:
    virtual void start() = 0;

    virtual void cancel() = 0;

    virtual bool completed() const = 0;

    virtual void onCompleted(
        CompleteCallback callback) = 0;
};
```

然后上层状态机：

```cpp
void StateMachine::start()
{
    action_->start(
        [this](ActionResult result) {

            if (result == ActionResult::Success) {
                transitionTo(State::Running);
            } else {
                transitionTo(State::Error);
            }
        });
}
```

这样**状态机根本不需要关心 RPC callback 是什么时候执行的**。

它只关心：

```text
Action 完成事件
```

这实际上是一个非常重要的架构边界：

```text
┌──────────────────────────────┐
│        State Machine         │
│                              │
│   Start → Running → Stop     │
└──────────────┬───────────────┘
               │
               │ Action abstraction
               ▼
┌──────────────────────────────┐
│       Action Controller      │
│                              │
│ RPC callback                 │
│ Pub/Sub event                │
│ timeout                      │
│ cancellation                 │
│ request_id correlation       │
└──────────────┬───────────────┘
               │
        ┌──────┴──────┐
        ▼             ▼
      gRPC           brpc
```

**这里最关键的一句话是：**

> **不要让业务层 callback 直接等同于 RPC callback。RPC callback 是通信层事件；业务 callback 应该由“动作完成事件”驱动。**

尤其是你之前提到的“RPC 异步调用只表示 accepted，最终必须等待下位机 Pub/Sub 状态”的场景，这种分层会比把所有逻辑塞进 RPC callback 可靠得多。


可以，而且你问到的是 **RPC 框架内部“请求完成点（completion point）”的定制**。

如果你的目标是：

> **服务端已经收到 RPC，但不要马上把 RPC response / callback 通知客户端，而是等业务动作真正完成后，再让 RPC 完成，从而触发客户端 callback。**

那么本质上不是“延迟 callback”，而是：

> **延迟 RPC 的 completion。**

---

## 1. 先看标准 RPC 的生命周期

普通同步 RPC：

```text
Client                         Server
  │                              │
  │────── Request ──────────────>│
  │                              │
  │                              │ 业务处理
  │                              │
  │<────── Response ─────────────│
  │                              │
  │        callback()            │
  │                              │
```

如果服务端改成异步：

```text
Client                         Server
  │                              │
  │──── StartAction ────────────>│
  │                              │
  │<──── Accepted ───────────────│
  │                              │
  │        callback()            │
  │                              │
  │                              │
  │                         Action执行
  │                              │
  │                         Action完成
```

你现在想要的是：

```text
Client                         Server
  │                              │
  │──── StartAction ────────────>│
  │                              │
  │                              │
  │                         Action执行
  │                              │
  │                         Action完成
  │                              │
  │<──── Response ───────────────│
  │                              │
  │        callback()            │
```

也就是：

```text
RPC Request
    │
    ▼
Server IO Thread
    │
    ├── 创建 AsyncRequest
    │
    └── 不完成 RPC
             │
             ▼
        Business Worker
             │
             │
             ▼
        Action Completed
             │
             ▼
       Complete RPC
             │
             ▼
        IO Thread发送Response
             │
             ▼
       Client callback
```

---

# 2. 最重要的概念：不要阻塞 IO 线程

有一个非常容易踩的坑：

```cpp
void Service::StartAction(...)
{
    do_action();  // 阻塞很久
}
```

这虽然可以让 callback 延迟：

```text
do_action()
    ↓
return
    ↓
callback
```

但这是**错误的异步实现**。

因为：

```text
IO Thread
   │
   ├── 收到 RPC
   │
   ├── do_action()
   │       ↓
   │      5秒
   │       ↓
   │
   └── return
```

这个 IO thread 被占用了。

高并发下：

```text
IO Thread 1 → Action A → 阻塞
IO Thread 2 → Action B → 阻塞
IO Thread 3 → Action C → 阻塞
...
```

最终整个 RPC server 被拖死。

---

# 3. 正确方案是“挂起 RPC”

也就是：

```text
IO Thread
    │
    │ 收到 RPC
    ▼
创建一个 PendingRpc
    │
    │
    ├──────────────→ Worker Thread
    │                       │
    │                       │ 执行动作
    │                       │
    │                       ▼
    │                  ActionCompleted
    │                       │
    │                       ▼
    │                Complete PendingRpc
    │
    │
    ▼
IO Thread
发送 Response
```

注意：

> **RPC 在逻辑上还没有完成，但 IO 线程已经返回去处理其他 RPC 了。**

这是关键。

---

# 4. 可以抽象一个 AsyncRpcContext

例如：

```cpp
class AsyncRpcContext {
public:
    virtual ~AsyncRpcContext() = default;

    virtual void complete() = 0;

    virtual void fail(int error_code) = 0;
};
```

Service：

```cpp
class DeviceService {
public:
    void StartAction(
        AsyncRpcContext* rpc)
    {
        auto action = std::make_shared<Action>();

        pending_[action->id()] = rpc;

        executor_.submit(
            [this, action] {

                action->run();

                onActionCompleted(
                    action->id());
            });
    }

private:
    void onActionCompleted(ActionId id)
    {
        auto it = pending_.find(id);

        if (it == pending_.end()) {
            return;
        }

        auto* rpc = it->second;

        pending_.erase(it);

        rpc->complete();
    }

private:
    std::unordered_map<
        ActionId,
        AsyncRpcContext*> pending_;

    ThreadPool executor_;
};
```

但这里还有一个非常重要的问题：

```cpp
AsyncRpcContext*
```

的生命周期。

不能假设：

```text
Service函数返回
    ↓
rpc context还永远有效
```

因此真正工业级实现通常需要：

```cpp
std::shared_ptr<AsyncRpcContext>
```

或者由 RPC framework 提供专门的：

```text
RequestContext
Closure
Controller
CallData
CompletionQueue
```

来管理生命周期。

---

# 5. gRPC 最适合用 CompletionQueue 做这种模型

gRPC 的异步 API 本身就是围绕：

```text
Request
CompletionQueue
Tag
State Machine
```

设计的。

典型思想：

```text
CQ thread
   │
   │ RPC arrived
   ▼
CallData
   │
   ├── CREATE
   ├── PROCESS
   └── FINISH
```

可以把它理解成：

```cpp
class CallData {
public:
    void Proceed(bool ok)
    {
        switch (state_) {

        case State::Create:
            requestRpc();

            state_ = State::Process;
            break;

        case State::Process:
            executeAsyncAction();
            break;

        case State::Finish:
            finishRpc();
            break;
        }
    }
};
```

关键就在于：

```text
Process
   │
   │ 启动业务动作
   │
   └── 不进入 Finish
```

等业务完成：

```cpp
void onActionCompleted()
{
    state_ = State::Finish;

    responder_.Finish(
        response_,
        grpc::Status::OK,
        this);
}
```

于是：

```text
Action完成
    ↓
Finish()
    ↓
gRPC CompletionQueue
    ↓
Response发送
    ↓
Client callback
```

这就是你要的效果。

---

# 6. gRPC 中可以做成真正的“业务异步 RPC”

例如：

```text
              gRPC
Client ────────────────────────> Server
                                  │
                                  │
                                  ▼
                            CallData::Process
                                  │
                                  │
                                  ▼
                           ActionManager
                                  │
                                  │
                            asynchronous
                                  │
                                  ▼
                            Device Action
                                  │
                                  ▼
                              Complete
                                  │
                                  ▼
                         CallData::Finish
                                  │
                                  ▼
                            gRPC Response
                                  │
                                  ▼
Client <──────────────────────────┘
        callback()
```

于是客户端：

```cpp
stub->PrepareAsyncStartAction(
    ...,
    &cq);
```

最终：

```cpp
callback()
```

确实可以代表：

```text
业务动作已经完成
```

而不是：

```text
RPC request accepted
```

---

# 7. 这其实比“RPC + Pub/Sub + Client callback”更简单

你前一个问题中的模型是：

```text
RPC callback
      ↓
Accepted

Pub/Sub
      ↓
ActionCompleted
      ↓
Business callback
```

如果你能够控制 RPC Server，而且动作生命周期比较明确，那么可以进一步封装成：

```text
RPC
 │
 ▼
AsyncRpc
 │
 ▼
Action
 │
 ▼
ActionCompleted
 │
 ▼
RPC Complete
 │
 ▼
Client callback
```

这样客户端看到的就是：

```cpp
client.start(
    [](Result result) {
        // 这里就是 Action 真正完成
    });
```

客户端完全不需要知道：

```text
RPC callback
Pub/Sub
Action event
Server worker
```

这些内部细节。

---

# 8. 但是有一个非常重要的问题：RPC 超时

假设：

```text
Client
 │
 │ StartAction
 ▼
Server
 │
 │
 │ Action执行
 │
 │────────────── 30s
 │
```

客户端可能：

```text
deadline = 5s
```

那么：

```text
T0  Start RPC
T1  Server开始Action
T5  Client deadline exceeded
T6  Server Action仍然执行
T30 Action完成
```

这时候如果你只是简单地保存 RPC：

```cpp
pending_rpc_[id] = rpc;
```

就会出现：

```text
Client已经不要这个RPC了
             ↓
Server 30秒后才完成
             ↓
还试图发送Response
```

所以必须设计：

```text
RPC lifetime
Action lifetime
```

两者不能简单绑定。

---

# 9. 我更推荐做一个“两阶段生命周期”

```text
                    ┌──────────────┐
                    │ RPC Context  │
                    └──────┬───────┘
                           │
                           ▼
                    Request Accepted
                           │
                           │
                    ┌──────▼───────┐
                    │ Action       │
                    │ Context      │
                    └──────┬───────┘
                           │
                           ▼
                    Action Running
                           │
                 ┌─────────┴─────────┐
                 ▼                   ▼
             Completed            Cancelled
                 │                   │
                 └─────────┬─────────┘
                           ▼
                      Complete RPC
```

同时：

```text
RPC timeout
     │
     ├── RPC失效
     │
     └── Action是否取消？
             │
          可配置
```

比如：

```cpp
struct ActionContext {
    ActionId id;

    std::atomic<bool> cancelled{false};

    // RPC已经断开
    std::atomic<bool> rpc_alive{true};

    // Action本身完成
    std::atomic<bool> completed{false};
};
```

这样就可以支持：

```text
客户端取消
      ↓
RPC结束
      ↓
Action继续执行
```

或者：

```text
客户端取消
      ↓
RPC结束
      ↓
cancel Action
      ↓
Action停止
```

---

# 10. brpc 也可以做类似设计

Baidu RPC (brpc) 的核心思想同样是：

```text
不要在 RPC handler 中阻塞等待业务完成
```

而是：

```text
RPC handler
    │
    ├── 保存请求上下文
    │
    ├── 启动异步任务
    │
    └── return
          │
          │
          ▼
       Worker
          │
          ▼
      Action完成
          │
          ▼
      触发RPC完成
```

具体到 brpc 内部，你需要围绕它的 `Controller`、`Closure`、异步 RPC 生命周期以及 server response 的发送时机做封装，而不是简单地：

```cpp
done->Run();
```

提前执行。

---

# 11. 如果你想“自定义 RPC 框架”，我建议直接抽象成 Completion Token

这是我认为非常适合你现在 C++ 架构学习/实践的设计。

定义：

```cpp
class RpcCompletion {
public:
    virtual ~RpcCompletion() = default;

    virtual void complete(
        const RpcResponse& response) = 0;

    virtual void cancel() = 0;

    virtual bool isAlive() const = 0;
};
```

RPC Server：

```cpp
void Service::StartAction(
    Request request,
    std::shared_ptr<RpcCompletion> completion)
{
    auto action = action_manager_.create(
        request.action_id);

    action->start(
        [completion](
            ActionResult result) {

            if (!completion->isAlive()) {
                return;
            }

            RpcResponse response;
            response.result = result;

            completion->complete(response);
        });
}
```

于是 RPC 层和业务层之间变成：

```text
                 RPC Layer
                     │
              RpcCompletion
                     │
                     ▼
              ActionManager
                     │
                     ▼
                Action
                     │
                     ▼
              ActionCompleted
                     │
                     ▼
              RpcCompletion
                     │
                     ▼
                 RPC IO
                     │
                     ▼
                  Client
```

这个结构非常干净。

---

# 12. 最值得注意的是：不要真的“延缓 IO 线程”

你的原话是：

> **让服务端的 IO 线程能够延缓回调通知时机**

这里我建议稍微改一下概念。

不要：

```text
IO Thread
   │
   ├── RPC
   │
   ├── 等待 10 秒
   │
   └── callback
```

而应该：

```text
IO Thread
   │
   ├── RPC
   │
   └── 注册 Completion
             │
             ▼
        立即释放 IO Thread

             ↓
       Business Thread
             │
             ▼
          Action
             │
             ▼
          Complete
             │
             ▼
       Schedule IO Response
             │
             ▼
          Client
```

也就是说：

> **延迟的是 RPC completion，不是占住 IO thread 等待。**

这是整个设计最核心的区别。

---

## 13. 如果套到你现在的“上位机—下位机—状态机”架构

我甚至建议你把 RPC 封装成：

```cpp
class IAsyncRpc {
public:
    virtual ~IAsyncRpc() = default;

    virtual ActionHandle start(
        const StartRequest& request,
        CompletionCallback callback) = 0;
};
```

底层：

```text
IAsyncRpc
   │
   ├── GrpcAsyncRpc
   │
   └── BrpcAsyncRpc
```

业务层：

```text
StateMachine
      │
      ▼
ActionManager
      │
      ▼
IAsyncRpc
      │
 ┌────┴─────┐
 ▼          ▼
gRPC       brpc
```

然后统一语义：

```cpp
rpc.start(request,
    [](ActionResult result) {
        // 必须保证：
        // 这里表示业务动作完成
    });
```

底层 gRPC/brpc 的：

```text
RPC callback
CompletionQueue
Closure
Controller
Pub/Sub
timeout
cancel
```

全部隐藏在 `GrpcAsyncRpc` / `BrpcAsyncRpc` 内部。

这样你就真正实现了：

> **自定义 RPC 语义，而不是修改 RPC 框架本身。**

如果你的目标是深入理解这个机制，我下一步建议直接做一个**“C++17 自制 Mini-RPC”**：实现 `IO Thread + Worker Thread + AsyncRpcContext + CompletionQueue + request_id + timeout + cancel + delayed response`，然后分别映射到 **gRPC CompletionQueue 和 brpc Closure**。这样能非常直观地看清楚“RPC callback 到底是谁、什么时候、在哪个线程触发”的完整生命周期。



# 光刻机洞察报告：一台机器，为什么能够成为现代工业技术的“缩影”

**报告时间：2026 年 9 月**
**观察对象：以 ASML EUV / High-NA EUV 为核心，同时观察 DUV、光学、光源、计量、软件与产业链**

![Image](https://edge.sitecorecloud.io/asmlnetherlaaea-asmlcom-prd-5369/media/project/asmlcom/asmlcom/asml/images/products/euv-lithography-systems/nxe3400_simplified_front_semiclosed.jpg?h=1080\&iar=0\&w=1920)

![Image](https://edge.sitecorecloud.io/asmlnetherlaaea-asmlcom-prd-5369/media/project/asmlcom/asmlcom/asml/images/technology/43678-inside-nxe3400-wafer-handler.png?mw=1920)

![Image](https://edge.sitecorecloud.io/asmlnetherlaaea-asmlcom-prd-5369/media/project/asmlcom/asmlcom/asml/images/news/stories/2024/high-na/high-na-euv-still.png?h=619\&iar=0\&w=1195)

## 一、核心结论

如果只把光刻机理解成“把电路图案印到晶圆上的机器”，很容易低估它真正的技术含量。

更准确的理解是：

> **先进光刻机本质上是一套把光学、精密机械、真空、激光、材料、控制、计量、算法、软件和供应链组织成一个闭环的超复杂系统。**

它最值得研究的地方，不只是 **EUV 13.5 nm、0.55 NA、纳米级定位**这些参数，而是：

**它如何把大量单项技术，在工业现场组织成一个可重复、可维护、可量产的系统。**

ASML 2025 年度报告显示，公司 2025 年销售额约 **327 亿欧元**、研发投入约 **47 亿欧元**，供应商数量约 **5,100 家**，员工超过 **44,000 人**。这说明光刻机产业不是传统意义上的“大型设备制造”，而更接近一个高度协同的工业技术生态。([ASML][1])

而当前技术路线正在从传统 EUV 向 **High-NA EUV** 演进：ASML 的 High-NA 平台将 NA 从 0.33 提升到 0.55，目标覆盖未来先进 Logic 和 Memory 节点。ASML 表示 EXE 平台面向 2025–2026 年的高量产导入。([ASML][2])

因此，研究光刻机，真正值得关注的不是：

> **“光刻机有多精密？”**

而是：

> **“人类是如何把数十万甚至更多工程约束，组织成一个稳定运行的闭环系统的？”**

这恰恰是光刻机对软件工程、系统工程和 C++ 工程师最有价值的地方。

---

# 二、先建立一个正确的光刻机认知模型

可以把现代光刻系统抽象成：

```text
                 ┌────────────────────┐
                 │      光源系统       │
                 │ Laser / EUV Source │
                 └─────────┬──────────┘
                           │
                           ▼
                 ┌────────────────────┐
                 │      照明系统       │
                 │ Illumination       │
                 └─────────┬──────────┘
                           │
                           ▼
                 ┌────────────────────┐
                 │      掩模系统       │
                 │ Reticle / Mask     │
                 └─────────┬──────────┘
                           │
                           ▼
                 ┌────────────────────┐
                 │      投影光学       │
                 │ Projection Optics  │
                 └─────────┬──────────┘
                           │
                           ▼
                    ┌──────────────┐
                    │    Wafer     │
                    │    晶圆       │
                    └──────────────┘


     ┌─────────────┐       ┌───────────────┐
     │ 运动控制系统 │◄─────►│ 传感器/计量系统 │
     └─────────────┘       └───────────────┘
             ▲                       │
             │                       ▼
             └──────────┬────────────────
                        │
                  ┌─────▼─────┐
                  │ 控制软件   │
                  │ Algorithms │
                  └─────┬─────┘
                        │
                  ┌─────▼─────┐
                  │ 系统管理   │
                  │ Diagnostics│
                  │ Calibration│
                  └───────────┘
```

这幅图非常重要。

因为光刻机真正困难的地方，从来不是某一个模块，而是：

> **模块之间存在强耦合，而且每一个模块都要求极高性能。**

例如，晶圆台位置变化，不只是“机械问题”；它会影响：

* 曝光位置
* overlay
* focus
* vibration
* thermal drift
* image distortion
* wafer-to-wafer consistency

于是一个机械误差最终会变成芯片良率问题。

这就是典型的**系统工程问题**。

---

# 三、为什么要使用 13.5 nm EUV？

光刻的核心物理关系可以从 Rayleigh 公式理解：

$$
R = k_1 \frac{\lambda}{NA}
$$

其中：

* \(R\)：可实现的分辨率
* \(\lambda\)：光波长
* NA：数值孔径
* \(k_1\)：工艺相关系数

因此，想让晶体管继续缩小，大体有两条路：

```text
降低 λ
     ↓
提高 NA
     ↓
降低 k1
```

传统 DUV 主要使用：

* 365 nm
* 248 nm
* 193 nm

其中 ArF 193 nm 是先进 DUV 的核心波长。ASML 资料显示，ArF 系统通过浸没式光刻可以达到 NA 1.35；而 EUV 采用 13.5 nm 波长，即使 NA 为 0.33，也能够实现更先进的特征尺寸。([ASML][3])

EUV 的关键变化不是简单地：

> “把灯换成更短波长的灯。”

而是：

> **整个光学体系发生范式变化。**

因为 13.5 nm EUV 光会被几乎所有材料吸收。

所以 EUV 不能像传统光学系统那样：

```text
空气
 ↓
透镜
 ↓
晶圆
```

而变成：

```text
真空
 ↓
反射镜
 ↓
反射镜
 ↓
反射镜
 ↓
晶圆
```

ASML 与 ZEISS 的 EUV 光学系统因此大量采用多层反射镜，并在真空中工作。ZEISS 公开资料显示，High-NA EUV 投影光学系统包含超过 **40,000 个部件**，重量约 **12 吨**。([Zeiss][4])

这说明一个非常深刻的问题：

> **先进技术往往不是“增加一个更强的组件”，而是整个系统架构一起变化。**

---

# 四、EUV 最大的挑战，其实是“怎么产生光”

EUV 的光源并不是普通激光器直接产生的。

ASML 当前 EUV 光源采用 **Laser Produced Plasma，LPP**。

其基本过程可以抽象成：

```text
液态锡微滴
   │
   ▼
高速喷射
   │
   ▼
激光第一次击中
   │
   ▼
锡滴压扁
   │
   ▼
第二次高功率激光
   │
   ▼
形成高温等离子体
   │
   ▼
产生 13.5 nm EUV
```

ASML 公开资料显示，液态锡滴直径约 25 微米，以约 70 m/s 的速度喷射；随后由激光精准击中并产生等离子体。为了获得足够的 EUV 光源输出，这一过程需要每秒重复数万次。其最新商业光源公开资料描述为约 **60,000 次/秒**。([ASML][5])

这时候就出现了一个非常有意思的问题：

### 你到底在控制什么？

并不是简单地控制：

```cpp
laser.fire();
```

而更接近：

```text
Tin droplet generation
        ↓
Position sensing
        ↓
Velocity estimation
        ↓
Laser timing prediction
        ↓
Pulse shaping
        ↓
Plasma generation
        ↓
EUV power measurement
        ↓
Feedback correction
        ↓
Next pulse
```

因此它已经不是传统的：

> **控制 → 执行**

而是：

> **测量 → 建模 → 预测 → 执行 → 再测量 → 校正**

这就是现代高端装备的软件本质之一。

---

# 五、光刻机真正的核心：闭环，而不是开环

这是我认为研究光刻机最值得获得的第一个洞察。

普通自动化设备经常是：

```text
Command
   ↓
Actuator
   ↓
Machine
```

而先进光刻系统更接近：

```text
                ┌──────────────┐
                │   Target     │
                └──────┬───────┘
                       ↓
                ┌──────────────┐
                │ Controller   │
                └──────┬───────┘
                       ↓
                ┌──────────────┐
                │   Actuator   │
                └──────┬───────┘
                       ↓
                ┌──────────────┐
                │ Physical Sys │
                └──────┬───────┘
                       ↓
                ┌──────────────┐
                │   Sensors    │
                └──────┬───────┘
                       ↓
                ┌──────────────┐
                │ Estimation   │
                └──────┬───────┘
                       │
                       └──────────► Controller
```

这意味着：

> **机器不是“知道状态后执行动作”，而是持续观察世界，并不断修正自己的行为。**

ASML 资料中就公开提到，其晶圆台可以在每次曝光过程中定位到亚纳米量级，并进行高频测量和调整；NXE 系统还会结合 wafer 上的测量结果进行逐片校正。([ASML][2])

对于软件工程师，这个思想非常重要。

它对应到软件设计就是：

```text
Command
State
Measurement
Model
Control
Correction
Fault
Recovery
```

这已经非常接近：

**实时控制系统 + 状态机 + 反馈系统 + 故障管理。**

---

# 六、为什么“0.1 nm”比“1 nm”难很多？

很多人看到“纳米级精度”会简单理解成：

> 把机械做得更精密。

实际上远远不是。

现实系统里面存在：

* 机械振动
* 温度变化
* 材料热膨胀
* 气流
* 电磁噪声
* 激光噪声
* 光学误差
* 结构形变
* 传感器误差
* 执行器误差
* 计算延迟
* 时间同步误差

假设：

```text
机械误差        0.5 nm
热漂移          0.3 nm
传感器误差      0.2 nm
控制误差        0.2 nm
结构变形        0.4 nm
```

问题不是：

```text
0.5 + 0.3 + ...
```

这么简单。

这些误差之间可能：

* 相关
* 耦合
* 随时间变化
* 随温度变化
* 随位置变化
* 随工艺状态变化

于是系统工程真正面对的是：

$$
Error=f(position,time,temperature,vibration,process,...)
$$

所以先进装备的核心能力之一是：

> **建立误差模型，并实时识别误差。**

这使得软件从传统的：

```cpp
if (...)
    doSomething();
```

逐步变成：

```text
Measurement
      ↓
State Estimation
      ↓
Model Update
      ↓
Prediction
      ↓
Control
      ↓
Correction
```

---

# 七、High-NA EUV：为什么“更强”反而带来更多问题？

这是光刻技术非常有意思的一点。

传统 EUV：

```text
NA = 0.33
```

High-NA EUV：

```text
NA = 0.55
```

理论上：

```text
NA ↑
→ Resolution ↑
```

但 NA 增加后，整个光学系统也要变得更大、更复杂。

ZEISS 给出的 High-NA 信息非常有代表性：

* illumination system 超过 6 吨
* projection optics 约 12 吨
* 投影光学系统超过 40,000 个部件
* 光学镜面尺寸更大
* 镜面制造精度进一步提高

而且 High-NA 使用 anamorphic optics，因此曝光场发生变化，某些情况下需要更多曝光次数。([Zeiss][4])

这体现出一个典型工程规律：

> **一个性能指标被推高之后，系统往往会沿着其他维度“反弹”。**

例如：

```text
Resolution ↑
    ↓
NA ↑
    ↓
Optics size ↑
    ↓
Mass ↑
    ↓
Dynamics ↓
    ↓
Stage challenge ↑
    ↓
Control challenge ↑
    ↓
Software complexity ↑
```

所以高端工程永远不是：

> “优化一个指标”。

而是：

> **在多个互相冲突的指标之间寻找系统最优解。**

---

# 八、真正让人震撼的是：软件其实是光刻机的一部分

大众经常认为：

> ASML = 光学 + 机械。

其实这并不完整。

ASML 自己明确把软件描述成其光刻系统的重要组成部分，并指出，没有先进软件，越来越小的芯片结构无法实现稳定生产。

可以把先进光刻机理解为：

```text
                   Lithography System
                           │
        ┌──────────────────┼──────────────────┐
        ↓                  ↓                  ↓
     Physics            Hardware           Software
        │                  │                  │
      Optics             Motion            Control
      Plasma             Vacuum             Planning
      Materials          Mechatronics       Optimization
      Chemistry          Sensors            Diagnostics
      Thermal            Electronics        Data
                           │                  │
                           └────────┬─────────┘
                                    ↓
                              Manufacturing
```

这里的软件并不是普通意义上的：

```text
UI
+
业务逻辑
+
数据库
```

而更接近：

```text
实时控制
+
状态估计
+
运动规划
+
误差补偿
+
参数管理
+
故障诊断
+
校准
+
优化
+
数据分析
+
设备管理
```

这就是为什么先进工业设备的软件工程，实际上与：

* Robotics
* Real-time systems
* Control systems
* Distributed systems
* Embedded systems
* Scientific computing

存在大量交叉。

---

# 九、光刻机最大的秘密：它是一个“系统之系统”

如果拆解 ASML 的生态，会发现它并不是一个公司独立完成全部技术。

典型结构类似：

```text
                     ASML
                       │
       ┌───────────────┼────────────────┐
       ↓               ↓                ↓
    Optics           Source          Mechatronics
       │               │                │
     ZEISS           Cymer           multiple
       │
       └───────────────┬────────────────┘
                       ↓
                  ASML System
                       ↓
              Semiconductor Fab
```

例如 ZEISS 是关键光学合作伙伴；EUV 光源则涉及 ASML / Cymer 等技术体系。([Zeiss][6])

这意味着一个重要事实：

> **真正难以复制的，不一定是一项技术，而是整个技术网络。**

ASML 2025 年度资料显示，其供应商数量约 **5,100 家**。([ASML][1])

所以“造出一台机器”和“建立能够持续迭代、量产、服务全球客户的工业体系”，完全是两回事。

---

# 十、为什么光刻机不是“技术越先进越好”

还有一个容易被忽略的事实：

## 芯片制造不是只需要 EUV。

ASML 自己也明确指出，一颗先进芯片可能拥有上百层结构，不同层使用不同类型的光刻系统；先进 EUV、ArF、KrF、i-line 等技术长期共存。([Zeiss][7])

也就是说：

```text
EUV
 ↓
先进关键层

DUV
 ↓
大量成熟层

KrF / i-line
 ↓
更大尺寸结构

其他工艺
 ↓
封装 / 特殊器件
```

这体现另一个工业规律：

> **先进系统从来不是由单一“最强技术”组成，而是由不同成熟度技术组合而成。**

这和大型软件系统极其相似。

一个工业软件平台里：

```text
高性能核心模块
+
成熟基础设施
+
旧系统接口
+
工具链
+
监控
+
运维
```

往往比“全部重写成最新技术”更可靠。

---

# 十一、为什么光刻机的竞争实际上也是“时间”的竞争

很多人分析半导体设备，只看：

```text
能不能做
```

而真正工业化时还要问：

```text
能不能连续做？
能不能稳定做？
能不能高速做？
能不能低缺陷做？
能不能维护？
能不能快速恢复？
能不能复制到 100 台？
```

所以先进设备实际上同时优化多个指标：

$$
Technology =
Resolution
+
Throughput
+
Overlay
+
Availability
+
Yield
+
Cost
+
Maintainability
$$

例如：

**分辨率提高 10%** 并不一定比：

**产能提高 20%**

更有商业价值。

这也是为什么光刻机一定要关注：

* throughput
* uptime
* MTBF
* MTTR
* calibration
* service
* predictive maintenance

而不仅仅是“最小线宽”。

---

# 十二、光刻机的另一个核心：计量系统

光刻机并不是：

> “执行曝光，然后结束。”

而是：

```text
曝光
 ↓
测量
 ↓
发现误差
 ↓
模型更新
 ↓
修正下一次曝光
```

这实际上是在构建一个：

> **机器自我观测、自我校正的系统。**

所以未来先进制造装备越来越像：

```text
Machine
   +
Sensors
   +
Software
   +
Models
   +
Data
```

最终形成：

> **Cyber-Physical System（信息物理系统）**

这一点对于未来智能制造非常关键。

---

# 十三、为什么光刻机特别适合拿来学习 C++ 架构

如果从你的 C++ 开发方向来看，光刻机甚至可以作为一个非常好的系统工程学习对象。

可以人为构建：

```text
LithographyMachine
│
├── Source
│   ├── Laser
│   ├── TinDropletGenerator
│   ├── Plasma
│   └── SourceMonitor
│
├── Optics
│   ├── Illumination
│   ├── Projection
│   └── Mirror
│
├── WaferStage
│   ├── PositionController
│   ├── Servo
│   └── PositionSensor
│
├── ReticleStage
│
├── Vacuum
│
├── Metrology
│   ├── Focus
│   ├── Overlay
│   └── Alignment
│
├── Thermal
│
├── Diagnostics
│
├── Scheduler
│
├── Recipe
│
└── StateMachine
```

然后逐渐引入真正复杂的问题。

第一阶段：

```cpp
machine.start();
machine.expose();
machine.stop();
```

第二阶段：

```cpp
machine.start();
    ↓
Source::start()
    ↓
Vacuum::ready()
    ↓
Stage::initialize()
    ↓
Optics::initialize()
    ↓
Calibration
    ↓
Ready
```

第三阶段：

```text
启动任务
       │
       ├── Source
       ├── Vacuum
       ├── Stage
       └── Sensors
               │
               ▼
            Ready
```

第四阶段加入异步事件：

```text
RPC Start
   ↓
Task Start
   ↓
Lower subsystem
   ↓
Async accepted
   ↓
wait event
   ↓
Published status
   ↓
validate
   ↓
Next step
```

第五阶段加入：

```text
Start
 │
 ├── Step 1
 ├── Step 2
 ├── Step 3
 ├── Step 4
 └── Step 5
       ↑
       │
      Stop
       │
       ↓
   Cancellation
```

这与你当前关注的：

* RPC
* Pub/Sub
* FSM
* 异步回调
* 状态映射
* 超时
* cancellation
* 组件状态与业务状态分离

实际上高度吻合。

---

# 十四、从光刻机可以学到的第一个软件架构原则：不要让“状态”变成垃圾桶

高级设备软件通常不会只有一个：

```cpp
State state;
```

更合理的是：

```text
System State
Component State
Process State
Health State
Safety State
Fault State
```

例如：

```text
System:
    Ready

Source:
    Running

Vacuum:
    Stable

WaferStage:
    Calibrated

Exposure:
    Idle

Health:
    Warning

Safety:
    Normal
```

最终系统状态并不是：

```cpp
system.state = Ready;
```

这么简单。

而更像：

```text
SystemState =
    f(
        component states,
        safety,
        process state,
        fault,
        interlock,
        calibration,
        connectivity
    )
```

这正是复杂设备软件里非常典型的状态建模问题。

---

# 十五、第二个软件架构原则：不要用一张巨型状态表表达整个世界

大型设备如果采用：

```cpp
map<
    tuple<
        SourceState,
        VacuumState,
        StageState,
        OpticsState,
        ...
    >,
    FinalState
>
```

理论上可以做。

但随着子系统增加：

$$
N=N_1N_2N_3...N_k
$$

状态空间呈组合爆炸。

更好的方向是：

```text
Local State Machine
        +
Capability
        +
Constraint
        +
Workflow
        +
Global Policy
```

例如：

```text
ExposureAllowed =
    Source.Ready
    &&
    Vacuum.Stable
    &&
    Stage.Calibrated
    &&
    Reticle.Valid
    &&
    Safety.Normal
```

再进一步：

```text
Constraint:
    Source must be Ready

Forbidden:
    Stage Moving && Exposure Active

Exception:
    Calibration Mode may bypass normal exposure constraint
```

这比无限扩大的：

```cpp
StateCombination -> State
```

更接近真实工业系统。

---

# 十六、第三个软件架构原则：命令、状态和事件必须分开

复杂设备中：

```text
Command != State != Event
```

例如：

```text
Start()
```

不是：

```text
Running
```

而可能是：

```text
StartCommand
    ↓
Accepted
    ↓
Starting
    ↓
Subsystem Event
    ↓
Running
```

同样：

```text
StopCommand
```

也不意味着：

```text
Stopped
```

而应该经历：

```text
Stop Requested
     ↓
Stopping
     ↓
lower subsystem event
     ↓
Stopped
```

这对 RPC + Pub/Sub 系统尤其重要。

---

# 十七、第四个软件架构原则：不要把“异步调用成功”当成“业务动作完成”

这是工业设备软件非常容易犯的错误。

例如：

```cpp
rpc->openAsync();
```

返回：

```text
RPC success
```

只意味着：

> 请求被接受。

而不是：

> 设备已经打开。

真正的流程可能是：

```text
RPC accepted
    ↓
Lower subsystem executing
    ↓
Pub/Sub event
    ↓
Status change
    ↓
Validation
    ↓
Business transition
```

这与普通 Web 服务的：

```text
HTTP 200
```

完全不同。

工业设备的：

> **命令完成**

通常需要一个可观测的物理状态证据。

---

# 十八、光刻机为什么如此昂贵：真正昂贵的是“极端约束同时成立”

假设我们分别解决：

```text
光源
机械
光学
控制
真空
软件
计量
```

每一个都做到了 99.9%。

是不是就够了？

并不一定。

假设：

```text
模块 A 可靠度 = 99.9%
模块 B       = 99.9%
模块 C       = 99.9%
模块 D       = 99.9%
模块 E       = 99.9%
```

多个模块组成系统之后，系统级可靠性不会自动保持 99.9%。

真正困难的是：

> **如何让这些模块在边界条件、异常、漂移和长期运行中仍然协同工作。**

因此先进装备的价值很大程度来自：

```text
Integration
+
Validation
+
Calibration
+
Characterization
+
Service
```

而不是单纯：

```text
Component Technology
```

---

# 十九、产业竞争真正竞争的是什么？

从光刻机来看，技术竞争至少可以拆成六层：

```text
Level 1
基础科学
│
├── 光学
├── 等离子体
├── 材料
└── 激光

Level 2
核心零部件
│
├── 镜片/镜面
├── 激光器
├── 传感器
├── 执行器
└── 精密机械

Level 3
Subsystem
│
├── Source
├── Optics
├── Stage
├── Vacuum
└── Metrology

Level 4
System Integration
│
├── Control
├── Software
├── Calibration
└── Diagnostics

Level 5
Manufacturing
│
├── Yield
├── Throughput
├── Reliability
└── Service

Level 6
Ecosystem
│
├── Suppliers
├── Customers
├── Process
├── Talent
└── R&D Network
```

越往下面，越难复制。

这也是为什么：

> **“拥有某个技术”与“拥有完整工业能力”是两个完全不同的概念。**

---

# 二十、竞争格局：EUV 为什么形成了极高壁垒

目前 ASML 是全球唯一提供 EUV 量产光刻系统的厂商；Nikon 和 Canon 仍然在 DUV 等光刻领域拥有产品和技术布局。Nikon 当前的 ArF immersion 系统公开指标包括 193 nm、NA 1.35 和 38 nm 级分辨率规格。([Zeiss][4])

另一方面，Canon 正在推进另一条路线——**Nanoimprint Lithography（NIL）**。

NIL 不使用传统意义上的投影曝光，而是类似“纳米级印章”直接把图案转移到晶圆上。Canon 当前 FPA-1200NZ2C 公开资料给出的最小线宽为 14 nm，并将其定位于先进逻辑、存储等方向。([Canon Global][8])

这说明未来未必是：

```text
EUV
  ↓
High-NA
  ↓
更高 NA
```

这一条路线无限延伸。

也可能出现：

```text
EUV
     ├── High-NA
     └── Further optical evolution

NIL
     └── imprint

Multi-patterning
     └── process optimization

Other patterning technologies
```

未来的竞争可能越来越表现为：

> **不同物理路线 + 软件算法 + 工艺能力之间的组合竞争。**

---

# 二十一、地缘政治进一步说明了：光刻机已经不是普通设备

半导体制造设备已经成为全球科技竞争的重要基础设施。

美国近年来持续扩大先进半导体制造设备及相关软件的出口管制范围，涉及光刻、刻蚀、沉积、计量检测等设备。2025 年 BIS 又调整了部分针对中国境内外资晶圆厂的相关许可政策；2026 年 BIS 还对 Applied Materials 涉及中国的设备出口案件作出了高额处罚。

这意味着：

> **先进制造设备本身已经成为国家级战略能力的一部分。**

因此光刻机的价值，已经超出了“卖一台机器赚多少钱”。

它涉及：

```text
Computing
+
Semiconductor
+
AI
+
Industrial capability
+
National security
+
Supply chain
```

---

# 二十二、但真正值得注意的是：摩尔定律可能正在发生变化

传统摩尔定律主要依赖：

```text
更小 transistor
      ↓
更多 transistor
      ↓
更高性能
```

而现在逐渐变成：

```text
Scaling
+
3D integration
+
Advanced Packaging
+
Chiplet
+
Memory integration
+
Architecture
+
Software
```

所以未来的“继续缩小”不一定仅仅意味着：

> 把线宽从 3 nm 做到 2 nm，再做到 1.x nm。

而可能变成：

```text
More transistor density
+
Better transistor architecture
+
3D stacking
+
Better packaging
+
More specialized accelerators
```

光刻机依然重要，但它可能从“唯一主角”逐渐成为：

> **先进计算系统整体创新的一环。**

---

# 二十三、对软件工程师最有价值的五条洞察

如果把整篇报告压缩成五句话，我会选择这五句。

### 1. 高性能不是单个模块决定的，而是系统级闭环决定的

```text
Sensor
 ↓
Model
 ↓
Control
 ↓
Actuator
 ↓
Machine
 ↓
Sensor
```

这比单纯追求：

```cpp
algorithm faster
```

更重要。

---

### 2. 复杂系统最难的不是正常流程，而是异常流程

真正复杂的是：

```text
Start
↓
Step 1
↓
Step 2
↓
Sensor timeout
↓
Partial state
↓
Retry
↓
Operator intervention
↓
Resume
```

所以高级 C++ 工程师应该大量研究：

* cancellation
* timeout
* retry
* compensation
* rollback
* recovery
* degraded mode
* fault containment

---

### 3. “状态”应该来自观测，而不是来自程序自己宣布

坏设计：

```cpp
state = Running;
```

更合理：

```text
Command
 ↓
Execution
 ↓
Physical measurement
 ↓
Validated status
 ↓
State transition
```

---

### 4. 系统工程的核心是约束管理

例如：

```text
Exposure requires:
    vacuum stable
    source ready
    wafer positioned
    reticle valid
    stage stable
    safety normal
```

这实际上就是：

```text
Requirement
+
Constraint
+
Capability
+
State
+
Workflow
```

而不是单纯的一张 FSM 表。

---

### 5. 真正高级的软件不是“代码多”，而是“复杂性被控制住了”

最终希望做到：

```text
1000 个底层状态
        ↓
几十个稳定抽象
        ↓
几个系统级概念
```

而不是：

```text
1000 个状态
 ↓
1000 个 if
 ↓
1000 个特殊情况
```

这其实就是高级软件架构的核心能力之一。

---

# 二十四、如果把 ASML 光刻机当成一个 C++ 学习项目

我认为可以直接建立下面这个学习路线：

```text
Phase 1
物理模型
│
├── Light
├── Optics
├── Wafer
└── Exposure

        ↓

Phase 2
设备模型
│
├── Source
├── Stage
├── Vacuum
├── Optics
└── Sensor

        ↓

Phase 3
控制系统
│
├── Feedback
├── PID
├── Position Control
└── Calibration

        ↓

Phase 4
设备软件
│
├── FSM
├── Scheduler
├── Event Bus
├── RPC
├── Pub/Sub
└── Diagnostics

        ↓

Phase 5
可靠性
│
├── Timeout
├── Retry
├── Cancellation
├── Recovery
├── Fault handling
└── Safe shutdown

        ↓

Phase 6
系统工程
│
├── Multi-subsystem coordination
├── Requirement
├── Constraint
├── Capability
├── Recipe
└── Configuration

        ↓

Phase 7
高级工业架构
│
├── Real-time
├── Distributed control
├── Digital twin
├── Predictive maintenance
└── Optimization
```

到了最后，你学习的已经不再只是 C++。

而是：

> **如何用 C++ 构建一个复杂的 Cyber-Physical System。**

这比单纯刷几十道 C++ 算法题，更接近高级设备软件工程师真正需要解决的问题。

---

# 二十五、最终判断：光刻机真正值得研究的不是“它有多先进”

我认为观察光刻机，最值得得到的最终认识是：

> **光刻机不是一台机器，而是一种工业文明的组织方式。**

它把：

```text
物理学
光学
化学
材料
机械
电子
控制
软件
算法
计量
制造
供应链
服务
```

压缩到了一个系统里。

而它最可怕的地方不是某一个数字：

> 13.5 nm
> 0.55 NA
> 0.x nm precision

而是：

> **这些极端指标能够同时存在，并且在工厂里连续运行数千、数万小时。**

因此，如果从工程师视角给光刻机下一个定义，我会给出：

> **光刻机是以纳米级物理精度为约束、以闭环控制为核心、以软件和计量为神经系统、以供应链协同为基础的超复杂工业 Cyber-Physical System。**

这也是为什么，**研究光刻机其实是在研究“现代工业系统是如何被构建出来的”。**

---

## 参考资料

本文涉及的关键技术事实主要依据 ASML、ZEISS、Nikon、Canon、美国 BIS 等公开资料：

ASML 2025 年报显示其 2025 年销售额为 €32.7B、研发投入约 €4.7B，供应商约 5,100 家；2026 年公司预计 EUV 收入继续显著增长。([ASML][9])

ASML 对 EUV / High-NA EUV 的官方技术说明，包括 13.5 nm 光源、0.33 → 0.55 NA、晶圆台控制以及 EXE 平台。([ASML][2])

ZEISS 对 High-NA EUV 光学系统、镜面精度、部件数量和系统规模的公开说明。([Zeiss][4])

Nikon 当前 DUV / ArF immersion 产品线及规格。([Nikon][10])

Canon 当前 Nanoimprint Lithography 产品及其先进节点应用方向。([Canon Global][8])

BIS 关于先进半导体制造设备出口管制的公开政策文件。

如果把这份报告继续往你当前的 **C++ 上位机/设备软件**方向推进，下一步最有价值的是直接把它落成一套**“光刻机 C++17 软件架构”**：从 `Machine / Subsystem / FSM / EventBus / RPC / Scheduler / Cancellation / Fault & Recovery` 一路设计到可编译的工程，这样能把这里的系统工程洞察真正变成代码。

[1]: https://www.asml.com/en/investors/annual-report/2025?utm_source=chatgpt.com "ASML 2025 Annual Report"
[2]: https://www.asml.com/en/en/products/euv-lithography-systems?utm_source=chatgpt.com "EUV lithography systems – Products | ASML"
[3]: https://www.asml.com/en/technology/lithography-principles/lenses-and-mirrors?utm_source=chatgpt.com "Lenses & mirrors - Lithography principles | ASML"
[4]: https://www.zeiss.com/semiconductor-manufacturing-technology/inspiring-technology/high-na-euv-lithography.html?utm_source=chatgpt.com "High-NA-EUV Lithography: the next EUV generation | ZEISS SMT"
[5]: https://www.asml.com/en/investors/annual-report/2025/strategy-and-stories?utm_source=chatgpt.com "Strategy & stories | ASML 2025 Annual Report"
[6]: https://www.zeiss.com/semiconductor-manufacturing-technology/news-and-events/smt-press-releases/2025/euv-30-years.html?utm_source=chatgpt.com "Extremely precise, extremely successful: 30 years of EUV lithography optics at ZEISS SMT"
[7]: https://www.zeiss.com/semiconductor-manufacturing-technology/smt-magazine/duv-lithography.html?utm_source=chatgpt.com "DUV Lithography: Light creating digitalization"
[8]: https://global.canon/en/product/indtech/semicon/fpa1200nz2c.html?utm_source=chatgpt.com "FPA-1200NZ2C | Canon Global"
[9]: https://www.asml.com/en/en/investors/annual-report?utm_source=chatgpt.com "Annual reports | ASML"
[10]: https://www.nikon.com/business/semi/lineup/?utm_source=chatgpt.com "Lineup | Semiconductor Lithography Systems | Nikon Business"

