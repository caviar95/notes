好的，我将为您调研EUV（极紫外）光刻机的控制系统总体架构，包括其核心控制模块、系统通信方式、主要功能分区（如光学、机械、真空系统等）的控制协调机制，以及与纳米级精度同步控制相关的关键技术。
完成后我会向您汇报相关结构图、技术原理和架构层次的整理结果。


EUV光刻机控制系统总体架构
EUV（极紫外）光刻机是高度复杂的光机电系统，其控制系统规模宏大、精度极高。据估计，光刻机控制系统（硬件+软件+算法）约占整机成本的三分之一
arxiv.org
。典型的EUV光刻机由多个高精度子系统构成，包括光源/照明系统、掩模（光罩）台、投影光学系统、晶圆台、真空与温控系统以及多种对位/测量模块等
pmc.ncbi.nlm.nih.gov
fab.cba.mit.edu
。控制系统需要对这些子系统进行统一的调度与控制，其总体结构通常呈分层式设计：从顶层主控到底层传感执行层逐级划分，各层之间通过总线和协议相连
arxiv.org
arxiv.org
。 控制系统主要组成层次：最底层为传感与执行层，包括各种传感器（如激光干涉仪、光学编码器、温度传感器、CCD摄像头等）和执行器（如线性电机、光阑执行机构、真空阀等），负责物理量采集和动作输出
arxiv.org
arxiv.org
。其上为I/O接口层，将底层传感器/执行器信号转换并传输给控制器。再上层是硬件控制/测量层，完成基本的实时控制环路和测量计算（通常在FPGA/ASIC中实现高速逻辑运算），例如晶圆台位置伺服控制回路和光束强度调节等
arxiv.org
arxiv.org
。再上层为子系统控制层，对应每个功能子系统的控制单元（如光源控制模块、对位控制模块、扫描控制模块等），通常采用DSP、FPGA、嵌入式CPU（PowerPC、ARM等）并运行实时操作系统，实现子系统功能和部分闭环控制
arxiv.org
arxiv.org
。最高层是主控制层，由工作站或主控PC组成（运行Windows或Unix/Linux等），负责全局协调、工艺控制和人机界面交互
arxiv.org
arxiv.org
。控制系统的分层结构示意如图所示： 图为ASML双工件台光刻机的系统截面示意：左图为浸没式DUV系统（NXT:1960Bi），右图为EUV系统（NXE:3100）。图中标注了光源（illumination）、掩模（mask）、投影光学、晶圆台等主要组件
pmc.ncbi.nlm.nih.gov
fab.cba.mit.edu
。控制系统的各层次模块则对应该图中的硬件功能块，其中主控制层（工作站）与下层控制板卡相连，底层传感器与运动执行机构集成在子系统中。
子系统之间的协调与通信架构
各子系统之间通过分层总线和协议进行通信与同步。一般架构为：主控层PC通过以太网/TCP协议与主控板卡通信，主控板卡通过VME总线或类似背板总线与分系统控制板相连，分系统控制板再通过现场总线（Fieldbus）与各执行层的I/O控制器通信
patents.google.com
。例如，一项双工件台扫描机通信方案中，上位机（Windows下基于MFC）通过TCP/IP与嵌入式工控机（运行VxWorks）交换数据，工控机作为通信枢纽，其下行通过VME总线与运动控制卡、同步控制卡、信号采集板卡等连接
patents.google.com
。整个网络结构中常见的通信协议包括以太网、CAN总线、RS-485等，用于不同级别的数据交换和指令分发
arxiv.org
patents.google.com
。 同步控制方面，为了实现晶圆台与掩模台的精准同步扫描和剂量控制，系统设置了专门的同步触发机制。在硬件层面通常采用主从式同步总线架构：一个主同步控制板（MSB）通过同步总线对多个从同步控制板（SSB）发出触发信号，各从板接收信号后按预定要求同时执行动作
arxiv.org
。该同步总线可以是短程的高速线路，也可通过光纤链路实现更远距离的同步
arxiv.org
arxiv.org
。由于整个光刻流程对时序的严格要求，很多重要操作在硬件层面完成触发，以减少软件延迟并保证亚毫秒级同步精度
arxiv.org
arxiv.org
。 **控制系统通信架构示意图：**如上图所示，控制系统各层级通过标准总线连接。主控制层（上位机和主控板卡）通过VME总线与分系统控制层（总线控制板）通信，分系统控制层再通过现场总线与各执行层的I/O控制器相连
patents.google.com
。每个执行层可对应一个子系统的传感/执行硬件架构（如光源控制IO、晶圆台IO等），通过现场总线实现与上层的命令反馈和数据通信。
控制技术
高精度运动控制： 晶圆台、掩模台和其他机械平台采用高速闭环伺服控制。通常使用线性电机、磁浮支撑等无摩擦驱动，通过高频采样位置（激光干涉仪、线性编码器）并在FPGA或DSP上实现伺服算法
arxiv.org
arxiv.org
。为抑制结构振动、提高带宽，还会用并行控制、多通道反馈等技术。在嵌入式硬件中，FPGA和DSP承担实时控制运算和逻辑任务
arxiv.org
，从而保证亚微米级甚至皮米级的位置精度。
同步扫描控制： 光刻扫描方式要求晶圆与掩模的运动严格同步，即使二者具有不同面积比率。除上文提到的同步总线硬件触发外，控制软件也会配合同步算法确保实时误差最小化。总的同步机制可以视为一种专用的实时总线通信方案
arxiv.org
。
实时通信协议： 不同层次采用不同总线：芯片和模块级可用I2C/SPI、UART等，板卡间常用VME总线或其它并行背板；系统级采用以太网（部分使用实时以太网）、CAN总线或RS-485等网络进行数据交换
arxiv.org
。为了满足严格的实时性，有时还使用光纤通信协议（如HSSL）进行主控板与远端从板之间的高速互联
arxiv.org
arxiv.org
。
硬实时保障： 对于要求极高的实时任务（响应时间远低于1毫秒），优先采用硬件实现，如将关键控制逻辑移植到FPGA、ASIC等硬件电路上，而非仅靠软件调度来完成
arxiv.org
。这种硬件加速可以显著减少延迟抖动，满足光刻节拍和同步要求。
控制软件与硬件平台
软件平台： 主控软件一般运行在工业计算机或工作站上，可能采用Windows或Linux等操作系统。在工业侧常见专用框架（如MFC）用于开发人机界面和监控功能
patents.google.com
。各子系统控制器运行实时操作系统，如VxWorks、RT-Linux（Xenomai、RTX64）或其它嵌入式RTOS，以确保控制周期的确定性
arxiv.org
patents.google.com
。
处理器架构： 控制系统采用多种CPU架构协同工作：高层控制可使用SPARC或x86架构（运行Solaris/Linux/Windows）处理数据、运行高层算法或图形界面
arxiv.org
；低层实时控制常用PowerPC、ARM等嵌入式CPU（加载VxWorks或Linux）执行运动控制与数据采集
arxiv.org
patents.google.com
；此外，FPGA用于实现总线协议和关键逻辑，DSP用于高带宽信号处理和运动算法
arxiv.org
。
辅助硬件： 控制系统还包含PLC和ASIC芯片等。PLC可用于系统安全和通用I/O控制，与主控系统隔离以增强可靠性
arxiv.org
。专用ASIC芯片（如CCD图像传感器、CMOS、AD/DA转换器等）用于高性能信号采集和处理
arxiv.org
。上位机与各硬件模块间还需数据库或中间件支持数据交换与参数配置
arxiv.org
。
测量、校准与反馈集成
光刻机对测量和校准功能高度依赖。控制系统通常包括专门的**对位（Alignment）模块和测量校准（Measurement & Calibration）**模块
arxiv.org
。在曝光前，子系统会使用对位传感器（如双轴干涉仪、编码器、摄像机系统等）测量晶圆与掩模的位置、倾斜度和畸变，并通过反馈调整晶圆台高度、镜筒透镜位置或光阑光强等参数
arxiv.org
。例如，在软件结构中就设有对位和校准专用功能块，实时处理测量数据并执行校正操作
arxiv.org
。整个控制闭环依赖高速反馈——光刻过程中，干涉仪等传感器以千至万赫兹速率提供位置信息，控制器实时计算前馈与反馈补偿，使系统维持纳米级稳定精度。同时，光刻机还进行周期性校准：通过已知结构的标准样片或内置标尺测量，修正系统几何和光学误差，保证批次间和时间上的一致性。 参考文献： 上述内容综合了光刻机控制系统相关公开资料，包括科研论文和专利等
arxiv.org
arxiv.org
arxiv.org
pmc.ncbi.nlm.nih.gov
arxiv.org
fab.cba.mit.edu
arxiv.org
arxiv.org
arxiv.org
patents.google.com
patents.google.com
。各参考来源提供了对EUV光刻机控制架构和实现技术的详细描述，进一步阅读可深入了解各子系统的控制细节。
Citations

https://arxiv.org/pdf/2402.15693
Thermal Control Systems in Projection Lithography Tools: A Comprehensive Review - PMC
https://pmc.ncbi.nlm.nih.gov/articles/PMC12388758/
Control of Wafer Scanners: Methods and Developments
https://fab.cba.mit.edu/classes/865.24/topics/mechanical_design/mechanical_design/precision-frontier/Control_of_Wafer_Scanners_Methods_and_Developments.pdf

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693
CN106324999B - 一种用于光刻设备的控制架构 - Google Patents
https://patents.google.com/patent/CN106324999B/zh
CN104158876A - 基于Vxworks操作系统的双工件台光刻机控制系统通信装置 - Google Patents
https://patents.google.com/patent/CN104158876A/zh

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693

https://arxiv.org/pdf/2402.15693
All Sources

arxiv
pmc.ncbi.nlm.nih
fab.cba.mit
patents.google


明白了，我将分别介绍EUV光刻机光源控制系统的以下方面：

1. 光源的种类和工作原理，尤其是激光等离子体（LPP）技术。
2. 光源控制系统的架构，包括硬件和软件层次。
3. 能量输出调控与剂量稳定机制。
4. 与曝光控制系统等其他子系统的协调机制。
5. 光源主要供应商及其技术路线（如ASML Cymer、Gigaphoton等）。

整理完成后，我会以结构化方式展示内容，包含技术原理说明、架构图（如有）、关键技术点和行业现状。

极紫外(EUV)光刻机光源控制系统调研
光源的种类与工作原理
EUV光源主要分为激光等离子体（LPP）和放电等离子体（DPP）两种方案
cnbeta.com.tw
。LPP方案使用高功率激光照射锡（Sn）靶材（通常为微米级液滴），将其蒸发加热成高温等离子体，从而发射13.5nm的EUV辐射；而DPP方案则通过高压电脉冲在靶材周围形成电弧放电产生等离子体
cnbeta.com.tw
。目前，主流商用系统（如ASML/Cymer）采用LPP方案，其高转化效率和功率可扩展性使其成为唯一量产路线
researchgate.net
cnbeta.com.tw
。例如，ASML介绍其LPP光源工作流程：直径约25μm的锡液滴以70m/s速度喷出，先由低能量预脉冲激光将液滴压扁成薄盘状，再由高能量主脉冲将其蒸发成高温等离子体发射EUV光，该过程以每秒约5万次的频率重复
asml.com
。
图：Gigaphoton公司提出的高功率LPP-EUV光源示意（预脉冲激光/主脉冲激光照射锡液滴形成等离子体，并用磁场约束Sn离子以减少污染）
degruyterbrill.com
researchgate.net
。
在等离子体产生过程中，常采用双脉冲技术提高效率：首先用皮秒或纳秒级预脉冲膨胀锡液滴形成雾状薄层，再在合适延迟后用高能主脉冲激光照射，将锡雾加热成高温等离子体
degruyterbrill.com
。锡等离子体的辐射光谱由离子跃迁决定，锡（Z=50）在13.5nm附近有强烈发射线，使其成为目前最优靶材
researchgate.net
。此外，中国等也在研究其他方案，如同步辐射（SRF）和飞秒级电子束自由电子激光（FEL）等，但都未见商业化应用。稳态微聚束（SSMB）加速器光源是清华提出的创新方案，目前已验证原理并申请国家重大科技专项立项
tsinghua.edu.cn
。总体来说，现阶段EUV光源的研究重心仍在提升LPP方案的输出功率和稳定性。
光源控制系统总体架构
EUV光源控制系统需确保输出EUV功率高效稳定，并按曝光机需求同步发射。其硬件层主要包括：
驱动激光器：高功率CO₂激光系统（MOPA结构），用以产生激光主脉冲和预脉冲
researchgate.net
；
光束传输：包括聚焦透镜、反射镜和光束定位系统，将激光准确聚焦到锡液滴位置
researchgate.net
；
锡液滴发生器：在真空室中产生直径约20–30μm的锡液滴，并精确控制液滴位置与速度（常见频率约50kHz
asml.com
）；
集光系统：自由曲面多层膜反射镜，用于收集并聚焦13.5nm EUV辐射至中间焦点（IF）；
污染控制模块：包括磁场约束装置、液滴捕集器和离子截留器等，用于减小锡碎片和等离子体对光学元件的污染；
光学计量模块：光电探测器、CCD相机等传感器，用于测量EUV输出能量、成像锡液滴/等离子体形态及监测激光束参数
cymer.com
researchgate.net
。
在软件层面，光源控制系统通常运行在实时嵌入式控制器上，执行脉冲同步、能量调节和反馈算法等。例如，源控制器（Source Controller）根据扫描仪发出的曝光命令生成激光脉冲序列
cymer.com
。为了实现精确控制，系统集成了高性能控制算法和操作系统，负责激光器的高频调制、液滴喷射同步、以及计量反馈闭环运算等。学者曾提出利用光学成像跟踪和电子控制软件的综合系统，通过实时图像检测锡液滴并进行反馈调整，从而极大提高了液滴稳定性和输出稳定性
stars.library.ucf.edu
。总体而言，光源控制系统通过硬件模块与多层次反馈回路（实时监测锡供应、激光定时与能量、等离子体状态等
future-bridge.us
）相结合，实现对高能量密度EUV光源的精细控制。
能量输出调控与剂量稳定机制
稳定的EUV剂量输出依赖于对激光功率、射频率和靶材供应的精细调节以及反馈控制：
激光能量和脉冲频率调控：通过改变CO₂激光器的泵浦功率和放大器增益，可调节每脉冲的能量。当前系统所需的CO₂输入功率已达到数十kW量级（例如Gigaphoton曾使用20kW/27kW激光进行250W输出演示
researchgate.net
researchgate.net
）。根据目标功率需求，系统会实时调整脉冲重复频率（通常几十kHz量级）和脉冲能量，使平均输出满足曝光要求
future-bridge.us
。
锡液滴供应控制：锡液滴发生器需要稳定地以固定频率（如50kHz）和精确相位发射液滴。通过伺服控制液滴喷射设备的振动和墨盒供锡机制，可保持液滴大小、间隔和位置恒定
asml.com
。同时，控制系统可根据反馈改变液滴频率或位置来补偿热漂移或堵塞等状况。
输出反馈闭环：光源安装有多种监测传感器，对输出能量和等离子体状态进行实时测量。例如，EUV功率探测器（光电二极管）监测中间焦点处能量强度，CCD相机或光学传感器成像等离子体和液滴位置
cymer.com
researchgate.net
。控制系统将这些信号反馈至控制器，自动调整激光输出和液滴对准等参数，以减小脉冲间能量抖动和时序误差。实际工作中，ASML等系统已通过先进的闭环控制技术实现了极高的剂量稳定度：据报道采用MOPA+预脉冲技术可输出250W的稳定EUV源（光学收集效率约6%），并将剂量抖动控制在0.1%以内
researchgate.net
。
剂量控制与稳定性：光源控制器根据扫描仪曝光需求以脉冲串模式输出能量，并利用上述反馈回路极大提高重复性。例如Cymer文献中提到，通过带宽控制器和多级反馈环，能快速抑制人为扰动，实现低至0.05%量级的输出稳定度
researchgate.net
stars.library.ucf.edu
。在实际曝光中，这意味着每个芯片版图的剂量误差极小，以支持亚纳米级图案生成。
光源控制系统与其他子系统协同与数据交互
EUV光源控制系统与光刻机其他子系统紧密协同：
曝光/扫描控制：扫描仪（刻写控制系统）负责整个晶圆的运动和曝光时序，并向光源控制系统发送“开启/关闭激光脉冲”的指令
cymer.com
。光源控制器根据这些指令生成相应的激光脉冲序列和锡液滴喷射定时，确保射线曝光与晶圆移动同步，从而获得均匀剂量。
剂量反馈系统：曝光过程中，扫描仪内置的计量传感器（如光电二极管、在制品测量阵列等）实时监测晶圆表面的实际曝光剂量分布。如果检测到局部剂量偏差，系统可向光源控制器反馈，调整后续扫描的激光功率或曝光时间，以修正总剂量。虽然具体反馈算法属于机器控制范畴，但其关键在于两者的数据交互和实时响应。
掩模控制：掩模平面上图案的控制主要由掩模台和光学元件完成，光源系统不直接参与掩模的定位调节。但系统会协同工作，例如光源输出特性需满足掩模斜率和反射率设计；同时，掩模平坦度控制和颗粒污染也会通过曝光成像反馈，间接影响光源工作模式。
系统集成：总的来说，光源控制器、扫描控制器和掩模控制器通过总线和实时网络相连，交换时序信号和监控数据。源控制系统提供EUV功率和稳定度指标给曝光监控子系统，曝光系统则提供场位置信息与剂量需求给光源。在这种协同机制下，EUV光刻机才能实现高产量和高良率生产。
主要光源供应商、技术路线与产业现状
目前，全球EUV光源技术主要由以下几类厂商推动：
ASML（Cymer）：目前唯一量产EUV光刻机供应商，其光源系统由内部子公司Cymer开发。采用CO₂-LPP路线，核心技术包括高功率CO₂激光器、大规模MOPA激光腔、精准锡液滴发生器和多层膜反射集光镜等。ASML的NXE:3400B机型（2017年投产）标配约250W中间焦点功率（满场曝光对应125W实用功率，转化率约6%）
researchgate.net
。后续NXE:3400C已实现250W/170WPH（96片/小时）水平
researchgate.net
；最新实验室内测试中，ASML在下一代NXE:3600D原型机上已达到420W剂量控制输出和530W开环峰值
researchgate.net
researchgate.net
。为了进一步提升功率，Cymer等团队正在研究更高功率激光（目前已达27kW CO₂）和新型喷雾技术
researchgate.net
researchgate.net
。总的来说，ASML/Cymer掌握了行业最高的生产化技术和控制算法，持续引领EUV光源的进步。
Gigaphoton（Komatsu）：日本Komatsu旗下公司，从2000年起开发EUV光源，同样走CO₂-LPP路线。其高功率EUV光源（如GL200E）在研发阶段实现了250W输出示范，并报告过采用皮秒预脉冲可获得高达4.7%转化率
researchgate.net
。Gigaphoton研发团队强调双脉冲优化和磁场约束技术，如通过纳秒/皮秒预脉冲生成亚微米锡雾，再用20kW CO₂激光加热，实现了250W功率级别
degruyterbrill.com
researchgate.net
。据报道，Gigaphoton已将激光泵浦功率提升至27kW，实现短时间内>360W输出，并在测试中稳定运行270W
researchgate.net
。目前该公司暂无出售量产机，但其光源研发进展对产业形成竞争压力。
Ushio（含Xtreme Tech）：主要研发DPP（放电等离子体）EUV光源。Ushio曾收购飞利浦旗下Xtreme公司，将DPP技术纳入组合。但DPP方案至今输出功率远低于LPP，仅处于实验室演示阶段
cnbeta.com.tw
。随着光源市场重组（日本小松与Ushio于2011年终止Gigaphoton合资
cnbeta.com.tw
），Ushio目前更多作为设备和子系统供应商参与海外项目。总体上，DPP路线尚未获得HVM认可。
中国及其他研究机构：尽管中国尚无量产EUV工具，国产厂商和科研机构积极跟踪国外技术。除了研究LPP和DPP外，清华等提出了稳态微聚束（SSMB）加速器光源概念，并已在粒子加速器测试中验证原理
tsinghua.edu.cn
。中国团队还在探索使用2μm波长激光和纳米锡喷雾等创新方案（如报告称2μm驱动锡靶转化率超过3%
researchgate.net
）。然而，这些路线距离实际产线应用仍有较大差距，目前业界仍以提升LPP输出和可靠性为主流趋势。
产业现状与发展趋势：EUV光源正向更高功率和更高稳定性发展。ASML目标将源功率提升到500W甚至更高，以满足下一代工艺需求；同时各方加紧优化预脉冲技术、喷雾系统和污染控制，以延长系统可用时间
researchgate.net
researchgate.net
。长期来看，若目前LPP路线遇到瓶颈，业界可能转向包括多光纤相干合成、纳米级靶材技术乃至SSMB/FEL等创新方案，但这些都需克服复杂的技术挑战。总体而言，ASML/Cymer凭借成熟的CO₂-LPP技术占据领先地位，Gigaphoton紧随其后，各方均以提高输出功率、系统稳定性及集成度为主要研发方向
researchgate.net
researchgate.net
。
参考资料：以上内容综合了ASML官网介绍、Cymer/Gigaphoton技术报告以及相关行业论文和会议资料
asml.com
cnbeta.com.tw
researchgate.net
future-bridge.us
researchgate.net
researchgate.net
tsinghua.edu.cn
等

明白了，我将分别介绍EUV光刻机光源系统中的六个关键模块：驱动激光器、光束传输、锡液滴发生器、集光系统、污染控制模块、光学计量模块。

每个模块将包括：

1. 结构组成
2. 工作原理
3. 关键技术参数
4. 控制方法
5. ASML与Gigaphoton等主要厂商的技术路线对比（如适用）
6. 模块结构图或示意图（如有）

整理完成后，我会以清晰分节的方式进行汇报。

驱动激光器模块
驱动激光器通常采用主振荡器-功率放大器（MOPA）结构，高功率CO₂激光器产生激发脉冲。主振荡器（Seed Laser）产生短脉冲（采用Q开关或腔内抽取技术），然后通过逐级放大产生更强的脉冲。TRUMPF（为ASML提供光源）等公司设计的系统包含高功率种子模块和4~5级主放大器链路，可将初始几瓦的种子脉冲放大至约40 kW级功率
pdf.dfcfw.com
trumpf.com
。通过光学平台（FFA）分离出低功率的预脉冲与高功率的主脉冲，前者预热并展开锡滴，后者以数十kW击中锡盘产生等离子体
trumpf.com
pdf.dfcfw.com
。
架构组成：种子振荡器、隔离器、前置放大器和主放大器链路（简称HPSM和HPAC模块）。TRUMPF激光器由约45.7万零件组成，重量约17吨
pdf.dfcfw.com
。Gigaphoton采用自有的GigaTwin双腔注入锁定技术（一腔产生低功率种子，另一腔放大至高功率）
gigaphoton.com
。
脉冲产生方式：通过调Q或腔抽取得到短脉冲（典型脉宽~10 ns，重复频率匹配滴频50 kHz），FFA光学平台按需将种子脉冲分裂为“预脉冲”（低功率）和“主脉冲”（满功率），前者先对锡滴预加热、形成薄饼，后者打出等离子体
trumpf.com
。
主要参数：波长10.6 μm，脉冲重复率几十kHz（对应锡滴发生频率），典型平均功率40 kW级（单脉冲峰值可达兆瓦级）
trumpf.com
pdf.dfcfw.com
。
控制方法：实时监测放大器功率，通过闭环反馈调节激光输出稳定度（TRUMPF系统配备在线测量实现功率稳定）
trumpf.com
pdf.dfcfw.com
。激光束经过光束传输系统（BTS）后，再经焦点单元准确聚焦于锡滴，对准精度要求在微米量级。
厂商差异：ASML/Cymer（与TRUMPF合作）使用TRUMPF定制激光器系统；Gigaphoton则与三菱电机合作开发CO₂放大器，并采用上述双腔注入式设计
gigaphoton.com
heyangtek.cn
。两者基本原理相似，都需要预脉冲加热与主脉冲全功率输出，但Gigaphoton宣称已开发出118W级别的EUV光源并正研发更高功率激光器
heyangtek.cn
。
光束传输系统
光束传输系统（BTS）负责将CO₂激光束从激光器稳定地送至锡滴腔。系统由准直透镜、反射镜和支撑构件组成，可在约30米距离内传输激光束
pdf.dfcfw.com
。反射镜采用特殊涂层以尽量减小激光能量损失
pdf.dfcfw.com
；光学平台（FFA）则用于将激光束分裂为预脉冲和主脉冲并进行调制与整形
pdf.dfcfw.com
，最终聚焦在锡液滴上。传输过程中要求对光束的大小、轮廓和位置精确控制：Gigaphoton在光路中引入了高速可调光束扩束器（BEX），通过实时监测并调节光束尺寸确保激发光束的稳定性和均匀度
researching.cn
；同时配备高速激光轴控制系统，捕捉锡滴位置并反馈给激光定时系统，实现激光束与滴的精准同步
researching.cn
researching.cn
。
结构与原理：BTS包括准直/聚焦透镜和一系列反射镜，将激光束传输到微滴发生腔。光学平台（FFA）内含分束器，可按能量分配产生低功率预脉冲和高功率主脉冲，并通过独特设计优化光束形状与空间分布
pdf.dfcfw.com
。
控制精度：光束尺寸和位置偏差需严格控制，否则可导致聚焦误差或损伤光学元件。为此系统中增设光束动态整形模块及位置传感器，对光斑轮廓和对准进行闭环调整
researching.cn
researching.cn
。
技术差异：不同厂商镜面材料或涂层略有差异，但均选用高耐损的低吸收材料。在焦距控制方面，ASML/TRUMPF常用大直径准直透镜配合精密调节机构；Gigaphoton则重点使用可调光束扩束技术来实时优化光斑尺寸
researching.cn
。总体来说，ASML系统和Gigaphoton系统在BTS设计上思路相似，只是具体实现细节（如镜面涂层工艺或激光束控制软件）有所区别。
锡液滴发生器
锡液滴发生器通过连续注射熔融锡流并利用瑞利射流不稳定性形成液滴。在发生器内部，固态锡被加热熔化后，经滤器和惰性气体加压后由微小喷嘴挤出细小液柱
researching.cn
。喷嘴处环绕有压电驱动元件，施加高频振动扰动液柱，导致其以固定频率断裂形成单分散锡滴
researching.cn
researching.cn
。由于商用EUV要求极高重复率（通常数十至上百kHz），一般采用雷利射流连续出滴方案，而非按需喷射（DoD）方法，因为后者频率受限（<10 kHz）
researching.cn
。
结构组成：主要包括锡熔融腔、滤网、微喷嘴与环形压电振子。锡液由加热器加热至熔点后流向喷嘴，经膜滤器去除杂质，再经惰性气体推动进入喷嘴
researching.cn
。喷嘴周围安装压电环，通过电信号驱动产生径向微振动，从而扰动出流液柱使其断裂成滴。
液滴形成机制：利用雷利射流不稳定性，在压电激励下液柱定期颈缩断裂产生液滴。液滴尺寸通常几十微米（ASML约27 μm，Gigaphoton约20 μm），发射频率与激光脉冲重复率同步（ASML ~50 kHz，Gigaphoton可达100 kHz）
researching.cn
。滴柱断裂后的液滴会由球形演化为略扁形以抵消表面张力振荡
researching.cn
。
尺寸与频率：液滴大小与频率是权衡转换效率和碎片量的关键参数
researching.cn
。ASML系统采用较大锡滴（约27 μm）配合50 kHz发射；Gigaphoton则使用较小滴径（约20 μm）配合100 kHz发射
researching.cn
。
同步与定位精度：锡滴下落速度在数十米/秒级，为了精确击中，系统需要微米级和纳秒级同步控制。通过高速照相与激光轴反馈，实时捕捉锡滴位置，并将信息反馈给滴发生器和激光定时器
researching.cn
researching.cn
。控制回路自动调整预脉冲/主脉冲延时，使激光束始终准确击中液滴，射击精度可控制在±2 μm以内
researching.cn
。ASML系统已实现约1 μm级的横向稳定性（标准要求约5 μm）
researching.cn
。
集光系统
集光系统采用曲面反射镜（接近近轴后向入射的非球面或椭球面），将锡等离子体发射的13.5 nm EUV光聚焦至中间会聚点（IF），然后由多镜投影物镜进入晶圆。集光镜通常是大口径自由曲面（边缘入射角接近45°），并在表面镀制50~60对Mo/Si多层膜以获得高反射率
researching.cn
researching.cn
。由于曲率变化，镀膜厚度从镜心到边缘需呈梯度变化，厚度控制精度须达到原子量级
researching.cn
。前述Mo/Si系统是目前最优选材：尽管Mo/Be在反射率上可能更高，但需更多膜层、带宽变窄，且Be为有毒元素，综合效率并不优于Mo/Si
researching.cn
。为了延长寿命和减小界面扩散，还会在多层膜中加入碳或B₄C阻隔层，并在顶层加超薄氧化物保护膜
researching.cn
。
自由曲面结构：集光镜呈大口径曲面（如椭球面）形式，设计焦距将EUV光会聚到投影物镜的中间焦点
researching.cn
。表面倾斜角高，要求极高的加工精度和沉积精度。
多层膜材料：经典材料是Mo/Si交替层，多层数50对以上可将反射率提高至~70%左右
researching.cn
。目前实验中ASML/蔡司制备的首代NXE:3100集光镜在13.5 nm处实测最高反射率达70.15%
researching.cn
。为了进一步提高效率，界面工程（碳、B₄C等阻隔层）和界面钝化（超薄氧化层保护膜）成为重要技术
researching.cn
。
镜面精度与热变形控制：集光镜面粗糙度需控制到皮米/纳米级并保持形貌稳定。由于等离子体产生的光学负荷高，集光镜一般内置冷却系统以散热并抑制热型变形，从而维持聚焦质量。
效率与寿命：光谱带宽与多层层数成反比，过多层数虽提高峰值反射率但降低带宽通量。研究表明Mo/Be方案虽然理论反射率略高，但需要更多层数、带宽窄，实际成像通量不及Mo/Si
researching.cn
。因此业界采用Mo/Si为主，并通过抗污染层和定期清洗（或惰性气体保护）延长寿命。
污染控制模块
污染控制模块采用多种装置协同防护集光镜免受锡碎片污染。系统通常包括磁场偏转装置、离子屏蔽和物理捕集装置等。在真空腔中通入氢气等缓冲气体（典型密度相当于约0.1 Torr）以降低锡离子的能量并稀释污染物
patents.google.com
。集光镜背面安装螺旋形线圈或多组永磁体产生强磁场，将带电锡离子偏转离镜面，结合浮动金属箔片或网格陷阱捕集非带电碎片
patents.google.com
patents.google.com
。这些措施可最大程度减缓碎片飞行速度，避免锡原子或离子直接沉积于镜面，从而减缓反射率的下降
researching.cn
patents.google.com
。
原理与构成：磁偏转系统由线圈（或钕铁硼等材料的永磁体阵列）构成，置于集光镜与锡靶之间，用以偏转离子碎片
patents.google.com
。同时在集光镜前方设置旋转金属箔陷阱或静电网，物理截留未偏转的锡原子团簇。系统中还通入氢气等缓冲气体（如0.1 Torr H₂）进一步与飞行碎片碰撞，使其动能衰减
patents.google.com
。
控制目标：主要防止锡沉积在集光镜和其他光学元件上（锡沉积会快速降低反射率并引入氧化层）
researching.cn
。磁场与陷阱的联用可保护镜面“几乎免受等离子体产生的离子污染”
patents.google.com
。
厂商差异：ASML和Gigaphoton等均采用类似多重防护策略，但具体实现有所区别。ASML设备已知使用多级旋转箔片与高电流线圈来清除碎屑，而Gigaphoton也在强调其“电-磁场”复合防护方法（参见ASML专利US7196342）。目前公开资料中未详细披露两者结构差异，但思路相当：磁场偏转离子、缓冲气体减速、物理陷阱捕捉中性颗粒，同时回收有价值的锡液（回收装置）以循环使用
pdf.dfcfw.com
。
光学计量模块
光学计量模块布置高灵敏度传感器实时监测关键参数并反馈控制系统。常见的检测装置包括阴影成像CCD、高速相机、光电探测器等。阴影成像系统在锡滴下方垂直方向设置CCD传感器和高速脉冲LED闪光灯，利用同步触发技术捕获飞行液滴的轮廓和尺寸
researching.cn
。这些图像信息可用于监测滴径、下落轨迹并调节喷嘴驱动。激光束的位置和强度通常由专用传感器监控，结合高速光轴控制系统对准液滴
researching.cn
。此外，在中间焦点等处布置光电探测器可实时测量输出的EUV功率及谱纯度，作为光源功率和光束品质的反馈指标。所有测量数据经由高速控制回路反馈到激光和发生器控制器，实现能量、轨迹、定时等多维参数的闭环调节
researching.cn
。
成像测量：典型做法是在照明光轴侧放置CCD或CMOS相机，并用高功率LED同步闪光来“冻结”运动中的锡滴
researching.cn
。清晰的阴影图像可准确测量液滴直径和偏移，确保湿锌靶在正确的位置。
激光与EUV功率监测：利用分束镜和光电二极管实时采样CO₂激光脉冲参数，以及在IF附近使用EUV光电探测器监测集光效率和瞬时功率。系统可根据检测到的偏差自动微调激光能量或对准。
闭环控制：如前述射击控制回路所示，液滴位置、激光光轴和脉冲定时构成闭环调节系统
researching.cn
researching.cn
。实测中，ASML系统通过该闭环可在每次击中中保持激光-液滴对准在±2 μm以内
researching.cn
，从而稳定输出。Gigaphoton同样强调光学测量和软件反馈（其Fabscape平台可整合多种供应商设备），以保障长期稳定运行。


下面把 **EUV 光源舱**（以 ASML/Cymer 为参考）中常见的气体/环境与等离子体参数监测方案做系统化介绍：测什么、怎么测、放在哪儿、数据如何用、以及在工业光刻机中的典型实现与注意点。为便于阅读我把内容分节并给出常见仪器/方法优缺点对比与用途建议。

> 重要资料来源（部分）：激光等离子体/等离子体诊断综述、NIST 光学束线说明、ASML 相关专利与光源/收集器专利、RGA/真空监测器与 OES/Langmuir 诊断研究。([Physical Review Links][1], [NIST][2], [Google Patents][3], [MDPI][4], [thinksrs.com][5])

---

# 1) 需要监测的对象（简要清单）

* **气体成分与纯度**：H₂、Ar、He（如果使用）、残留 O₂、H₂O（水蒸气）、碳氢类（有机污染物）、锡相关蒸汽/团簇（Sn 原子/分子）。
* **真空/压力**：总压、各分区局部压强（例如激光传输段、滴腔、集光区、投影腔等）。
* **等离子体关键量**：电子密度、电子温度（或激发度）、离子种类与谱线强度、等离子体辐射光谱（EUV 强度与谱构成）。
* **环境与过程参数**：温度、湿度（真空前/气体供给段）、气体流量与质量流量控制器 (MFC) 状态、气体纯度报警。
* **污染/沉积速率**：光学面上沉积速率（量级）、颗粒/碎片计数（debris counters）、QCM（石英晶体微天平）或见证片测厚。
* **安全与辅助量**：氢气泄漏检测（安全）、进/排气口压力与阀位、泵状态等。

---

# 2) 常用检测方法与传感器（方法 → 说明 → ASML/工业化适用性）

### A. 残余气体分析 — **四极杆质谱 / RGA（Residual Gas Analyzer）**

* **做什么**：测各分子/原子成分的相对与（经校准）绝对 **分压/含量**（例如 O₂、H₂、H₂O、有机碎片、Snx）。
* **原理**：抽取真空局部气样 → 电离 → 四极杆质量选择 → 检测。
* **优点**：能定性/半定量识别多种残留气体，灵敏（10⁻⁹–10⁻¹¹ Torr 级别可检测）。适用于持续监测与故障分析。
* **局限**：对短脉冲等离子体的瞬态响应较差（需要额外采样/触发策略）；需要定期校准。
* **工业实践**：ASML/beamline 等在真空腔和气体进出点常装 RGA 用于气体纯度与 outgassing 监测（NIST 等示例）。([NIST][2], [thinksrs.com][5])

### B. 光学发射光谱（OES — Optical Emission Spectroscopy）

* **做什么**：测等离子体中发射谱线（原子/离子种类、相对激发强度），用于推断**物种存在、相对浓度、电子温度/激发态**等指标。
* **原理**：用光纤或望远镜收集等离子体发射光，入光谱仪/光栅或 ICCD/光谱相机分析谱线。
* **优点**：无创、快速，可获得瞬态（ns–μs）信息，适合监测等离子体谱线（如 Sn 离子谱）、诊断激发状态与放电/激光击中效果。
* **局限**：通常给出相对强度；要做绝对密度需结合 actinometry（标定气体技法）或其他参考。对 E IU V 本身的直接测量需配合专门 EUV 探测器。
* **工业实践**：用来在线监测等离子体质量/稳定性、检测异常放电或污染物发射线；常与其他诊断联用（见下）。([Physical Review Links][1])

### C. Langmuir 探针（针式探针）与电学探测

* **做什么**：直接测电子密度、电子温度、等离子体电势等（实验室常用）。
* **原理**：在等离子体中插入探针，测 I–V 曲线。
* **优点**：直观、可得电子能量分布/密度。
* **局限**：**侵入式**，高温/折损/污染环境中不耐用，在工业 LPP 高能环境里往往难以长期在线部署；多用于研发/试验台。常与 OES 组合以互校正。([MDPI][4], [AIP Publishing][6])

### D. 微波/毫米波干涉仪与反射/散射诊断（非侵入式测电子密度）

* **做什么**：测等离子体电子密度（平均或沿线积分密度）。
* **原理**：微波或激光穿过等离子体，相位/振幅发生变化（干涉或反射），从相位延迟反推出电子密度。
* **优点**：非侵入、快速、对时间演化敏感。
* **局限**：需要开口或窗口和较复杂的校准；工业化部署在高能短脉冲 LPP 中实现难度较高但在研发中常用。([Physical Review Links][1])

### E. 谱线展宽 / Stark 展宽、谱学温度诊断、Thomson 散射

* **做什么**：通过谱线展宽评估电子密度（Stark 展宽）或用 Thomson 散射精确测电子温度/密度。
* **原理**：高分辨光谱测线形或用激光作探针测散射光谱。
* **优点**：Thomson 非常准确且非侵入；谱线方法在等离子体诊断中常用。
* **局限**：Thomson 实验复杂/信号弱，工业上少用于常规在线监测，多见于科研。([Physical Review Links][1])

### F. 真空/压力传感（电容式压力计、热导、冷阴极/热阴极/离子规）

* **做什么**：测总压与分区压力（例如滴腔压力、集光区压力、泵导管压力）。
* **说明**：热导与电容式压力计适合较高压范围；冷阴极/离子规（ion gauge）适合高真空到超高真空。压力是保护镜面与控制气氛的关键参数。
* **工业实践**：多点布置、与气体流量控制器联动。([NIST][2])

### G. 气体纯度/漏检与在线气体分析（氧传感器、氢传感器、气相色谱）

* **做什么**：对进气（供给 H₂、Ar 等）的纯度与泄漏检测（安全用 H₂ 检漏）。
* **说明**：工业上在气路末端常装高灵敏氧/水/氢传感器与质量流量控制器（MFC）反馈；对于深度分析可用便携/在线气相色谱或专用痕量 O₂/水分析仪。

### H. EUV 输出 / 辐射测量（光电二极管、功率计、光谱仪）

* **做什么**：测中间焦点（IF）的 EUV 功率、脉冲能量与谱构成。
* **说明**：常用 EUV 专用光电二极管、辐照计或热量计（calorimeter）来做瞬时功率/能量测定；结合光谱仪可得谱纯度。ASML 等对 EUV 输出功率的在线计量非常重视（见专利）。([Google Patents][3])

### I. 污染/沉积监测（QCM、见证片 / witness plates、光学反射率监测）

* **做什么**：监测镜面沉积速率或透镜/镜面反射率变化（长期退化）。
* **说明**：QCM（石英晶体微天平）放置在近镜面或代表位点可实时给出沉积速率；见证片周期性取出或通过内置反射测量看到反射率损失。用于决定清洗/维护节奏。([Google Patents][7])

---

# 3) 传感器布置（典型位置）

* **气路/供气端**：MFC、纯度传感器、气体泄漏（H₂）与安全阀；在这里在线气相/痕量 O₂ 或水分析能控制进气质量。
* **真空腔体若干取样口**：RGA（或 QMS）通常连在泵旁或专门采样口，用于长期残气监测与突发污染诊断。([NIST][2])
* **等离子体视场**：通过光纤或望远镜采集 OES 信号并引入光谱仪/ICCD；高速相机/阴影摄影口用于液滴成像。
* **中间焦点（IF）或收集镜近旁**：放置 EUV 光电探测器、见证片、QCM，以测 EUV 功率与沉积速率（注意这些位置对光学和热负荷有严格要求）。([Google Patents][3])
* **集光镜周边**：布置磁偏转线圈与陷阱，同时在外环布置监测端口用于测局部离子/中性粒子（通常通过小口径 RGA 或探测器）。

---

# 4) 如何把测到的数据用于控制与保护（典型闭环/联动）

* **气体回路控制**：RGA + 纯度传感器发现 O₂ / H₂O 升高 → 自动降低/切换供气源、触发更换过滤器或暂停光源运行（以防光学污染或反应）。
* **等离子体稳定化**：OES / EUV 探测器指示发射强度/谱变化 → 调整激光能量、预脉冲能量或脉冲定时以恢复目标发射线强度与能量分布。
* **镜面保护/清洗触发**：QCM 或反射率监测显示沉积速率超阈值 → 启动旋转箔片、增加 H₂ 冲洗、或触发清洗维护策略（减少集光镜寿命损失）。([Google Patents][7])
* **安全 interlocks**：H₂ 报警或压力异常 → 快速切断激光、关闭供气、停止泵或进入安全态。
* **过程诊断**：RGA 突增特定碎片谱（例如 Sn 聚集）→ 结合高速相机影像诊断液滴异常、喷嘴堵塞或主脉冲异常，从而精准定位故障模块。([NIST][2])

---

# 5) ASML / 工业化要点与实践经验（要点式）

1. **多传感器复合诊断**：单一方法不能完整表征 LPP 瞬态过程。ASML/工业实践把 RGA（气体成分）+ OES（等离子体）+ EUV 探测器（功率）+ 高速影像（滴/碎片）+ QCM（沉积）联合使用以获得可操作的控制量。([Physical Review Links][1], [NIST][2])
2. **在线/瞬态差异管理**：RGA 擅长稳态/慢变化；OES/高速探测用于瞬态（ns–μs）诊断。工业系统把二者结合，且给 OES/相机等更高带宽的数据通路，以便对激光-液滴事件进行微秒级或更短触发采样。([Physical Review Links][1])
3. **探针类诊断多用于研发**：Langmuir、Thomson 等高精度/侵入式诊断在开发台上常见，但工业化机台更多采用无接触光学/质谱/热量计等可长期运行的传感器。([MDPI][4], [Physical Review Links][1])
4. **反馈控制链路必须有安全优先与冗余**：因为光源对光学污染极敏感，控制策略通常把“保护镜面/光学”放在优先级高于短期产能的层级（检测到污染风险系统会优先保护镜面）。专利与工程文献对此有明确描述。([Google Patents][7])

---

# 6) 工业化实施建议（如果您要设计或评估光源检测子系统）

* **短期（产线可行）配置建议**：RGA（泵旁/采样口）、高带宽 OES（多视角，收集 Sn 光谱段和 H/Ar 特征谱）、EUV 光电探测器（IF 处）、QCM（代表镜位）、高速相机（滴成像）与多点压力计。把这些信号汇入实时控制器（FPGA/DSP）做快速闭环，慢速数据送到主控机做趋势/报警。
* **研发/深入诊断配置**：补充微波干涉或 Thomson 散射用于电子密度精确测量；在试验台上可用 Langmuir 探针做点测以校准 OES/模型。([Physical Review Links][1], [MDPI][4])
* **标定与互校**：使用标准气源、校准光源与见证片定期校准 RGA、OES、EUV 探测器与 QCM，避免“漂移误判”。NIST/beamline 等方法可作为校准参考。([NIST][2])

---

# 7) 结论（摘要）

* EUV 光源舱内的气体与等离子体检测是一个**多传感器、跨频带**的诊断体系，常见方法分别侧重于**成分（RGA）**、**瞬态光谱/状态（OES、相机）**、**电子参数（Langmuir/干涉/Thomson 在研发中）**、**沉积/污染（QCM/见证片/反射率监测）** 与 **EUV 输出（光电二极管/功率计）**。工业化机台（ASML）把这些测量集成到控制回路中，用于实时稳定化、污染防护与安全联锁。([Physical Review Links][1], [NIST][2], [Google Patents][3])

---

如果您愿意，我可以：

* 把上面每一种检测方法做成一页的技术说明表（包含典型厂商型号、量程/灵敏度、响应时间、安装要点），便于工程选型；或者
* 针对 **“如何在光源舱布置 RGA + OES + QCM + EUV 探测器 的实时闭环”** 给出示意框图与数据流/控制时序建议。

您想先看哪一种？

[1]: https://link.aps.org/doi/10.1103/RevModPhys.94.035002?utm_source=chatgpt.com "Optical diagnostics of laser-produced plasmas | Rev. Mod. Phys."
[2]: https://www.nist.gov/laboratories/tools-instruments/beamline-8-surface-chemistry-durability-euv-optics?utm_source=chatgpt.com "Beamline 8: Surface chemistry & durability of EUV optics"
[3]: https://patents.google.com/patent/US7394083B2?utm_source=chatgpt.com "US7394083B2 - Systems and methods for EUV light source ..."
[4]: https://www.mdpi.com/2076-3417/10/22/8117?utm_source=chatgpt.com "Langmuir Probe Diagnostics with Optical Emission ..."
[5]: https://www.thinksrs.com/products/rga.htm?utm_source=chatgpt.com "Residual Gas Analyzer"
[6]: https://pubs.aip.org/aip/pop/article/27/8/083501/108909/Langmuir-probe-optical-and-mass-characterization?utm_source=chatgpt.com "Langmuir probe, optical, and mass characterization of a ..."
[7]: https://patents.google.com/patent/US20070023705A1/en?utm_source=chatgpt.com "US20070023705A1 - EUV light source collector lifetime improvements"


下面给您一份针对 **ASML（含其 Cymer 光源子公司）在 EUV 光源系统控制** 上的详尽说明：包含控制目标、架构（软/硬件）、关键闭环、诊断与测量、碎屑/镜面保护、气体/安全联锁、典型实现细节以及近期发展方向（例如三脉冲）。我把内容分节，便于查阅 —— 每节后标注最关键的参考来源。

---

# 1. 控制目标（为什么要复杂控制）

ASML 对光源控制的目标非常明确：在尽可能高的 **中间焦点（IF）净EUV功率** 下，同时保证 **极低的剂量波动、最小的碎屑/污染、可接受的可用度（uptime）和安全运行**。为此光源控制不仅要给出脉冲/能量、滴-激光同步、真空/气体品质与碎屑防护的实时控制，还要实现故障预测与维护调度。([ASML][1], [EUV Litho][2])

---

# 2. 总体控制架构（分层与模块）

典型分层（从上到下）：

* **主控层（大机/工作站）**：工艺计划、产能目标、长期统计与维护管理（MES/SCADA接口）。（非实时）
* **协调/序列器层**：调度曝光节拍、分配光源功率预算、下发场/片级曝光指令。
* **源控制器（Source Controller，实时）**：负责将上层策略转为激光/滴发生/气体/真空/保护等动作序列，管理多路反馈回路（亚毫秒到微秒级）。
* **硬实时子模块（FPGA/DSP/嵌入式控制器）**：实现纳秒–微秒的精确定时（激光触发、预/主脉冲延时、滴-激光时序）、伺服控制（喷嘴、光轴）与高速数据采集（相机/光谱）。
* **现场设备层**：激光放大器、MOPA 控制板、滴发生器驱动、真空泵/阀、磁偏转线圈、旋转箔单元、传感器（RGA、OES、EUV 探测器、QCM、压力表等）。

硬实时控制与时序通常由 FPGA/专用定时卡实现；上层运行工业 PC 或实时OS 做协调与人机界面。ASML 收购 Cymer 后将这些功能高度工程化以满足 HVM 要求。([Cymer][3], [ASML][1])

---

# 3. 时序与同步（最关键的实时任务）

* **滴-激光同步（droplet timing）**：锡液滴以高频（典型 \~50 kHz）抛出，控制器必须检测单个滴的到位相位并以纳秒级精度触发预脉冲与主脉冲。为此采用高速光学/散射探测（光电二极管或高速相机）捕获滴到位信号，并由 FPGA 计算延迟发出激光触发。该类专利与实现说明了“在硬件层面做时间配准并补偿滴速/相位漂移”的方法。([Google Patents][4], [Cymer][3])
* **双脉冲（pre-pulse + main-pulse）与多脉冲拓展**：控制器精确控制两（或更多）个激光输出的能量、波形和延时（ns 级），以最大化转换效率与减少碎屑。ASML 正在推进“三脉冲”方案以进一步提升转换率，这将对控制器提出更高的多通道纳秒级定时与参数优化要求。([STROBE][5], [SemiWiki][6])
* **实时闭环带宽分配**：某些波段（滴-激光相位、激光脉冲能量）要求微秒或更短的响应；其它（如 RGA 气体变化）为秒级。系统在硬件/软件上区分“硬实时回路”和“慢回路”，各自由不同硬件（FPGA vs IPC）承担并通过总线协调。([Cymer][3])

---

# 4. 关键闭环与诊断回路（哪些量被实时闭环控制）

* **激光能量闭环**：使用激光前端与放大链路的功率传感器/探测器做内部反馈，维持单脉冲能量与平均功率在目标值附近（补偿温漂、器件衰减）。上游测量与下游 EUV 探测器结合来做“能量—输出”相关性校正。([Cymer][3])
* **滴位/射击闭环**：高速摄像或光电传感器检测滴位置/形状 → FPGA 计算并实时调整激光触发延时与预脉冲参数；当滴异常（失相、破碎、尺寸偏差）时触发保护或尝试重同步。专利中描述了多传感器融合以提高滴时序控制鲁棒性。([Google Patents][4])
* **等离子体质量监测→能量调节**：通过 **OES（光学发射光谱）**、EUV 光电探测器、以及必要时的 RGA/质谱信息判断等离子体发射谱与强度；若发现目标谱/功率偏离，控制器调整主脉冲/预脉冲能量或滴尺寸/频率。([Cymer][3], [EUV Litho][2])
* **污染/沉积阈值闭环**：QCM、见证片或反射率监测若检测到沉积速率上升，控制器可自动增加保护措施（如提高磁偏转场、旋转箔清除动作、短时开更多冲洗 H₂）、或降额运行并发出维护警报（保护镜面优先）。ASML 专利详细描述了此类多级保护策略。([Google Patents][7])

---

# 5. 碎屑与镜面保护控制（磁场、旋转箔、气体护罩）

ASML/Cymer 的工程实现包含多个层级的保护与主动控制：

* **磁偏转场/离子偏转**：在集光器与等离子体之间设置磁场线圈或永磁体阵列，将带电粒子偏离光学路径。控制器可调节线圈电流以优化偏转强度（依据诊断信号）。相关专利展示了该思路并描述了控制方法。([Google Patents][7])
* **旋转箔（rotating foil）或箔条陷阱**：机械旋转/移动的金属薄片截取中性颗粒与碎屑，控制器监控箔片寿命并在阈值时安排更换或切换冗余单元。
* **缓冲气体/氢冲洗**：在出口套管注入 H₂ 形成化学/物理保护层、同时降低氧化和沉积速率；控制器精细调节 MFC 与阀门以保持局部压力与流速在最优范围（注意：此处需平衡光学传输与污染防护）。ASML 专利与工程资料中都有对气流/压差控制的说明。([Google Patents][8])

---

# 6. 诊断传感器套件（ASML 实用集合）

常见并被整合到源控制的传感器包括：

* **中间焦点（IF）EUV 光电二极管 / 辐照计**：实时测 IF 输出功率/脉冲能量（直接用于剂量/功率反馈）。([Cymer][3])
* **OES（光发射光谱）**：监测 Sn 离子谱线与其它杂质谱线（快速、非侵入、对瞬态响应好）。([Cymer][3])
* **高速相机 / 光电探测器（滴成像）**：用于滴定时与碎屑成像，为滴-激光同步提供触发基准。([Google Patents][4])
* **RGA / 四极杆质谱**：用于慢变的残余气体分析、泄露/污染源定位与工况诊断（非快速瞬态响应，但在维护/故障分析很关键）。([Cymer][3])
* **QCM / 见证片 / 反射率监测**：评估沉积速率与镜面退化，触发保护/清洗。([Google Patents][9])

这些传感器的数据被分成“硬实时处理”（FPGA直采）和“慢数据/趋势分析”（上位机数据库），并用来驱动闭环或决策逻辑。([Cymer][3])

---

# 7. 气体与真空控制（H₂、Ar、泵与 MFC）

* **气体供应控制**：光源腔与出口套管需要严格的气体控制（如 H₂ 冲洗、保护/稀释气体等），由高精度 MFC、压力传感器与阀门协同运行；控制器对 MFC 进行闭环调节以保持设定流量/压力。
* **真空段多区分段**：不同分区（滴腔、集光腔、IF 传输）维持不同压力梯度，通过阀/差压控制避免气体回流或锡蒸汽入侵敏感区。压力异常会引发 interlock（停泵/关闭激光等）。ASML 的专利文件和资料常说明分隔套管与差压供气的细节。([Google Patents][8])

---

# 8. 软件/硬件实现要点

* **硬件**：高带宽 FPGA（纳秒时序）、实时 DSP（伺服算法）、冗余 PLC（安全、慢回路）、工业 PC（协调、人机界面）。激光器与泵等设备有专用控制板（放大器驱动、冷却控制、栅极驱动）。([Cymer][3])
* **软件/OS**：实时子系统在 RTOS 下运行以保证确定性延迟（或在 FPGA 中实现关键定时），上层用工业 Linux/Windows 做管理与接口。版本控制、诊断日志、远程维护与数据上云（或厂区 MES）是必备功能。
* **安全/冗余**：氢气泄露、真空失效或镜面污染事件都会触发硬 interlock 优先保护光学。控制系统设计须有分级冗余与安全策略。([ASML][1])

---

# 9. 维护、校准与可用性（ASML 的工程实践）

* **例行校准/互校**：定期使用见证片、QCM、参考光源与标准气源校准 EUV 探测器、OES 与 RGA。
* **预测性维护**：用历史传感器数据（沉积速率、滴异常统计、激光功率漂移等）预测喷嘴寿命、旋转箔寿命与镜面清洗窗口，以减少非计划停机。ASML 工程文档强调“工程化可靠性”与维护流程。([ASML][1])

---

# 10. 新发展方向（对控制系统的影响）

* **三脉冲及更多脉冲优化**：使得控制器需管理更多并行激光源、在纳秒尺度内自适应调整多脉冲能量与延时；这对 FPGA/定时卡与算法提出更高要求（更复杂的优化与实时决策）。([SemiWiki][6])
* **更高平均功率（>250 W 目标 → 500–1000 W）**：意味着更激进的碎屑控制、镜面冷却管理和气体控制；控制器需做更高维的优化（能量 vs 污染 vs 寿命）。([EUV Litho][2])

---

# 11. 把上述要点“串成一套工作流”示意（文字版）

1. 上层生产计划下发每场/每片期望剂量与产能目标（工作站）。
2. 源控制器分配 IF 功率预算并配置激光参数（重复率、单脉冲能量、脉冲结构）。
3. FPGA 接收滴到位触发（来自高速光传感器/相机），计算延时并触发预脉冲与主脉冲（纳秒精度）。
4. OES / EUV 探测器 / QCM 的数据被 FPGA/控制器实时采集并进入本地闭环；RGA/压力/气体传感器给出慢速回路信息。
5. 若检测到异常（滴失步长、EUV 输出掉落、沉积加速或氢泄漏），控制器按策略：先保护镜面（停射/切换保护单元）、再尝试重建同步或降额运行，必要时通知维护。([Cymer][3], [Google Patents][7])

---

# 12. 参考（五条最关键来源）

* Cymer — Laser Produced Plasma Light Source for EUVL（SPIE / Cymer paper）— 系统架构、双脉冲与诊断说明。([Cymer][3])
* ASML — 产品/技术概述（EUV 系统发展、光源目标与工程化实践）。([ASML][1])
* US Patent: System and method for controlling droplet timing in an LPP …（droplet timing 专利，时序与检测实现）。([Google Patents][4])
* US Patent: Debris protection system having a magnetic field for an EUV light source（碎屑偏转/保护设计）。([Google Patents][7])
* Performance at 250 W and Key Technologies for Power（EUV 源功率与转换效率讨论，光源控制的目标值参考）。([EUV Litho][2])

---

如果您愿意，我可以接着做下面两件事（任选其一或两个都做）：
A. 把**源控制器内部的数据流与模块接口**画成一张详细的示意图（含 FPGA 定时卡、激光触发链路、OES/RGA/QCM 的数据路径与阈值触发逻辑）；
B. 给出**工程级的控制策略模板**：列出具体的传感器量程/接口、FPGA 时序约束、闭环带宽建议与 interlock 条件（便于系统集成或评估供应商方案）。

您想先看 A 还是 B？或者两者都要？

[1]: https://www.asml.com/products/euv-lithography-systems?utm_source=chatgpt.com "EUV lithography systems – Products"
[2]: https://euvlitho.com/2017/S1.pdf?utm_source=chatgpt.com "Performance at 250 W and Key Technologies for Power ..."
[3]: https://www.cymer.com/wp-content/uploads/2018/12/Cymer_SPIE_AdvancedLithography_2011.pdf?utm_source=chatgpt.com "Laser Produced Plasma Light Source for EUVL"
[4]: https://patents.google.com/patent/US9241395B2/en?utm_source=chatgpt.com "System and method for controlling droplet timing in an LPP ..."
[5]: https://strobe.colorado.edu/wp-content/uploads/STROBE_ASML-EUV-Sources_Purvis_25-Sept-2020-1.pdf?utm_source=chatgpt.com "An Introduction to EUV Sources for Lithography"
[6]: https://semiwiki.com/forum/threads/asml%E2%80%99s-breakthrough-3-pulse-euv-light-source.22703/?utm_source=chatgpt.com "ASML's Breakthrough 3-Pulse EUV Light Source"
[7]: https://patents.google.com/patent/US8519366B2/en?utm_source=chatgpt.com "Debris protection system having a magnetic field for an ..."
[8]: https://patents.google.com/patent/US20070102653A1/en?utm_source=chatgpt.com "US20070102653A1 - EUV light source"
[9]: https://patents.google.com/patent/US9268031B2/en?utm_source=chatgpt.com "Advanced debris mitigation of EUV light source"


# EUV 光源舱 — 源控制器数据流与工程控制策略模板

> 文档说明：本稿为源控制器（Source Controller）内部数据流与模块接口的详尽示意与工程化控制策略模板，适用于在光源舱内布置 RGA + OES + QCM + EUV 探测器 的实时闭环控制与保护。文档包含：
>
> * 模块级 ASCII 示意图（可打印）
> * 数据流说明与接口定义
> * FPGA / 定时卡 时序约束与示例
> * 传感器规格表（推荐量程、采样率、接口）
> * 闭环带宽建议与控制器实现要点
> * Interlock 与保护策略清单
> * RGA + OES + QCM + EUV 实时闭环示意与时序建议

---

## 目录

1. 源控制器总体模块示意（ASCII 图）
2. 模块接口与数据流说明（逐接口详细）
3. FPGA 时序约束与触发链路（纳秒级/微秒级示例）
4. 传感器规格与接口建议表（RGA/OES/QCM/EUV/相机/压力）
5. 闭环带宽与控制律建议（伺服/预测/滤波）
6. Interlock 条件与优先级策略
7. 实际工程示例：在光源舱布置 RGA + OES + QCM + EUV 探测器 的实时闭环（图与时序）
8. 部署注意事项与维护/校准建议

---

# 1. 源控制器总体模块示意（ASCII 图）

```
+-------------------------------------------------------------+
|                       主控工作站 (Host PC)                 |
|  - 调度 / 工艺目标 / 报警 / 日志 / MES 接口               |
+--------------------------+----------------------------------+
                           |
      Ethernet/TCP/IP      |  High-level commands & recipes
                           |
+--------------------------v----------------------------------+
|                   源控制器 (Source Controller)              |
|  (Industrial PC + RTOS / Database)                          |
|  - 场级序列器 (Sequencer)                                    |
|  - 参数管理 (recipes, thresholds)                           |
|  - 慢回路控制 (seconds..ms)                                 |
+----+----------------------+----------------+----------------+
     |                      |                |
     |                      |                |
     |                      |                |
     |                      |                |
     |    Real-time bus     |  Slow bus (Modbus/Profibus)    |
     |  (optical fibre LVDS) |  (EtherCAT/Profinet)           |
+----v----+   +-------------v-----------+   +v-------------+  
| FPGA/    |   | Laser Timing & Drive    |   | Gas & Vacuum  |  
| Timing   |   | (MOPA controller)      |   | Controller    |  
| Card     |   | (ns timing, pre/main)  |   | (MFCs, valves) |  
| (ns-ms)  |   +-----------+------------+   +--+-----------+  
+-----+----+               |                    |              
      |                    |                    |              
      |  LVDS/TTL triggers  |   High-voltage     |  Analog/RS485
      |                    |   & interlocks     |              
+-----v----------------+   +v----------------+  v              
| High-speed ADC/DAQ    |  | Laser Head Power |  | Pumps, Valves |
| (OES spectrometer,    |  | Amplifier drives |  | MFCs, gauges   |
|  EUV photodiode ADC)  |  | (local servo)    |  |               |
+-----+-----------------+  +------------------+  +--------------+
      |                      |                       |
      |   Fast feedback      |                       |
      +--->(EUV power loop)--+                       |
      |                                              |
+-----v----------------+   +-----------------+   +----v---------+
| High-speed Camera /   |   | QCM / Witness  |   | RGA / QMS     |
| Photodiode (drop detect)|  | sensors        |   | (slow/trigger)|
+-----------------------+   +-----------------+   +--------------+

Legend:
- Solid arrows: primary data/trigger paths (hard real-time)
- Dashed paths: slow telemetry / diagnostics / database (soft real-time)

```

> 注：示意图强调两类时间尺度：FPGA/Timing卡处理纳秒—微秒级触发；Source Controller（嵌入式PC）处理毫秒—秒级策略与与主控通信。

---

# 2. 模块接口与数据流说明（逐接口详细）

### 2.1 Host PC <-> Source Controller

* 协议：Ethernet/TCP-IP 或 OPC-UA，建议使用基于时间戳的同步消息（ISO 8601）并有消息序号与确认机制。
* 主要数据：曝光功率目标（IF-target W）、产能模式、维护命令、传感数据汇总、报警与日志。
* 时延容忍：上层信息允许 100 ms \~ 1 s 延迟。

### 2.2 Source Controller <-> FPGA / Timing Card

* 协议：LVDS/PCIe/Optical link（高带宽低延迟），Timing 卡实现硬触发与精确延时。
* 数据：触发事件（drop\_detect）、预脉冲延时、主脉冲延时、脉冲能量因子、实时阈值。
* 时延要求：命令下发到触发固化路径的总延迟 < 1 μs（最好 < 200 ns）以避免时序乱差。

### 2.3 FPGA <-> Laser Driver / MOPA Controller

* 信号：TTL/LVDS trigger、analog setpoint (0-10V)、digital control (SPI/I2C for config)；必要时使用 fibre-optic link for isolation.
* 需要保护 interlock lines (hardwired) 直接连至激光安全门与高压断电。

### 2.4 FPGA <-> High-speed Sensors (EUV PD, OES, Highspeed Camera)

* EUV Photodiode ADC: 14-18 bit ADC @ 1 MSPS\~20 MSPS（脉冲峰值测量需高采样）
* OES Spectrometer: 通常由 ICCD / sCMOS + spectrograph 给出短脉冲谱信息，接口为 CameraLink / CoaXPress 或 10GbE streaming
* High-speed Camera: 用于drop-detect，建议 >50 kfps（视drop频率），低延迟触发输出 TTL 到 FPGA

### 2.5 Slow Sensors (RGA, QCM, Pressure Gauges)

* RGA: RS-232/USB/Ethernet 接口，采样频率较低（1 Hz \~ 1 min），在发生异常事件时可触发快采样（snapshot）到 FPGA/Controller
* QCM: 通常以频率变化输出，通过精密计数器或 DAQ 接收，接口为 analog/digital
* Pressure: 电容式/热导/ion gauge via 4-20 mA 或 Modbus

---

# 3. FPGA 时序约束与触发链路（纳秒级/微秒级示例）

## 3.1 关键时序节点（典型 LPP 双脉冲场景）

* t0: drop detection (optical photodiode or camera) — 触发信号产生
* t0 + Δt1: pre-pulse trigger (ns 精度)
* t0 + Δt2: main-pulse trigger (Δt2 > Δt1，ns 精度，可为数十到几百 ns)
* t0 + Δt3: EUV emission window start (用于 PD 采样窗口)
* t0 + Δt4: EUV PD peak sampling (ADC 高速采样)

## 3.2 时序约束建议值（示例）

* drop detection latency (sensor detect -> FPGA input) ≤ 200 ns (optical diode) 或 ≤ 1 μs (camera with TTL out)
* FPGA trigger output jitter ≤ 5 ns (目标纳秒级)
* pre-pulse -> main-pulse delay 可微调范围 10 ns \~ 1 μs，分辨率 ≤1 ns
* EUV PD sampling window: 100 ns \~ 10 μs 取决于主脉冲与等离子体发光持续时间

## 3.3 实现建议

* 将核心触发路径（drop detect -> timing -> laser trigger）实现为硬连线/FPGA logic，避免操作系统干预。
* 使用 PLL 或外部时钟参考（GPS/10MHz 或光刻机主时钟）使各卡片时钟同步。
* 对 ADC 数据流使用 DMA 直接写入 Ring Buffer，避免 CPU 介入导致非确定性延迟。

---

# 4. 传感器规格与接口建议表（RGA/OES/QCM/EUV/相机/压力）

| 传感器                             |                         推荐量程 / 灵敏度 |                     采样率 | 接口                           | 用途                       |
| ------------------------------- | ---------------------------------: | ----------------------: | ---------------------------- | ------------------------ |
| EUV Photodiode (PD)             |     0–1 mW peak, rise time < 10 ns |           1–20 MSPS ADC | LVDS / ADC (14-18 bit)       | 脉冲能量、瞬时功率闭环              |
| OES (spectrometer + ICCD)       | 200–1100 nm (可扩展到 VUV) 分辨率0.1–1 nm |              ns–μs 门控使用 | CameraLink/CoaXPress / 10GbE | 等离子体物种、相对强度、放电异常检测       |
| High-speed camera (drop detect) |             微米级分辨率, exposure <1 μs |             10–200 kfps | TTL trigger out + CameraLink | drop timing & morphology |
| RGA (QMS)                       |              1e-11 – 1e-3 Torr 分子级 |           \~0.1–1 Hz（慢） | Ethernet/RS232               | 残余气体谱与污染源定位              |
| QCM                             |                ng/cm² 分辨率（或 Å/min） |                0.1–1 Hz | Analog / Counter             | 沉积速率监测、清洗触发              |
| Pressure gauge                  |        10^-9 – 10^2 Torr depending | fast ion gauge 0.1–1 Hz | 4-20 mA / Modbus             | 确保分区压力/泵状态               |

> 备注：以上是工程推荐范围，具体选型需根据厂商数据表（响应时间、长期漂移）做最终决策。

---

# 5. 闭环带宽与控制律建议

## 5.1 针对不同回路的带宽划分

* **硬实时滴-激光触发回路**：带宽非常高（以时间分辨率和抖动计），要求低延迟和极低jitter；实现方式：FPGA硬件逻辑（带宽并非传统频率意义，重在时延/jitter）。
* **EUV 能量快速闭环**：目标带宽 1 kHz – 10 kHz，用于抑制主脉冲间的能量漂移（举例：快速探测脉冲峰值并调整下一脉冲放大器或增益参数）。控制器实现：PID + 前馈（feedforward），并结合预测滤波（如 Kalman predictor）来对补偿时滞做预估。参考 ASML 使用预测器来提高 throughput 的做法。
* **等离子体质量 / OES 相关回路**：对谱线强度或离子丰度的控制可置于 10 Hz – 1 kHz 区间；可用谱带能量作为反馈信号，调节主/预脉冲能量或滴直径。
* **沉积 / 污染监测回路**：带宽低（0.01 – 1 Hz），QCM 与见证片数据用于长期策略（清洗周期、箔片更换）。

## 5.2 控制律建议

* **短时能量控制（脉冲-脉冲）**：使用高速采样 + PID with anti-windup；若存在显著延迟或非最小相位，可以用 Model Predictive Control (MPC) 或 1-step Kalman predictor。
* **滴-激光相位控制**：以硬件时间延迟表+在线微调（微步）为主，采用环内补偿（若观测到速度漂移则调整喷嘴驱动频率）。
* **污染防护策略**：为防止误动作，引入多传感器确认（QCM 趋势 + RGA 突变 + PD 下降）后才触发高优先级 interlock。

---

# 6. Interlock 条件与优先级策略

> Interlock 分层：硬 interlock (hardwired, immediate cut) > 半硬 interlock (RTOS, FPGA enforced) > 软 interlock (Host PC policy)

**硬 Interlocks (立即动作，无须软件决策)**

* 氢气泄漏报警（H₂ sensor）
* 激光放大器温度超限
* 高压或冷却系统失效
* 真空破坏导致镜面曝露到空气
* 任何安全门打开

**半硬/FPGA Interlocks (ms 内)**

* drop detect 丢失或大量 drop miss (> N misses / s)
* EUV PD 峰值低于阈值（表明未有效产生等离子体）
* QCM 沉积率突变（表明镜面快速污染）
* 激光触发失败或触发 jitter 超阈

**软 Interlocks / 维护建议（上位机处理）**

* RGA 中 O₂ / H₂O / HC 超阈值（触发预警并启动清洗程序）
* 长期能量漂移趋势（触发维护或校准）

**优先级策略示例**

1. 若硬 interlock 触发，系统立即切断激光并安全停止泵/阀，记录日志并向 Host PC 报警。
2. 若半硬 interlock 触发，尝试自动恢复（短时降载/切换到低功率模式）2 次，仍未恢复则触发硬 interlock 并上报维护。
3. 软 interlock 触发则进入降档运行并通知维护计划。

---

# 7. 实际工程示例：RGA + OES + QCM + EUV 实时闭环（框图与控制时序）

```
[drop detect]--(TTL)-->[FPGA Timing]--(trigger)->[Laser Driver]
                                 |                     |
                                 |                     v
                                 |                 [EUV emission]
                                 |                     |
                                 |                [EUV PD -> ADC]
                                 |                     |
                                 +----> [Fast EUV loop: FPGA/DSP]
                                 |             (compute pulse energy error)
                                 |                     |
                                 |---> send correction to Laser Driver (next N pulses)

[OES Fiber]->[Spectrometer+ICCD]->(stream)->[FPGA/IPC]
                                     |                 
                                     +-> extract Sn line intensity -> if drift -> adjust pre/main pulse energy (ms-timescale)

[RGA]->(Ethernet)->[Source Controller DB] (0.1-1 Hz)
                                     |
                                     +-> if O2/H2O/Hc spike -> raise flag -> escalate to maintenance / enable protective cleaning

[QCM]->(Analog)->[DAQ]->[Source Controller]
           |                             |
           +-> sediment rate high -> trigger increase magnetic field / rotate foil / reduce power

Timing sketch (per drop event):
 t0: drop detect (FPGA) -> t0 + 100ns: pre-pulse trigger -> t0 + 500ns: main-pulse -> t0 + 700ns..ms: EUV PD sampling
 OES gate windows: gated around t0 + 100ns .. t0 + 10us (ICCD gating)
 QCM / RGA sampled at slow rate but their alarms can preempt and cause immediate protective actions
```

> 说明：快速 EUV 能量回路可在 FPGA 中以脉冲为单位工作，但由于激光放大器链路存在热惯性，真实执行通常是对后续几次脉冲做补偿（例如基于测到的误差对下一个 10 脉冲集进行能量偏移）。这需要控制算法（在 Source Controller）计算出合理的补偿量并下发到 FPGA（硬实时路径）。

---

# 8. 部署注意事项与维护/校准建议

* **多点冗余传感**：关键量如 drop detect、EUV PD 应设计冗余通道以避免单点故障。
* **定期互校**：使用标准光源和见证片校准 EUV PD / OES 灵敏度；用标定气体校准 RGA。
* **保护先行**：策略上优先保护光学（镜面）以延长寿命，产能次之。
* **日志与回放**：所有高速事件（drop, triggers, PD traces）应做循环缓冲并在故障时回放，便于故障定位。
* **维修性设计**：旋转箔、QCM、见证片等易损耗件应易于替换并有状态监控。

---

# 结束语

本文档为工程级模板，旨在帮助集成工程师或系统设计者快速搭建或评估光源舱内的源控制器架构与实时闭环。您可以把本文件作为基础，在其中替换具体传感器型号、接口与参数后，用于系统设计评审或招标资料。

如果需要：我可以把关键示意图导出为 PDF 或 SVG（便于放入设计文档），或者把传感器选型表扩展为带厂商/型号建议的 Excel 表格。
