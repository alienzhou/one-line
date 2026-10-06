#include "fortune_mail.h"
#include "fortune_art.h"
#include <string.h>

/* Original offline letters: each beat is written for a small illustrated page.
 * Stable story indices also identify saved bookmarks. Six beats make a letter. */
typedef struct { const char *text; uint8_t scene; } beat_t;
static const beat_t WIND[] = {
    {"你好。\n我是旧邮局的阿遥。",68},
    {"清空房子的那天，\n门缝里多了一封信。",68},
    {"没有地址。\n信封画着一扇窗。",0},
    {"“风停时，\n请替我把窗打开。”",0},
    {"我把钥匙放回口袋。\n今天，先不锁门。",78},
    {"窗钩动了一下。\n像有人轻轻点头。",0},
    {"窗开了。\n一根蓝线，轻轻晃。",0},
    {"线头系着一片布。\n已经褪色了。",38},
    {"布角缝着两个字：\n修伞。",38},
    {"桥边的婆婆说：\n是老许的邮袋。",70},
    {"说完，她看了眼\n长椅的另一头。",11},
    {"我们没说话。\n水沿着伞，一滴滴。",70},
    {"雨停了一会。\n婆婆自己说起老许。",70},
    {"他每天都擦长椅。\n不送信了，只等人。",10},
    {"总有一头，\n比另一头干净。",10},
    {"他在等姐姐。\n约定里，没有日期。",40},
    {"我去时，椅子空着。\n一片叶子坐在那里。",10},
    {"我把松动的木板，\n重新钉好了。",18},
    {"长椅不晃了。\n老许坐了很久。",10},
    {"临走，他留下地图。\n折痕比河流还多。",58},
    {"姐姐曾沿着这条河，\n去很远的地方造船。",40},
    {"最后一个圈：\n白沙渡。",40},
    {"圈旁的地址，\n被铅笔描了好几遍。",58},
    {"老许说，太久了。\n邮票在口袋里发硬。",68},
    {"第二天，\n我又翻了一遍邮袋。",78},
    {"明信片，藏在夹层。\n落款是他的姐姐。",68},
    {"“我在白沙渡。\n造船，也等你。”",40},
    {"“风停时，\n替我把窗打开。”",0},
    {"他看着门缝里的信。\n“是我抄的。”",78},
    {"纸在他手里，\n轻轻响了一声。",0},
    {"他读完了。\n眼镜摘下，又戴上。",0},
    {"灯泡换好的时候，\n门外有人停了下来。",48},
    {"老许在桌边写：\n姐姐，这里灯亮了。",60},
    {"收信地址：白沙渡。\n这次，字写得很大。",68},
    {"空出来的屋子，\n好像没有那么空了。",60},
    {"纸和笔摆好了。\n窗外的风翻过一页。",78},
    {"桌上的纸少了几张。\n笔，换了个方向。",78},
    {"这天，白沙渡的回信\n比雨先到。",68},
    {"“等河面平静，\n我就回来。”",40},
    {"老许把信折好。\n没有问是哪一天。",60},
    {"我也铺开一张纸。\n收信人，写自己。",78},
    {"信封封好了。\n日期，留着以后填。",78},
    {"写给自己的信，\n先收在蓝布袋里。",38},
    {"河面平了。\n桥边，多了条小船。",40},
    {"老许走得很慢。\n船上的人也没催。",40},
    {"长椅两头，\n终于都坐了人。",10},
    {"他们先聊的，\n是今天风很小。",10},
    {"我的信写完了。\n窗，替你开着。",0}
};
static const beat_t PLATFORM[] = {
    {"你好。\n我在末班车站值夜。",30},
    {"昨晚收来一只手套。\n左手的。",49},
    {"指尖缝着一颗星。\n歪了一点。",30},
    {"失物本上，\n我给它画了右手。",60},
    {"今早翻开本子，\n右手里多了一朵花。",60},
    {"铅笔还在桌边。\n尖，刚削过。",60},
    {"今晚，\n花旁边多了一把伞。",70},
    {"我画了一场雨。\n只下在纸上。",70},
    {"清晨换班时，\n雨下面站了两个人。",30},
    {"一高一矮。\n手都藏在口袋里。",49},
    {"我问白班的老陈。\n他低头系鞋带。",30},
    {"“画小一点，\n本子快用完了。”",60},
    {"周五，\n有人来找那只手套。",30},
    {"她说，星星是\n女儿第一次缝的。",0},
    {"我翻开失物本。\n她笑出了声。",60},
    {"“这把伞，\n是她爸爸画的吧。”",70},
    {"老陈从门口进来。\n鞋带又散了。",30},
    {"这回，他没低头。\n先把椅子拉开。",60},
    {"手套领走以后，\n那页纸还留着。",60},
    {"夜里有个孩子，\n弄丢了车票。",30},
    {"等补票的工夫，\n我把本子推过去。",60},
    {"他画了一列火车。\n每扇窗都亮着。",30},
    {"信就写到这里。\n末班车快来了。",30},
    {"最后一节车厢，\n我替你留了扇窗。",30}
};
static const beat_t LIGHTHOUSE[] = {
    {"你好。\n灯塔这周归我照看。",21},
    {"交班的纸上，\n只有三个字：别急。",21},
    {"第一晚，灯没亮。\n海倒是一直响。",40},
    {"我把工具铺了一地。\n螺丝滚进鞋里。",8},
    {"天亮才发现，\n开关在门的背面。",21},
    {"我在交班纸上，\n画了一扇门。",78},
    {"第二晚，灯亮了。\n远处也亮了一下。",21},
    {"隔了很久，\n又一下。",40},
    {"我翻出旧航海图。\n那里没有岛。",58},
    {"卖鱼的人说：\n是阿满的船。",40},
    {"“他眼睛不好，\n总要多认一会。”",40},
    {"于是我坐下来，\n等他再亮一下。",21},
    {"风大的那天，\n船比平时回来得迟。",40},
    {"我把晚饭热了两遍。\n筷子没动。",6},
    {"门响了。\n一双湿鞋停在门口。",21},
    {"阿满抱着一袋橘子。\n说灯修得挺好。",6},
    {"我没提那扇门。\n他也没提风。",21},
    {"剥开的橘子皮，\n在桌上绕了一圈。",6},
    {"今天该交班了。\n工具少了一颗螺丝。",8},
    {"后来在鞋里找到。\n已经磨得发亮。",8},
    {"我把它放在窗台。\n旁边留了两个橘子。",0},
    {"交班纸的背面，\n我又写了一句。",78},
    {"“看见远处亮灯，\n可以多坐一会。”",21},
    {"写给你的这一封，\n天亮再寄。",40}
};
static const beat_t ROOFTOP[] = {
    {"你好。\n我搬到了楼顶下面。",10},
    {"上楼晒被子，\n碰见一排空花盆。",3},
    {"每只盆上，\n都写着一个名字。",3},
    {"只有最小的那只，\n写着：随便。",13},
    {"我把橘子籽埋进去。\n真的很随便。",13},
    {"第二天，\n旁边多了半壶水。",3},
    {"我留了张纸条：\n谢谢，水够了。",78},
    {"晚上纸条翻了面：\n不是给你的。",3},
    {"花盆后面，\n有只瘸腿的猫。",73},
    {"它喝完水，\n坐进了我的空篮子。",73},
    {"楼下有人笑。\n我没看见脸。",10},
    {"被子收晚了，\n带着一点夜里的凉。",10},
    {"橘子籽没发芽。\n隔壁的葱倒长高了。",3},
    {"我正要挖开看看，\n一只手递来把小铲。",13},
    {"“别挖。\n借你种点别的。”",13},
    {"是楼下卖面的阿姨。\n她带来三棵葱。",3},
    {"猫占着篮子。\n我们蹲着分土。",73},
    {"最小的花盆上，\n又添了两个名字。",13},
    {"今天上楼，\n阿姨在收被子。",10},
    {"她指了指小花盆。\n没说话。",13},
    {"两片圆叶子，\n挤在葱的旁边。",13},
    {"猫凑过来闻了闻。\n似乎没什么意见。",73},
    {"我给你画在信上。\n小得差点看不见。",78},
    {"空篮子还在。\n风先坐了进去。",10}
};
typedef struct { const char *title; const beat_t *beats; uint8_t count; } story_t;
#define STORY(title, beats) {title,beats,sizeof(beats)/sizeof(beats[0])}
static const story_t STORIES[FORTUNE_MAIL_STORIES] = {
    STORY("风停邮局",WIND), STORY("月台失物",PLATFORM),
    STORY("海边修灯人",LIGHTHOUSE), STORY("屋顶花园",ROOFTOP)
};
unsigned fortune_mail_pages(unsigned story) { return story<FORTUNE_MAIL_STORIES?STORIES[story].count:0; }
const char *fortune_mail_title(unsigned story) { return story<FORTUNE_MAIL_STORIES?STORIES[story].title:""; }
const char *fortune_mail_text(unsigned story,unsigned page) {
    return page<fortune_mail_pages(story)?STORIES[story].beats[page].text:"";
}
uint32_t fortune_mail_art(unsigned story,unsigned page,unsigned style) {
    unsigned scene=page<fortune_mail_pages(story)?STORIES[story].beats[page].scene:68;
    return FORTUNE_LEGACY_ART_COUNT+(style%6)*FORTUNE_SKIN_COUNT+
        ((style/6+1)%4)*FORTUNE_SCENE_COUNT+scene;
}
void fortune_mail_defaults(fortune_mail_t *s) {
    memset(s,0,sizeof(*s)); s->selected=FORTUNE_MAIL_NONE;
}
bool fortune_mail_valid(const fortune_mail_t *s) {
    if (!s || (s->selected>=FORTUNE_MAIL_STORIES && s->selected!=FORTUNE_MAIL_NONE)) return false;
    for (unsigned i=0;i<FORTUNE_MAIL_STORIES;++i)
        if (s->bookmarks[i]>fortune_mail_pages(i)+1) return false;
    return !s->active || (s->selected<FORTUNE_MAIL_STORIES && s->bookmarks[s->selected]>0 &&
                         s->bookmarks[s->selected]<=fortune_mail_pages(s->selected));
}
bool fortune_mail_select(fortune_mail_t *s,unsigned story) {
    if (!fortune_mail_valid(s) || story>=FORTUNE_MAIL_STORIES) return false;
    s->selected=story; s->active=true;
    if (!s->bookmarks[story] || s->bookmarks[story]>fortune_mail_pages(story)) s->bookmarks[story]=1;
    return true;
}
bool fortune_mail_step(fortune_mail_t *s,int direction) {
    if (!fortune_mail_valid(s) || !s->active || (direction!=1 && direction!=-1)) return false;
    uint8_t *page=&s->bookmarks[s->selected];
    if (direction<0 && *page==1) return false;
    *page+=direction;
    if (*page>fortune_mail_pages(s->selected)) s->active=false;
    return true;
}
void fortune_mail_restore_view(const fortune_mail_t *s,fortune_mail_view_t *v) {
    *v=(fortune_mail_view_t){0};
    if (s->selected<FORTUNE_MAIL_STORIES) {
        v->story=s->selected; v->reading=s->active;
        if (v->reading) v->page=s->bookmarks[s->selected]-1;
    }
}
uint8_t fortune_mail_draw(uint8_t *bag,uint8_t previous,uint32_t random) {
    const uint8_t all=(1U<<FORTUNE_MAIL_STORIES)-1;
    *bag&=all;
    if (!*bag) *bag=all;
    uint8_t eligible=*bag;
    if (previous<FORTUNE_MAIL_STORIES && (eligible & ~(1U<<previous))) eligible&=~(1U<<previous);
    unsigned count=0;
    for (unsigned i=0;i<FORTUNE_MAIL_STORIES;++i) count+=(eligible>>i)&1;
    unsigned pick=random%count;
    for (unsigned i=0;i<FORTUNE_MAIL_STORIES;++i) if (eligible&(1U<<i)) {
        if (!pick--) { *bag&=~(1U<<i); return i; }
    }
    return 0;
}
/* FNV-1a over explicit bytes; the record stays 16 bytes across both versions. */
static uint32_t checksum(const uint8_t *data) {
    uint32_t h=2166136261U;
    for (unsigned i=0;i<12;++i) h=(h^data[i])*16777619U;
    return h;
}
size_t fortune_mail_encode(const fortune_mail_t *s,uint8_t *out,size_t size) {
    if (!out || size<FORTUNE_MAIL_BYTES || !fortune_mail_valid(s)) return 0;
    memset(out,0,FORTUNE_MAIL_BYTES); memcpy(out,"MAIL",4); out[4]=2;
    out[5]=s->selected; out[6]=s->active;
    memcpy(out+7,s->bookmarks,FORTUNE_MAIL_STORIES);
    uint32_t hash=checksum(out);
    for (unsigned i=0;i<4;++i) out[12+i]=(uint8_t)(hash>>(i*8));
    return FORTUNE_MAIL_BYTES;
}
bool fortune_mail_decode(fortune_mail_t *s,const uint8_t *data,size_t size) {
    if (!s || !data || size!=FORTUNE_MAIL_BYTES || memcmp(data,"MAIL",4)) return false;
    uint32_t hash=0;
    for (unsigned i=0;i<4;++i) hash|=(uint32_t)data[12+i]<<(i*8);
    if (hash!=checksum(data)) return false;
    fortune_mail_t candidate; fortune_mail_defaults(&candidate);
    if (data[4]==1) {
        unsigned delivered=data[5],page=data[6],replies=data[7],sent=data[8];
        if (delivered>8 || page>=6 || sent>1 || data[9]>1 || data[10] || data[11] ||
            (replies>>delivered) || (sent && (!delivered || page)) ||
            (delivered==8 && !sent)) return false;
        candidate.selected=0;
        candidate.bookmarks[0]=delivered*6+page+1;
        candidate.active=data[9] && delivered<8;
    } else if (data[4]==2 && data[6]<=1 && !data[11]) {
        candidate.selected=data[5]; candidate.active=data[6];
        memcpy(candidate.bookmarks,data+7,FORTUNE_MAIL_STORIES);
    } else return false;
    if (!fortune_mail_valid(&candidate)) return false;
    *s=candidate; return true;
}
