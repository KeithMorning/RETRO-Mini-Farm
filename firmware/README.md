# firmware/

## backup/

烧测试程序前，从 `ARCADE-F4` 盘备份的原始内容：

| 文件 | 大小 | 说明 |
|---|---:|---|
| `CURRENT.UF2` | 1.0 MB | 出厂示例固件 "falling duck"（MakeCode Arcade 4.0.14），SHA-256 前缀 `6ab2464acf56db4d` |
| `CONFIG.BIN` | 32 KB | Bootloader 配置（屏幕 ILI9341 / 引脚） |

恢复方法：把 `CURRENT.UF2` 拖回 `/Volumes/ARCADE-F4`，等设备重启。

> 注意：不要随意写 `CONFIG.BIN`，它是 bootloader 的板级配置。

## m0-test.ts

M0 真机测试程序，测按键 / 声音 / 存档 / FPS。

1. 打开 <https://arcade.makecode.com/>
2. 新建项目 → 切到 **JavaScript**
3. 用 `m0-test.ts` 替换 `main.ts`
4. 硬件选 **F4** → 下载
5. 屏幕顶部：左 = A 键计数，右 = FPS

测试项：

- A：计数 +1、响一声、写入 `settings`
- B：生成 24 个移动精灵，再按一次清除
- Menu：操作说明
- 关机再开机：左上角计数应保留

记录结果填到 [`docs/m0-hardware-check.md`](../docs/m0-hardware-check.md)。
