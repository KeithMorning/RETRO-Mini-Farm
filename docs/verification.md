# Mini Farm v1 验证记录

日期：2026-10-18

## 1. 固件

- 源码：`native/src/`（`game.c`、`save.c`、`main.c`、`font8.c`）
- 构建：`cd native && make clean && make -j4`
- 结果：无 warning / error
- App 大小：约 25 KB
- RAM：约 178 KB / 256 KB（68%）
- UF2：`native/build/app.uf2`，100 blocks，25,600 bytes payload
- App 区：`0x08010000 .. 0x0801637B`
- 存档区：Flash sector 7，`0x08060000`，与 App 代码不重叠
- SHA-256：`007b451667ac95c8da6163a9fa32e36e5baef23c7cf9ae19de1dd7767033c256`

## 2. 模拟器截图

`cd native && make sim` 生成 `native/sim/out/`：

- `t0_title` 标题
- `t1_farm` 农场（水壶/体力条）
- `t2_town` 小镇 + 2 个 NPC
- `t3_shop_inside` 商店室内
- `t4_dialogue` 对话 + 心
- `t5_gift` 送礼
- `t6_home` 玩家家
- `t7_mira_home` Mira 家
- `t8_winter` 冬天 + 雪
- `t9_rain` 春雨
- `t10_home_from_door` 农场家门 → 家
- `t11_ending` 春季结局
- `t12_status` 农场手册
- `t13_sleep` 睡觉过场
- `t14_exit` 家 → 农场
- `t15_town_shop` 小镇商店门面
- `t16_summer` 季节切换过场
- `t17_year_end` 年度结局
- `t18_shop_counter` 商店柜台
- `t19_tom_home` Tom 家
- `t20_lily_home` Lily 家
- `t21_summer` 夏天
- `t22_fall` 秋天
- `t23_shop_door` 小镇 → 商店门

## 3. 主机存档测试

`native/sim/save_test.c` + `save_file.c`：

- 设成 fall day 5 / 777G / 玩家家
- 睡觉写档
- 重新 `game_init()` 读档
- 结果：`fall 6 06:00 777G`，场景为玩家家，显示 `welcome back`
- 截图：`native/sim/out/save_after.png`

注意：这是文件模拟，验证的是 `game_to_save` / `game_from_save` 的序列化；Flash 写入需要真机验证。

## 4. 玩法闭环测试

`native/sim/gameplay_test.c`：

- 标题 → 商店买胡萝卜
- 在空地块播种
- 浇水 → 睡觉 → 再浇水 → 睡觉
- 收获（HUD 变成胡萝卜 ×1）
- 走到出货箱 → `in bin`
- 睡觉卖出：金币 40 → 65，bin 0
- 截图：`g1_plant` … `g7_coins`

## 5. 功能清单

- [x] 28 天 × 4 季，112 天一年
- [x] 16 作物（春 4 / 夏 4 / 秋 4 / 冬 2）
- [x] 锄地、播种、浇水（15 格水壶）、收获、再生
- [x] 体力、时钟、床、Menu 睡觉
- [x] 商店、金币、出货箱、农场手册
- [x] 农场、小镇、玩家家、商店、Mira/Tom/Lily 家
- [x] 门、淡入淡出、室内固定相机
- [x] 3 NPC 作息、走路/闲逛、对话、送礼、5 心
- [x] 雨/雪、四季调色板、季节切换
- [x] 春季结局、年度结局
- [x] 全小写 + 2× 字号
- [x] 存档（睡觉时写 sector 7）

## 6. 真机状态

- 已由用户要求，把 `app.uf2` 复制到 `ARCADE-F4`；复制后盘符消失，设备自动重启。
- Bootloader 重启太快，目标文件的 SHA-256 没来得及读；源文件 SHA-256 已记录。
- 屏幕显示待用户确认；如果黑屏/旧版，用 Menu+B 或关机重开后可重烧。
- 首次启动应为 `mini farm` 标题；睡觉后关机再开，应显示 `welcome back` 并保留进度。

## 7. 已知限制

- 音效/震动未接（扬声器/马达引脚未确认）。
- NPC 跨场景是到点传送；同场景内走路。
- 美术为程序化占位图，可换 Aseprite tileset。
- 存档为单槽；升级固件可能清档。
