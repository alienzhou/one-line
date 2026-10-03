#include "fortune_ui.h"
#include "fortune_text.h"
#include "lvgl.h"
#include "bsp_display_rounding.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LV_FONT_DECLARE(fortune_font_12);
static uint16_t s_pixels[240*320];
static uint8_t s_draw[240*40*2];
static void advance(unsigned milliseconds) {
    for(unsigned n=0;n<milliseconds;n+=125) { lv_tick_inc(125); lv_timer_handler(); }
}

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    uint16_t *source = (uint16_t *)pixels;
    for (int y = area->y1; y <= area->y2; ++y) {
        int32_t left, right;
        bool visible = bsp_display_rounded_row_span(y, 240, 320, 30, &left, &right);
        for (int x = area->x1; x <= area->x2; ++x) {
            uint16_t color = *source++;
            s_pixels[y*240+x] = visible && x >= left && x <= right ? color : 0;
        }
    }
    lv_display_flush_ready(display);
}

static void check_labels(void) {
    lv_obj_t *root = lv_screen_active();
    uint32_t count = lv_obj_get_child_count(root);
    for (unsigned i = 0; i < count; ++i) {
        lv_obj_t *label = lv_obj_get_child(root, i);
        lv_area_t a; lv_obj_get_coords(label, &a);
        if (a.x1 < 0 || a.x2 >= 240 || a.y1 < 0 || a.y2 >= 320) {
            fprintf(stderr, "Outside panel: %s (%d,%d..%d,%d)\n",lv_label_get_text(label),
                    (int)a.x1,(int)a.y1,(int)a.x2,(int)a.y2); abort();
        }
        for (unsigned j = i+1; j < count; ++j) {
            lv_area_t b; lv_obj_get_coords(lv_obj_get_child(root,j), &b);
            if (a.x1<=b.x2 && b.x1<=a.x2 && a.y1<=b.y2 && b.y1<=a.y2) {
                fprintf(stderr,"Overlapping labels: %s / %s\n",lv_label_get_text(label),
                        lv_label_get_text(lv_obj_get_child(root,j))); abort();
            }
        }
        /* Check the active widget font, not merely the font asset. */
        const unsigned char *p = (const unsigned char *)lv_label_get_text(label);
        while (*p) {
            uint32_t cp = *p++;
            if (cp >= 0xE0) { cp = (cp&15)<<12; cp |= (*p++&63)<<6; cp |= *p++&63; }
            else if (cp >= 0xC0) { cp = (cp&31)<<6; cp |= *p++&63; }
            if (cp < 32) continue;
            lv_font_glyph_dsc_t g = {0};
            assert(lv_font_get_glyph_dsc(lv_obj_get_style_text_font(label, 0), &g, cp, 0));
            assert(!g.is_placeholder);
        }
    }
}

static void snapshot(const char *directory, const char *name) {
    lv_obj_update_layout(lv_screen_active()); lv_obj_invalidate(lv_screen_active()); lv_refr_now(NULL);
    char path[1024]; snprintf(path,sizeof(path),"%s/%s.ppm",directory,name);
    FILE *f = fopen(path,"wb"); assert(f); fprintf(f,"P6\n240 320\n255\n");
    for (unsigned i = 0; i < 240*320; ++i) {
        uint16_t c = s_pixels[i];
        uint8_t rgb[] = {(uint8_t)(((c>>11)&31)*255/31),(uint8_t)(((c>>5)&63)*255/63),(uint8_t)((c&31)*255/31)};
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}

static int compare_hash(const void *aa, const void *bb) {
    uint64_t a = *(const uint64_t *)aa, b = *(const uint64_t *)bb;
    return a < b ? -1 : a > b;
}

int main(int argc, char **argv) {
    assert(argc == 2 || argc == 3); setvbuf(stdout,NULL,_IOLBF,0);
    lv_init();
    lv_display_t *d = lv_display_create(240,320); assert(d);
    lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(d,s_draw,NULL,sizeof(s_draw),LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d,flush);
    assert(fortune_ui_fonts_valid());
    lv_font_glyph_dsc_t g = {0};
    assert(!lv_font_get_glyph_dsc(&fortune_font_12,&g,0x9F98,0) || g.is_placeholder);
    assert(fortune_ui_create());
    fortune_state_t s; fortune_defaults(&s,42);
    fortune_ui_update(&s,FORTUNE_HOME,88,NULL); snapshot(argv[1],"home"); check_labels();
    for (unsigned style=0; style<4; ++style) for (unsigned mood=0; mood<9; ++mood) {
        s.style=style; s.mood=mood;
        fortune_ui_update(&s,FORTUNE_HOME,-1,NULL); lv_obj_update_layout(lv_screen_active()); check_labels();
    }
    for (unsigned id=0; id<FORTUNE_COUNT; ++id) {
        s.current.quote=id; s.current.art=id*113%FORTUNE_ART_COUNT;
        fortune_ui_update(&s,FORTUNE_REVEAL,100,NULL);
        char decoded[128],rendered[128]; assert(fortune_decode(id,decoded,sizeof(decoded)));
        const char *label_text=lv_label_get_text(lv_obj_get_child(lv_screen_active(),2)); unsigned n=0;
        for(const char *p=label_text;*p;++p) if(*p!='\n') rendered[n++]=*p;
        rendered[n]=0; assert(!strcmp(decoded,rendered));
        assert(!strstr(label_text,"\n，") && !strstr(label_text,"\n。") && !strstr(label_text,"\n："));
        lv_obj_update_layout(lv_screen_active()); check_labels();
        if (id==0 || id==88 || id==176) {
            char name[64]; snprintf(name,sizeof(name),"style-%u",FORTUNE_RECORDS[id].style);
            snapshot(argv[1],name);
        }
    }
    /* Exercise the supported 32-character upper bound, beyond current samples. */
    char longest[97];
    for (unsigned i=0; i<32; ++i) memcpy(longest+3*i,"我",3);
    longest[96]=0;
    lv_label_set_text(lv_obj_get_child(lv_screen_active(),2),longest);
    lv_obj_update_layout(lv_screen_active()); check_labels();
    s.pinned=s.current;
    for (unsigned p=0; p<6; ++p) {
        s.pinned.art=2976+p;
        fortune_ui_update(&s,FORTUNE_SHOWCASE,88,NULL);
        advance(375);
        char name[64]; snprintf(name,sizeof(name),"palette-%u",p); snapshot(argv[1],name); check_labels();
    }
    s.pinned.art=2976;
    fortune_ui_update(&s,FORTUNE_SHOWCASE,88,NULL);
    advance(375);
    for(unsigned i=0;i<32;++i) {
        char name[64]; snprintf(name,sizeof(name),"motion-%02u",i); snapshot(argv[1],name);
        fortune_ui_tick();
    }
    uint16_t paused[240*320];
    fortune_ui_motion(false); lv_refr_now(NULL); memcpy(paused,s_pixels,sizeof(paused));
    advance(2000); lv_refr_now(NULL);
    assert(!memcmp(paused,s_pixels,sizeof(paused)));
    fortune_ui_motion(true);
    s.current=s.pinned;
    fortune_ui_update(&s,FORTUNE_REVEAL,88,NULL);
    char expected[128]; snprintf(expected,sizeof(expected),"%s",lv_label_get_text(lv_obj_get_child(lv_screen_active(),2)));
    fortune_ui_begin_reveal(); assert(fortune_ui_revealing());
    assert(strcmp(expected,lv_label_get_text(lv_obj_get_child(lv_screen_active(),2))));
    for(unsigned i=0;i<11;++i) {
        char name[64]; snprintf(name,sizeof(name),"unwrap-%02u",i); snapshot(argv[1],name); check_labels(); advance(125);
    }
    assert(!fortune_ui_revealing());
    assert(!strcmp(expected,lv_label_get_text(lv_obj_get_child(lv_screen_active(),2))));
    fortune_ui_begin_reveal(); fortune_ui_finish_reveal(); assert(!fortune_ui_revealing());
    assert(!strcmp(expected,lv_label_get_text(lv_obj_get_child(lv_screen_active(),2))));
    const char *notices[] = {FT_SAVE_ERROR,FT_INPUT_ERROR,FT_NO_PIN,FT_EXHAUSTED_HINT,
        "旧存档不兼容，已使用新签库","已重新洗牌"};
    for (unsigned i=0;i<sizeof(notices)/sizeof(notices[0]);++i) {
        fortune_ui_update(&s,FORTUNE_HOME,-1,notices[i]); lv_obj_update_layout(lv_screen_active()); check_labels();
    }
    lv_mem_monitor_t m; lv_mem_monitor(&m);
    printf("Fortune UI: PASS (all %u texts, 36 filters, error states, six scenes, timed reveal/skip, paused motion; LVGL used=%u/%u largest=%u)\n",
           FORTUNE_COUNT, (unsigned)(m.total_size-m.free_size),(unsigned)m.total_size,(unsigned)m.free_biggest_size);
    if (argc == 3 && strcmp(argv[2], "--all-art") == 0) {
        fortune_ui_motion(false);
        uint64_t *hashes = malloc(FORTUNE_ART_COUNT*sizeof(uint64_t)); assert(hashes);
        s.pinned.quote = 0;
        for (uint32_t i=0; i<FORTUNE_ART_COUNT; ++i) {
            s.pinned.art=i;
            fortune_ui_update(&s,FORTUNE_SHOWCASE,88,NULL);
            advance(375);
            lv_obj_update_layout(lv_screen_active()); lv_refr_now(NULL);
            uint64_t h = 14695981039346656037ULL;
            /* Only the artwork region, so unique quote/ID text cannot inflate diversity. */
            for (unsigned y=38; y<154; ++y) for (unsigned x=12; x<228; ++x) {
                h ^= s_pixels[y*240+x]; h *= 1099511628211ULL;
            }
            hashes[i]=h;
            if (i%4096 == 4095) printf("Rendered %u/%u appearances\n",i+1,FORTUNE_ART_COUNT);
        }
        qsort(hashes,FORTUNE_ART_COUNT,sizeof(uint64_t),compare_hash);
        unsigned unique=1;
        for (unsigned i=1;i<FORTUNE_ART_COUNT;++i) if(hashes[i]!=hashes[i-1]) ++unique;
        printf("Distinct artwork-region pixel hashes: %u/%u\n",unique,FORTUNE_ART_COUNT);
        assert(unique == FORTUNE_ART_COUNT);
        free(hashes);
    }
    lv_deinit(); return 0;
}
