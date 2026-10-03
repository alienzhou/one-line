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
LVGL 主机测试检查字体覆盖、实际控件字体、缺字负例和全部 2112 条文字布局；真机中文显示仍未验证。

## 签文内容

`fortune/corpus.json` 保存八种心情、三种口吻的 2112 条逐条创作的完整句子，每组 88 条。
达到用户后续提出的 2000 条以上阶段要求；最初 10001 条以上的扩充目标仍未完成。
没有运行时模板或短句组合。`tools/pack_fortunes.py` 使用无损字符字典编码完整签文；
实测签文、字典和索引共 58686 字节。`--release` 执行当前明确的 2001 条最低门槛。
详见[产品设计](../docs/fortune-card.zh_CN.md)。

四个原创像素系列由 `main/fortune_pixels.c` 绘制，不嵌入下载的画作或场景图片。
忽略目录 `build/fortune-preview/` 中的预览来自实际 LVGL 主机渲染。

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
| [`images/fortune-cover.png`](images/fortune-cover.png) | 1086 × 1448，PNG | 一签社区封面，原创 AI 玩法示意图。 |
| [`images/fortune-letters.png`](images/fortune-letters.png) | 1086 × 1448，PNG | 拆信玩法示意图。 |
| [`images/fortune-voices.png`](images/fortune-voices.png) | 1086 × 1448，PNG | 口吻与心情示意图。 |
| [`images/fortune-collections.png`](images/fortune-collections.png) | 1086 × 1448，PNG | 四个像素系列示意图。 |
| [`images/fortune-signature.png`](images/fortune-signature.png) | 1086 × 1448，PNG | 留作个人签名示意图。 |
| [`images/fortune-ui-pixel-collection.png`](images/fortune-ui-pixel-collection.png) | 960 × 350，PNG | 实际 LVGL 主机渲染的四张卡片合集。 |
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

可复用的音乐与音效源码放在 `music/`。

- 记录来源、许可、采样率、位深、声道、转换命令与目标路径。
- 与当前 BSP 音频路径匹配时优先采用 16 kHz、16 位单声道 PCM。
- 嵌入音频前评估 Flash 与内部 RAM 成本；长录音应流式或分块。
- 无再分发许可不提交媒体文件。
