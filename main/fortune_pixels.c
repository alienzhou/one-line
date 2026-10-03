#include "fortune_pixels.h"
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

fortune_colors_t fortune_pixel_colors(uint32_t id) {
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

static void scene(canvas_t c,uint32_t id,uint32_t frame) {
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
    for(unsigned i=0;i<9;++i) box(c,37+i*4,55,2,1,i<=step?p.accent:p.muted);
}
