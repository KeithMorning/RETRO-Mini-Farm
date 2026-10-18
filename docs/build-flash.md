# 编译与烧录

## 环境

- ARM GNU 工具链（xPack `arm-none-eabi-gcc`）
- `make`
- Python 3
- `libopencm3` 不提交；`make deps` 会拉取固定版本 `v0.8.0`，第一次 `make` 会自动编译它
- 主机模拟器使用系统 `cc` 和 Python 3（不依赖 SDL）

默认工具链路径：

```text
toolchain/xpack-arm-none-eabi-gcc-15.2.1-1.1/bin
```

也可以指定：

```bash
cd native
make TOOLCHAIN=/path/to/arm-none-eabi/bin
```

## 编译固件

```bash
cd native
make deps
make -j4
```

输出：

- `native/build/app.uf2` — 烧录文件
- `native/build/app.bin` — 原始二进制
- `native/build/app.elf` — 带调试符号

## 主机模拟器

```bash
cd native
make sim
```

模拟器和固件共用 `src/game.c`。输出在 `native/sim/out/`：

- 每个场景/测试的 `.bin` 帧
- 同名 `.png`
- `sheet.png` 汇总图

模拟器测试覆盖：

- 存档/读档
- 农场循环
- 菜单/背包
- 房子/水壶升级
- 季节
- 海滩/钓鱼
- 宠物/小恐龙
- 故事线
- 矿洞
- 彩蛋

## 回归测试

```bash
cd native
make test
```

会编译并运行存档、农场、菜单、升级、季节、海滩、宠物、故事、矿洞、彩蛋测试。

## 烧录

1. 按住 **Menu + B** 重启，进入 bootloader
2. 电脑出现 `ARCADE-F4` U 盘
3. 把 `native/build/app.uf2` 拷进去
4. 设备自动重启

macOS：

```bash
cp native/build/app.uf2 /Volumes/ARCADE-F4/
```

Linux 的挂载点可能是 `/media/$USER/ARCADE-F4`。  
也可以：

```bash
cd native
make flash
```

## 存档

- 当前 **v5**
- 存在内部 Flash sector 7（`0x08060000`）
- 睡觉、Menu → save、升级、完成故事时写入
- v1–v4 存档会自动迁移
- 普通固件升级不会清掉 sector 7

## 已知限制

- 锄头目前是占位工具（农田已经是翻好的土）
- 矿洞只有一层
- 除水壶外还没有工具升级
- 内部 Flash 写入会短暂停顿
- 模拟器不模拟真实帧率
