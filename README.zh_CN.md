**简体中文** · [English](README.md)

# 一签

**抽一句懂你的话，留作自己的签名。**

把 AI Passport 变成随身的来信收藏夹。生活有点难、心里有点累的时候，选一种心情，
拆开一封未知的信。抽到喜欢的句子，就把它留在像素卡片上，当作自己的个人签名展示。
中文界面、中文签文，完全离线使用。

<p align="center">
  <img src="assets/images/fortune-cover.png" alt="一签示意封面：打开的信封与四周的像素邮票" width="360">
</p>

| 完整签文 | 三种口吻 | 四个像素系列 | 不同的程序生成画面 |
| --- | --- | --- | --- |
| **2,112** 条独立创作句子 | 温柔有锋芒、抽象嘴替、克制诗意 | 夜航电台、旷野来信、宇宙邮局、口袋花园 | **24,576** 个基础画面 |

当前版本内置 2,112 条完整签文。最初提出的一万条以上独立签文仍是后续扩充目标；
画面变化与签文数量分别统计。

## 给困难日子一个小仪式

选一种心情，也可以随缘。封好的信先轻轻晃动，封口打开，信纸升起，然后揭晓句子。
拆信约 1.25 秒，按确定可直接看结果。重新抽签时，已保存的个人签名会保留，
确定留下另一张才会替换。

想被温柔接住、想要一句带网感的嘴替，或只想安静地读一点诗意，都有对应的口吻。
同一轮抽签不重复，切换筛选也会记住已经抽过的句子。无需账号、联网或付费，
没有每日额度、连续签到或稀有等级。

<table>
  <tr>
    <td><img src="assets/images/fortune-letters.png" alt="选心情、拆信、揭晓签文的玩法示意" width="300"></td>
    <td><img src="assets/images/fortune-voices.png" alt="三种口吻与八种心情的玩法示意" width="300"></td>
  </tr>
  <tr><td>选一种心情，拆一封来信。</td><td>找到此刻适合自己的口吻。</td></tr>
  <tr>
    <td><img src="assets/images/fortune-collections.png" alt="四个像素画面系列的玩法示意" width="300"></td>
    <td><img src="assets/images/fortune-signature.png" alt="将选中的签文留作个人签名的玩法示意" width="300"></td>
  </tr>
  <tr><td>四种场景，配一点安静的微动效。</td><td>喜欢这句话，可以只换它的风景。</td></tr>
</table>

*封面与四张海报是 AI 生成的玩法示意图，图中文字根据已实现功能整理，图中均标有“玩法示意”。*

## 五步上手

1. 开机即可使用，完全离线，无需配网；若屏幕已熄灭，先按任意键唤醒。
2. 心情页用上/下键选择心情，长按下键切换口吻，按确定抽签。
3. 等待信封展开、签文揭晓；拆信时再按确定可直接看结果。
4. 结果页按上键再抽，下键只换外观；喜欢这句话，按确定留下并进入签名展示。
5. 长按确定回心情页；在心情页长按确定查看已留签名。重启会继续显示签名，再抽会保留它，确定留下另一张才会替换。

签文用于鼓励和娱乐，不作未来预测，也不承诺事情一定如何发生。

## 看看实际界面

以下预览来自程序的主机渲染，使用实际字体、排版和圆角屏幕遮罩；尚非真机照片。

![四个系列的实际界面渲染](assets/images/fortune-ui-pixel-collection.png)

<p align="center">
  <img src="assets/images/fortune-ui-unwrap.gif" alt="实际渲染的拆信动效" width="240">
  <img src="assets/images/fortune-ui-pixel-motion.gif" alt="实际渲染的像素场景微动效" width="240">
</p>

*左侧拆信，右侧场景微动效；文字保持静止，便于阅读。*

## 构建与验证

先读 [AGENTS.md](AGENTS.md)、[环境准备](docs/development/engineering/environment-setup.zh_CN.md)
和[构建指南](docs/development/engineering/build-and-test.zh_CN.md)。应用复用上游 BSP，
拥有独立设计的页面和交互。激活 ESP-IDF 5.5.3 后运行：

```bash
./tools/validate.sh
./tools/test_fortune_ui.sh
```

完整门禁生成并校验 `build/FoloToy-AI-Passport-full.bin`，从 **0x0** 刷写。
匹配的 ELF、MAP 和清单归档在 `build/firmware/` 下。固件与私有设备日志不提交 Git。
操作设备前请遵循[刷写与数据政策](docs/development/engineering/firmware-layout.zh_CN.md#烧录与已存数据)。

| 检查 | 当前结果 |
| --- | --- |
| Build | **PASS**：完整门禁与合并镜像校验通过 |
| Host tests | **PASS**：签库解码、抽签记录、持久化、字体、全部 2,112 条文字布局、按键与动画；全部 24,576 个实渲染基础画面的像素哈希不同 |
| Device tests | **有限 PASS**：2026-10-03 经授权烧录，写入校验通过，启动观察 15 秒未见错误 |
| Unverified | 实体按键交互、真机中文可读性与裁切、动画流畅度、重启及写入断电后的签名恢复、闲置与唤醒、电量准确性、耗电和续航 |

完整编码签库占 **57.3 KiB**；外观通过场景规则绘制，不存储整套背景图片，
画布使用 **12,528 字节** RAM。存储测量与测试边界详见[产品与工程设计](docs/fortune-card.zh_CN.md)。

## 阅读与扩展

- [产品设计、按键、存储与验收](docs/fortune-card.zh_CN.md)
- [逐条创作的完整签库](assets/fortune/corpus.json)
- [应用入口](main/main.c)与[像素画面生成器](main/fortune_pixels.c)
- [素材来源、许可与图片记录](assets/README.zh_CN.md)
- [AI 示意图提示词](assets/fortune/publication-image-prompts.json)
- [上游硬件与开发文档](docs/README.zh_CN.md)

应用代码与本项目原创素材遵循仓库的 [MIT 许可证](LICENSE)；Noto Sans CJK 字库保留
[SIL Open Font License](assets/fonts/OFL.txt)。上游平台为
[FoloToy AI Passport](https://github.com/FoloToy/ai-passport)。
