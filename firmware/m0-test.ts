/**
 * M0 hardware test — ELECFREAKS Retro Arcade for Education (F4)
 *
 * 用法：
 * 1. 打开 https://arcade.makecode.com/
 * 2. 新建项目 → 切到 JavaScript
 * 3. 用本文件替换 main.ts
 * 4. 硬件选 F4 → 下载到 ARCADE-F4 盘
 *
 * 测试内容：
 * - A：计数 +1，写入 settings，响一声（测按键 / 声音 / 存档）
 * - B：生成 24 个移动精灵，再按一次清除（测帧率 / 内存）
 * - Menu：显示操作说明
 * - 顶部左侧数字 = 累计 A 次数；右侧 = 当前 FPS
 *
 * 断电重启后，左侧计数应该保留 —— 这就是存档测试。
 */

let stored = settings.readNumber("m0count")
if (isNaN(stored)) {
    stored = 0
}

let frames = 0
let fps = 0
let stress: Sprite[] = []

scene.setBackgroundColor(12)
info.setScore(stored)
info.setLife(0)

game.onUpdate(function () {
    frames += 1
})

game.onUpdateInterval(1000, function () {
    fps = frames
    frames = 0
    info.setLife(fps)
})

controller.A.onEvent(ControllerButtonEvent.Pressed, function () {
    stored += 1
    settings.writeNumber("m0count", stored)
    info.setScore(stored)
    music.playTone(440 + (stored % 8) * 55, 80)
})

controller.B.onEvent(ControllerButtonEvent.Pressed, function () {
    if (stress.length > 0) {
        for (let i = 0; i < stress.length; i++) {
            stress[i].destroy()
        }
        stress = []
        game.showLongText("24 sprites cleared", DialogLayout.Bottom)
    } else {
        for (let i = 0; i < 24; i++) {
            let im = image.create(16, 16)
            im.fill(2 + (i % 14))
            let s = sprites.create(im, SpriteKind.Food)
            s.setPosition(20 + (i % 6) * 20, 20 + Math.floor(i / 6) * 20)
            s.setVelocity(20 + (i % 5) * 10, 20 + (i % 7) * 8)
            s.setBounceOnWall(true)
            stress.push(s)
        }
        game.showLongText("24 sprites on — watch FPS", DialogLayout.Bottom)
    }
})

controller.menu.onEvent(ControllerButtonEvent.Pressed, function () {
    game.showLongText("A: +1 and save\nB: 24 sprites / clear\nMenu: this help", DialogLayout.Bottom)
})
