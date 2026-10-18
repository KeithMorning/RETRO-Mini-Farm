# 原生路线：绕开 MakeCode，直接用 C 写

> 目标：把「本地编译 → 拖 UF2 → 运行」变成几秒钟的事，不再依赖网页编辑器。

## 为什么可以绕开

实机读数（见 [M0 真机读数](m0-results.md)）：

| 项 | 值 |
|---|---|
| MCU | **STM32F412RE**：100 MHz，512 KB Flash，**256 KB SRAM** |
| 屏幕 | **ILI9341**，320×240，8-bit 并口 / FSMC |
| Bootloader | UF2 v2.8.1，App 起始 `0x08010000`，UF2 family `0x57755A57` |
| 官方 App 区 | 448 KB |

MakeCode 只用 96 KB RAM；原生固件可以用满 **256 KB**。
并且 bootloader 是开源的 `uf2-stm32f` 分支，板级配置就在 flash 里，引脚可以直接读出来。

## 已读出的板级配置

从 bootloader 的 `configData` 解析（36 项）：

| 功能 | 引脚 |
|---|---|
| A | **PC00** |
| B | **PC01** |
| 左 | **PA15** |
| 右 | **PC09** |
| 上 | **PB05** |
| 下 | **PB02** |
| Menu | **PC13** |
| 屏 CS | **PB12** |
| 屏 DC / FSMC 地址 | **PC03** |
| 屏 RST | **PB07** |
| 屏 BL | **PB09** |
| 屏数据/控制 | PA2–PA5、PB14、PC2、PC3、PC5、PC6、PC11、PC12（FSMC AF） |
| 音频 | PA08（SND）、PB06（TX）、PC10（PWREN） |
| 震动马达 | PC08 |
| 电池检测 | PC04 |
| 加速度计 | SCL PB10 / SDA PB03 / INT PA04 |
| LED | PA07 |

屏幕参数：

- `DISPLAY_TYPE = 0x247D`（ILI9341）
- `DISPLAY_WIDTH = 320`，`DISPLAY_HEIGHT = 240`
- `DISPLAY_CFG0 = 0x08`，`CFG1 = 0x10ff`，`CFG2 = 0x1000004`
- FSMC 命令地址 `0x60000000`，数据地址 `0x60000001`
- 初始化序列与 FSMC 时序可直接复用 bootloader 的 `screen.c`

## 工具链

```
arm-none-eabi-gcc   # 本地编译
make / cmake
git
python3             # bin → uf2
libopencm3          # 轻量 HAL（bootloader 同款）
uf2conv.py          # 微软的 UF2 打包脚本，或自己写 50 行
```

工作流：

```
make            # 几秒
make uf2        # 生成 app.uf2
cp app.uf2 /Volumes/ARCADE-F4/   # 自动烧录、重启
```

不需要浏览器、不需要云端、不需要登录。

## 固件内存布局

| 区域 | 地址 | 用途 |
|---|---|---|
| Bootloader | `0x08000000`–`0x0800FFFF` | UF2，保留 |
| App | `0x08010000`–`0x0807FFFF` | 我们的固件（448 KB） |
| RAM | `0x20000000`–`0x2003FFFF` | 256 KB，随便用 |

App 的向量表必须放在 `0x08010000`，初始 SP 可以设到 `0x20040000`。

## 显示方案

- 320×240，RGB565（16 bit/像素）。
- 单缓冲全屏 = **153.6 KB**，256 KB 里放得下，剩余 ~100 KB 给游戏。
- 或 8-bit 调色板缓冲 = **76.8 KB**，双缓冲 153.6 KB，剩余 ~100 KB。
- 或 4-bit / 16 色缓冲 = **38.4 KB**，双缓冲 76.8 KB，剩余 ~180 KB。

推荐先用 **8-bit 调色板 + 单缓冲**：256 色足够画农场，内存也宽松。
渲染顺序：tilemap → 作物 → 玩家 → UI，只重绘脏矩形。

## N0 结果（2026-10-18）

- 本地 `arm-none-eabi-gcc` + libopencm3 编译通过；固件约 2 KB。
- 屏幕点亮：320×240 彩条 + 可移动方块。
- **关键坑**：面板是旋转安装的，`CASET` 要用 y 范围、`RASET` 要用 x 范围（与 bootloader `screen.c` 一致）；用标准窗口会全黑。
- 当前时钟：HSI 16 MHz（未开 PLL），显示正常；性能优化放到 N1 之后。
- 震动马达：按配置的 PC08 没有反应，待查（可能引脚或驱动方式不同）。
- 烧录：App 不实现 USB，每次烧录需要重启，bootloader 只开 3 秒窗口。

## 里程碑

| 阶段 | 内容 | 预计 |
|---|---|---|
| **N0 点亮** | 本地工具链；FSMC + ILI9341 初始化；全屏色块；按键读取 | 1 天 |
| **N1 引擎** | 320×240 调色板渲染、tilemap、摄像机、玩家移动 | 2–3 天 |
| **N2 农场** | 锄地/播种/浇水/收获、日夜、商店、存档（内部 Flash） | 1 周 |
| **N3 内容** | 美术、音效、更多作物、NPC | 1–2 周 |

## 风险

| 风险 | 应对 |
|---|---|
| 晶振频率未知 | N0 先用内部 HSI 16 MHz，不碰 PLL；点亮后再试 96 MHz |
| FSMC 时序/引脚 | 直接复用 bootloader `screen.c` 的初始化 |
| UF2 打包错误 | 用 `uf2conv.py --family 0x57755a57 --base 0x08010000`，先烧 hello 验证 |
| 变砖 | Bootloader 不动；原厂固件已备份，随时拖回 `CURRENT.UF2` |
| 没有调试器 | 先靠屏幕输出；需要时再接 SWD |

## 参考

- UF2 bootloader（含 `screen.c` / 板级配置）：<https://github.com/mmoskal/uf2-stm32f>
- PXT config key 枚举（引脚编号来源）：`pxt-arcade` 的 `pxtconfig.h`
- STM32F412RE 数据：<https://www.st.com/en/microcontrollers-microprocessors/stm32f412re.html>
