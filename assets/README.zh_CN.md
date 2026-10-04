<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 资源目录（Assets）

本目录集中存放可复用的资源（字库、图片、音乐等），按资源类型分子目录管理。每个资源放在其类型对应的子目录，并记录放置路径、命名方式、集成方式与来源/许可。二进制资源（字体、图片、音频）不属于纯 markdown 文档，请勿与文档混放。涉及版权/授权的资源需注明来源与许可。

## 字库（fonts）

抽签应用使用官方
[Noto CJK 仓库](https://github.com/notofonts/noto-cjk/blob/main/Sans/OTF/SimplifiedChinese/NotoSansCJKsc-Regular.otf)
中的 Noto Sans CJK SC Regular，许可证见 `fonts/OFL.txt`。
源文件 SHA-256：`2c76254f6fc379fddfce0a7e84fb5385bb135d3e399294f6eeb6680d0365b74b`。
固件只链接 12/20 px、4 bpp、未压缩的生成子集；16 MB 原始 OTF 是开发素材，不进入固件。
使用 1.5.3 版转换器运行 `python3 tools/generate_fortune_fonts.py --converter /path/to/lv_font_conv` 可重建。
脚本收集完整签文与所有界面文本，字符清单为 `fonts/fortune-characters.txt`。
LVGL 主机测试检查字体覆盖、实际控件字体、缺字负例和全部 2200 条当前和 2112 条旧版文字布局；真机中文显示仍未验证。

## 签文内容

`fortune/corpus.json` 保存 2200 张完整签文：1980 条原创短句与 220 条古诗词摘句。
八主题比例为 20/10/15/15/15/10/10/5%，不使用运行时拼句；当前阶段超过 2000 条，10001 条目标待扩充。
`fortune/legacy-corpus.json` 冻结旧版 2112 条，仅用于保留已固定的签名。
`fortune/poetry-sources.json` 保留每条诗词的作者、完整篇名、源段落与固定提交链接，
核对来源为 [chinese-poetry](https://github.com/chinese-poetry/chinese-poetry)，
其许可保存在 `fortune/sources/chinese-poetry-LICENSE.txt`。古代作品为公有领域，繁简转换采用 opencc-python-reimplemented 0.1.7，
摘句只调整标点，不改写成原创；长篇名在界面缩略，源数据保留全名。
当前文本流、字典与索引合计 67915 字节，另有出处、旧库和迁移映射。
`tools/pack_fortunes.py --release` 仍执行 2001 条最低要求，完整测量见[产品设计](../docs/fortune-card.zh_CN.md)。

十个原创像素系列由 `main/fortune_pixels.c` 绘制，不嵌入下载的画作或场景图片。
忽略目录 `build/fortune-preview/` 中的预览来自实际 LVGL 主机渲染。
新版组合 80 种场景和 4 种主体，得到 320 款皮肤，每款 6 组配色；全部 1920 张完整签名卡
保存在 `images/skins/`。使用 `./tools/test_fortune_ui.sh --skins` 完成渲染后，
运行 `python3 tools/render_fortune_gallery.py` 转换并打包（需要 Pillow），生成
[`fortune/skin-gallery.html`](fortune/skin-gallery.html) 与含每张图片 SHA-256 的
[`fortune/skin-catalog.json`](fortune/skin-catalog.json)。图库是程序实际渲染，全部完成后检查；
不作为真机截图，不进入固件。旧版发布海报 `fortune-collections.png` 作为首版历史素材保留。

可复用的字库文件与生成的字库源码放在 `fonts/`。

- 命名要能反映字族、字重、字级与格式。
- 记录来源、许可、字符范围、转换命令与目标放置路径。
- 添加字库前评估 Flash 与内部 RAM 影响；ESP32-C3 无 PSRAM。
- 不提交许可不允许分发的字库。

## 图片（images）

可复用的源图与生成的显示资产放在 `images/`。

| 文件 | 尺寸与格式 | 用途与来源 |
| --- | --- | --- |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160，JPEG | 嵌入中英文项目 README 的产品主图，突出 AI Passport 产品形象与开放、人人可创作的理念。 |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724，PNG RGBA | 保留为可选技术参考图，不再用于首页主视觉。于 2026-09-17 使用内置图像生成工具为本仓库生成；已根据文档中的硬件能力契约核对图中的六项标签与参数。 |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336，PNG RGBA | 从仓库原始 `images/logo.png` 中精确裁切并去除背景的黑色字标；用于中英文项目 README 的浅色主题。 |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336，PNG RGBA | 提取字标的白色版本；README 使用 `<picture>` 在 GitHub 深色主题下显示。 |
| `images/fortune-themes.png` | 1160 × 960，PNG | 已完成的实际 LVGL 渲染组成的八主题总览；MIT。 |
| `images/fortune-home.png`、`fortune-theme-1.png`～`fortune-theme-8.png` | 240 × 320，PNG | 实际应用渲染的首页与主题示例；MIT。 |
| [`images/fortune-cover.png`](images/fortune-cover.png) | 1086 × 1448，PNG | 一签社区封面，原创 AI 玩法示意图。 |
| [`images/fortune-letters.png`](images/fortune-letters.png) | 1086 × 1448，PNG | 拆信玩法示意图。 |
| [`images/fortune-voices.png`](images/fortune-voices.png) | 1086 × 1448，PNG | 口吻与心情示意图。 |
| [`images/fortune-collections.png`](images/fortune-collections.png) | 1086 × 1448，PNG | 四个像素系列示意图。 |
| [`images/fortune-signature.png`](images/fortune-signature.png) | 1086 × 1448，PNG | 留作个人签名示意图。 |
| [`images/fortune-ten-collections.png`](images/fortune-ten-collections.png) | 1086 × 1448，PNG | 十个系列的实际程序渲染预览；MIT。 |
| [`images/fortune-skins-overview.png`](images/fortune-skins-overview.png) | 2172 × 7240，PNG | 全部 320 款皮肤的实际插画区域总览，程序渲染；MIT。 |
| `images/skins/overview-0.png` 至 `overview-9.png` | 各 1086 × 1448，PNG | 十个系列的 32 款皮肤总览，程序渲染；MIT。 |
| `images/skins/skin-000-0.png` 至 `skin-319-5.png` | 各 240 × 320，PNG | 全部 320 × 6 张完整签名卡，实际 LVGL 渲染；MIT。 |
| [`images/fortune-ui-pixel-collection.png`](images/fortune-ui-pixel-collection.png) | 1200 × 700，PNG | 实际 LVGL 主机渲染的十张卡片合集。 |
| [`images/fortune-volume.png`](images/fortune-volume.png) | 240 × 320，PNG | 实际 LVGL 主机渲染的音量面板；MIT，由 `tools/test_fortune_ui.sh` 生成，非实机照片。 |
| [`images/fortune-ui-unwrap.gif`](images/fortune-ui-unwrap.gif) | 240 × 320，GIF | 实际主机渲染的拆信过程。 |
| [`images/fortune-ui-pixel-motion.gif`](images/fortune-ui-pixel-motion.gif) | 240 × 320，GIF | 实际主机渲染的场景微动效。 |

五张竖版示意图于 2026-10-03 使用 Codex 内置图像生成工具制作。五个任务均明确完成后，
逐张打开实际上传文件检查成品；图中标有“玩法示意”，不冒充真机照片。
提示词保存在 [`fortune/publication-image-prompts.json`](fortune/publication-image-prompts.json)。
图片用于分支项目 README 和社区提交，不嵌入固件；本项目原创图片遵循仓库 MIT 许可证。
三份界面预览来自实际应用渲染器，使用相同许可证。示意海报与主机预览各自注明用途，
真机画面验收仍待完成。

- 使用描述性命名，并记录尺寸、像素格式、转换步骤与目标路径。
- 优先采用适合 240 × 320 RGB565 显示的格式，并纳入 Flash 与内部 RAM 考量。
- 许可允许时保留可编辑源文件，并记录来源与许可。
- 图片中不得包含设备二维码秘密、凭证或个人数据。

## 音乐与音效（music）

原创音乐盒音效由 `main/fortune_sound.c` 实时合成，采用仓库 MIT 许可证。
`tools/render_fortune_audio.py` 将已经完成的 16 kHz、16-bit 单声道 WAV 试听导出到
`music/`：`fortune-draw-1.wav` 至 `fortune-draw-4.wav`、`fortune-keep.wav`、
`fortune-skin.wav`，以及组合试听 `fortune-audition.wav`。`fortune-audio.json` 记录完成状态、
长度和哈希。没有下载采样或第三方乐曲。固件只链接紧凑乐谱与正弦表，试听 WAV 仅为
开发和展示素材，不嵌入固件；试听不包含实体音频芯片和扬声器的音染。

可复用的音乐与音效源码放在 `music/`。

- 记录来源、许可、采样率、位深、声道、转换命令与目标路径。
- 与当前 BSP 音频路径匹配时优先采用 16 kHz、16 位单声道 PCM。
- 嵌入音频前评估 Flash 与内部 RAM 成本；长录音应流式或分块。
- 无再分发许可不提交媒体文件。
