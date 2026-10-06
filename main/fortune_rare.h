#pragma once
#include "fortune_art.h"
#include <stdbool.h>
#include <stdint.h>

#define FORTUNE_RARE_QUOTE 0x20000000U
#define FORTUNE_RARE_NONE 255U
#define FORTUNE_RARE_TOPIC 10U
#define FORTUNE_RARE_FIRST_PITY 10U
#define FORTUNE_RARE_PITY 30U
#define FORTUNE_RARE_PERCENT 1U
#define FORTUNE_RARE_MASK ((1U << FORTUNE_RARE_COUNT) - 1U)

static inline unsigned fortune_rare_index(uint32_t quote) {
    return quote >= FORTUNE_RARE_QUOTE && quote < FORTUNE_RARE_QUOTE + FORTUNE_RARE_COUNT ?
        quote - FORTUNE_RARE_QUOTE : FORTUNE_RARE_NONE;
}
static inline bool fortune_rare_art(uint32_t art) {
    return art >= FORTUNE_RARE_ART_BASE && art < FORTUNE_ART_COUNT;
}
/* Freeze the first six cards' three palette IDs; append new cards in triples. */
static inline uint32_t fortune_rare_art_id(unsigned index,unsigned tone) {
    return FORTUNE_RARE_ART_BASE + (index<FORTUNE_FIRST_RARE_COUNT ?
        index+tone*FORTUNE_FIRST_RARE_COUNT :
        FORTUNE_FIRST_RARE_COUNT*FORTUNE_RARE_PALETTES+
        (index-FORTUNE_FIRST_RARE_COUNT)*FORTUNE_RARE_PALETTES+tone);
}
static inline unsigned fortune_rare_art_index(uint32_t art) {
    if (!fortune_rare_art(art)) return FORTUNE_RARE_NONE;
    unsigned offset=art-FORTUNE_RARE_ART_BASE;
    return offset<FORTUNE_FIRST_RARE_COUNT*FORTUNE_RARE_PALETTES ? offset%FORTUNE_FIRST_RARE_COUNT :
        FORTUNE_FIRST_RARE_COUNT+(offset-FORTUNE_FIRST_RARE_COUNT*FORTUNE_RARE_PALETTES)/FORTUNE_RARE_PALETTES;
}
static inline unsigned fortune_rare_art_tone(uint32_t art) {
    if (!fortune_rare_art(art)) return 0;
    unsigned offset=art-FORTUNE_RARE_ART_BASE;
    return offset<FORTUNE_FIRST_RARE_COUNT*FORTUNE_RARE_PALETTES ? offset/FORTUNE_FIRST_RARE_COUNT :
        (offset-FORTUNE_FIRST_RARE_COUNT*FORTUNE_RARE_PALETTES)%FORTUNE_RARE_PALETTES;
}
static inline const char *fortune_rare_title(unsigned index) {
    static const char *const titles[] = {
        "鲸游星海", "末班月车", "雨停之后", "千灯入梦", "极光花园", "银河来信",
        "星鹿入林", "云海灯塔", "花火时刻", "水母舞会", "月下茶会", "星河书房",
        "天空帆船", "雪夜列岛", "萤火之森", "时间沙漏", "金鱼游梦", "风铃长廊",
        "流星小镇", "黎明之门", "星环旅馆", "雪原来客", "星尘乐盒", "月光转轮",
        "深海珍珠", "云端邮差", "蝶落星云", "浮岛飞瀑", "极夜唱片", "银杏车站"
    };
    return index < FORTUNE_RARE_COUNT ? titles[index] : "奇遇珍藏";
}
static inline const char *fortune_rare_text(unsigned index) {
    static const char *const texts[] = {
        "今晚，海替天空保管星星。",
        "末班车，开往还没做完的梦。",
        "天还没放晴，水洼先知道了。",
        "万家灯火里，也有一盏在等你。",
        "夜走到最深处，开始开花。",
        "你寄出的微光，银河收到了。",
        "鹿角接住月光，森林亮了一整夜。",
        "灯塔不问归期，只把海照亮。",
        "这一秒的光，值得把头抬起来。",
        "海底没有舞曲，水母自己发光。",
        "月亮坐在对面，茶慢慢凉下来。",
        "翻到空白那页，星星落了进去。",
        "把帆交给风，把远方留给自己。",
        "雪把世界调轻，窗里留着暖光。",
        "走进夜色，口袋里装满萤火。",
        "沙漏落下的，是金色的此刻。",
        "梦里的金鱼，游出了画框。",
        "风路过长廊，替铃铛说了晚安。",
        "小镇睡着以后，流星来敲窗。",
        "推开那扇门，天刚好亮了。",
        "绕过星环，有一扇窗为你亮着。",
        "雪地里的脚印，走成了相遇。",
        "转动小小发条，星空开始唱歌。",
        "慢慢转一圈，烦恼留在地面。",
        "安静的海底，也藏着一束光。",
        "云朵替你，把想念送得很远。",
        "轻轻振翅，夜空多了一种颜色。",
        "瀑布落进云里，远方没有尽头。",
        "让这支旧曲，陪夜色慢慢转动。",
        "秋天落满长椅，下一站有好事。"
    };
    return index < FORTUNE_RARE_COUNT ? texts[index] : "";
}
/* Hand-set line breaks keep every authored sentence's phrases intact. */
static inline const char *fortune_rare_display_text(unsigned index) {
    static const char *const lines[] = {
        "今晚，海替天空\n保管星星。", "末班车，开往\n还没做完的梦。",
        "天还没放晴，\n水洼先知道了。", "万家灯火里，\n也有一盏在等你。",
        "夜走到最深处，\n开始开花。", "你寄出的微光，\n银河收到了。",
        "鹿角接住月光，\n森林亮了一整夜。", "灯塔不问归期，\n只把海照亮。",
        "这一秒的光，\n值得把头抬起来。", "海底没有舞曲，\n水母自己发光。",
        "月亮坐在对面，\n茶慢慢凉下来。", "翻到空白那页，\n星星落了进去。",
        "把帆交给风，\n把远方留给自己。", "雪把世界调轻，\n窗里留着暖光。",
        "走进夜色，\n口袋里装满萤火。", "沙漏落下的，\n是金色的此刻。",
        "梦里的金鱼，\n游出了画框。", "风路过长廊，\n替铃铛说了晚安。",
        "小镇睡着以后，\n流星来敲窗。", "推开那扇门，\n天刚好亮了。",
        "绕过星环，\n有一扇窗为你亮着。", "雪地里的脚印，\n走成了相遇。",
        "转动小小发条，\n星空开始唱歌。", "慢慢转一圈，\n烦恼留在地面。",
        "安静的海底，\n也藏着一束光。", "云朵替你，\n把想念送得很远。",
        "轻轻振翅，\n夜空多了一种颜色。", "瀑布落进云里，\n远方没有尽头。",
        "让这支旧曲，\n陪夜色慢慢转动。", "秋天落满长椅，\n下一站有好事。"
    };
    return index < FORTUNE_RARE_COUNT ? lines[index] : "";
}
