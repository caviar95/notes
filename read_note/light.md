# 光刻技术历史沿革

光刻技术是半导体制造的核心工艺，其演进与摩尔定律紧密相连。\*\*第一代（1960年代）\*\*光刻机多采用接触式或近接式，最早由贝尔实验室在1955年开始采用光刻技术，1961年美国GCA公司推出了第一台接触式光刻机。由于接触式光刻掩膜和晶圆直接接触，分辨率低且掩膜寿命短，后来发展出近接式光刻（掩膜与晶圆间保留微小气隙），虽减少了污染但分辨率仍不高。\*\*第二代（1970年代）\*\*投影式光刻登场，第一台投影式光刻机由美国Perkin Elmer于1973年率先推出，投影系统引入物镜后既避免了污染又实现了图形倍缩，至1970年代末约占据90%的市场。**第三代（1980年代）**进入步进式和扫描式光刻时代：1980年尼康推出首台商用步进式光刻机NSR-1010G，分辨率约1微米；随后的扫描式步进投影机配合准分子激光光源（KrF 248nm、ArF 193nm）使分辨率不断提高，节点从0.5μm推进到几十纳米。进入**第四代（2000年代）**，浸没式光刻技术出现（2004年ASML首推），通过在镜头与晶圆间注入折射率更高的水介质，将193nm实际等效缩短至\~134nm，有效提高了数值孔径和分辨率。\*\*第五代（2010年代至今）\*\*极紫外（EUV）光刻投入工业化：采用13.5nm波长的EUV光源，分辨率进一步提升以支撑7nm及以下工艺。ASML与Cymer合作于2010年推出首台EUV原型机NXE:3100，并于2017年推出NXE:3400B等量产设备。至2020年代，EUV系统已用于5nm/3nm量产，ASML是唯一供应商。总体而言，光刻机经历了五代技术演进，光源波长从紫外可见光436nm缩短到极紫外13.5nm，相应工艺节点从微米级演进到最先进的3nm。

## 光刻机类型与工作原理

光刻技术主要分为**掩模光刻**和**直写光刻**两大类。掩模光刻是主流的芯片前道工艺，利用光学系统将掩模上的电路图案缩小投影到硅片上；直写光刻则无需掩模，直接用光束或电子束扫描成像，精度稍低，多用于PCB、封装和掩膜版制版等领域。

* **接触式光刻**：掩模直接贴合晶圆暴露，基于近场菲涅尔衍射成像。设备结构简单，可实现与掩模同等的图形分辨率。但掩模与晶圆接触易损伤掩膜，曝光对掩膜和晶圆胶层污染严重，掩膜寿命仅5–25次，缺陷率高。代表厂商已无专门产品，此技术已被淘汰。分辨率大致为微米级（>0.5µm），波长多用436nm Hg灯。
* **近接式光刻**：在掩模与晶圆间保留数微米空隙，避免直接接触污染，但衍射效应降低分辨率。产能略低于接触式，仍主要用于老旧工艺。其工作原理与接触式类似，只是掩膜与晶圆分离了微小距离。
* **投影式光刻（静态步进）**：采用光学投影系统，掩模和晶圆之间加入物镜组，将掩模图像以缩小比例投影到晶圆上。初期步进式光刻机（Stepper）一次曝光一个区域，然后步进晶圆（或掩模）到下一区域。典型代表是Perkin Elmer、GCA等早期产品。分辨率约在0.2–0.5µm之间（取决于波长和NA），对准方式为光学对准（通过对准标记）。产能低于扫描机，但在上世纪80年代广泛应用。
* **扫描式步进投影**：在投影光学系统基础上加入动态扫描，投影镜头和晶圆在X、Y两个方向同步移动进行曝光，可实现更大曝光视场和均匀性。主流产品如ASML TWINSCAN、Nikon NSR系列、Canon FPA系列等使用KrF(248nm)和ArF(193nm)激光。随着浸没技术应用，2000年代后步进扫描式光刻机成为主流。典型分辨率已提升至20nm级别（虚拟分辨），实测分辨率通常约为曝光波长的1/4至1/5，利用分辨率增强技术（RET）继续提高。对准方式采用光学对准站，双工作台设计实现高产能。Nikon NSR-S636E浸没机可达≤38nm分辨率、NA=1.35、波长193nm，产能≥280片/小时。
  &#x20;*图：ASML NXE:3400C极紫外光刻机外观（EUV光刻机用于7-5nm工艺）*
* **极紫外光刻（EUV）**：使用波长13.5nm的极紫外激光脉冲（利用锡等离子体发射光子）作为光源，通过多层镜面进行反射成像。由于EUV在空气中衰减，系统须在真空中运行、采用全反射光学。ASML NXE系列是唯一商用EUV设备，NXE:3400C支持5-7nm节点量产，镜头NA=0.33、最大曝光面积26×33mm，产能170片/小时（20mJ/cm²）。对准系统同样光学实现，以天平格栅对准技术保证亚纳米级对准精度。EUV可实现10nm以下线宽，但光源效率和设备成本极高。
* **电子束直写**：不使用光和掩模，采用聚焦电子束直接在光刻胶上扫描绘图。代表厂商包括瑞士Raith、日本JEOL、Elionix等。电子束的有效“波长”极短（主流EBL可达<1nm），分辨率极高（线宽可达单纳米级），但只能逐点扫描，写入速度极慢（通常一个晶圆需数小时以上），难以应用于大规模前道生产。目前主要用于高端掩模版制版及研究级应用。

## 各类光刻机参数比较

| 类型        | 分辨率（线宽）      | 波长                         | 对准方式             | 产能（300mm晶圆）             | 成本（单台）        | 典型工艺节点       |
| :-------- | :----------- | :------------------------- | :--------------- | :---------------------- | :------------ | :----------- |
| 接触式/近接式   | ≈0.5–1 μm    | 可见光436nm/365nm (Hg灯)       | 掩膜与晶圆接触或微距对准（手动） | 较低（极慢）                  | 极低            | ≥0.5 μm      |
| 步进式（静态）   | ≈0.2–0.5 μm  | i线365nm/DUV248nm/193nm     | 光学对准             | ≲20\~100 wafer/小时（视代工厂） | 中等            | 0.5\~0.2 μm  |
| 扫描式（扫描步进） | ≲50 nm       | KrF 248nm / ArF 193nm (干法) | 光学对准             | \~200–300 wafer/小时      | 高             | 90\~14 nm    |
| ArF浸没扫描   | ≲38 nm       | ArF 193nm (浸没)             | 光学对准             | \~200–300 wafer/小时      | 最高（>6000万美元）  | 45\~14 nm    |
| 极紫外EUV    | ≲13 nm       | EUV 13.5nm                 | 光学对准（反射投影）       | \~100–150 wafer/小时      | 极高 (\~1.8亿欧元) | 14\~3 nm     |
| 电子束直写     | ≲10 nm（掩模制作） | —（电子波）                     | 无掩模，电子束扫描        | 极低（<<1 wafer/小时）        | 高             | 掩模工艺10\~1 nm |

各参数仅供示意。分辨率由光源波长、数值孔径、工艺因子等决定；对准方式均为光学成像系统中的精密对准，电子束直写除外。表中产能以300mm晶圆为例，EUV产能受光源功率限制并根据曝光剂量而变动。成本方面，EUV光刻机远高于DUV浸没机，后者又高于KrF/i线系统。典型制程节点反映了各技术的主流应用：i线用于0.3–0.5μm节点，KrF/ArF用于0.1μm–10nm节点（多重曝光配合），EUV用于7nm及以下。

## 主要厂商与技术路线

目前全球光刻设备市场由荷兰ASML和日本Nikon、Canon三家垄断。**ASML**（荷兰）是全球光刻行业绝对龙头，市场份额长期超过60%以上。1984年ASML由飞利浦和ASMI合资成立，1991年推出PAS 5500步进机一举成名，2001年推出双工作台（TWINSCAN）系统，2003年与台积电合作推出浸没式光刻系统。2010年首台EUV系统NXE:3100面世，2013年收购光源巨头Cymer并推出NXE:3300B，2017年推出NXE:3400B。当前ASML覆盖从KrF、ArF干/浸没到EUV全系列产品，其EUV设备主导了5nm、3nm节点制造。ASML与合作伙伴蔡司提供高NA镜头，正在研发高NA EUV（NA≈0.55）以推动2nm及以下工艺。

**Nikon**（日本）历史悠久：1978年超LSI项目委托，1978年完成原型，1980年推出日本首台商用步进机NSR-1010G（1μm分辨）。此后Nikon推出多代扫描光刻机（NSR-S系列），包括ArF浸没机。Nikon的产品覆盖中高端领域，但自2008年失去台湾/韩国市场后出货量锐减。截至2023年底，Nikon仍提供最新ArF浸没Scanner（NSR-S636E，38nm分辨率、≥280wph），但已无EUV计划，只在中低端市场与ASML竞争。

**Canon**（日本）也长期参与光刻机制造，早期与Nikon并驾齐驱。Canon曾投入研发干式UV光刻，但成本高昂未获商业化，2000年代后期逐步退出高端DUV领域。目前Canon主要生产i-line步进机和KrF设备（如FPA系列），并涉足OLED面板光刻。在全球前道市场，Canon多集中低端（i线）产品。其代表型号如i线Stepper FPA-3030i系列，以及数年前的KrF扫描机（如FPA-6300ES6a）。

\*\*SMEE（上海微电子装备）\*\*是中国唯一能够研制投影式光刻机的厂商，成立于2002年。其产品以200mm晶圆设备为主，使用ArF、KrF和i线光源，主攻集成电路后道封装和FPD面板市场。截至2020年代，SMEE已实现90nm节点的ArF步进机生产。SMEE宣称将于2021–2022年交付国产浸没式ArF光刻机以打破90nm瓶颈，有望实现一次跨代到28nm。在国内市场，SMEE在后道封装设备领域市占率高达80%，全球约40%；但在前道前沿市场其技术与ASML等巨头仍有较大差距。

总体来看，ASML凭借领先技术和持续投入占据超高端市场，Nikon次之，Canon局限于低端，SMEE作为后起之秀专注国产化替代。市场份额方面，2022年ASML出货量占据全球极紫外与浸没式DUV的绝对优势，ArF/KrF领域也占据超八成份额；Canon、Nikon主要分食剩余低端市场。此外，卡尔蔡司（德国）长期为ASML和Nikon等提供高性能光学镜头，是核心元件供应商。

\*\*参考资料：\*\*引文来自公开研究报告和厂商资料，对光刻技术及主要厂商作了概述等。


ASML 是全球领先的光刻设备制造商，其产品涵盖了从传统的深紫外光刻（DUV）到最先进的极紫外光刻（EUV）全系列设备。下面是对 ASML 光刻机各类产品的详细分类、型号及其特点介绍：

---

## 一、ASML 光刻机产品类型概览

| 产品类型          | 系列名      | 使用波长         | 应用制程节点          | 典型代表型号                | 主要特点                        |
| ------------- | -------- | ------------ | --------------- | --------------------- | --------------------------- |
| KrF（248nm）    | PAS / XT | 248nm (KrF)  | ≥130nm          | PAS 5500, XT:400K     | 成熟稳定、适用于成熟工艺如模拟电路           |
| ArF（193nm）干式  | XT       | 193nm (ArF)  | 90\~65nm        | XT:1460               | 成本低、适用于非高端逻辑与存储             |
| ArF（193nm）浸没式 | NXT      | 193nm (浸没式)  | 65\~7nm（配合多重曝光） | NXT:1980Di, NXT:2000i | 市场主力产品，适用于高端芯片制程            |
| EUV（13.5nm）   | NXE      | 13.5nm (EUV) | ≤7nm            | NXE:3400C, NXE:3600D  | 唯一商用EUV系统，支持3\~5nm，未来拓展至2nm |
| 高NA EUV（开发中）  | EXE      | 13.5nm (EUV) | <2nm（目标）        | EXE:5000（测试中）         | 数值孔径提升至0.55，分辨率更高，支持2nm以下   |

---

## 二、DUV 产品系列详细介绍（KrF / ArF）

### 1. **KrF 系列（XT / PAS 5500）**

* **波长**：248nm
* **代表型号**：

  * **PAS 5500/100D**：经典步进式光刻机，主要用于逻辑/模拟芯片制造。
  * **XT:400K**：现代KrF扫描设备，适用于大批量生产。
* **主要特点**：

  * 适用于 130nm 及以上节点（部分配合 RET 可达90nm）
  * 成熟可靠、光学系统简单、适合中小型晶圆厂
  * 成本低、能耗低、维护方便
* **应用场景**：

  * 晶圆厂中段/后段工艺、模拟芯片、功率器件、CMOS 图像传感器等。

---

### 2. **ArF 干式 DUV 系列（XT 系列）**

* **波长**：193nm
* **代表型号**：

  * **XT:1460**、**XT:860M** 等
* **主要特点**：

  * 成本比浸没式低，适用于≤90nm 制程
  * 扫描式成像、分辨率≈65nm（配合RET可优化）
* **应用场景**：

  * 存储、微控制器（MCU）、射频芯片、成熟逻辑制程等

---

### 3. **ArF 浸没式 DUV 系列（NXT 系列）**

* **波长**：193nm（浸没液折射率提升有效分辨率）
* **代表型号**：

  * **NXT:1950Hi**, **NXT:1980Di**, **NXT:2000i**, **NXT:2100i**
* **主要特点**：

  * 采用浸没技术提升分辨率（等效波长约134nm）
  * 分辨率可达 38nm 以下，适用于14nm、10nm及7nm节点（搭配多重曝光）
  * 双平台结构（TWINSCAN）：曝光和对准并行，提高产能
  * 高产能：≥275 片/小时
  * 自动对准、实时对焦补偿、高精度工艺控制
* **应用场景**：

  * 高端逻辑芯片、存储器（DRAM/Flash）、先进封装等
* **当前市场地位**：

  * 目前DUV中最重要产品，许多晶圆厂即使使用EUV也需多层搭配NXT设备

---

## 三、EUV 产品系列详解（NXE 系列）

### 1. **标准 EUV（NXE 系列）**

* **波长**：13.5nm（极紫外）
* **代表型号**：

  * **NXE:3400B/C**, **NXE:3600D**
* **主要特点**：

  * 波长更短 → 分辨率提升到 ≈13nm
  * 真空系统 + 多层反射镜 → 成本与复杂性高
  * 无需多重曝光即支持 5nm / 3nm 制程
  * NXE:3400C 产能可达 175 片/小时
  * NXE:3600D 优化光源功率与对准精度，已用于 3nm 大规模量产
* **技术亮点**：

  * **多重曝光消除**：与ArF相比显著减少图形对准误差
  * **图形保真度高**：适合FinFET、GAA结构等高精度特征
* **应用场景**：

  * 台积电、三星、英特尔用于3\~5nm前道关键层
  * EUV光罩制造配合纳米级图形增强技术

---

### 2. **高数值孔径 EUV（EXE 系列，研发中）**

* **目标数值孔径（NA）**：0.55（现有NA=0.33）
* **代表型号**：EXE:5000、EXE:5200（开发中）
* **主要特点**：

  * 分辨率进一步提高，可支持2nm甚至更小制程
  * 替换原有镜头系统，兼容部分工艺模块
  * 开发难度极高：需新一代反射镜、掩模对准系统
* **现状**：

  * ASML已在2023年向英特尔交付首台EXE:5000（用于研发测试）
  * 商用预计2025年后开始量产用于2nm及以下工艺节点

---

## 四、产品技术进化路线图（简略）

| 年代    | 技术演进         | 代表型号           | 应用节点         |
| ----- | ------------ | -------------- | ------------ |
| 1990s | 步进式DUV       | PAS 5500 系列    | 250\~180nm   |
| 2001  | TWINSCAN系统推出 | XT:400K        | 130\~90nm    |
| 2004  | 浸没式DUV发布     | XT:1900i → NXT | 90\~28nm     |
| 2010  | EUV原型发布      | NXE:3100       | 14\~10nm（测试） |
| 2017  | EUV量产化       | NXE:3400B      | 7\~5nm       |
| 2021  | NXE优化版本      | NXE:3600D      | 5\~3nm       |
| 2023  | 高NA EUV测试机   | EXE:5000 交付英特尔 | 2nm以下（研发）    |

---

## 五、总结：ASML 产品特点总览

* **技术领先**：唯一掌握EUV与高NA EUV核心技术的厂商。
* **产品线完整**：从i线/KrF到高NA EUV，涵盖所有主流制程需求。
* **系统模块化**：TWINSCAN 架构适配多类型光刻方案，支持工艺协同。
* **高稳定性与良率**：特别在NXT与NXE系列中体现，为先进制程提供保障。
* **服务全球前沿客户**：台积电、英特尔、三星全部采用 ASML 设备作为主力前道光刻平台。

---

如果您希望获取每一代机型的详细技术规格（如NA、DoF、Overlay精度等）或希望了解某些设备在某个晶圆厂中的具体部署和实际性能表现，也可以继续深入查询。需要我整理一份详细的机型对比表格或产品发展时间轴也可以告诉我。


在**光刻机的光源腔（尤其是EUV或DUV系统）**中，为了确保**光源生成、镜面保护、清洗、热管理**等多个目的，会使用多种气体。除了常见的**氟气（F₂）**与**氯气（Cl₂）**，还可能使用以下几类气体，每种气体都有明确的技术角色：

---

## 一、EUV 光刻机光源腔中的常见气体

### 1. **氢气（H₂）**

* **用途**：镜面保护、Sn污染控制
* **原理**：

  * EUV 系统中，激光轰击锡（Sn）靶生成等离子体，释放出13.5nm极紫外光。但同时会产生飞溅的锡颗粒，污染反射镜。
  * 在腔体中引入 **氢气**，Sn颗粒在氢气环境下会和H反应形成易被抽除的气体（如 SnH₄），并抑制颗粒沉积。
  * 同时，氢气有助于冷却反射镜系统，减缓热积累。
* **优势**：成本低、无腐蚀性、可形成保护层、真空兼容性强。

---

### 2. **氖气（Ne）**

* **用途**：冷却与气体载体（Buffer Gas）
* **原理**：

  * EUV光源中，氖气用作**激光等离子体的缓冲气体**，可以减缓粒子飞行、提高系统稳定性。
  * 由于氖为惰性气体，不与反射镜或光源反应，适合作为保护性稀释剂。
  * 在某些EUV发射技术中（如LPP：Laser Produced Plasma），Ne用于稳定等离子体形成区域的气氛。
* **优势**：惰性、热导率适中、不会对镜面产生化学污染。

---

### 3. **氩气（Ar）**

* **用途**：

  * 激光等离子形成过程中的辅助气体
  * EUV腔体冲洗或维护
* **原理**：

  * Ar气体在高能激光束作用下可提供电子碰撞增强（电离补偿效应），优化等离子体密度与光强度。
  * 也用于设备保养时的 **镜面吹扫或惰性保护**。
* **优势**：常见、廉价、惰性、可承受高温。

---

### 4. **氟化氢（HF）或氟化物混合气体（如 NF₃）**

* **用途**：光学元件在线/离线清洗
* **原理**：

  * HF、NF₃ 等可分解为 F• 原子，用于清洗镜面表面的污染（如Sn或有机残留）。
  * HF 主要用于 **离线湿法清洗**，NF₃ 可用于等离子体清洗。
* **注意**：HF 腐蚀性强，多在维护阶段使用，NF₃ 更安全一些且适于真空环境。

---

### 5. **臭氧（O₃）或氧气（O₂）**

* **用途**：DUV系统光学透镜的有机污染清除
* **原理**：

  * 在DUV（如 ArF 光刻）中，镜头容易被光致分解的有机物污染，形成“光刻雾”或碳沉积。
  * 臭氧/氧气可参与**UV/O₃干洗反应**，氧化碳基残留物：

    ```
    C + O₃ → CO₂↑
    ```
  * 适合 DUV 系统中的光学清洗，不常用于 EUV。

---

## 二、总结：EUV / DUV 光源腔可能使用的气体

| 气体名称         | 类别    | 主要用途         | 使用设备类型    | 特点与说明             |
| ------------ | ----- | ------------ | --------- | ----------------- |
| **F₂**       | 活性气体  | 镜面清洗、Sn去除    | EUV       | 极高活性，可挥发式去除沉积金属   |
| **H₂**       | 还原性气体 | 镜面保护、Sn颗粒防护  | EUV       | 防止Sn颗粒沉积，提高镜面寿命   |
| **Ne**       | 惰性气体  | 缓冲气氛、冷却介质    | EUV       | 惰性、低热导、提高系统稳定性    |
| **Ar**       | 惰性气体  | 激光放电辅助、吹扫、冷却 | EUV / DUV | 常用惰性气体，成本低        |
| **HF / NF₃** | 氟化气体  | 光学部件清洗       | EUV / DUV | 提供 F• 清洗能力，腐蚀性需管理 |
| **O₃ / O₂**  | 氧化性气体 | 光学镜片去碳、有机物清除 | DUV       | 适合ArF等系统的镜头清洗     |

---

## 三、气体的使用场景图示（概念）

```
         EUV 光源腔内示意：
    ┌────────────┐
    │     激光发射器     │ → 轰击 Sn 靶材（液滴）
    └────────────┘
             ↓
         Sn + 激光 → 等离子体 → EUV 光

   气体注入：
   - H₂：防Sn颗粒污染反射镜
   - F₂：反射镜清洗（形成 SnF₂ 气体）
   - Ne / Ar：缓冲、冷却
   - 维持真空/惰性环境

    多层反射镜 ←→ 腔体真空、温控气体
```

---

## 四、结语

EUV/DUV 光刻机的光源腔是一项极为精密复杂的系统工程，所使用的气体必须满足：

* 高纯度（9N及以上）
* 真空兼容
* 不破坏镜面或光学元件
* 精确流控与在线清洗功能

**这些气体不是随机选择的，而是为了满足亚纳米级制造过程中的光学清洁度、热稳定性、反应性与安全性的严格平衡**。

如您有兴趣，我可以继续介绍 ASML 在气体管理上的管控系统（气体流控、在线监测、反应产物处理）或 EUV 系统中的 Sn 清洗反应机制。是否继续？


我们来分两部分详细解释：

---

## 一、EUV 光是什么？

### ▍1. 全称：

**EUV（Extreme Ultraviolet）**，中文叫做“极紫外光”。

### ▍2. 波长范围：

* 波长约为 **13.5 纳米（nm）**
* 比可见光（400\~700nm）和传统DUV光（193nm, 248nm）更短，属于电磁波谱中的**软X射线区域**

### ▍3. 为什么使用 EUV？

* 波长越短，光的**分辨率越高**，可以曝光更小的芯片结构
* EUV 使得芯片制造可以达到 **7nm、5nm、3nm，甚至2nm制程节点**
* 减少多重曝光层数，提高良率与生产效率

---

## 二、EUV 光是怎么通过等离子体产生的？

在 ASML 的 EUV 光刻机中，采用的是 **激光等离子体（LPP, Laser Produced Plasma）技术**。其工作流程如下：

---

### ▍1. 主要原理概述：

```
高能激光 → 击中锡（Sn）液滴 → 爆炸成等离子体 → 辐射出 13.5nm 的 EUV 光
```

---

### ▍2. 详细过程分解：

#### ① 液态锡喷射

* 在真空腔中，以高速连续喷出微小的 **液态锡（Sn）微滴**（直径约30微米）
* 每秒喷出数千个

#### ② 激光聚焦轰击

* 高能脉冲激光（波长10.6μm，CO₂激光器）精确打在锡微滴上
* 微滴被瞬间加热到几万度

#### ③ 等离子体形成

* 锡原子电离成带电粒子（Sn⁺、Sn²⁺……），形成**高温等离子体云**
* 在这个过程中，锡的高能离子态跃迁，会**发射13.5nm的极紫外光子**

#### ④ EUV 光收集与反射

* 发出的 EUV 光向四周扩散
* 光源系统使用多层莫尔镜（多层布拉格反射镜）进行**选向与聚焦**
* 将高强度 EUV 光引导至光刻系统的投影光路中

---

## 三、图示总结（文字版）

```
激光器 → 打在锡微滴上 → 形成锡等离子体 → EUV光辐射 → 多层反射镜收集 → 投影系统 → 晶圆曝光
```

---

## 四、等离子体辐射的核心机制

* 锡的多价离子在退激时发出光子
* Sn⁸⁺、Sn¹³⁺ 等离子态间跃迁产生连续谱
* EUV光主要集中在\*\*13.5nm ± 1%\*\*这一波段，经过反射镜过滤与聚焦

---

## 五、为什么选择锡（Sn）？

| 条件   | 原因                                  |
| ---- | ----------------------------------- |
| 光谱特性 | 锡等离子体发射的EUV波段强，集中在13.5nm附近          |
| 易得性  | 相对便宜、安全、可控制液滴尺寸                     |
| 可再生  | 液滴式供给系统可持续生成，不易堵塞                   |
| 可清洗性 | 与H₂、F₂反应生成挥发物SnH₄、SnF₂，可通过气体系统清除沉积物 |

---

## 六、总结

| 项目       | 描述                                 |
| -------- | ---------------------------------- |
| **EUV光** | 波长13.5nm，极短紫外线，能实现纳米级图形转移          |
| **产生方式** | 激光打锡 → 高温等离子体 → 等离子跃迁发射EUV → 反射镜引导 |
| **优势**   | 高分辨率、可简化光刻层数、适用于先进制程（7nm、5nm、3nm）  |
| **难点**   | 真空运行、锡污染清洗、激光定位精度、反射镜热稳定性          |

---

如你需要我补充 “等离子体发射光谱图”、“反射镜多层结构构造原理”、“EUV 光学系统传输路径” 等详细技术图示，我可以继续生成。是否继续？
