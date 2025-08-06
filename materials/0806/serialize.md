在 C++ 端对 Protobuf 生成的 `Message` 做“打桩”注入（即在运行时填入测试／模拟数据）并再做序列化／反序列化，通常有两种思路：

1. **静态类型 + Reflection API**
2. **动态消息（DynamicMessageFactory）+ DescriptorPool**

下面分别给出两种方案的示例和要点。

---

## 1. 静态类型 + Reflection API

如果你已经通过 `protoc` 生成了 `.pb.h/.pb.cc`，并且在编译期就知道要操作的具体消息类型，可以直接用 Reflection API：

```cpp
#include "your_proto.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection.h>
#include <cassert>
#include <iostream>

void StubInjectAndRoundTrip() {
    // 1. 构造具体的消息类型
    YourProtoNamespace::YourMessage msg;

    // 2. 通过 Reflection 填字段
    const google::protobuf::Descriptor* desc = msg.GetDescriptor();
    const google::protobuf::Reflection* refl = msg.GetReflection();

    // 假设你的 proto 定义里有一个 int32 id = 1; string name = 2;
    const google::protobuf::FieldDescriptor* f_id   = desc->FindFieldByName("id");
    const google::protobuf::FieldDescriptor* f_name = desc->FindFieldByName("name");
    assert(f_id && f_name);

    refl->SetInt32(&msg,   f_id,   12345);
    refl->SetString(&msg,  f_name, "stub_name");

    // 3. 序列化到二进制
    std::string binary;
    bool ok = msg.SerializeToString(&binary);
    assert(ok);

    // 4. 反序列化到另一个实例
    YourProtoNamespace::YourMessage msg2;
    ok = msg2.ParseFromString(binary);
    assert(ok);

    // 5. 验证结果
    std::cout << "id="   << msg2.id()
              << ", name=" << msg2.name() << "\n";
}
```

* **优点**：类型安全、性能最高。
* **缺点**：编译时就要知道所有字段、类型；每次改 proto，都要重新编译。

---

## 2. 动态消息 + DescriptorPool

如果你想在运行时「动态」地根据 descriptor 信息来打桩，或者甚至在运行时加载 `.desc` 文件（`--descriptor_set_out` 生成的二进制描述符），可以用 `DynamicMessageFactory`：

```cpp
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <fstream>
#include <iostream>
#include <cassert>

// 从文件里加载 DescriptorSet
bool LoadFileDescriptorSet(const std::string& path,
                           google::protobuf::FileDescriptorSet* fds) {
    std::ifstream in(path, std::ios::binary);
    return fds->ParseFromIstream(&in);
}

void DynamicStubInjectAndRoundTrip(const std::string& desc_path,
                                   const std::string& message_type) {
    // 1. 读取 descriptor_set
    google::protobuf::FileDescriptorSet fds;
    assert(LoadFileDescriptorSet(desc_path, &fds));

    // 2. 注册到 DescriptorPool
    google::protobuf::DescriptorPool pool;
    for (int i = 0; i < fds.file_size(); ++i) {
        pool.BuildFile(fds.file(i));
    }

    // 3. 拿到 Message 的 Descriptor
    const google::protobuf::Descriptor* desc =
        pool.FindMessageTypeByName(message_type);
    assert(desc);

    // 4. 用 Factory 创建动态消息
    google::protobuf::DynamicMessageFactory factory(&pool);
    std::unique_ptr<google::protobuf::Message> msg(
        factory.GetPrototype(desc)->New()
    );

    // 5. 通过 Reflection 设置字段（按名字）
    const auto* refl = msg->GetReflection();
    const auto* fd_id   = desc->FindFieldByName("id");
    const auto* fd_name = desc->FindFieldByName("name");
    assert(fd_id && fd_name);

    refl->SetInt32(msg.get(),  fd_id,   67890);
    refl->SetString(msg.get(), fd_name, "dynamic_stub");

    // 6. 序列化
    std::string bin;
    assert(msg->SerializeToString(&bin));

    // 7. 反序列化到新实例
    std::unique_ptr<google::protobuf::Message> msg2(
        factory.GetPrototype(desc)->New()
    );
    assert(msg2->ParseFromString(bin));

    // 8. 打印
    const auto* refl2 = msg2->GetReflection();
    std::cout << "dynamic id=" 
              << refl2->GetInt32(*msg2, fd_id)
              << ", name=" 
              << refl2->GetString(*msg2, fd_name)
              << "\n";
}
```

* **优点**：完全在运行时控制，支持未知类型、热加载 proto 描述。
* **缺点**：比静态生成开销大一些，且不够类型安全（字段名写错在编译期检测不出来）。

---

### 如何选择？

* 如果只是给自己写单元测试／集成测试，或业务里字段变化很少，推荐 **方案 1（静态 + Reflection）**，简单直接、性能好。
* 如果你需要做一个「通用工具」，能够针对任意 `.desc` 做打桩、模拟，或在运行时动态切换 proto 版本，就选 **方案 2（DynamicMessageFactory）**。

---

#### 额外建议

* **文本格式更直观**
  对于调试，也可以先把 Message 转成 JSON 或 TextFormat，手动插入字段再反序列化：

  ```cpp
  std::string text;
  google::protobuf::TextFormat::PrintToString(msg, &text);
  // 手动在 text 里打桩，或者用脚本生成新的 text
  google::protobuf::Message* msg2 = ...;
  google::protobuf::TextFormat::ParseFromString(text, msg2);
  ```

* **Any 类型**
  如果需要在 proto 消息里嵌入任意数据块，也可以把要 stub 的结构先序列化成一个子 Message，再用 `google::protobuf::Any` 包裹注入。

* **自定义 Options**
  如果打桩的场景是为生成代码添加“元信息”，不妨在 `.proto` 定义层面使用 `extend google.protobuf.MessageOptions`，这样打桩信息和代码生成都可以通过 `descriptor.proto` 拿到。

---

以上两种方案，基本能覆盖绝大多数对「打桩注入信息并反序列化」的需求。根据你的具体场景（测试用例 vs. 通用工具）选择即可。
在 `.proto` 文件里给每个字段指定一个数字（称为 **field number** 或 **tag**）是 Protobuf 序列化设计的核心，主要有以下几个原因：

1. **二进制格式中的标识符**
   在 Protobuf 的二进制编码里，每个字段都会被写成一个「键（key）＋值（value）」对，其中「键」实际上就是由 field number 和 wire type 组成的一个 varint。接收端靠这个数字来判断接下来字节对应哪个字段，以及应该用哪种方式解析它。

   ```text
   key = (field_number << 3) | wire_type
   ```

   例如，`id = 1` 在二进制里，key 会是 `(1 << 3) | 0 = 0x08`（0 表示 varint 类型），后面紧跟 id 的值编码。

2. **向前／向后兼容**

   * **增加字段**：你可以在消息后面随时新增字段，只要给它一个新的、未被占用的 tag，比如 `new_field = 10;`。旧版本程序看到不认识的 tag，会自动跳过，不会报错。
   * **删除或废弃字段**：如果要「弃用」某个字段，也只需要在注释里标记，然后保留它的 tag，或者把它 `reserved` 掉，避免未来重用同一个数字导致冲突。

3. **Tag 范围和性能**

   * 可用的 field number 范围是 1 到 2³⁰−1（proto2/3）／2²⁹−1（proto3），但 1–15 和 16–2047 在 varint 编码长度上有所差别：

     * **1–15**：编码成 key 只需一个字节，最省空间。
     * **16–2047**：编码成 key 需要两个字节。
   * 所以对于最常用、最关键的字段（比如主 ID、常见短字符串等），把 tag 设在 1–15 范围内能节省一点网络／存储开销。

4. **保持消息顺序无关性**
   在二进制流里，字段的顺序和在 `.proto` 中的声明顺序不必严格一致——wire 格式只看 tag。接收端解析时会根据 tag 自动映射到对应字段，这就简化了消息合并、流水线转换等场景的处理。

---

### 小结

* `= 1, = 2, = 3` 等数字就是 Protobuf 在二进制里辨识字段的「标签」。
* 它们决定了消息编码时的 key 值，并影响兼容性和空间效率。
* 选用合适的数字范围，对常用字段放低 tag（1–15），能获得更紧凑的编码。
