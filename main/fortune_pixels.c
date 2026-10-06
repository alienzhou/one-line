#include "fortune_pixels.h"
#include "fortune_rare.h"
#include <stdlib.h>

typedef struct { uint16_t *p; int x1,x2; } canvas_t;
static uint16_t rgb(uint32_t v) { return ((v>>8)&0xF800)|((v>>5)&0x7E0)|((v>>3)&31); }
static uint32_t mix(uint32_t n) { n ^= n>>16; n *= 0x7feb352dU; n ^= n>>15; n *= 0x846ca68bU; return n^(n>>16); }
static void dot(canvas_t c,int x,int y,uint32_t v) {
    if(x>=c.x1 && x<c.x2 && y>=0 && y<FORTUNE_PIXEL_H) c.p[y*FORTUNE_PIXEL_W+x]=rgb(v);
}
static void box(canvas_t c,int x,int y,int w,int h,uint32_t v) {
    for(int j=y;j<y+h;++j) for(int i=x;i<x+w;++i) dot(c,i,j,v);
}
static void line(canvas_t c,int x,int y,int xx,int yy,uint32_t v) {
    int dx=abs(xx-x),sx=x<xx?1:-1,dy=-abs(yy-y),sy=y<yy?1:-1,e=dx+dy;
    for(;;) { dot(c,x,y,v); if(x==xx && y==yy) break; int e2=2*e;
        if(e2>=dy) { e+=dy; x+=sx; } if(e2<=dx) { e+=dx; y+=sy; } }
}
static void disc(canvas_t c,int x,int y,int r,uint32_t v) {
    for(int j=-r;j<=r;++j) for(int i=-r;i<=r;++i) if(i*i+j*j<=r*r) dot(c,x+i,y+j,v);
}
static void star(canvas_t c,int x,int y,int r,uint32_t v) {
    box(c,x-r,y,r*2+1,1,v); box(c,x,y-r,1,r*2+1,v);
}
static uint32_t blend(uint32_t a,uint32_t b,unsigned amount);

fortune_colors_t fortune_pixel_colors(uint32_t id) {
    if (fortune_rare_art(id)) {
        static const fortune_colors_t jewel[]={
            {0x101329,0xFFF0C7,0xC6B58F,0xEDC06C},
            {0x24132D,0xFFE2CF,0xCCA4B9,0xF4AF9D},
            {0x092832,0xE4FFF5,0xA2CCC2,0x93E5CF}
        };
        return jewel[fortune_rare_art_tone(id)];
    }
    static const fortune_colors_t expanded[4][6] = {
        {{0x151D30,0xF5DDB5,0xB5A78D,0xEBA76C},{0x25203C,0xF1DCE4,0xB7A1BF,0xF3AB9F},{0x132B32,0xE4EAD7,0xA1BDB3,0xF3CE8B},
         {0x292639,0xF7DFC2,0xB5A1A5,0xD99881},{0x1B3043,0xDCE8E8,0x9AAFC4,0xDBBB80},{0x322638,0xEFDEE5,0xBDA6B9,0xD7A77F}},
        {{0xF1EADB,0x293D3A,0x5B7065,0xB76046},{0xE6EBCD,0x354938,0x657954,0x9B623D},{0xEADCE1,0x504355,0x826A82,0xB56469},
         {0xE8DBC8,0x4A4438,0x84745C,0xBC8153},{0xE0E8E2,0x324B52,0x6B8485,0xAC725C},{0xECE2CB,0x494A3A,0x8B8766,0xB98159}},
        {{0x1E2441,0xEBDDC6,0xADB0CC,0xE2A76C},{0x312039,0xEFDADE,0xBBA4C0,0xF1BD82},{0x132E36,0xD9EBDF,0xA0B8B3,0xD9BC82},
         {0x302C46,0xE5E0EF,0xACA5C8,0xC6A082},{0x233747,0xDCE5DE,0xA0B5C0,0xD2BA82},{0x332A35,0xEDDFCC,0xBCA899,0xDCA18A}},
        {{0xEDE6CF,0x334B44,0x6A7961,0xCA7654},{0xF1DDCA,0x604539,0x956E57,0xAA6553},{0xDFE6E7,0x354654,0x687D8C,0x8D7594},
         {0xE8DEC9,0x4B4C38,0x85866D,0xB78356},{0xE4E8D8,0x3A5145,0x7D8E75,0xA77567},{0xEADCE3,0x514459,0x8E7A95,0xB77783}}
    };
    if(id>=FORTUNE_LEGACY_ART_COUNT) {
        uint32_t offset=id-FORTUNE_LEGACY_ART_COUNT;
        unsigned family=offset%FORTUNE_FAMILY_COUNT,tone=(offset/FORTUNE_SKIN_COUNT)%6;
        if(family<4) return expanded[family][tone];
        static const fortune_colors_t extra[]={
            {0x183F4B,0xE5DDC1,0x82B1AD,0xE1A68A}, /* underwater */
            {0xEAE3DC,0x454B65,0x9B9FB4,0xBD856C}, /* cloud islands */
            {0xE9DBC7,0x4A403D,0x998774,0xBD7054}, /* coffee */
            {0x303246,0xEEDAC0,0xB4A6B4,0xD89C87}, /* vintage fair */
            {0xDDD7BB,0x354D49,0x849387,0xBC7951}, /* workshop */
            {0xDFE8E4,0x3C5260,0x91A6AF,0xB87B67}  /* winter */
        };
        static const uint32_t tints[]={0xE8DFC8,0xD8A5AA,0x91B4BB,0xC5A574,0xA5BCA1,0xBDA5C8};
        fortune_colors_t p=extra[family-4];
        if(tone) {p.paper=blend(p.paper,tints[tone],1);p.ink=blend(p.ink,tints[tone],1);
            p.muted=blend(p.muted,tints[tone],3);p.accent=blend(p.accent,tints[tone],4);}
        return p;
    }
    static const fortune_colors_t sets[4][3] = {
        {{0x151D30,0xF5DDB5,0xB5A78D,0xEBA76C},{0x25203C,0xF1DCE4,0xB7A1BF,0xF3AB9F},{0x132B32,0xE4EAD7,0xA1BDB3,0xF3CE8B}},
        {{0xF1EADB,0x293D3A,0x5B7065,0xB76046},{0xE6EBCD,0x354938,0x657954,0x9B623D},{0xEADCE1,0x504355,0x826A82,0xB56469}},
        {{0x1E2441,0xEBDDC6,0xADB0CC,0xE2A76C},{0x312039,0xEFDADE,0xBBA4C0,0xF1BD82},{0x132E36,0xD9EBDF,0xA0B8B3,0xD9BC82}},
        {{0xEDE6CF,0x334B44,0x6A7961,0xCA7654},{0xF1DDCA,0x604539,0x956E57,0xAA6553},{0xDFE6E7,0x354654,0x687D8C,0x8D7594}}
    };
    return sets[id%4][(id/4)%3];
}

static void cat(canvas_t c,int x,int y,uint32_t coat,uint32_t eye,int pose,uint32_t f) {
    box(c,x+2,y+4,11,8,coat); box(c,x+1,y+1,3,6,coat); box(c,x+10,y,3,7,coat);
    box(c,x+3,y+12,9,5,coat); box(c,x,y+15,4,2,coat);
    line(c,x+12,y+14,x+16,y+12,coat); line(c,x+16,y+12,x+16,y+9,coat);
    int blink=f%32==27;
    box(c,x+4,y+6,2,blink?1:2,eye); box(c,x+9,y+6,2,blink?1:2,eye);
    dot(c,x+7,y+9,eye);
    if(pose&1) { box(c,x+3,y+12,9,2,0xD38E69); dot(c,x+9,y+14,0xD38E69); }
}

static void night(canvas_t c,uint32_t seed,unsigned tone,uint32_t f,fortune_colors_t p) {
    static const uint32_t sky[]={0x263D58,0x413954,0x285356,0x253246,0x3E304E,0x26494B};
    uint32_t s=sky[tone]; box(c,0,0,108,58,p.paper);
    /* An asymmetrical window, foreground desk and tiny radio. */
    box(c,4,2,72,47,p.muted); box(c,6,4,68,43,s);
    int moon=50+(seed%17); disc(c,moon,12,5,p.accent); disc(c,moon+2,10,5,s);
    for(int i=0;i<9;++i) {
        uint32_t h=mix(seed*31+i); int x=6+i*8,y=24+(h%12);
        box(c,x,y,7,47-y,0x192B3B);
        for(int j=0;j<3;++j) if(h&(1U<<(j+6))) box(c,x+2,y+3+j*6,2,2,p.accent);
    }
    /* Sparse rain in the glass; only one-pixel steps, no full-screen movement. */
    for(unsigned i=0;i<11;++i) {
        uint32_t h=mix(seed*107+i*311); int x=8+h%63,y=6+((h>>8)+(f/2)%4)%35;
        box(c,x,y,1,2,0x739399);
    }
    box(c,39,4,2,43,p.muted); box(c,6,29,68,2,p.muted);
    box(c,2,47,77,3,p.accent); box(c,0,53,108,5,0x101F2A);
    /* Lamp: stepped shade, brass stem, lit notebook. */
    box(c,86,20,8,2,p.accent); box(c,83,22,14,3,p.accent); box(c,80,25,20,4,p.accent);
    box(c,89,29,2,22,p.muted); box(c,84,50,13,2,p.muted);
    box(c,48,50,22,2,p.ink); box(c,58,50,1,2,p.muted);
    if(seed&1) cat(c,17,33,p.paper,p.accent,seed,f);
    else { box(c,13,39,14,10,p.accent); box(c,15,41,9,5,p.paper); line(c,23,39,29,31,p.muted); box(c,16,43,1,2,p.ink); box(c,19,43,1,2,p.ink); }
    /* A hanging print varies its small, legible constellation. */
    box(c,84,3,15,11,p.muted); box(c,85,4,13,9,p.paper);
    for(unsigned i=0;i<5;++i) { uint32_t h=mix(seed+i*831); dot(c,87+h%9,6+(h>>8)%5,p.accent); }
}

static void wild(canvas_t c,uint32_t seed,unsigned tone,uint32_t f,fortune_colors_t p) {
    static const uint32_t skies[]={0xC3DAD1,0xD1D7AF,0xC9BBCE,0xDBC3A3,0xB2CDD0,0xDEBDC0};
    uint32_t sky=skies[tone]; box(c,0,0,108,58,sky);
    disc(c,79-(seed%12),13,7,0xF3DFAB);
    for(int j=0;j<2;++j) {
        int x=9+(seed*3+j*44)%54+(f/8)%2,y=7+j*9;
        box(c,x,y,19,3,0xEDEAD7); box(c,x+4,y-2,10,2,0xEDEAD7);
    }
    int peak=23+seed%25;
    for(int x=0;x<108;++x) {
        int y=16+abs(x-peak)/2; if(y<58) box(c,x,y,1,58-y,0x779D93);
        y=24+abs(x-84)/3; if(y<58) box(c,x,y,1,58-y,0x456E6A);
    }
    /* A terraced meadow and a thin, winding stream. */
    for(int y=37;y<58;++y) box(c,0,y,108,1,y<45?0xA4B087:0x778F6D);
    for(int y=38;y<58;++y) { int x=66+(y-38)*(y-38)/60; box(c,x,y,3+(y-38)/4,1,0xD3DBB5); }
    for(unsigned i=0;i<43;++i) {
        uint32_t h=mix(seed*163+i*331); int x=h%108,y=39+(h>>8)%19;
        if(x>64 && x<83) continue;
        box(c,x,y,1,2,0x3E6860); if(i%5==0) dot(c,x+1,y-1,p.accent);
    }
    int home=15+(seed%9);
    if(seed&1) { /* cabin */
        box(c,home,33,15,12,0xE6CCA3); for(int i=0;i<7;++i) box(c,home-3+i,32-i,21-i*2,1,0xAC7256);
        box(c,home+3,37,4,4,0x496C62); box(c,home+10,38,3,7,0x735B49);
        box(c,home+12,26,2,5,0xAC7256); dot(c,home+13+(f/8)%2,23,0xEDEAD7);
    } else { /* a camp tent */
        for(int y=0;y<12;++y) box(c,home+8-y,31+y,y*2+1,1,p.accent);
        for(int y=0;y<8;++y) box(c,home+8-y/2,35+y,y+1,1,0x3E6860);
        line(c,home-4,44,home+23,44,0xE6CCA3);
    }
    /* Fireflies have a long quiet cycle. */
    if(f%24<6) { uint32_t h=mix(seed+519); star(c,87+h%13,41+(h>>8)%9,1,0xF3DFAB); }
}

static void space(canvas_t c,uint32_t seed,unsigned tone,uint32_t f,fortune_colors_t p) {
    static const uint32_t skies[]={0x2D395D,0x4E3655,0x285354,0x364465,0x523E59,0x355C5B};
    uint32_t sky=skies[tone]; box(c,0,0,108,58,p.paper);
    /* Perforated, stepped-edge miniature postage stamp. */
    box(c,5,1,98,56,p.muted); box(c,7,3,94,52,sky);
    for(int x=8;x<103;x+=7) { box(c,x,0,3,3,p.paper); box(c,x,55,3,3,p.paper); }
    for(int y=6;y<54;y+=7) { box(c,4,y,3,3,p.paper); box(c,101,y,3,3,p.paper); }
    for(unsigned i=0;i<24;++i) {
        uint32_t h=mix(seed*71+i*321); int x=11+h%86,y=7+(h>>8)%43;
        if(i<3) star(c,x,y,(f/4+i)%8==0?1:0,p.ink); else dot(c,x,y,p.muted);
    }
    int px=48+seed%13,py=28,r=13+(seed%3);
    disc(c,px,py,r,p.accent);
    for(int y=-r;y<=r;++y) for(int x=-r;x<=r;++x) {
        if(x*x+y*y>r*r) continue;
        if(x+y>r/2 && ((x+y)&1)==0) dot(c,px+x,py+y,0x956778);
        else if(x+y< -r/2 && (x-y)%3==0) dot(c,px+x,py+y,0xE8CA9D);
    }
    for(int x=-27;x<=27;++x) {
        int y=x/4;
        if(abs(x)>r || x>0) { dot(c,px+x,py+y,p.ink); dot(c,px+x,py+y+2,p.muted); }
    }
    /* Message envelope in the lower corner. */
    box(c,14,42,15,9,p.ink); line(c,14,42,21,47,sky); line(c,21,47,28,42,sky);
    box(c,84,8,10,1,p.muted); box(c,86,11,8,1,p.muted); box(c,88,14,6,1,p.muted);
}

static void garden(canvas_t c,uint32_t seed,unsigned tone,uint32_t f,fortune_colors_t p) {
    static const uint32_t shade[]={0xCECDB0,0xDDBBA3,0xBDC9CB,0xC6CCAA,0xD2B6A8,0xAFBECA};
    box(c,0,0,108,58,p.paper);
    /* A botanical specimen label: double square corners, asymmetric plant. */
    box(c,2,1,104,1,p.muted); box(c,2,56,104,1,p.muted);
    box(c,2,1,1,56,p.muted); box(c,105,1,1,56,p.muted);
    box(c,5,4,98,50,shade[tone]);
    for(int y=8;y<52;y+=5) for(int x=9;x<101;x+=6) dot(c,x,y,p.paper);
    int pot=21+(seed%10); box(c,pot,41,21,3,p.accent); box(c,pot+2,44,17,8,p.accent);
    box(c,pot+5,45,2,5,p.paper); line(c,pot+10,42,pot+10,18,p.ink);
    for(int i=0;i<5;++i) {
        int side=i%2?1:-1,x=pot+10+side*(6+(seed>>(i+2))%4),y=34-i*4;
        line(c,pot+10,y+4,x,y,p.ink); box(c,x-3,y-2,7,4,p.ink); box(c,x-1,y-3,4,6,p.ink);
        dot(c,x-1,y-1,p.muted);
    }
    int flower=pot+10;
    box(c,flower-4,12,9,3,p.accent); box(c,flower-2,10,5,7,p.accent); box(c,flower-1,12,3,3,p.paper);
    if(seed&1) cat(c,65,35,p.ink,p.paper,seed,f);
    else { /* small ceramic cup, a slowly rising pixel of steam */
        box(c,66,38,15,11,p.paper); box(c,69,48,9,3,p.paper); box(c,81,39,4,7,p.paper); box(c,81,41,2,3,shade[tone]);
        box(c,64,51,23,1,p.ink); box(c,70,37,7,1,p.ink);
        dot(c,73+(f/6)%2,34-(f/6)%3,p.muted);
    }
    /* The seed changes the botanical print, not the signature wording. */
    for(unsigned i=0;i<8;++i) {
        uint32_t h=mix(seed*193+i*29); int x=57+h%39,y=9+(h>>8)%19;
        star(c,x,y,i==0 && f%32<5?2:1,(i&1)?p.paper:p.accent);
    }
}

uint32_t fortune_skin_index(uint32_t id) {
    return id<FORTUNE_LEGACY_ART_COUNT || fortune_rare_art(id)?UINT32_MAX:(id-FORTUNE_LEGACY_ART_COUNT)%FORTUNE_SKIN_COUNT;
}
uint32_t fortune_scene_index(uint32_t id) {
    uint32_t skin=fortune_skin_index(id);
    return skin==UINT32_MAX?UINT32_MAX:skin%FORTUNE_SCENE_COUNT;
}
const char *fortune_scene_name(uint32_t id) {
    if (fortune_rare_art(id)) return fortune_rare_title(fortune_rare_art_index(id));
    static const char *const old[]={"夜航电台","旷野来信","宇宙邮局","口袋花园"};
    static const char *const names[FORTUNE_FAMILY_COUNT][FORTUNE_LAYOUT_COUNT]={
        {"窗边电台","天台晚风","雨夜书店","末班列车","港口灯火","城市阳台","深夜图书","雨巷小店"},
        {"山溪木屋","峡谷石桥","海边灯塔","森林树屋","沙丘营地","雪山湖畔","风车麦田","花野远山"},
        {"星环来信","飞船座舱","月面营地","山顶观星","火箭站台","银河咖啡","轨道花园","彗星旷野"},
        {"窗台花语","玻璃温室","瓶中森林","蔷薇拱门","莲池小亭","花车集市","蘑菇小屋","秘密庭院"},
        {"珊瑚邮局","海底车站","水母夜灯","鲸落花园","潜艇窗口","珍珠贝屋","沉船书架","泡泡茶馆"},
        {"云上风车","漂浮小屋","彩虹桥头","气球港口","天空花园","云端车站","风筝山丘","星眠云床"},
        {"手冲清晨","烘焙厨房","窗边甜点","咖啡花店","唱片午后","街角报亭","屋顶茶座","小巷面包"},
        {"旋转木马","摩天轮下","街机角落","售票小亭","棉花糖车","游园列车","木偶剧场","气球小摊"},
        {"齿轮工房","修表小店","陶艺小屋","染布庭院","玻璃灯坊","星图工作","邮票刻坊","手作纸屋"},
        {"雪夜邮局","壁炉来信","冰湖小屋","雪人庭院","冬日站台","松林营地","暖灯小镇","雪中书亭"}
    };
    uint32_t skin=fortune_skin_index(id);
    return skin==UINT32_MAX?old[id%4]:names[skin%FORTUNE_FAMILY_COUNT][(skin/FORTUNE_FAMILY_COUNT)%FORTUNE_LAYOUT_COUNT];
}
const char *fortune_family_name(unsigned family) {
    static const char *const names[]={"夜航电台","旷野来信","宇宙邮局","口袋花园","海底漫游",
        "云端小岛","街角咖啡","复古游园","微光工坊","冬日慢邮"};
    return names[family%FORTUNE_FAMILY_COUNT];
}

static uint32_t blend(uint32_t a,uint32_t b,unsigned amount) {
    uint32_t result=0;
    for(unsigned shift=0;shift<24;shift+=8) {
        unsigned aa=(a>>shift)&255,bb=(b>>shift)&255;
        result|=((aa*(8-amount)+bb*amount)/8)<<shift;
    }
    return result;
}
static void roof(canvas_t c,int x,int y,int w,int height,uint32_t color) {
    for(int j=0;j<height;++j) box(c,x+height-j,y+j,w-2*(height-j),1,color);
}
static void cabin(canvas_t c,int x,int y,fortune_colors_t p) {
    box(c,x,y,25,17,0xDFC49B); roof(c,x-3,y-9,31,10,p.accent);
    box(c,x+4,y+4,6,6,0x456A69); box(c,x+16,y+6,5,11,0x765847);
    box(c,x+2,y+13,11,1,0xAC8C68); box(c,x+13,y-8,3,8,p.muted);
}
static void plant(canvas_t c,int x,int y,uint32_t flower,fortune_colors_t p) {
    box(c,x+2,y+16,10,3,p.accent); box(c,x+3,y+19,8,5,p.accent);
    line(c,x+7,y+16,x+7,y+3,0x456E5B);
    box(c,x+1,y+7,6,3,0x456E5B); box(c,x+7,y+10,6,3,0x456E5B);
    box(c,x+3,y+2,9,3,flower); box(c,x+6,y,3,7,flower); dot(c,x+7,y+3,p.paper);
}
static void moon(canvas_t c,int x,int y,uint32_t sky,fortune_colors_t p) {
    disc(c,x,y,6,p.accent); disc(c,x+3,y-2,6,sky);
}
static void stars(canvas_t c,uint32_t seed,uint32_t color,int y1,int height) {
    for(unsigned i=0;i<13;++i) {
        uint32_t h=mix(seed*113+i*97); int x=5+h%98,y=y1+(h>>8)%height;
        if(i<2) star(c,x,y,1,color); else dot(c,x,y,color);
    }
}
static void skyline(canvas_t c,uint32_t seed,int bottom,fortune_colors_t p) {
    for(unsigned i=0;i<10;++i) {
        uint32_t h=mix(seed+i*71); int x=(int)i*11,y=bottom-8-h%15;
        box(c,x,y,9,bottom-y,blend(p.paper,0x0D1E2C,4));
        for(unsigned j=0;j<4;++j) if(h&(1U<<(j+9))) box(c,x+2,y+3+j*4,2,2,p.accent);
    }
}
static void window(canvas_t c,int x,int y,int w,int h,uint32_t sky,fortune_colors_t p) {
    box(c,x,y,w,h,p.muted); box(c,x+2,y+2,w-4,h-4,sky);
    box(c,x+w/2,y+2,1,h-4,p.muted); box(c,x+2,y+h/2,w-4,1,p.muted);
}
static void mountain(canvas_t c,int peak,int height,uint32_t color) {
    for(int x=0;x<108;++x) { int y=height+abs(x-peak)/3; if(y<58) box(c,x,y,1,58-y,color); }
}
static void water(canvas_t c,int y,uint32_t seed,uint32_t color,fortune_colors_t p) {
    box(c,0,y,108,58-y,color);
    for(unsigned i=0;i<20;++i) {
        uint32_t h=mix(seed+i*177); box(c,3+h%98,y+1+(h>>8)%(57-y),3+h%5,1,blend(color,p.ink,2));
    }
}
static void tree(canvas_t c,int x,int y,int height,fortune_colors_t p) {
    box(c,x+7,y,3,height,0x765B46);
    for(int j=0;j<3;++j) { int yy=y-8+j*6; roof(c,x-j*3,yy,17+j*6,9,blend(0x3E6B5D,p.ink,1)); }
}
static void books(canvas_t c,int x,int y,int w,int h,fortune_colors_t p) {
    box(c,x,y,w,h,p.muted); box(c,x+2,y+2,w-4,h-4,p.paper);
    for(int yy=y+4;yy<y+h-4;yy+=11) {
        box(c,x+2,yy+8,w-4,1,p.muted);
        for(int xx=x+4;xx<x+w-4;xx+=5) box(c,xx,yy,3,6+(xx%3),((xx+yy)&1)?p.accent:p.ink);
    }
}

static void night_scene(canvas_t c,unsigned layout,unsigned tone,uint32_t seed,fortune_colors_t p) {
    static const uint32_t skies[]={0x293F59,0x43374F,0x285057,0x464050,0x2E5061,0x4C3949};
    uint32_t sky=skies[tone]; box(c,0,0,108,58,p.paper);
    switch(layout) {
    case 0: /* room, lamp and the night through a broad window */
        window(c,4,3,58,39,sky,p); moon(c,44,12,sky,p); skyline((canvas_t){c.p,c.x1<6?6:c.x1,c.x2>60?60:c.x2},seed,40,p);
        box(c,31,5,2,35,p.muted); books(c,66,4,34,20,p);
        box(c,0,51,108,7,0x705646); box(c,5,45,50,3,p.accent);
        box(c,11,48,3,10,p.muted); box(c,45,48,3,10,p.muted);
        box(c,17,28,12,3,p.accent); box(c,20,31,2,13,p.muted); box(c,15,43,13,2,p.muted);
        box(c,33,41,16,3,p.ink); box(c,40,41,1,3,p.muted); break;
    case 1: /* roof terrace and far-away towers */
        box(c,0,0,108,40,sky); stars(c,seed,p.muted,3,16); moon(c,74,10,sky,p); skyline(c,seed,44,p);
        box(c,0,44,108,14,p.paper); box(c,0,39,108,2,p.muted);
        for(int x=5;x<108;x+=12) box(c,x,39,1,11,p.muted);
        box(c,9,44,27,3,p.accent); box(c,12,47,2,8,p.muted); box(c,31,47,2,8,p.muted);
        plant(c,42,28,p.ink,p); break;
    case 2: /* illuminated bookshop */
        box(c,0,0,108,58,sky); box(c,6,4,58,49,p.paper);
        for(int y=7;y<49;y+=7) {box(c,7,y,56,1,p.muted);for(int x=9+(y%2)*4;x<60;x+=12)box(c,x,y,1,7,p.muted);}
        window(c,12,17,32,27,p.ink,p); books(c,16,21,24,19,p);
        box(c,48,16,11,33,p.accent); box(c,50,19,7,17,sky);
        box(c,9,12,52,5,p.accent); for(int x=11;x<60;x+=8) box(c,x,12,3,5,p.ink);
        box(c,18,6,26,4,p.muted); box(c,0,53,108,5,p.muted); box(c,43,49,20,4,p.muted); break;
    case 3: /* last train: windows, luggage rack and bench */
        box(c,0,0,108,58,blend(p.paper,p.muted,2));
        for(int x=5;x<97;x+=31) { window(c,x,10,27,25,sky,p); box(c,x+5,29,3,4,p.accent); box(c,x+12,24,6,9,p.muted); }
        box(c,2,4,104,2,p.ink); box(c,0,37,108,3,p.muted); box(c,0,51,108,7,p.paper);
        box(c,7,42,51,5,p.accent); box(c,12,47,3,9,p.muted); box(c,51,47,3,9,p.muted);
        box(c,13,6,15,3,p.accent); box(c,35,6,11,3,p.muted); break;
    case 4: /* harbor lamps, pier and a distant sailboat */
        box(c,0,0,108,58,sky); moon(c,59,11,sky,p); stars(c,seed,p.muted,4,15); water(c,28,seed,0x24475A,p);
        box(c,15,13,9,22,p.ink); roof(c,10,7,19,7,p.accent); box(c,16,10,7,5,p.accent);
        box(c,33,40,75,6,0x795E52); for(int x=37;x<106;x+=14)box(c,x,46,3,12,p.muted);
        box(c,49,22,2,20,p.muted); box(c,44,18,12,5,p.accent);
        line(c,86,21,86,32,p.muted); for(int y=0;y<9;++y)box(c,86-y,22+y,y+1,1,p.ink);
        box(c,77,32,20,2,p.accent); break;
    case 5: /* sheltered balcony */
        box(c,0,0,108,58,sky); moon(c,36,11,sky,p); skyline(c,seed,44,p);
        box(c,58,0,50,58,p.paper); window(c,65,4,35,23,p.ink,p); box(c,59,28,49,3,p.muted);
        box(c,0,47,108,11,blend(p.paper,p.muted,2)); box(c,0,40,61,2,p.ink);
        for(int x=3;x<60;x+=10)box(c,x,41,2,11,p.ink);
        plant(c,10,21,p.accent,p); plant(c,29,25,p.ink,p); break;
    case 6: /* library shelves and an arched night window */
        books(c,4,3,39,44,p); box(c,50,3,52,29,p.muted); box(c,52,5,48,25,sky);
        moon(c,77,13,sky,p); box(c,75,5,2,25,p.muted);
        box(c,0,51,108,7,0x75564B); box(c,9,44,49,3,p.accent); box(c,13,47,3,11,p.muted);box(c,50,47,3,11,p.muted);
        box(c,25,39,17,5,p.ink); box(c,31,34,2,9,p.accent); star(c,32,32,1,p.accent); break;
    default: /* wet lane and a small corner cafe */
        box(c,0,0,108,58,sky); box(c,0,2,49,40,p.paper); window(c,5,10,21,25,p.ink,p);
        box(c,29,11,12,28,p.accent); box(c,31,13,8,16,sky); box(c,0,7,47,5,p.accent);
        box(c,53,5,30,27,blend(sky,p.muted,2)); window(c,57,10,9,12,p.accent,p);
        box(c,0,42,108,16,p.paper); for(int y=44;y<58;y+=5)box(c,8+(y-44)*2,y,32,2,p.muted);
        box(c,49,40,53,1,p.accent); box(c,90,8,2,24,p.muted);box(c,86,7,11,4,p.accent); break;
    }
}

static void wild_scene(canvas_t c,unsigned layout,unsigned tone,uint32_t seed,fortune_colors_t p) {
    static const uint32_t skies[]={0xC3DAD1,0xD1D7AF,0xC9BBCE,0xDBC3A3,0xB2CDD0,0xDEBDC0};
    uint32_t sky=skies[tone]; box(c,0,0,108,58,sky); disc(c,85,12,6,0xF3DFAB);
    box(c,7,8,19,3,p.paper); box(c,13,6,8,2,p.paper);
    switch(layout) {
    case 0:
        mountain(c,35,17,0x789D91); mountain(c,88,26,0x456E6A); box(c,0,42,108,16,0x96A57D);
        for(int y=38;y<58;++y)box(c,52+(y-38)/3,y,6+(y-38)/4,1,0xCCE0CB);
        cabin(c,12,29,p); box(c,9,47,37,2,p.muted); break;
    case 1:
        box(c,0,19,32,39,0x647B68); box(c,38,24,70,34,0x82967A);
        box(c,21,21,10,37,0xD4E1D4); box(c,24,25,2,28,0x9BBDC0);
        water(c,48,seed,0x769A98,p); box(c,32,37,56,5,0xB6AB86);
        for(int x=34;x<85;x+=8)box(c,x,33,2,5,0x7B785D);
        box(c,47,41,5,14,0xB6AB86); box(c,76,41,5,14,0xB6AB86); break;
    case 2:
        water(c,26,seed,0x74A5A9,p); for(int y=36;y<58;++y)box(c,0,y,45+(y-36)*3,1,0xD9C7A0);
        box(c,15,17,12,24,p.paper); box(c,14,15,14,5,p.accent); roof(c,10,7,22,9,p.accent);
        box(c,17,17,8,3,0xF2DAAA); box(c,18,31,5,10,p.muted);
        line(c,46,29,46,36,p.muted); box(c,45,36,19,2,p.accent); break;
    case 3:
        for(int x=1;x<108;x+=18)tree(c,x,13+(x%7),43,p);
        box(c,0,47,108,11,0x91A17A); cabin(c,21,23,p); box(c,17,39,35,3,p.muted);
        line(c,33,41,33,56,p.muted); line(c,40,41,40,56,p.muted); for(int y=43;y<55;y+=4)box(c,33,y,8,1,p.muted); break;
    case 4:
        box(c,0,0,108,58,blend(sky,0xE7CAA2,3)); moon(c,89,13,blend(sky,0xE7CAA2,3),p);
        mountain(c,17,25,0xC7A783); mountain(c,95,32,0xB99273);
        box(c,0,44,108,14,0xDDC099); roof(c,9,26,47,18,p.accent); box(c,14,43,36,2,p.accent);
        roof(c,22,32,23,12,0x685A4C); line(c,8,44,4,49,p.muted); line(c,53,44,59,49,p.muted); break;
    case 5:
        mountain(c,31,13,0x809DA0); mountain(c,88,21,0x537A77); roof(c,16,12,31,11,p.paper);
        water(c,34,seed,0x80ABA5,p); box(c,0,53,108,5,0x859578);
        box(c,46,42,52,4,0xA78B66); box(c,49,46,2,10,p.muted); box(c,86,46,2,10,p.muted);
        for(int x=47;x<97;x+=7) { box(c,x,42,1,4,p.muted); }
        break;
    case 6:
        mountain(c,35,31,0x9CA77D); box(c,0,43,108,15,0xB4B681);
        box(c,19,24,14,24,p.paper); roof(c,15,16,22,9,p.accent);
        line(c,26,21,26,40,p.muted); line(c,17,30,36,30,p.muted);
        box(c,24,20,4,8,p.ink);box(c,17,28,8,4,p.ink);box(c,28,28,8,4,p.ink);box(c,24,33,4,8,p.ink);
        for(int x=2;x<108;x+=10) {line(c,x,49,x+3,44,p.muted);line(c,x,49,x-2,45,p.muted);} break;
    default:
        mountain(c,30,24,0x789D91); mountain(c,83,29,0x527C6A); box(c,0,42,108,16,0x91A778);
        for(unsigned i=0;i<35;++i) {uint32_t h=mix(seed+i*37);int x=3+h%61,y=40+(h>>8)%17;box(c,x,y,3,2,i&1?p.accent:p.paper);dot(c,x+1,y-1,p.muted);}
        for(int y=43;y<58;++y) { box(c,54-(y-43)/2,y,4+(y-43)/3,1,0xD8C9A3); }
        break;
    }
}

static void space_scene(canvas_t c,unsigned layout,unsigned tone,uint32_t seed,fortune_colors_t p) {
    static const uint32_t skies[]={0x2D395D,0x4E3655,0x285354,0x454363,0x355C5B,0x523D52};
    uint32_t sky=skies[tone]; box(c,0,0,108,58,sky); stars(c,seed,p.muted,4,37);
    switch(layout) {
    case 0:
        box(c,3,1,102,1,p.muted);box(c,3,55,102,1,p.muted);
        for(int x=5;x<106;x+=7){box(c,x,0,3,3,p.paper);box(c,x,54,3,3,p.paper);}
        disc(c,35,27,15,p.accent); disc(c,31,23,4,p.ink);box(c,40,29,5,2,p.muted);
        line(c,9,35,59,20,p.ink); line(c,10,38,61,23,p.muted);
        box(c,8,44,17,10,p.ink); line(c,8,44,16,50,sky);line(c,16,50,24,44,sky); break;
    case 1:
        box(c,0,0,108,58,p.paper);box(c,5,4,98,35,p.muted);box(c,8,7,92,29,sky);
        disc(c,30,22,9,p.accent); box(c,51,7,3,29,p.muted); box(c,0,40,108,18,p.paper);
        for(int x=7;x<57;x+=10){box(c,x,43,7,5,p.muted);box(c,x+1,44,4,2,p.accent);}
        box(c,6,52,53,2,p.muted); box(c,47,26,12,15,p.muted);box(c,44,38,18,4,p.accent);break;
    case 2:
        disc(c,29,12,8,0x9EC4AE); box(c,26,8,6,3,0x547F90); box(c,0,39,108,19,0x87959E);
        for(unsigned i=0;i<12;++i){uint32_t h=mix(seed+i*89);box(c,h%64,43+(h>>8)%13,6,2,0x687883);}
        disc(c,31,37,17,p.muted);box(c,13,37,36,8,p.ink);box(c,26,29,10,15,sky);box(c,26,37,10,1,p.accent);
        box(c,53,25,1,22,p.ink);box(c,54,25,10,6,p.accent);break;
    case 3:
        mountain(c,34,27,0x394958);box(c,0,50,108,8,0x495D64);disc(c,29,27,16,p.muted);
        box(c,11,27,36,23,p.paper);box(c,16,33,27,3,p.accent);box(c,24,42,8,8,sky);
        line(c,51,42,58,25,p.muted);box(c,50,22,17,6,p.ink);box(c,46,21,5,8,p.accent);
        line(c,56,39,47,48,p.muted);line(c,56,39,65,49,p.muted);break;
    case 4:
        box(c,0,46,108,12,0x5C6574);box(c,20,9,15,34,p.ink);roof(c,20,1,15,9,p.accent);
        box(c,24,18,7,8,sky);box(c,24,32,7,4,p.accent);roof(c,14,31,27,12,p.accent);
        box(c,7,13,3,35,p.muted);box(c,7,14,11,2,p.muted);box(c,7,26,11,2,p.muted);
        box(c,42,41,18,5,p.muted);box(c,44,43,6,1,p.accent);break;
    case 5:
        box(c,0,0,108,58,p.paper); window(c,4,3,62,34,sky,p);disc(c,35,20,10,p.accent);line(c,15,23,53,15,p.ink);
        box(c,71,6,27,13,p.muted);box(c,77,9,7,6,p.ink);box(c,85,10,4,4,p.ink);
        box(c,5,43,53,3,p.accent);box(c,11,46,3,12,p.muted);box(c,49,46,3,12,p.muted);
        box(c,15,35,11,8,p.ink);box(c,26,37,3,4,p.ink);box(c,17,34,7,1,p.accent);box(c,39,40,10,3,p.ink);break;
    case 6:
        disc(c,32,24,18,p.muted);disc(c,32,24,14,sky);box(c,28,5,8,38,p.ink);
        box(c,11,20,42,7,p.ink);box(c,15,22,7,3,p.accent);box(c,40,22,7,3,p.accent);
        box(c,3,44,102,6,p.muted);for(int x=8;x<62;x+=16)plant(c,x,22,p.accent,p);
        for(int x=5;x<106;x+=8) { box(c,x,47,3,1,p.ink); }
        break;
    default:
        mountain(c,25,34,0x6F788C);box(c,0,47,108,11,0x788591);
        for(int x=14;x<54;x+=12){roof(c,x,24-x%9,11,12,p.accent);box(c,x+3,34-x%9,5,13,p.accent);line(c,x+5,26-x%9,x+5,44,p.ink);}
        line(c,65,4,92,17,p.muted);line(c,72,4,94,16,p.ink);disc(c,96,18,3,p.accent);break;
    }
}

static void garden_scene(canvas_t c,unsigned layout,unsigned tone,uint32_t seed,fortune_colors_t p) {
    (void)seed;
    static const uint32_t walls[]={0xCECDB0,0xDDBBA3,0xBDC9CB,0xD0C4AC,0xBFCDAF,0xD2BDCB};
    uint32_t wall=walls[tone];box(c,0,0,108,58,p.paper);box(c,3,3,102,52,wall);
    switch(layout) {
    case 0:
        window(c,8,6,48,33,blend(wall,0x9FBAC1,4),p);box(c,6,39,53,3,p.muted);
        plant(c,12,18,p.accent,p);plant(c,35,17,p.ink,p);box(c,0,51,108,7,p.paper);break;
    case 1:
        roof(c,8,2,90,17,p.muted);roof(c,12,6,82,13,blend(wall,p.paper,4));box(c,10,19,84,31,blend(wall,p.paper,4));
        for(int x=10;x<96;x+=17) { box(c,x,17,2,35,p.muted); }
        box(c,10,32,84,2,p.muted);
        for(int x=15;x<60;x+=17) { plant(c,x,26,p.accent,p); }
        box(c,6,51,94,3,p.muted);break;
    case 2:
        box(c,21,6,23,4,p.muted);box(c,17,11,31,4,p.ink);box(c,11,15,42,30,p.ink);box(c,14,18,36,24,blend(wall,0x9FBAC1,2));
        box(c,15,33,34,9,0x789574);tree(c,21,27,13,p);plant(c,34,20,p.accent,p);box(c,19,43,28,3,p.muted);
        box(c,0,51,108,7,p.muted);box(c,55,43,8,8,p.accent);break;
    case 3:
        box(c,12,15,4,37,p.muted);box(c,49,15,4,37,p.muted);roof(c,9,3,48,15,p.muted);roof(c,14,9,38,9,wall);
        for(int y=10;y<48;y+=7){box(c,10,y,8,5,0x71815C);box(c,48,y+3,8,5,0x71815C);disc(c,13,y+1,2,p.accent);disc(c,51,y+4,2,p.accent);}
        for(int y=38;y<58;++y) { box(c,27-(y-38)/3,y,12+(y-38)/2,1,p.paper); }
        break;
    case 4:
        water(c,29,23,blend(wall,0x779D91,4),p);box(c,18,16,3,21,p.muted);box(c,47,16,3,21,p.muted);roof(c,10,3,48,15,p.accent);
        box(c,15,35,38,3,p.muted);box(c,23,24,4,9,p.paper);box(c,43,24,4,9,p.paper);
        for(int x=5;x<61;x+=13){box(c,x,43+x%9,9,2,0x628F78);disc(c,x+4,42+x%9,2,p.accent);}
        box(c,62,51,44,7,p.muted);break;
    case 5:
        box(c,10,8,4,34,p.muted);box(c,51,8,4,34,p.muted);box(c,7,8,51,8,p.accent);
        for(int x=9;x<57;x+=9)box(c,x,8,4,8,p.paper);
        box(c,8,35,51,14,p.muted);box(c,12,40,43,5,p.accent);disc(c,18,51,5,p.ink);disc(c,49,51,5,p.ink);
        for(int x=13;x<50;x+=12) { plant(c,x,16,p.accent,p); }
        break;
    case 6:
        for(int x=4;x<105;x+=25) { tree(c,x,15,41,p); }
        box(c,0,49,108,9,0xA3AF87);
        box(c,22,28,23,22,p.paper);box(c,16,21,35,9,p.accent);roof(c,12,8,43,15,p.accent);
        box(c,21,19,7,4,p.paper);box(c,37,18,6,4,p.paper);box(c,27,34,10,16,p.muted);window(c,25,34,14,16,p.muted,p);break;
    default:
        for(int y=5;y<38;y+=8){box(c,3,y,102,1,p.muted);for(int x=10+(y%3)*7;x<102;x+=21)box(c,x,y,1,8,p.muted);}
        for(int x=7;x<65;x+=17)plant(c,x,9,p.accent,p);
        disc(c,36,43,19,p.muted);disc(c,36,41,16,p.paper);box(c,34,22,4,20,p.muted);box(c,25,27,22,3,p.muted);
        box(c,33,21,6,5,p.accent);line(c,27,31,25,38,0x8AA6A0);line(c,46,31,47,38,0x8AA6A0);break;
    }
}

static void bubble(canvas_t c,int x,int y,int r,uint32_t water,fortune_colors_t p) {
    disc(c,x,y,r,p.muted);disc(c,x,y,r-1,water);dot(c,x-1,y-1,p.ink);
}
static void fish(canvas_t c,int x,int y,fortune_colors_t p) {
    box(c,x,y,8,3,p.accent);box(c,x+2,y-1,4,5,p.accent);
    box(c,x+8,y-2,2,7,p.muted);dot(c,x+1,y,p.ink);
}
static void sea_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    uint32_t sea=blend(p.paper,0x1B5361,3);box(c,0,0,108,58,sea);
    for(unsigned i=0;i<7;++i) {uint32_t h=mix(seed+i*83);bubble(c,5+h%96,5+(h>>8)%31,2,sea,p);}
    box(c,0,51,108,7,blend(p.muted,0xC8BA91,4));
    switch(layout) {
    case 0:
        box(c,18,17,28,29,p.muted);roof(c,12,8,40,10,p.accent);window(c,23,22,18,17,sea,p);
        box(c,12,43,40,4,p.accent);plant(c,3,26,p.accent,p);plant(c,49,26,p.ink,p);break;
    case 1:
        box(c,5,9,58,34,p.muted);window(c,9,13,50,24,sea,p);box(c,7,45,58,3,p.accent);
        for(int x=12;x<60;x+=11) {box(c,x,39,5,3,p.ink);box(c,x,48,2,5,p.muted);}
        fish(c,17,24,p);fish(c,41,18,p);break;
    case 2:
        disc(c,31,22,13,p.accent);box(c,17,22,28,8,p.accent);box(c,19,27,25,2,p.ink);
        for(int x=21;x<45;x+=6) {line(c,x,30,x-3,42,p.muted);line(c,x-3,42,x+1,46,p.muted);}
        bubble(c,57,32,4,sea,p);break;
    case 3:
        box(c,9,20,42,13,p.muted);disc(c,15,26,8,p.muted);box(c,49,16,6,9,p.muted);
        box(c,54,14,5,15,p.accent);box(c,9,32,44,3,p.ink);dot(c,12,22,p.paper);
        for(int x=5;x<62;x+=13) {plant(c,x,28,p.accent,p);}
        break;
    case 4:
        disc(c,32,28,23,p.muted);disc(c,32,28,19,p.ink);disc(c,32,28,16,sea);
        for(unsigned i=0;i<6;++i) {dot(c,12+i*7,9,p.accent);dot(c,12+i*7,46,p.accent);}
        fish(c,25,25,p);box(c,3,50,57,3,p.accent);break;
    case 5:
        disc(c,31,24,23,p.accent);box(c,7,22,49,22,sea);box(c,11,38,42,8,p.muted);
        for(int x=13;x<51;x+=8) {line(c,31,39,x,10,p.ink);}
        disc(c,31,38,7,p.ink);disc(c,29,36,2,p.paper);break;
    case 6:
        line(c,7,47,57,40,p.accent);line(c,7,44,57,37,p.muted);box(c,10,31,43,10,p.muted);
        books(c,18,12,29,27,p);line(c,52,3,52,38,p.ink);line(c,52,5,65,24,p.muted);
        plant(c,2,27,p.accent,p);break;
    default:
        disc(c,31,24,21,p.muted);disc(c,31,24,18,sea);box(c,7,43,53,3,p.accent);
        box(c,18,32,13,9,p.ink);box(c,31,34,4,5,p.ink);box(c,18,30,13,2,p.accent);
        box(c,44,34,8,7,p.accent);bubble(c,18,17,4,sea,p);bubble(c,43,12,3,sea,p);break;
    }
}
static void cloud(canvas_t c,int x,int y,int w,fortune_colors_t p) {
    uint32_t light=blend(p.paper,0xFFFFFF,5);
    box(c,x,y,w,5,light);box(c,x+3,y-4,w-6,8,light);box(c,x+8,y-7,w-16,9,light);
}
static void island(canvas_t c,int x,int y,int w,fortune_colors_t p) {
    box(c,x,y,w,4,p.muted);for(int j=0;j<10;++j) {box(c,x+j,y+4+j,w-j*2,1,p.accent);}
}
static void balloon(canvas_t c,int x,int y,fortune_colors_t p) {
    disc(c,x,y,9,p.accent);box(c,x-3,y-9,5,16,p.ink);line(c,x-6,y+6,x-3,y+14,p.muted);
    line(c,x+6,y+6,x+3,y+14,p.muted);box(c,x-4,y+14,9,4,p.muted);
}
static void cloud_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    uint32_t sky=blend(p.paper,0xA6BDC9,3);box(c,0,0,108,58,sky);
    cloud(c,70,12,28,p);cloud(c,5,10,23,p);box(c,65,54,43,4,p.ink);
    switch(layout) {
    case 0:
        island(c,9,40,50,p);box(c,24,18,16,22,p.paper);roof(c,21,10,22,9,p.accent);
        line(c,32,9,32,31,p.muted);line(c,20,20,45,20,p.muted);
        box(c,29,8,6,9,p.ink);box(c,18,17,11,5,p.ink);box(c,35,17,11,5,p.ink);break;
    case 1:
        island(c,9,42,51,p);cabin(c,19,25,p);plant(c,46,22,p.accent,p);cloud(c,2,53,21,p);break;
    case 2:
        island(c,2,40,24,p);island(c,50,38,15,p);
        for(int j=0;j<5;++j) {line(c,17,32+j,55,24+j,j&1?p.accent:p.muted);}
        line(c,18,32,18,23,p.muted);line(c,55,24,55,15,p.muted);break;
    case 3:
        island(c,7,47,51,p);balloon(c,24,14,p);balloon(c,49,22,p);
        box(c,10,42,47,3,p.muted);box(c,13,38,2,9,p.muted);break;
    case 4:
        island(c,6,44,57,p);window(c,12,14,43,26,sky,p);
        for(int x=13;x<56;x+=15) {plant(c,x,17,p.accent,p);}
        roof(c,8,4,52,12,p.muted);break;
    case 5:
        island(c,2,47,63,p);box(c,8,23,43,20,p.paper);roof(c,4,14,51,10,p.accent);
        window(c,14,27,30,11,sky,p);box(c,52,22,2,22,p.muted);box(c,50,20,11,5,p.accent);break;
    case 6:
        island(c,3,43,59,p);line(c,26,41,44,16,p.muted);roof(c,33,8,23,12,p.accent);
        line(c,37,22,43,32,p.ink);line(c,43,32,49,30,p.ink);box(c,15,38,17,3,p.muted);break;
    default:
        cloud(c,4,43,58,p);box(c,15,24,43,18,p.paper);box(c,11,24,4,21,p.muted);
        box(c,54,23,4,22,p.muted);box(c,17,31,36,10,p.accent);box(c,18,25,11,5,p.ink);
        stars(c,seed,p.muted,4,18);break;
    }
}
static void cup(canvas_t c,int x,int y,fortune_colors_t p) {
    box(c,x,y,13,10,p.ink);box(c,x+13,y+2,5,6,p.muted);box(c,x+14,y+3,2,3,p.paper);
    box(c,x-2,y+10,21,2,p.muted);line(c,x+4,y-2,x+2,y-6,p.muted);
}
static void cafe_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    (void)seed;uint32_t wall=blend(p.paper,0xCBAA89,2);box(c,0,0,108,58,wall);
    box(c,0,51,108,7,p.muted);box(c,65,44,39,3,p.accent);
    switch(layout) {
    case 0:
        window(c,6,4,48,28,0x9BAA97,p);box(c,6,42,56,4,p.accent);cup(c,16,30,p);
        box(c,38,31,11,10,p.muted);roof(c,36,27,15,6,p.ink);box(c,41,46,3,11,p.muted);break;
    case 1:
        box(c,7,12,48,31,p.muted);window(c,15,22,30,16,p.ink,p);box(c,7,17,48,2,p.accent);
        roof(c,5,3,52,10,p.accent);box(c,7,43,51,4,p.accent);
        for(int x=15;x<53;x+=13) {disc(c,x,13,4,p.ink);}
        break;
    case 2:
        window(c,7,3,51,31,0xA3B7B0,p);box(c,7,44,51,4,p.muted);cup(c,13,32,p);
        box(c,38,33,14,8,p.accent);roof(c,37,28,16,6,p.ink);box(c,40,40,10,2,p.ink);break;
    case 3:
        window(c,6,6,34,30,0x8DAD9D,p);plant(c,44,11,p.accent,p);plant(c,16,20,p.ink,p);
        box(c,5,45,55,3,p.accent);cup(c,39,33,p);break;
    case 4:
        books(c,5,3,28,37,p);box(c,9,45,51,3,p.muted);box(c,33,19,28,21,p.accent);
        disc(c,47,29,9,p.ink);disc(c,47,29,3,p.muted);line(c,53,23,57,32,p.muted);break;
    case 5:
        box(c,11,18,42,30,p.muted);roof(c,5,5,55,15,p.accent);books(c,15,23,34,23,p);
        box(c,48,37,13,9,p.ink);box(c,50,39,9,1,p.muted);box(c,50,42,7,1,p.muted);break;
    case 6:
        box(c,0,0,108,40,0xB6C6C0);skyline(c,91,40,p);box(c,1,39,60,2,p.muted);
        for(int x=6;x<60;x+=12) {box(c,x,40,1,12,p.muted);}
        box(c,9,42,45,3,p.accent);cup(c,15,30,p);plant(c,49,24,p.accent,p);break;
    default:
        box(c,8,5,45,41,p.paper);window(c,14,16,27,26,p.muted,p);box(c,6,10,51,6,p.accent);
        for(int x=8;x<55;x+=10) {box(c,x,10,5,6,p.ink);}
        box(c,14,28,27,2,p.accent);roof(c,47,25,14,10,p.accent);box(c,49,34,10,14,p.muted);break;
    }
}
static void fair_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    box(c,0,0,108,58,p.paper);stars(c,seed,p.muted,3,27);box(c,0,51,108,7,blend(p.paper,p.muted,2));
    switch(layout) {
    case 0:
        roof(c,5,8,58,15,p.accent);box(c,8,24,53,3,p.ink);box(c,11,46,48,4,p.muted);
        for(int x=17;x<53;x+=15) {line(c,x,25,x,47,p.ink);box(c,x-5,33,11,5,p.accent);box(c,x-5,37,2,5,p.muted);}
        break;
    case 1:
        disc(c,32,24,22,p.muted);disc(c,32,24,20,p.paper);line(c,14,49,32,24,p.ink);line(c,32,24,50,49,p.ink);
        line(c,10,24,54,24,p.muted);line(c,32,2,32,45,p.muted);line(c,17,8,47,39,p.muted);line(c,17,39,47,8,p.muted);
        for(int x=11;x<55;x+=20) {box(c,x,18,9,7,p.accent);box(c,x,35,9,7,p.ink);}
        break;
    case 2:
        box(c,12,7,35,42,p.muted);box(c,9,6,41,7,p.accent);window(c,17,15,25,20,0x384B57,p);
        box(c,14,35,31,6,p.accent);box(c,20,38,2,2,p.paper);box(c,31,38,4,2,p.ink);box(c,18,45,23,2,p.ink);break;
    case 3:
        box(c,15,19,37,29,p.muted);roof(c,7,5,54,15,p.accent);window(c,21,24,23,17,p.paper,p);
        box(c,22,32,18,3,p.accent);box(c,46,24,7,7,p.ink);break;
    case 4:
        box(c,11,32,44,15,p.muted);disc(c,19,49,5,p.ink);disc(c,47,49,5,p.ink);
        box(c,8,21,51,6,p.accent);line(c,12,21,12,34,p.ink);line(c,54,21,54,34,p.ink);
        for(int x=19;x<51;x+=12) {line(c,x,32,x,16,p.muted);disc(c,x,13,5,p.ink);}
        break;
    case 5:
        box(c,10,25,47,20,p.accent);window(c,15,29,14,10,p.paper,p);window(c,35,29,16,10,p.paper,p);
        box(c,7,21,53,4,p.muted);disc(c,19,47,5,p.ink);disc(c,47,47,5,p.ink);box(c,54,14,5,8,p.ink);break;
    case 6:
        box(c,8,11,51,38,p.muted);box(c,13,17,41,28,p.paper);box(c,13,17,10,28,p.accent);box(c,44,17,10,28,p.accent);
        disc(c,33,28,4,p.ink);box(c,30,32,6,8,p.ink);line(c,28,18,30,32,p.muted);line(c,38,18,36,32,p.muted);break;
    default:
        box(c,7,38,48,10,p.muted);disc(c,15,50,4,p.ink);disc(c,47,50,4,p.ink);
        for(int x=16;x<53;x+=15) {disc(c,x,13+x%9,7,x&1?p.ink:p.accent);line(c,x,20+x%9,30,38,p.muted);}
        break;
    }
}
static void gear(canvas_t c,int x,int y,int r,fortune_colors_t p) {
    disc(c,x,y,r,p.muted);box(c,x-r-2,y-2,2*r+5,5,p.muted);box(c,x-2,y-r-2,5,2*r+5,p.muted);
    disc(c,x,y,r-3,p.paper);disc(c,x,y,2,p.accent);
}
static void work_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    (void)seed;box(c,0,0,108,58,p.paper);box(c,4,4,100,45,blend(p.paper,p.muted,1));
    box(c,5,46,58,4,p.accent);box(c,0,54,108,4,p.muted);
    switch(layout) {
    case 0:
        gear(c,22,20,12,p);gear(c,45,32,9,p);box(c,9,37,18,7,p.muted);
        box(c,53,6,7,25,p.ink);box(c,55,9,3,4,p.accent);break;
    case 1:
        disc(c,30,23,18,p.muted);disc(c,30,23,15,p.ink);line(c,30,23,30,12,p.paper);line(c,30,23,40,27,p.paper);
        box(c,12,40,38,4,p.muted);box(c,52,16,5,25,p.accent);break;
    case 2:
        box(c,21,35,27,6,p.muted);box(c,31,41,5,8,p.ink);disc(c,34,30,9,p.accent);box(c,29,17,10,13,p.accent);
        box(c,27,16,14,3,p.muted);box(c,8,16,12,4,p.muted);box(c,11,19,6,18,p.accent);break;
    case 3:
        box(c,9,8,3,35,p.muted);box(c,54,8,3,35,p.muted);line(c,10,12,56,12,p.ink);
        for(int x=15;x<53;x+=13) {box(c,x,13,9,24,x&1?p.accent:p.muted);box(c,x+2,18,5,2,p.ink);}
        break;
    case 4:
        books(c,8,4,48,12,p);for(int x=17;x<57;x+=16) {line(c,x,17,x,23,p.ink);disc(c,x,30,7,p.muted);disc(c,x,30,4,p.accent);}
        box(c,12,39,42,4,p.muted);break;
    case 5:
        window(c,8,5,46,33,0x324F58,p);stars((canvas_t){c.p,c.x1<11?11:c.x1,c.x2>51?51:c.x2},103,p.ink,9,24);
        box(c,12,39,38,5,p.muted);line(c,19,42,43,24,p.accent);box(c,44,23,8,4,p.accent);break;
    case 6:
        box(c,9,7,49,32,p.muted);for(int y=11;y<37;y+=12) {for(int x=13;x<56;x+=13) {box(c,x,y,9,9,p.ink);star(c,x+4,y+4,2,p.accent);}}
        box(c,23,39,14,6,p.ink);box(c,28,33,4,8,p.accent);break;
    default:
        box(c,12,10,43,3,p.muted);for(int x=17;x<55;x+=14) {line(c,x,13,x,20,p.ink);roof(c,x-5,20,11,10,p.accent);}
        box(c,12,36,17,8,p.ink);box(c,34,33,21,11,p.muted);line(c,34,33,45,40,p.paper);line(c,45,40,54,33,p.paper);break;
    }
}
static void snow_tree(canvas_t c,int x,int y,fortune_colors_t p) {
    tree(c,x,y,28,p);roof(c,x-3,y-8,23,10,p.paper);roof(c,x-6,y+2,29,10,p.paper);
}
static void winter_scene(canvas_t c,unsigned layout,uint32_t seed,fortune_colors_t p) {
    uint32_t sky=blend(p.paper,0x9AAEBC,3);box(c,0,0,108,58,sky);box(c,0,47,108,11,p.paper);
    for(unsigned i=0;i<16;++i) {uint32_t h=mix(seed+i*127);dot(c,2+h%104,4+(h>>8)%38,p.paper);}
    switch(layout) {
    case 0:
        box(c,13,18,37,29,p.muted);roof(c,5,4,54,16,p.paper);window(c,20,24,18,19,p.accent,p);
        box(c,46,33,14,15,p.accent);box(c,49,37,8,2,p.ink);box(c,47,48,12,2,p.paper);break;
    case 1:
        box(c,5,0,58,51,p.paper);box(c,17,11,33,30,p.muted);box(c,21,20,25,23,p.ink);box(c,24,24,19,19,0x68504B);
        roof(c,26,27,15,16,p.accent);roof(c,30,30,7,13,p.paper);box(c,11,10,45,4,p.accent);books(c,5,3,9,35,p);break;
    case 2:
        mountain(c,22,20,p.muted);water(c,36,101,blend(sky,p.ink,3),p);cabin(c,14,20,p);
        roof(c,11,11,31,10,p.paper);box(c,45,43,16,2,p.paper);box(c,52,45,2,8,p.muted);break;
    case 3:
        snow_tree(c,10,20,p);disc(c,46,38,12,p.paper);disc(c,46,23,8,p.paper);box(c,38,15,16,4,p.ink);
        dot(c,43,22,p.ink);dot(c,48,22,p.ink);line(c,51,25,55,25,p.accent);box(c,38,29,17,3,p.accent);break;
    case 4:
        box(c,4,18,59,28,p.muted);window(c,10,24,19,16,p.accent,p);window(c,34,24,22,16,p.accent,p);
        box(c,2,16,64,4,p.paper);box(c,0,45,65,3,p.ink);for(int x=7;x<62;x+=12) {box(c,x,48,7,2,p.muted);}
        break;
    case 5:
        snow_tree(c,3,18,p);snow_tree(c,44,16,p);roof(c,18,25,30,23,p.accent);roof(c,24,33,17,15,p.ink);
        box(c,27,45,14,3,p.muted);star(c,34,12,2,p.paper);break;
    case 6:
        for(int x=3;x<65;x+=21) {box(c,x,28-x%9,17,22,p.muted);roof(c,x-3,16-x%9,23,13,p.paper);window(c,x+5,31-x%9,7,9,p.accent,p);}
        line(c,54,8,54,42,p.ink);box(c,50,8,8,5,p.accent);break;
    default:
        box(c,13,19,42,30,p.muted);roof(c,6,4,56,17,p.paper);books(c,19,23,29,23,p);
        box(c,8,47,53,3,p.accent);snow_tree(c,0,28,p);break;
    }
}

static void subject(canvas_t c,unsigned kind,uint32_t frame,fortune_colors_t p) {
    int x=70,y=34;uint32_t cream=0xE8CBA3,outline=0x4B4B50;
    box(c,65,54,36,1,blend(p.muted,p.paper,3));
    switch(kind) {
    case 0:
        /* A one-pixel silhouette keeps cream fur readable on pale gardens. */
        cat(c,x-1,y,outline,outline,0,frame);cat(c,x+1,y,outline,outline,0,frame);
        cat(c,x,y-1,outline,outline,0,frame);cat(c,x,y+1,outline,outline,0,frame);
        cat(c,x,y,cream,outline,0,frame);box(c,89,44,12,6,p.ink);box(c,93,44,1,6,p.muted);
        box(c,93,31,2,11,p.muted);box(c,88,30,12,3,p.accent);box(c,90,27,8,3,p.accent);break;
    case 1: /* coat, satchel and a little lantern */
        box(c,x+3,y-4,9,3,outline);box(c,x+4,y-1,7,6,cream);box(c,x+4,y+4,10,11,p.accent);
        box(c,x+1,y+5,3,8,p.muted);box(c,x+14,y+5,2,9,p.accent);box(c,x+5,y+15,3,5,outline);box(c,x+11,y+15,3,5,outline);
        dot(c,x+10,y+1,outline);box(c,91,39,7,12,p.muted);box(c,92,40,5,7,p.accent);box(c,94,36,2,3,p.muted);break;
    case 2:
        box(c,x+1,y-3,15,11,outline);box(c,x+3,y-1,11,7,0xA6C4BB);box(c,x+5,y+2,2,frame%32==27?1:2,outline);box(c,x+10,y+2,2,frame%32==27?1:2,outline);
        box(c,x+7,y-6,2,3,outline);dot(c,x+7,y-7,p.accent);box(c,x+4,y+8,10,8,0x7AA69E);
        box(c,x+1,y+9,3,5,outline);box(c,x+14,y+9,3,5,outline);box(c,x+4,y+16,3,4,outline);box(c,x+11,y+16,3,4,outline);
        box(c,91,42,10,8,p.accent);box(c,92,43,8,4,p.paper);box(c,94,46,2,2,p.ink);line(c,98,41,101,35,p.muted);break;
    default:
        box(c,x+2,y-9,5,12,outline);box(c,x+10,y-10,5,13,outline);
        box(c,x+1,y-1,16,12,outline);box(c,x+4,y+9,11,9,outline);box(c,x+2,y+16,16,5,outline);
        box(c,x+3,y-8,3,10,cream);box(c,x+11,y-9,3,11,cream);box(c,x+4,y-7,1,6,p.accent);box(c,x+12,y-8,1,6,p.accent);
        box(c,x+2,y,14,10,cream);box(c,x+5,y+10,9,7,cream);box(c,x+3,y+17,14,3,cream);
        box(c,x+5,y+4,2,frame%32==27?1:2,outline);box(c,x+11,y+4,2,frame%32==27?1:2,outline);dot(c,x+9,y+7,p.accent);
        plant(c,89,29,p.accent,p);break;
    }
}

static void expanded_scene(canvas_t c,uint32_t id,uint32_t frame) {
    uint32_t offset=id-FORTUNE_LEGACY_ART_COUNT,skin=offset%FORTUNE_SKIN_COUNT;
    unsigned family=skin%FORTUNE_FAMILY_COUNT,layout=(skin/FORTUNE_FAMILY_COUNT)%8,tone=(offset/FORTUNE_SKIN_COUNT)%6;
    fortune_colors_t p=fortune_pixel_colors(id);uint32_t seed=mix(skin+991U);
    switch(family){case 0:night_scene(c,layout,tone,seed,p);break;case 1:wild_scene(c,layout,tone,seed,p);break;
        case 2:space_scene(c,layout,tone,seed,p);break;case 3:garden_scene(c,layout,tone,seed,p);break;
        case 4:sea_scene(c,layout,seed,p);break;case 5:cloud_scene(c,layout,seed,p);break;
        case 6:cafe_scene(c,layout,seed,p);break;case 7:fair_scene(c,layout,seed,p);break;
        case 8:work_scene(c,layout,seed,p);break;default:winter_scene(c,layout,seed,p);break;}
    subject(c,skin/FORTUNE_SCENE_COUNT,frame,p);
    /* Slow, sparse sparkle/rain/steam. The scene and signature stay still. */
    if(frame%16>7) {
        if(family==0) for(unsigned i=0;i<3;++i){uint32_t h=mix(seed+i*83);box(c,4+h%61,5+(h>>8)%13,1,2,p.muted);}
        else if(family==1) {uint32_t h=mix(seed);star(c,4+h%55,39+(h>>8)%10,1,p.accent);}
        else if(family==2) {star(c,59,9,1,p.ink);dot(c,7,15,p.ink);}
        else {dot(c,96,26,p.muted);dot(c,94,24,p.muted);}
    }
}

static void rare_ellipse(canvas_t c,int x,int y,int rx,int ry,uint32_t color) {
    for(int j=-ry;j<=ry;++j) for(int i=-rx;i<=rx;++i)
        if(i*i*ry*ry+j*j*rx*rx<=rx*rx*ry*ry) dot(c,x+i,y+j,color);
}

/* Additional originals: each composition has its own subject and visual story. */
static void rare_extended(canvas_t c,unsigned kind,uint32_t frame,fortune_colors_t p,uint32_t horizon) {
    uint32_t gold=p.accent,light=p.ink;
    switch(kind) {
    case 6: { /* Moon caught in a stag's branched constellation antlers. */
        disc(c,64,15,11,blend(horizon,gold,3)); disc(c,64,15,7,light);
        for(int i=0;i<6;++i) {
            int x=7+i*18,y=14+(i%3)*5;
            box(c,x,y,2,43-y,0x274455);
            for(int j=0;j<3;++j) roof(c,x-6,y+j*6,14,8,0x305567);
        }
        box(c,3,47,102,9,0x182D40);
        rare_ellipse(c,45,37,14,6,0xD5A875); box(c,55,25,6,13,0xEBC18B);
        rare_ellipse(c,61,25,6,3,light); line(c,66,25,70,23,light); dot(c,63,24,p.paper);
        for(int i=0;i<4;++i) line(c,34+i*7,40,33+i*7,52,gold);
        line(c,31,36,27,31,gold);
        line(c,59,23,53,11,gold); line(c,61,23,70,10,gold);
        line(c,55,16,46,12,gold); line(c,55,16,56,7,gold);
        line(c,67,15,78,13,gold); line(c,67,15,67,6,gold);
        star(c,46,12,2,light); star(c,56,7,1,light); star(c,78,13,2,light);
        for(int i=0;i<8;++i) star(c,16+i*11,49+(i*3)%5,((frame/4+i)%9==0)?1:0,gold);
        break;
    }
    case 7: { /* A lighthouse sends a beam over an ocean of clouds. */
        for(int x=3;x<105;++x) {
            int y=24+abs(x-72)/6;
            if((frame/8)%2) box(c,x,19-(x-53)/8,1,4,blend(horizon,light,3));
            else box(c,x,19+(x-53)/8,1,4,blend(horizon,light,3));
            box(c,x,y+22,1,12,blend(horizon,0xBDCCDF,4));
        }
        for(int i=0;i<8;++i) rare_ellipse(c,5+i*15,44+i%3,15,5,0x9AAFC8);
        rare_ellipse(c,53,45,19,5,0x31485F);
        for(int y=21;y<45;++y) box(c,49-(y-21)/10,y,8+(y-21)/5,1,y%8<4?light:0xCC9290);
        box(c,45,18,16,3,gold); box(c,47,11,12,7,0x72969F);
        box(c,49,12,8,5,light); roof(c,44,5,18,7,gold); box(c,46,27,14,2,gold);
        line(c,29,47,40,44,gold); line(c,68,46,78,49,gold);
        star(c,53,15,2,light); star(c,83,14,2,gold);
        break;
    }
    case 8: { /* Three firework blooms mirrored in the river. */
        static const int rays[8][2]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
        for(int i=0;i<3;++i) {
            int x=23+i*30,y=18+(i==1?6:0),r=8+(frame/3+i)%5;
            uint32_t color=i==0?0xEDABCB:i==1?gold:0x91DDE4;
            for(int j=0;j<8;++j) {
                int dx=rays[j][0],dy=rays[j][1];
                line(c,x+dx*3,y+dy*3,x+dx*r,y+dy*r,color);
                dot(c,x+dx*(r+2),y+dy*(r+2),light);
            }
            star(c,x,y,1,light); line(c,x,37,x,y+8,blend(horizon,color,3));
        }
        box(c,3,41,102,15,0x172D47);
        for(int i=0;i<16;++i) box(c,8+(i*23)%92,43+i%11,5+i%7,1,i%3?0xA691AF:gold);
        box(c,3,38,102,3,0x352943); box(c,5,33,3,5,gold); box(c,100,33,3,5,gold);
        break;
    }
    case 9: { /* Translucent jellyfish with long, dancing ribbons. */
        for(int i=0;i<3;++i) {
            int x=24+i*30,y=18+(i%2)*7,r=10-i%2;
            uint32_t color=i==0?0xCCA5E1:i==1?0x92DBD3:0xF0BFBA;
            for(int yy=-r;yy<=2;++yy) for(int xx=-r;xx<=r;++xx)
                if(xx*xx+yy*yy<=r*r) dot(c,x+xx,y+yy,blend(horizon,color,yy< -5?5:7));
            rare_ellipse(c,x,y, r,2,light);
            for(int j=0;j<5;++j) {
                int px=x-7+j*3;
                for(int yy=3;yy<24-i*3;++yy)
                    dot(c,px+((yy/4+(int)frame/4+j)%4<2?1:-1),y+yy,color);
            }
            star(c,x-3,y-5,1,light);
        }
        for(int i=0;i<10;++i) {
            int x=9+i*10,y=51-(frame/2+i*7)%44;
            disc(c,x,y,1,0x6E9BBD); dot(c,x,y-1,light);
        }
        break;
    }
    case 10: { /* Two cups on a terrace, the moon sitting opposite. */
        for(int x=8;x<101;++x) { int y=6+abs(x-54)/4; dot(c,x,y,gold); }
        box(c,8,18,2,37,gold); box(c,99,17,2,38,gold);
        disc(c,74,18,10,blend(horizon,gold,3)); disc(c,74,18,7,light); disc(c,77,16,7,blend(p.paper,horizon,2));
        rare_ellipse(c,51,43,34,6,0x98738C); rare_ellipse(c,51,41,34,5,gold);
        line(c,29,46,25,55,gold); line(c,73,46,78,55,gold);
        for(int i=0;i<2;++i) {
            int x=32+i*29; rare_ellipse(c,x+5,40,9,2,light);
            box(c,x,33,11,7,light); box(c,x+11,34,3,4,gold);
            box(c,x+1,33,9,2,0x936F72);
            line(c,x+4,29,x+5+(frame/4)%2,25,gold); dot(c,x+5,22-(frame/5)%2,light);
        }
        star(c,35,15,2,gold); star(c,23,25,1,light);
        break;
    }
    case 11: { /* A glowing open book spills a constellation into the room. */
        for(int i=0;i<3;++i) { box(c,6+i*7,14-i*3,5,28+i*3,0x59627F); box(c,7+i*7,18-i*3,3,1,gold); }
        box(c,5,43,98,3,0xB19498);
        for(int x=29;x<=90;++x) {
            int y=31+abs(x-59)/5; box(c,x,y,1,14,light);
            if(x%4==0) box(c,x,y+5,1,5,0xAE9BB2);
            dot(c,x,y+14,gold);
        }
        line(c,59,32,59,49,gold);
        line(c,53,29,47,20,gold); line(c,47,20,64,13,gold); line(c,64,13,80,19,gold);
        star(c,47,20,2,light); star(c,64,13,3,gold); star(c,80,19,1,light);
        for(int i=0;i<8;++i) dot(c,48+(i*11)%31,29-(i*7+(int)frame/2)%23,light);
        break;
    }
    case 12: { /* A galleon sails between clouds, its gilded sails open. */
        for(int i=0;i<5;++i) rare_ellipse(c,6+i*24,47+(i%2)*4,20,5,0x798FAB);
        for(int y=40;y<47;++y) box(c,25+(y-40),y,61-(y-40)*2,1,y<43?gold:0xBE8991);
        line(c,53,8,53,41,gold); line(c,76,17,76,42,gold);
        for(int y=11;y<37;++y) {
            box(c,54,y,27-(y-11)/2,1,light);
            box(c,51-(y-11)*3/4,y,(y-11)*3/4,1,0xD4B4D3);
        }
        line(c,30,40,53,11,gold); line(c,77,19,85,40,gold);
        for(int i=0;i<4;++i) box(c,37+i*11,42,3,2,p.paper);
        box(c,54,8,10,3,0xD89CB7); dot(c,64,8+(frame/4)%2,light);
        star(c,16,19,2,light); star(c,94,12,2,gold);
        break;
    }
    case 13: { /* Three snowy islands with windows glowing over dark water. */
        box(c,3,37,102,19,0x183448);
        for(int i=0;i<3;++i) {
            int x=19+i*32,y=35+(i==1?10:0);
            rare_ellipse(c,x,y+4,15,4,0x628698); rare_ellipse(c,x,y+2,15,3,0xD5E5E1);
            box(c,x-7,y-10,14,12,0x8B7380); roof(c,x-10,y-19,20,10,light);
            box(c,x-3,y-6,5,6,gold); box(c,x+4,y-5,3,7,0x443B52);
            box(c,x+5,y-18,2,5,gold); dot(c,x+5+(frame/6)%2,y-21,light);
            box(c,x-4,y+10,9,1,gold); box(c,x-2,y+13,5,1,0x8EA8A5);
        }
        for(int i=0;i<18;++i) dot(c,5+(i*37)%98,4+(i*13+(int)frame/3)%30,light);
        break;
    }
    case 14: { /* Fireflies illuminate an arch of old trees and a winding path. */
        for(int i=0;i<4;++i) {
            int x=9+i*29; box(c,x,8,4,47,0x426967);
            line(c,x+2,29,x-7,16,0x68877E); line(c,x+2,22,x+10,11,0x68877E);
            rare_ellipse(c,x,12,13,7,0x2B565C); rare_ellipse(c,x+8,8,10,5,0x3F6F6B);
        }
        for(int y=38;y<56;++y) { int x=52+(y-38)*(y-38)/40; box(c,x,y,2+(y-38)/3,1,0xBDCAAF); }
        for(int i=0;i<22;++i) {
            int x=8+(i*31)%92,y=18+(i*17)%35;
            uint32_t glow=(frame/3+i)%7<3?light:gold;
            if((frame/3+i)%7<3) disc(c,x,y,2,blend(horizon,gold,2));
            star(c,x,y,1,glow);
        }
        break;
    }
    case 15: { /* Ornamented hourglass with a falling thread of gold. */
        rare_ellipse(c,54,49,27,5,0x6E6684);
        box(c,31,7,46,3,gold); box(c,31,48,46,3,gold);
        for(int x=34;x<=74;x+=40) { box(c,x,10,2,38,gold); disc(c,x+1,6,2,light); disc(c,x+1,52,2,light); }
        line(c,40,11,67,11,light); line(c,40,11,54,29,0xBAABC9); line(c,67,11,54,29,0xBAABC9);
        line(c,54,29,40,46,0xBAABC9); line(c,54,29,67,46,0xBAABC9); line(c,40,46,67,46,light);
        for(int y=18;y<27;++y) box(c,46+(y-18)/2,y,17-(y-18),1,gold);
        for(int y=40;y<46;++y) box(c,53-(y-40)*2,y,3+(y-40)*4,1,gold);
        for(int y=29;y<40;++y) if((y+(int)frame/2)%3==0) dot(c,54,y,light);
        star(c,18,23,2,light); star(c,89,35,2,gold);
        break;
    }
    case 16: { /* A long-finned goldfish swims out of an ornate picture frame. */
        box(c,18,9,63,42,gold); box(c,20,11,59,38,light); box(c,22,13,55,34,0x354E69);
        rare_ellipse(c,59,29,17,8,0xE6B17D); rare_ellipse(c,58,28,14,5,0xF7D1A1);
        for(int i=0;i<17;++i) {
            int spread=i/2;
            box(c,43-i,29-spread+(int)(frame/6)%2,1,spread*2+1,i%3?0xD897A5:light);
        }
        for(int i=0;i<13;++i) { dot(c,55+i,20-i/3,gold); dot(c,57+i,36+i/3,0xE8BEA5); }
        dot(c,70,27,p.paper); dot(c,70,26,light);
        for(int i=0;i<5;++i) disc(c,80+i*4,22-i*4-(frame/7)%2,1,light);
        star(c,20,11,2,light); star(c,79,49,2,light); star(c,94,41,2,gold);
        break;
    }
    case 17: { /* Wind chimes in a lantern-lit corridor with deep perspective. */
        for(int i=0;i<4;++i) {
            int left=8+i*11,right=100-i*11,top=5+i*6,bottom=55-i*4;
            box(c,left,top,right-left,2,i%2?0xAB819A:gold);
            line(c,left,top,left,bottom,gold); line(c,right,top,right,bottom,gold);
        }
        line(c,9,55,52,35,0xAF97A3); line(c,98,55,57,35,0xAF97A3);
        for(int i=0;i<3;++i) {
            int x=25+i*28,top=9+i%2*5,shift=(int)((frame/5+i)%3)-1;
            line(c,x,top,x+shift,top+7,light);
            disc(c,x+shift,top+11,4,i==1?0x9ACDCD:0xECC1AA);
            box(c,x+shift-5,top+12,11,2,gold);
            line(c,x+shift,top+14,x+shift,top+21,light);
            box(c,x+shift-1,top+21,3,7,light);
        }
        break;
    }
    case 18: { /* A comet knocks on a sleepy town's bright attic window. */
        for(int i=0;i<5;++i) {
            int x=4+i*21,y=40-(i%2)*5;
            box(c,x,y,20,55-y,0x5F506A); roof(c,x-2,y-8,24,9,0x9D7D95);
            box(c,x+4,y+3,4,5,gold); box(c,x+12,y+3,4,5,0xCCAF99);
        }
        int x=74-(frame/3)%12,y=13+(frame/3)%12/3;
        for(int i=0;i<18;++i) dot(c,x+i,y-i/3,blend(horizon,gold,8-(unsigned)i/3));
        star(c,x,y,3,light); line(c,57,29,57,35,gold); box(c,54,28,7,7,light);
        star(c,21,17,1,gold); star(c,92,25,1,light);
        break;
    }
    case 19: { /* Open palace doors reveal a sunrise and a path of light. */
        for(int y=8;y<53;++y) box(c,29,y,51,1,blend(0xE4B2B5,0xFFE3A6,(unsigned)(y-8)/6));
        disc(c,54,33,12,light);
        for(int y=40;y<55;++y) box(c,48-(y-40)/2,y,13+y-40,1,0xF5D89C);
        for(int x=21;x<88;++x) { int y=6+abs(x-54)/4; dot(c,x,y,gold); dot(c,x,y+2,light); }
        box(c,23,15,4,39,gold); box(c,82,15,4,39,gold);
        for(int x=29;x<43;++x) box(c,x,13+(x-29)/3,1,40-(x-29)/2,0x655278);
        for(int x=68;x<81;++x) box(c,x,17-(x-68)/3,1,34+(x-68)/2,0x655278);
        line(c,42,18,42,48,gold); line(c,68,18,68,48,gold);
        box(c,37,30,2,5,light); box(c,72,30,2,5,light);
        star(c,14,22,2,gold); star(c,95,38,2,light);
        for(int i=0;i<7;++i) dot(c,46+i*3,40+(i*3+(int)frame/3)%13,light);
        break;
    }
    case 20: { /* A tiny illuminated hotel on a planet with a tilted golden ring. */
        for(int y=-10;y<=10;++y) for(int x=-45;x<=45;++x) {
            int d=x*x*100+y*y*2025;
            if(d<=202500 && d>155000 && y<0) dot(c,54+x,31+y+x/6,gold);
        }
        disc(c,54,30,19,0xA887B8);
        for(int y=-18;y<=18;++y) for(int x=-18;x<=18;++x)
            if(x*x+y*y<324) dot(c,54+x,30+y,blend(0x6B709F,0xE9C5B0,(unsigned)(y+18)/5));
        for(int y=-10;y<=10;++y) for(int x=-45;x<=45;++x) {
            int d=x*x*100+y*y*2025;
            if(d<=202500 && d>155000 && y>=0) dot(c,54+x,31+y+x/6,y%3?gold:light);
        }
        box(c,43,17,23,16,0x504668); roof(c,39,10,31,8,0xBA939C);
        box(c,46,20,4,5,light); box(c,58,20,4,5,light); box(c,52,26,5,7,gold);
        line(c,69,9,69,19,gold); box(c,70,9,7,4,0xE0ADB5);
        star(c,19,14,2,light); star(c,88,14,1,gold);
        dot(c,31+(frame/6)%3,43,light);
        break;
    }
    case 21: { /* A copper fox has followed little paw prints across blue snow. */
        disc(c,79,13,8,light); disc(c,82,11,8,blend(p.paper,horizon,1));
        for(int x=3;x<105;++x) box(c,x,39+abs(x-56)/10,1,18,0xB9D3DA);
        for(int i=0;i<3;++i) {
            int x=8+i*12,y=16+(i%2)*5;
            box(c,x+4,y+8,2,22,0x42577C); roof(c,x-2,y,14,15,0x769EB4);
            roof(c,x-4,y+10,18,12,0x93B8C5); line(c,x-2,y+11,x+11,y+11,light);
        }
        rare_ellipse(c,62,40,18,8,0xD79676); rare_ellipse(c,58,44,16,5,0xF2BB8D);
        rare_ellipse(c,48,40,8,5,light); disc(c,74,33,8,0xE5A67C);
        roof(c,66,20,7,10,0xD58E74); roof(c,76,19,7,11,0xD58E74);
        roof(c,68,23,3,6,0x6C4F66); roof(c,78,22,3,6,0x6C4F66);
        line(c,69,35,75,40,light); line(c,82,34,75,40,light); dot(c,75,39,p.paper);
        line(c,69,33,72,33,p.paper); line(c,78,32,81,32,p.paper);
        for(int i=0;i<5;++i) { dot(c,91-i*10,52-i%2*2,0x68889F); dot(c,93-i*10,53-i%2*2,0x68889F); }
        for(int i=0;i<12;++i) dot(c,7+(i*31)%94,5+(i*13+(int)frame/4)%29,light);
        break;
    }
    case 22: { /* An open music box with a tiny dancer and a turning star. */
        box(c,22,15,61,24,gold); box(c,24,17,57,20,0x645377); box(c,28,20,49,13,0x8C7998);
        for(int x=29;x<76;++x) dot(c,x,29-abs(x-53)/4,0xD4AFB0);
        rare_ellipse(c,54,46,35,7,0x583F61);
        box(c,22,38,62,12,0xBA859D); box(c,22,38,62,3,gold); box(c,25,48,56,2,gold);
        rare_ellipse(c,53,38,28,4,light); rare_ellipse(c,53,38,22,2,0xC7ADC5);
        box(c,49,47,8,5,gold); dot(c,53,49,p.paper);
        line(c,84,42,91,42,gold); disc(c,94,39,3,gold); disc(c,94,45,3,gold);
        disc(c,53,22,3,light); line(c,53,25,53,31,gold);
        line(c,53,27,44,24,light); line(c,53,27,61,21,light);
        roof(c,45,28,17,6,0xE6AFCB); line(c,51,33,54,38,gold); line(c,56,33,59,36,gold);
        star(c,63,13,2+(frame/6)%2,light); star(c,15,27,1,gold); star(c,90,19,2,gold);
        break;
    }
    case 23: { /* A moonlit ferris wheel with twelve lantern-like cabins. */
        static const int points[12][2]={{0,-21},{11,-18},{18,-11},{21,0},{18,11},{11,18},
            {0,21},{-11,18},{-18,11},{-21,0},{-18,-11},{-11,-18}};
        disc(c,90,12,7,light); disc(c,93,10,7,blend(p.paper,horizon,1));
        line(c,55,27,40,54,gold); line(c,55,27,70,54,gold); box(c,35,53,40,3,0x9E7998);
        for(int i=0;i<12;++i) {
            int x=55+points[i][0],y=27+points[i][1];
            line(c,x,y,55+points[(i+1)%12][0],27+points[(i+1)%12][1],gold);
            line(c,55,27,x,y,0xAD91B3);
            box(c,x-2,y,5,5,i%3?0xB992BB:0x83C1C6);
            box(c,x-1,y+1,3,2,(i+(int)frame/3)%4?light:gold);
        }
        disc(c,55,27,3,gold); star(c,55,27,1,light);
        box(c,5,48,20,7,0x574667); roof(c,3,43,24,6,0xC89BAA);
        box(c,10,50,4,5,light); box(c,84,49,18,6,0x6A587A); box(c,88,51,3,3,gold);
        break;
    }
    case 24: { /* A fan-shaped shell protects a luminous pearl on the seabed. */
        box(c,3,49,102,7,0x31465F);
        for(int i=0;i<3;++i) for(int y=34;y<53;++y) {
            int x=10+i*7+((y/3+i)%3); dot(c,x,y,i%2?0x76A9AA:0x4D8397);
        }
        for(int x=30;x<=78;++x) {
            int top=15+abs(x-54)*abs(x-54)/45;
            line(c,54,48,x,top,x%4?0xB898C3:0xF0C3D3);
            disc(c,x,top,1,gold);
        }
        rare_ellipse(c,54,49,28,5,0xDBABC1); rare_ellipse(c,54,47,25,4,light);
        disc(c,54,33,11,blend(0xB898C3,light,3)); disc(c,54,33,8,0xF6DFBC);
        disc(c,52,30,4,light); star(c,48,27,2,light); star(c,65,39,1,gold);
        for(int i=0;i<6;++i) {
            int x=84+i%2*7,y=47-(i*7+(int)frame/3)%40;
            disc(c,x,y,1,0x7CA5C3); dot(c,x,y-1,light);
        }
        star(c,19,19,2,gold);
        break;
    }
    case 25: { /* A balloon post office carries envelopes above the cloud line. */
        for(int i=0;i<7;++i) rare_ellipse(c,5+i*17,51+(i%2)*3,14,4,0x8E9DB7);
        rare_ellipse(c,52,21,18,17,0xCC93AC);
        for(int y=-16;y<=16;++y) for(int x=-17;x<=17;++x)
            if(x*x*289+y*y*324<93636)
                dot(c,52+x,21+y,abs(x)<5?light:abs(x)<12?gold:0xCB95B5);
        line(c,41,32,46,44,gold); line(c,63,32,58,44,gold);
        box(c,45,42,14,9,0xA97D96); box(c,44,41,16,2,gold);
        box(c,48,45,8,5,light); line(c,48,45,52,48,0xBD8D9F); line(c,56,45,52,48,0xBD8D9F);
        for(int i=0;i<3;++i) {
            int x=13+i*32,y=28+(i%2)*9+(frame/8+i)%2;
            if(i==1) continue;
            box(c,x,y,10,7,light); line(c,x,y,x+5,y+4,0xB38EA8); line(c,x+9,y,x+5,y+4,0xB38EA8);
        }
        star(c,20,12,2,light); star(c,88,17,2,gold);
        break;
    }
    case 26: { /* A butterfly's velvet wings hold two mirrored constellations. */
        for(int side=-1;side<=1;side+=2) {
            rare_ellipse(c,54+side*16,23,17,14,0x9371B5);
            rare_ellipse(c,54+side*16,23,14,11,0x6586B9);
            rare_ellipse(c,54+side*12,40,12,10,0xAD8CBD);
            rare_ellipse(c,54+side*12,39,9,7,0xCC9BB7);
            line(c,54,30,54+side*25,16,gold); line(c,54,31,54+side*18,43,gold);
            line(c,54+side*9,24,54+side*23,27,light);
            line(c,54+side*23,27,54+side*21,16,light);
            star(c,54+side*9,24,1,light); star(c,54+side*23,27,2,light);
            star(c,54+side*21,16,1,light); star(c,54+side*15,42,1,light);
            for(int i=0;i<6;++i) dot(c,54+side*(20+i),12+i*3,gold);
        }
        box(c,53,23,3,18,gold); disc(c,54,22,2,light);
        line(c,54,22,49,15,gold); line(c,54,22,59,15,gold);
        for(int i=0;i<8;++i) star(c,8+(i*31)%91,8+(i*17)%43,((frame/4+i)%7==0)?1:0,light);
        break;
    }
    case 27: { /* A hanging island pours a ribbon of water into a cloud ocean. */
        for(int i=0;i<6;++i) rare_ellipse(c,9+i*19,51+i%3,17,5,0x9EAAC3);
        for(int y=24;y<44;++y) box(c,25+(y-24)*3/2,y,60-(y-24)*3,1,(y%5<3)?0x736A93:0x555476);
        rare_ellipse(c,54,23,31,6,0x5E9B8E); rare_ellipse(c,54,21,30,4,0xA2C8AD);
        box(c,43,12,2,10,gold); roof(c,35,8,18,10,0x78AAA3); roof(c,38,4,12,8,0xB0D0AD);
        for(int y=23;y<53;++y) {
            int x=59+(y-23)/14;
            box(c,x,y,11-(y-23)/13,1,0x8ED2D9);
            dot(c,x+2,y,(y+(int)frame/2)%7<3?light:0xA9E3E0); dot(c,x+7,y,light);
        }
        rare_ellipse(c,65,52,16,2,light);
        for(int i=0;i<7;++i) dot(c,53+i*4,47-(i*3+(int)frame/3)%9,0xCFEAE4);
        star(c,16,18,2,gold); disc(c,89,13,4,light);
        break;
    }
    case 28: { /* A brass gramophone plays a glittering record through polar night. */
        rare_ellipse(c,53,49,36,5,0x443C5C);
        box(c,23,39,63,12,0x95627F); box(c,23,39,63,3,gold); box(c,28,50,6,4,gold); box(c,75,50,6,4,gold);
        rare_ellipse(c,49,39,23,5,0x28253E); rare_ellipse(c,49,39,18,3,0x5C507B);
        rare_ellipse(c,49,39,7,2,gold); dot(c,49,38,light);
        for(int y=19;y<34;++y) box(c,70-(y-19)/2,y,3,1,gold);
        line(c,64,34,62,39,gold); line(c,62,39,53,39,gold);
        for(int x=43;x<74;++x) {
            int radius=3+(73-x)/3; box(c,x,19-radius,1,radius*2+1,x%4?gold:0xD09C8B);
        }
        rare_ellipse(c,43,19,5,13,0xF2D2A5); rare_ellipse(c,43,19,3,10,0x73536E);
        for(int i=0;i<3;++i) {
            int x=16+i*7,y=19-i*5+(frame/6+i)%2;
            line(c,x,y,x,y+5,light); box(c,x,y,3,1,light); rare_ellipse(c,x-1,y+6,2,1,light);
        }
        star(c,87,18,2,gold); star(c,81,30,1,light);
        break;
    }
    case 29: { /* A golden ginkgo shades an empty bench at a quiet platform. */
        box(c,3,46,102,9,0x6B5274); box(c,3,46,102,2,gold);
        line(c,11,54,103,54,0xB795A1); line(c,12,51,104,51,0xA27E98);
        box(c,29,13,4,32,0xA28282); line(c,31,25,16,15,0xB99991); line(c,31,24,47,13,0xB99991);
        for(int i=0;i<8;++i) {
            int x=10+(i*13)%45,y=8+(i*7)%13;
            rare_ellipse(c,x,y,9,6,i%3?gold:0xE9AC75); star(c,x-2,y-1,1,light);
        }
        box(c,47,35,29,6,0xBC9593); box(c,45,42,33,3,gold);
        line(c,50,44,50,48,gold); line(c,73,44,73,48,gold);
        for(int x=50;x<75;x+=6) box(c,x,36,1,4,gold);
        box(c,86,20,2,27,gold); box(c,80,20,14,7,0xA0B8BC); box(c,82,22,10,1,light);
        disc(c,96,11,5,light);
        for(int i=0;i<9;++i) {
            int x=7+(i*19)%87,y=26+(i*7+(int)frame/5)%19;
            line(c,x,y,x+1,y+1,i%2?gold:0xE8AC86);
        }
        break;
    }
    default: break;
    }
}

/* Thirty authored scenes, never assembled from the common scene/subject pool.
 * Jewel skies, illuminated focal subjects and gold tracery at native 108x58. */
static void rare_scene(canvas_t c,uint32_t id,uint32_t frame) {
    unsigned kind=fortune_rare_art_index(id);
    fortune_colors_t p=fortune_pixel_colors(id);
    uint32_t gold=p.accent,light=p.ink;
    static const uint32_t horizons[]={0x315986,0x554475,0x507E87,0x754752,0x27695E,0x504480,
        0x395865,0x667096,0x554173,0x315A83,0x6C5175,0x4B5182,0x565C8C,0x405E81,
        0x285D58,0x605183,0x5C4C7A,0x5F4769,0x4B5185,0x8F687F,
        0x52487C,0x426780,0x694D7C,0x534474,0x325A7B,0x645B91,0x484D8B,0x42667F,0x584068,0x69516F};
    uint32_t horizon=blend(horizons[kind],gold,fortune_rare_art_tone(id));
    for(int y=0;y<58;++y) box(c,0,y,108,1,blend(p.paper,horizon,(unsigned)y/8));
    for(unsigned i=0;i<36;++i) {
        uint32_t h=mix(i*317+kind*31+911); int x=5+h%98,y=4+(h>>8)%36;
        dot(c,x,y,i%3?blend(horizon,light,3):gold);
        if ((frame/2+i)%19==0) star(c,x,y,1,light);
    }
    if (kind==0) { /* A luminous whale carrying a constellation across the sea. */
        disc(c,82,12,8,blend(horizon,gold,2)); disc(c,82,12,5,light);
        disc(c,84,10,5,blend(p.paper,horizon,1));
        for(int y=42;y<56;++y) {
            box(c,3,y,102,1,blend(p.paper,0x2A779A,(y-42)/3));
            for(int x=5;x<104;x+=17) box(c,x+(y*7+(int)frame/3)%9,y,8,1,blend(horizon,light,2));
        }
        int bob=(frame/8)%4==2?1:0;
        for(int y=-9;y<=9;++y) for(int x=-25;x<=25;++x)
            if(x*x*81+y*y*625<50625)
                dot(c,49+x,31+y+bob,y>3?0xB5E4EB:blend(0x397DA4,0x8EDCD9,(unsigned)(y+9)/3));
        for(int i=0;i<13;++i) {
            box(c,71+i,28-i/3+bob,1,7+i/4,0x75C8D3);
            box(c,81+i/2,24-i/2+bob,2,3,0xA9E5E6);
            box(c,82+i/2,34+i/3+bob,2,3,0x79BCD2);
        }
        line(c,40,37+bob,48,43+bob,0x75B9D0); line(c,41,37+bob,49,42+bob,light);
        dot(c,33,29+bob,p.paper); dot(c,33,28+bob,light);
        line(c,42,26+bob,42,16,0x7CD9D8); line(c,42,17,36,12,gold); line(c,42,17,47,10,gold);
        star(c,36,12,2,light); star(c,48,9,2,gold); star(c,41,8,1,light);
        line(c,44,27,53,24,gold); line(c,53,24,62,28,gold);
        star(c,44,27,1,light); star(c,53,24,1,light); star(c,62,28,1,light);
    } else if (kind==1) { /* Brass moon train, cat passenger, tracks into the sky. */
        disc(c,80,15,11,blend(horizon,gold,3)); disc(c,80,15,8,light);
        disc(c,84,12,8,blend(p.paper,horizon,1));
        for(int x=4;x<105;++x) {
            int y=52-(x-4)*(x-4)/620;
            dot(c,x,y,gold); dot(c,x,y+3,light);
            if(x%5==0) line(c,x,y-1,x+1,y+4,0xAD7896);
        }
        box(c,17,29,66,16,0xAD7085); box(c,19,26,60,3,gold);
        box(c,20,29,59,2,light); box(c,19,43,62,2,gold);
        box(c,80,32,7,10,0xAD7085); box(c,84,34,4,4,light);
        for(int i=0;i<4;++i) {
            int x=23+i*14; box(c,x,32,10,9,gold); box(c,x+1,33,8,7,0xFFE4B3);
            box(c,x+4,32,1,9,0xAD7085);
        }
        box(c,40,35,5,4,0x523D59); dot(c,40,34,0x523D59); dot(c,44,34,0x523D59);
        for(int i=0;i<4;++i) { disc(c,27+i*15,46,3,p.paper); dot(c,27+i*15,46,gold); }
        for(int i=0;i<5;++i) star(c,10+i*4,33-i*3-(frame/4)%3,1,i%2?gold:light);
        box(c,7,20,2,32,gold); box(c,5,20,7,2,light); disc(c,8,17,3,gold);
    } else if (kind==2) { /* Rainbow already reflected while rainclouds part. */
        static const uint32_t rainbow[]={0xF9B6A3,0xF5D18B,0xEAE8B0,0x83D2BD,0x8CB8E1,0xB7A3E0};
        for(int y=8;y<40;++y) for(int x=15;x<96;++x) {
            int d=(x-57)*(x-57)+(y-40)*(y-40);
            for(int band=0;band<6;++band) {
                int r=33-band*2;
                if(d<=r*r && d>(r-2)*(r-2)) dot(c,x,y,rainbow[band]);
            }
        }
        for(int i=0;i<5;++i) { disc(c,12+i*7,17+(i%2)*3,6,0x748DA2); disc(c,84+i*5,22-i%3,5,0x9AB8C1); }
        box(c,3,42,102,13,0x2C5269);
        for(int y=0;y<8;++y) box(c,33+y*2,45+y,54-y*4,1,rainbow[y%6]);
        box(c,11,43,3,8,gold); box(c,10,51,6,2,gold);
        for(int i=0;i<12;++i) box(c,8+i,37-abs(i-6)/2,1,5,0xF5B192);
        for(int i=0;i<7;++i) {
            int x=9+i*14,y=26+(i*7+(int)frame/2)%14;
            if(i<2 || i>5) box(c,x,y,1,2,0xB9E5E2);
        }
        star(c,47+(frame/6)%3,47,1,light); star(c,79,43,1,light);
    } else if (kind==3) { /* Floating lanterns over layered, inhabited rooflines. */
        disc(c,78,14,9,blend(horizon,gold,2)); disc(c,78,14,6,light);
        for(int layer=0;layer<2;++layer) for(int i=0;i<7;++i) {
            int x=i*17-6+layer*7,y=39+layer*7-(i%3)*3;
            box(c,x,y,16,57-y,layer?0x1C243B:0x41344B);
            roof(c,x-2,y-7,20,8,layer?0x35354A:0x604354);
            box(c,x+4,y+3,3,4,gold); box(c,x+10,y+3,3,4,light);
        }
        for(int i=0;i<9;++i) {
            int x=12+(i*29)%86,y=6+(i*11)%25-(frame/10+i)%2;
            box(c,x-1,y+1,7,7,blend(horizon,gold,2)); box(c,x,y,5,7,gold);
            box(c,x+1,y+1,3,4,light); line(c,x+1,y+7,x+3,y+9,gold); dot(c,x+2,y+10,light);
        }
        star(c,54,16,2,light);
    } else if (kind==4) { /* Aurora curtains, a glasshouse and night-blooming flowers. */
        for(int x=4;x<104;++x) {
            int crest=8+abs((x+(int)(frame/5)%3)%40-20)/3;
            for(int y=crest;y<crest+13;++y)
                dot(c,x,y,blend(horizon,x%30<15?0x76E5C4:0xB6A0E8,(unsigned)(13-y+crest)/2));
        }
        box(c,3,43,102,13,0x173F48);
        box(c,38,30,31,19,0x34666C);
        for(int x=35;x<74;++x) { int y=16+abs(x-54)/2; dot(c,x,y,gold); dot(c,x,y+1,light); }
        line(c,37,25,37,49,gold); line(c,71,25,71,49,gold); line(c,54,17,54,49,gold);
        line(c,46,22,46,49,0xAAC9AF); line(c,62,22,62,49,0xAAC9AF);
        box(c,38,34,33,1,gold); box(c,49,39,11,10,light); box(c,51,40,7,9,0x62938A);
        for(int i=0;i<14;++i) {
            int x=7+(i*23)%94,y=46+(i*7)%8;
            line(c,x,y,x,y+3,0x6AA68D);
            star(c,x,y,1,i%3?0xEBC2E4:light);
            if((frame/3+i)%13==0) star(c,x,y-2,1,gold);
        }
    } else if (kind==5) { /* An illuminated envelope beneath a spiral galaxy. */
        for(int i=0;i<80;++i) {
            uint32_t h=mix(i*771+177);
            int x=8+h%92,y=27-(x-54)/4+(int)((h>>12)%11)-5;
            disc(c,x,y,i%9==0?1:0,i%4?0xA4AAD9:light);
        }
        disc(c,60,24,10,blend(horizon,0xC9A2DD,3));
        disc(c,60,24,6,blend(horizon,gold,5)); disc(c,60,24,3,light);
        line(c,32,38,58,27,gold); line(c,59,26,79,16,gold);
        box(c,27,35,27,17,gold); box(c,29,37,23,13,light);
        line(c,29,37,41,45,0xA87C88); line(c,41,45,51,37,0xA87C88);
        line(c,29,49,37,43,0xA87C88); line(c,44,43,51,49,0xA87C88);
        disc(c,41,45,3,gold); star(c,41,45,1,light);
        star(c,77,37,3,gold); star(c,86,29,2,light);
        for(int i=0;i<5;++i) star(c,55+i*4,35-i*4-(frame/4)%2,1,light);
    } else rare_extended(c,kind,frame,p,horizon);
    /* Etched corners and tiny moving highlights frame every original. */
    line(c,2,2,22,2,gold); line(c,2,2,2,15,gold);
    line(c,85,2,105,2,gold); line(c,105,2,105,15,gold);
    line(c,2,42,2,55,gold); line(c,2,55,22,55,gold);
    line(c,85,55,105,55,gold); line(c,105,42,105,55,gold);
    star(c,4,4,1,light); star(c,103,53,1,light);
    dot(c,7+(frame/2)%13,2,light); dot(c,91+(frame/2)%11,55,light);
}

static void scene(canvas_t c,uint32_t id,uint32_t frame) {
    if(fortune_rare_art(id)){rare_scene(c,id,frame);return;}
    if(id>=FORTUNE_LEGACY_ART_COUNT){expanded_scene(c,id,frame);return;}
    unsigned family=id%4,tone=(id/4)%6; uint32_t seed=id/24;
    fortune_colors_t p=fortune_pixel_colors(id);
    switch(family) { case 0: night(c,seed,tone,frame,p); break; case 1: wild(c,seed,tone,frame,p); break;
        case 2: space(c,seed,tone,frame,p); break; default: garden(c,seed,tone,frame,p); break; }
}
void fortune_pixels(uint16_t *pixels,uint32_t id,uint32_t frame) {
    scene((canvas_t){pixels,0,FORTUNE_PIXEL_W},id,frame);
}
void fortune_pixel_wipe(uint16_t *pixels,uint32_t old_id,uint32_t new_id,unsigned step) {
    int width=step>=3?FORTUNE_PIXEL_W:(int)step*FORTUNE_PIXEL_W/3;
    scene((canvas_t){pixels,0,width},new_id,0);
    scene((canvas_t){pixels,width,FORTUNE_PIXEL_W},old_id,0);
}

void fortune_pixel_unwrap(uint16_t *pixels,uint32_t id,unsigned step) {
    canvas_t c={pixels,0,FORTUNE_PIXEL_W}; fortune_colors_t p=fortune_pixel_colors(id);
    box(c,0,0,108,58,p.paper);
    int shift=step<4 ? (int[]){0,-1,1,0}[step] : 0;
    int x=28+shift,y=22;
    /* The seal opens, then a blank letter rises. Content appears only at the end. */
    for(unsigned i=0;i<9;++i) {
        uint32_t h=mix(i*91+id); int sx=8+h%92,sy=5+(h>>8)%48;
        if((sx<24 || sx>82 || sy<10) && (step+i)%4<2) star(c,sx,sy,1,p.muted);
    }
    if(step>=4) {
        int lift=(step-3)*3;
        for(int j=0;j<12;++j) box(c,x+j,y-1-j,52-j*2,1,p.muted);
        box(c,x+5,y-lift,42,30,p.ink);
        box(c,x+12,y-lift+6,28,1,p.paper); box(c,x+12,y-lift+10,21,1,p.paper);
        star(c,x+26,y-lift+18,3,p.accent);
    }
    box(c,x,y,52,28,p.accent);
    line(c,x,y+27,x+21,y+11,p.paper); line(c,x+51,y+27,x+30,y+11,p.paper);
    if(step<4) { line(c,x,y,x+26,y+18,p.paper); line(c,x+26,y+18,x+51,y,p.paper); }
    else { line(c,x,y,x+26,y+14,p.paper); line(c,x+26,y+14,x+51,y,p.paper); }
    if(step<4) { disc(c,x+26,y+16,4,p.paper); star(c,x+26,y+16,2,p.ink); }
    if(fortune_rare_art(id)) {
        line(c,x+2,y+2,x+49,y+2,p.ink); line(c,x+2,y+25,x+49,y+25,p.ink);
        box(c,x+39,y+5,8,8,p.paper); star(c,x+43,y+9,2,p.accent);
        for(unsigned i=0;i<12;++i) {
            uint32_t h=mix(i*313+id); int sx=7+h%94,sy=4+(h>>8)%49;
            if(sx<x-3 || sx>x+54 || sy<y-6) star(c,sx,sy,(step+i)%3?1:2,(i&1)?p.ink:p.accent);
        }
        line(c,4,3,22,3,p.accent); line(c,4,3,4,16,p.accent);
        line(c,85,54,103,54,p.accent); line(c,103,41,103,54,p.accent);
    }
    for(unsigned i=0;i<9;++i) box(c,37+i*4,55,2,1,i<=step?p.accent:p.muted);
}
