这是个很好的问题 👍。你说的场景里，核心问题是 **rpc 生成的代码（比如 Protobuf + gRPC stub）通常是“强依赖的具体类”，而 gmock 的最佳实践是针对“接口（抽象类）”做 mock**。
所以要让 gmock 在单元测试里好用，需要在代码结构上做一些“适配层设计”，把自动生成的 stub 和业务逻辑隔离开来。

下面我分步骤解释可行的几种方案：

---

## 1. 利用 gRPC 官方提供的 *Mock 支持*

如果你用的是 **gRPC C++**，官方在 `grpc::testing` 里提供了 `MockClientReader`, `MockClientWriter`, `MockClientReaderWriter`，你可以直接拿来配合 gmock 使用。
但前提是你愿意在测试代码里和这些类打交道，这往往让测试代码比较复杂，不推荐直接暴露给业务层。

---

## 2. **引入接口层（最常见方案）**

假设 protobuf 定义了一个 service：

```proto
service MyService {
  rpc GetData (Request) returns (Response);
}
```

编译后会生成一个 `MyService::Stub`，通常你会这样调用：

```cpp
std::unique_ptr<MyService::Stub> stub = MyService::NewStub(channel);
auto status = stub->GetData(&context, req, &resp);
```

如果直接依赖 `Stub`，你没法 mock。
解决办法是 **在项目里再定义一个接口**：

```cpp
class IMyService {
public:
    virtual ~IMyService() = default;
    virtual grpc::Status GetData(const Request& req, Response* resp) = 0;
};
```

然后写一个 **真实实现（适配器）** 包装 gRPC 的 Stub：

```cpp
class MyServiceImpl : public IMyService {
public:
    explicit MyServiceImpl(std::unique_ptr<MyService::Stub> stub)
        : stub_(std::move(stub)) {}

    grpc::Status GetData(const Request& req, Response* resp) override {
        grpc::ClientContext ctx;
        return stub_->GetData(&ctx, req, resp);
    }

private:
    std::unique_ptr<MyService::Stub> stub_;
};
```

测试时，就可以写一个 mock 类：

```cpp
class MockMyService : public IMyService {
public:
    MOCK_METHOD(grpc::Status, GetData, (const Request& req, Response* resp), (override));
};
```

这样业务代码只依赖 `IMyService`，测试时用 `MockMyService`，生产环境用 `MyServiceImpl`，解耦完成 ✅。

---

## 3. **模板+依赖注入**

另一种写法是模板化业务逻辑，让它接收任意实现了特定 API 的类型。例如：

```cpp
template <typename Service>
class Business {
public:
    Business(Service* service) : service_(service) {}

    bool DoWork() {
        Request req;
        Response resp;
        auto status = service_->GetData(req, &resp);
        return status.ok();
    }
private:
    Service* service_;
};
```

然后在测试时，你可以传入一个 mock 类：

```cpp
class FakeService {
public:
    grpc::Status GetData(const Request&, Response* resp) {
        resp->set_value("mocked");
        return grpc::Status::OK;
    }
};
```

这种方式不需要虚函数，但要求你用模板实例化，可能导致编译时间长，不够灵活。

---

## 4. **用 gmock 直接 Mock Stub（不推荐）**

你也可以直接对 `MyService::StubInterface`（gRPC 生成的虚基类）做 mock：

```cpp
class MockStub : public MyService::StubInterface {
public:
    MOCK_METHOD(grpc::Status, GetData,
        (grpc::ClientContext* context, const Request& req, Response* resp), (override));
};
```

然后在测试里用 `MockStub` 替代真实的 `Stub`。
但这种方式会让你的业务代码直接依赖 gRPC 生成的类型，**耦合太强，不易维护**。

---

## 5. 推荐方案总结

* 如果代码量小、需求简单 👉 **直接 mock `StubInterface`**。
* 如果是中大型项目，RPC 调用很多 👉 **一定要引入接口层（方案2）**，保持业务逻辑和 RPC 框架解耦，mock 更加自然。
* 如果极度追求性能 👉 **模板方案（方案3）**，但测试写法没接口那么直观。

---

要不要我帮你给出一个完整的 **gmock 单元测试示例代码**（包含调用 `EXPECT_CALL`，校验 RPC 调用参数 & 返回值）？
