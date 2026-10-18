# M0 真机读数（2026-10-18）

设备已连接电脑。以下数据直接从 USB / UF2 引导区读出，不是估算。

## 设备识别

| 项 | 读数 |
|---|---|
| USB 设备 | Vendor **ELECFREAKS**，Product **Arcade** |
| 引导区 | `/Volumes/ARCADE-F4`，4.1 MB FAT |
| Bootloader | UF2 **v2.8.1 W** |
| Board-ID | **STM32F412-Arcade** |
| 当前固件 | MakeCode Arcade 4.0.14，示例游戏 **"falling duck"** |
| 屏幕驱动 | **ILI9341**，8-bit 并口，FSMC（bootloader 配置字符串） |

## 存储与内存

| 项 | 数值 | 依据 |
|---|---:|---|
| MCU | STM32F412RE | Board-ID + ST 数据（512 KB Flash / 256 KB SRAM / 100 MHz） |
| Bootloader | 64 KB | App 向量表在 `0x08010000` |
| App 区 | **448 KB** | `0x08010000`–`0x0807FFFF` |
| App 占用 | **约 326 KB** | UF2 中最后一个非 `0xFF` 字节 `0x0806182F` |
| App 区剩余 | **约 122 KB（27%）** | 448 − 326 |
| MakeCode 可用 RAM | **96 KB** | App 初始 SP = `0x20018000` |
| 芯片实际 SRAM | 256 KB | STM32F412RE 规格 |

> 关键发现：**硬件有 256 KB SRAM，但 MakeCode 的 F4 目标只链接了 96 KB。**
> 这是软件限制，不是芯片限制。以后如果真需要更多内存，自定义 target / 裸机可以解锁；现在按 96 KB 设计。

## 对游戏预算的影响

- Flash：**约 120 KB** 用于我们的代码 + 美术 + 音效（保守按 120 KB 规划）。
  - 16×16 的 4bpp 图 = 128 B/张；100 张 = 12.8 KB。
  - 一套 64 格 tileset ≈ 8 KB；玩家 8 帧 ≈ 1 KB；3 作物 × 3 阶段 ≈ 1.2 KB。
  - 音效 10 个 ≈ 5–10 KB。
  - 代码 30–50 KB。
  - 合计约 60–80 KB，**留 40–60 KB 余量**，可行。
- RAM：96 KB，和之前的预算一致；双缓冲 + 运行时占大头，游戏状态只有几 KB。

## 还需要真机按键确认

以下项目无法从 UF2 读出，需要烧一个测试程序：

- [ ] 十字键 / A / B / Menu 的映射与手感
- [ ] 蜂鸣器音量
- [ ] `settings` 写入后断电是否保留
- [ ] 24 / 60 个精灵的帧率
- [ ] 电池与充电状态

测试程序见 [`firmware/m0-test.ts`](../firmware/m0-test.ts)。
烧录前的原始固件已备份到 [`firmware/backup/`](../firmware/backup/)。

## 复现方式

```bash
python3 tools/uf2-info.py /Volumes/ARCADE-F4/CURRENT.UF2
```

输出会把 Bootloader 和 App 的向量表、App 末尾、剩余空间都列出来。
