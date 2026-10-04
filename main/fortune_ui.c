#include "fortune_ui.h"
#include "fortune_text.h"
#include "fortune_pixels.h"
#include "fortune_characters.h"
#include "lvgl.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <stdio.h>
#include <string.h>

LV_FONT_DECLARE(fortune_font_12);
LV_FONT_DECLARE(fortune_font_20);
static lv_obj_t *s_root, *s_meta, *s_battery, *s_quote, *s_caption, *s_help, *s_hint;
static uint16_t s_pixels[FORTUNE_PIXEL_W*FORTUNE_PIXEL_H];
static const lv_image_dsc_t s_image = {
    .header = {.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,
               .w=FORTUNE_PIXEL_W,.h=FORTUNE_PIXEL_H,.stride=FORTUNE_PIXEL_W*2},
    .data_size=sizeof(s_pixels),.data=(const uint8_t *)s_pixels
};
static uint32_t s_id=UINT32_MAX, s_frame;
static bool s_motion=true;
static bool s_revealing;
static unsigned s_reveal_step, s_timer_step;
static unsigned s_wipe_step=3;
static uint32_t s_old_id,s_quote_id=FORTUNE_NO_CARD;
static fortune_page_t s_page=FORTUNE_HOME;
static char s_final_quote[128],s_final_caption[128],s_final_help[128],s_final_hint[128];
static lv_timer_t *s_timer;
static void (*s_sound_callback)(fortune_sound_t,unsigned);
static bool s_sound_enabled=true;
static bool s_volume_visible;
static unsigned s_volume_percent;
void fortune_ui_sound_callback(void (*callback)(fortune_sound_t,unsigned)) { s_sound_callback=callback; }
void fortune_ui_sound_enabled(bool enabled) { s_sound_enabled=enabled; }

static bool closing(uint32_t cp) {
    return cp==0xFF0C || cp==0x3002 || cp==0xFF01 || cp==0xFF1F || cp==0xFF1A || cp==0xFF1B ||
        cp==0x3001 || cp==0x201D || cp==0x300B || cp==0xFF09;
}
/* Explicit CJK wrapping keeps closing punctuation off the start of a line.
 * Measured line breaks favor complete clauses and avoid a one-character tail.
 * All source bytes remain intact; at most 32 characters, no heap allocation. */
static void format_quote(const char *in,char out[128]) {
    unsigned offsets[33],count=0,pos=0; uint32_t points[32];
    while(in[pos] && count<32) {
        offsets[count]=pos; uint32_t cp=(unsigned char)in[pos++];
        if(cp>=0xE0) { cp=(cp&15)<<12; cp|=((unsigned char)in[pos++]&63)<<6; cp|=(unsigned char)in[pos++]&63; }
        else if(cp>=0xC0) { cp=(cp&31)<<6; cp|=(unsigned char)in[pos++]&63; }
        points[count++]=cp;
    }
    offsets[count]=pos;
    unsigned cost[33],next_break[32]; cost[count]=0;
    for (unsigned cursor=count; cursor>0; --cursor) {
        unsigned start=cursor-1; int width=0;
        cost[start]=UINT32_MAX/4; next_break[start]=start+1;
        for(unsigned end=start+1; end<=count; ++end) {
            width+=lv_font_get_glyph_width(&fortune_font_20,points[end-1],0);
            if(width>196) break;
            if(end<count && closing(points[end])) continue;
            unsigned slack=(unsigned)(196-width);
            unsigned candidate=cost[end]+350+slack*slack/64;
            if(end-start<3) candidate+=2000;
            if(end<count && closing(points[end-1])) candidate-=60;
            if(candidate<cost[start]) { cost[start]=candidate; next_break[start]=end; }
        }
    }
    unsigned written=0,start=0;
    while(start<count) {
        unsigned end=next_break[start];
        unsigned bytes=offsets[end]-offsets[start];
        memcpy(out+written,in+offsets[start],bytes); written+=bytes; start=end;
        if(start<count) out[written++]='\n';
    }
    out[written]=0;
}

static void artwork(lv_event_t *event) {
    lv_draw_image_dsc_t d; lv_draw_image_dsc_init(&d);
    d.src=&s_image; d.scale_x=d.scale_y=512; d.pivot.x=d.pivot.y=0; d.antialias=0;
    lv_area_t a={12,38,12+FORTUNE_PIXEL_W-1,38+FORTUNE_PIXEL_H-1};
    lv_draw_image(lv_event_get_layer(event),&d,&a);
    if(s_volume_visible) {
        fortune_colors_t p=fortune_pixel_colors(s_id);
        lv_draw_rect_dsc_t bar; lv_draw_rect_dsc_init(&bar);
        for(unsigned i=0;i<10;++i) {
            bar.bg_color=lv_color_hex(i<s_volume_percent/10?p.ink:p.muted);
            bar.bg_opa=i<s_volume_percent/10?LV_OPA_COVER:LV_OPA_30;
            lv_area_t segment={22+(int)i*20,237,38+(int)i*20,252};
            lv_draw_rect(lv_event_get_layer(event),&bar,&segment);
        }
    }
}

/* Called only under LVGL's lock. Each tick invalidates the illustration, never text. */
void fortune_ui_tick(void) {
    if(!s_motion || !s_root || s_id==UINT32_MAX) return;
    if(s_revealing || s_wipe_step<3) return;
    fortune_pixels(s_pixels,s_id,++s_frame);
    lv_image_cache_drop(&s_image);
    lv_area_t area={12,38,227,153}; lv_obj_invalidate_area(s_root,&area);
}
void fortune_ui_motion(bool enabled) { s_motion=enabled; }
bool fortune_ui_revealing(void) { return s_revealing; }
static void finish_reveal(bool audible) {
    if(!s_revealing) return;
    s_revealing=false;
    lv_label_set_text(s_quote,s_final_quote); lv_label_set_text(s_caption,s_final_caption);
    lv_label_set_text(s_help,s_final_help); lv_label_set_text(s_hint,s_final_hint);
    fortune_pixels(s_pixels,s_id,0); lv_image_cache_drop(&s_image); lv_obj_invalidate(s_root);
    if(audible && s_sound_callback) s_sound_callback(FORTUNE_SOUND_REVEAL,s_id);
}
void fortune_ui_finish_reveal(void) { finish_reveal(true); }
void fortune_ui_begin_reveal(void) {
    snprintf(s_final_quote,sizeof(s_final_quote),"%s",lv_label_get_text(s_quote));
    snprintf(s_final_caption,sizeof(s_final_caption),"%s",lv_label_get_text(s_caption));
    snprintf(s_final_help,sizeof(s_final_help),"%s",lv_label_get_text(s_help));
    snprintf(s_final_hint,sizeof(s_final_hint),"%s",lv_label_get_text(s_hint));
    s_revealing=true; s_reveal_step=0;
    lv_timer_reset(s_timer);
    s_wipe_step=3;
    lv_label_set_text(s_quote,"生活来信\n正在路上");
    lv_label_set_text(s_caption,"给此刻的你 / 待拆封");
    lv_label_set_text(s_help,"一点未知，一点期待"); lv_label_set_text(s_hint,"按确定可直接拆开");
    fortune_pixel_unwrap(s_pixels,s_id,0); lv_image_cache_drop(&s_image); lv_obj_invalidate(s_root);
    if(s_sound_callback) s_sound_callback(FORTUNE_SOUND_OPEN,s_id);
}
static void timer_tick(lv_timer_t *timer) {
    (void)timer;
    if(s_revealing) {
        if(++s_reveal_step>=10) { fortune_ui_finish_reveal(); return; }
        fortune_pixel_unwrap(s_pixels,s_id,s_reveal_step);
        lv_image_cache_drop(&s_image);
        lv_area_t area={12,38,227,153}; lv_obj_invalidate_area(s_root,&area);
    } else if(s_wipe_step<3) {
        fortune_pixel_wipe(s_pixels,s_old_id,s_id,++s_wipe_step); lv_image_cache_drop(&s_image);
        lv_area_t area={12,38,227,153}; lv_obj_invalidate_area(s_root,&area);
    } else if(++s_timer_step%2==0) fortune_ui_tick();
}
static void cleanup(lv_event_t *event) {
    lv_timer_t *timer=lv_event_get_user_data(event); lv_timer_delete(timer); s_root=NULL;
    s_timer=NULL; s_sound_callback=NULL; s_revealing=false; s_volume_visible=false;
}
static lv_obj_t *label(int x,int y,int width,const lv_font_t *font) {
    lv_obj_t *o=lv_label_create(s_root); if(!o) return NULL;
    lv_obj_set_pos(o,x,y); lv_obj_set_width(o,width);
    lv_obj_set_style_text_font(o,font,0); lv_obj_set_style_text_line_space(o,2,0);
    return o;
}
bool fortune_ui_fonts_valid(void) {
    const lv_font_t *fonts[]={&fortune_font_12,&fortune_font_20};
    for(unsigned f=0;f<2;++f) for(unsigned i=0;i<FORTUNE_CHARACTER_COUNT;++i) {
        lv_font_glyph_dsc_t d={0};
        if(!lv_font_get_glyph_dsc(fonts[f],&d,FORTUNE_CHARACTERS[i],0)||d.is_placeholder) return false;
    }
    return true;
}
bool fortune_ui_create(void) {
    if(!fortune_ui_fonts_valid()) return false;
    s_root=lv_obj_create(NULL); if(!s_root) return false;
    lv_obj_remove_style_all(s_root); lv_obj_set_style_bg_opa(s_root,LV_OPA_COVER,0);
    lv_obj_remove_flag(s_root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(s_root,artwork,LV_EVENT_DRAW_MAIN,NULL);
    s_meta=label(23,15,158,&fortune_font_12); s_battery=label(183,15,34,&fortune_font_12);
    s_quote=label(22,181,196,&fortune_font_20); s_caption=label(22,161,196,&fortune_font_12);
    s_help=label(16,284,208,&fortune_font_12); s_hint=label(22,302,196,&fortune_font_12);
    if(!s_meta||!s_battery||!s_quote||!s_caption||!s_help||!s_hint) {
        lv_obj_delete(s_root); s_root=NULL; return false;
    }
    lv_obj_set_style_text_align(s_battery,LV_TEXT_ALIGN_RIGHT,0);
    lv_obj_set_style_text_align(s_help,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_align(s_hint,LV_TEXT_ALIGN_CENTER,0);
    lv_timer_t *timer=lv_timer_create(timer_tick,125,NULL);
    if(!timer) { lv_obj_delete(s_root); s_root=NULL; return false; }
    lv_obj_add_event_cb(s_root,cleanup,LV_EVENT_DELETE,timer);
    s_timer=timer;
    lv_screen_load(s_root); return true;
}
void fortune_ui_update(const fortune_state_t *s,fortune_page_t page,int battery,const char *notice) {
    fortune_card_t card=page==FORTUNE_SHOWCASE?s->pinned:s->current;
    uint32_t id=page==FORTUNE_HOME||page==FORTUNE_TOPICS||card.quote==FORTUNE_NO_CARD?2976:card.art;
    if(s_revealing && id==s_id && card.quote==s_quote_id && page==s_page) {
        if(battery<0) lv_label_set_text(s_battery,"--%"); else lv_label_set_text_fmt(s_battery,"%d%%",battery);
        return; /* Battery refresh must not shorten the opening sequence. */
    }
    finish_reveal(false);
    fortune_colors_t p=fortune_pixel_colors(id);
    if(id!=s_id) {
        s_wipe_step=s_id!=UINT32_MAX && page!=FORTUNE_HOME && page==s_page && card.quote==s_quote_id ? 0:3;
        s_old_id=s_id; s_id=id; s_frame=0;
        if(s_wipe_step<3) fortune_pixel_wipe(s_pixels,s_old_id,s_id,0);
        else fortune_pixels(s_pixels,s_id,s_frame);
        lv_image_cache_drop(&s_image);
    }
    s_quote_id=card.quote; s_page=page;
    lv_obj_set_style_bg_color(s_root,lv_color_hex(p.paper),0);
    lv_obj_t *labels[]={s_meta,s_battery,s_quote,s_caption,s_help,s_hint};
    for(unsigned i=0;i<6;++i) lv_obj_set_style_text_color(labels[i],lv_color_hex(i<3?p.ink:p.muted),0);
    if(battery<0) lv_label_set_text(s_battery,"--%"); else lv_label_set_text_fmt(s_battery,"%d%%",battery);
    char quote[128];
    if(page==FORTUNE_HOME) {
        lv_label_set_text_fmt(s_meta,"一签 / 全库%s",s_sound_enabled?"":" · 静音"); lv_label_set_text(s_quote,FT_HOME);
        lv_label_set_text_fmt(s_caption,"八种主题 / %lu 张未读",(unsigned long)fortune_remaining(s));
        lv_label_set_text(s_help,FT_HOME_HELP);
        lv_label_set_text(s_hint,notice?notice:FT_HOME_HINT);
    } else if(page==FORTUNE_TOPICS) {
        lv_label_set_text(s_meta,"一签 / 选择主题");
        lv_label_set_text_fmt(s_quote,"< %s >",s->mood?FORTUNE_MOODS[s->mood]:"全库随机");
        lv_label_set_text_fmt(s_caption,"%lu 张未读 / 确定才生效",(unsigned long)fortune_remaining(s));
        lv_label_set_text(s_help,FT_TOPICS_HELP);
        lv_label_set_text(s_hint,notice?notice:FT_TOPICS_HINT);
    } else {
        if(!fortune_decode(card.quote,quote,sizeof(quote))) snprintf(quote,sizeof(quote),"暂无签文");
        char formatted[128]; format_quote(quote,formatted);
        lv_label_set_text_fmt(s_meta,"一签 / %s%s",s->mood?FORTUNE_MOODS[s->mood]:"全库随机",s_sound_enabled?"":" · 静音"); lv_label_set_text(s_quote,formatted);
        const char *citation = fortune_citation(card.quote);
        if (*citation) lv_label_set_text(s_caption,citation);
        else lv_label_set_text_fmt(s_caption,"NO.%04lu  /  %s",(unsigned long)(card.quote & ~(FORTUNE_LEGACY_QUOTE | FORTUNE_PREVIOUS_QUOTE))+1,card.quote<FORTUNE_COUNT?FORTUNE_MOODS[FORTUNE_RECORDS[card.quote].mood]:"已留签名");
        lv_label_set_text(s_help,page==FORTUNE_SHOWCASE?"留下一句，展示此刻":FT_REVEAL_HELP);
        lv_label_set_text(s_hint,notice?notice:page==FORTUNE_SHOWCASE?FT_SHOW_HINT:FT_REVEAL_HINT);
    }
    lv_obj_invalidate(s_root);
}
void fortune_ui_volume(bool visible,unsigned percent,const char *notice) {
    bool changed=s_volume_visible!=visible;
    s_volume_visible=visible; s_volume_percent=percent;
    if(visible) {
        lv_label_set_text(s_meta,"一签 / 声音");
        lv_label_set_text(s_caption,FT_VOLUME_CAPTION);
        lv_label_set_text_fmt(s_quote,"音量 / %u%%",percent);
        lv_label_set_text(s_help,FT_VOLUME_HELP);
        lv_label_set_text(s_hint,notice?notice:FT_VOLUME_HINT);
    }
    if(visible || changed) lv_obj_invalidate(s_root);
}
