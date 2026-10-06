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
static char s_final_meta[96];
static lv_timer_t *s_timer;
static void (*s_sound_callback)(fortune_sound_t,unsigned);
static bool s_sound_enabled=true;
static bool s_volume_visible;
static unsigned s_volume_percent;
static unsigned s_album_index,s_album_action,s_album_count;
static bool s_album_confirm;
static unsigned s_turn_step=8,s_keep_step=12;
static int s_turn_direction;
static bool is_album(fortune_page_t page) { return page>=FORTUNE_ALBUM && page<=FORTUNE_ALBUM_CONFIRM; }
void fortune_ui_album(unsigned index,unsigned action,bool confirm) {
    s_album_index=index; s_album_action=action; s_album_confirm=confirm;
}
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
    if (fortune_rare_art(s_id)) {
        fortune_colors_t p=fortune_pixel_colors(s_id);
        lv_draw_rect_dsc_t trim; lv_draw_rect_dsc_init(&trim);
        trim.bg_opa=LV_OPA_TRANSP; trim.border_color=lv_color_hex(p.accent);
        trim.border_opa=LV_OPA_50; trim.border_width=1; trim.radius=4;
        lv_area_t outer={7,7,232,274}; lv_draw_rect(lv_event_get_layer(event),&trim,&outer);
        trim.border_opa=LV_OPA_20; trim.radius=1;
        lv_area_t inner={10,10,229,271}; lv_draw_rect(lv_event_get_layer(event),&trim,&inner);
        trim.border_width=0; trim.bg_opa=LV_OPA_COVER; trim.bg_color=lv_color_hex(p.accent);
        for(int i=0;i<3;++i) {
            int r=i==1?2:1,x=111+i*9;
            lv_area_t jewel={x-r,262-r,x+r,262+r}; lv_draw_rect(lv_event_get_layer(event),&trim,&jewel);
        }
    }
    lv_draw_image_dsc_t d; lv_draw_image_dsc_init(&d);
    d.src=&s_image; d.scale_x=d.scale_y=512; d.pivot.x=d.pivot.y=0; d.antialias=0;
    static const int turn_offset[]={10,8,6,4,2,1,0,0,0};
    int dx=s_turn_direction*turn_offset[s_turn_step];
    lv_area_t a={12+dx,38,12+dx+FORTUNE_PIXEL_W-1,38+FORTUNE_PIXEL_H-1};
    lv_draw_image(lv_event_get_layer(event),&d,&a);
    if (is_album(s_page) && s_album_count && s_page!=FORTUNE_ALBUM_ACTIONS) {
        fortune_colors_t p=fortune_pixel_colors(s_id);
        lv_draw_rect_dsc_t dot; lv_draw_rect_dsc_init(&dot);
        dot.bg_color=lv_color_hex(p.paper); dot.bg_opa=LV_OPA_80; dot.radius=5;
        lv_area_t strip={32,140,207,153}; lv_draw_rect(lv_event_get_layer(event),&dot,&strip);
        for (unsigned i=0; i<FORTUNE_FAVORITE_CAPACITY; ++i) {
            bool selected=i==s_album_index;
            dot.bg_color=lv_color_hex(p.ink); dot.bg_opa=i<s_album_count?LV_OPA_COVER:LV_OPA_20;
            dot.radius=selected?2:1;
            lv_area_t mark={40+(int)i*10,selected?143:145,44+(int)i*10,selected?150:148};
            lv_draw_rect(lv_event_get_layer(event),&dot,&mark);
        }
    }
    if (s_page==FORTUNE_ALBUM_ACTIONS) {
        fortune_colors_t p=fortune_pixel_colors(s_id);
        lv_draw_rect_dsc_t focus; lv_draw_rect_dsc_init(&focus);
        focus.bg_color=lv_color_hex(p.ink); focus.bg_opa=LV_OPA_10; focus.radius=6;
        int y=179+(int)s_album_action*26;
        lv_area_t row={16,y,223,y+25}; lv_draw_rect(lv_event_get_layer(event),&focus,&row);
    }
    if (s_keep_step<12) {
        fortune_colors_t p=fortune_pixel_colors(s_id);
        lv_draw_rect_dsc_t seal; lv_draw_rect_dsc_init(&seal);
        seal.bg_color=lv_color_hex(p.ink); seal.bg_opa=s_keep_step<8?LV_OPA_90:(12-s_keep_step)*255/5;
        seal.radius=3;
        int y=132-(s_keep_step<4?(4-(int)s_keep_step)*2:0);
        lv_area_t tag={191,y,209,y+18}; lv_draw_rect(lv_event_get_layer(event),&seal,&tag);
        seal.bg_color=lv_color_hex(p.paper); seal.radius=1;
        lv_area_t tick1={195,y+9,199,y+12},tick2={199,y+5,202,y+10},tick3={202,y+3,205,y+6};
        lv_draw_rect(lv_event_get_layer(event),&seal,&tick1);
        lv_draw_rect(lv_event_get_layer(event),&seal,&tick2);
        lv_draw_rect(lv_event_get_layer(event),&seal,&tick3);
    }
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
    if(s_revealing || s_wipe_step<3 || s_turn_step<8 || s_keep_step<12) return;
    fortune_pixels(s_pixels,s_id,++s_frame);
    lv_image_cache_drop(&s_image);
    lv_area_t area={12,38,227,153}; lv_obj_invalidate_area(s_root,&area);
}
void fortune_ui_motion(bool enabled) { s_motion=enabled; }
bool fortune_ui_revealing(void) { return s_revealing; }
static void reset_turn(void) {
    s_turn_step=8;
    lv_obj_set_pos(s_quote,22,181); lv_obj_set_pos(s_caption,22,161);
    lv_obj_set_style_text_opa(s_quote,LV_OPA_COVER,0);
    lv_obj_set_style_text_opa(s_caption,LV_OPA_COVER,0);
}
void fortune_ui_turn(int direction) {
    if (!s_root || s_revealing) return;
    reset_turn(); s_keep_step=12; s_wipe_step=3; s_turn_step=0;
    s_turn_direction=direction<0?-1:1;
    lv_obj_set_pos(s_quote,22+s_turn_direction*10,181);
    lv_obj_set_pos(s_caption,22+s_turn_direction*10,161);
    lv_obj_set_style_text_opa(s_quote,LV_OPA_60,0);
    lv_obj_set_style_text_opa(s_caption,LV_OPA_60,0);
    lv_timer_set_period(s_timer,25); lv_timer_reset(s_timer); lv_obj_invalidate(s_root);
}
void fortune_ui_kept(void) {
    if (!s_root) return;
    reset_turn(); s_keep_step=0; s_wipe_step=3;
    lv_timer_set_period(s_timer,40); lv_timer_reset(s_timer); lv_obj_invalidate(s_root);
}
static void finish_reveal(bool audible) {
    if(!s_revealing) return;
    s_revealing=false;
    lv_label_set_text(s_meta,s_final_meta);
    lv_label_set_text(s_quote,s_final_quote); lv_label_set_text(s_caption,s_final_caption);
    lv_label_set_text(s_help,s_final_help); lv_label_set_text(s_hint,s_final_hint);
    fortune_pixels(s_pixels,s_id,0); lv_image_cache_drop(&s_image); lv_obj_invalidate(s_root);
    if(audible && s_sound_callback)
        s_sound_callback(fortune_rare_art(s_id)?FORTUNE_SOUND_RARE:FORTUNE_SOUND_REVEAL,s_id);
}
void fortune_ui_finish_reveal(void) { finish_reveal(true); }
void fortune_ui_begin_reveal(void) {
    reset_turn(); s_keep_step=12; lv_timer_set_period(s_timer,125);
    snprintf(s_final_quote,sizeof(s_final_quote),"%s",lv_label_get_text(s_quote));
    snprintf(s_final_meta,sizeof(s_final_meta),"%s",lv_label_get_text(s_meta));
    snprintf(s_final_caption,sizeof(s_final_caption),"%s",lv_label_get_text(s_caption));
    snprintf(s_final_help,sizeof(s_final_help),"%s",lv_label_get_text(s_help));
    snprintf(s_final_hint,sizeof(s_final_hint),"%s",lv_label_get_text(s_hint));
    s_revealing=true; s_reveal_step=0;
    lv_timer_reset(s_timer);
    s_wipe_step=3;
    bool rare=fortune_rare_art(s_id);
    if (rare) lv_label_set_text(s_meta,"一签 / 特别来信");
    lv_label_set_text(s_quote,rare?"有一封信\n正闪着光":"生活来信\n正在路上");
    lv_label_set_text(s_caption,rare?"星光邮戳 / 特别来信":"给此刻的你 / 待拆封");
    lv_label_set_text(s_help,rare?"这一次，遇见一点不平凡":"一点未知，一点期待");
    lv_label_set_text(s_hint,"按确定可直接拆开");
    fortune_pixel_unwrap(s_pixels,s_id,0); lv_image_cache_drop(&s_image); lv_obj_invalidate(s_root);
    if(s_sound_callback) s_sound_callback(FORTUNE_SOUND_OPEN,s_id);
}
static void timer_tick(lv_timer_t *timer) {
    (void)timer;
    if(s_turn_step<8) {
        static const int offset[]={10,8,6,4,2,1,0,0,0};
        ++s_turn_step;
        lv_obj_set_pos(s_quote,22+s_turn_direction*offset[s_turn_step],181);
        lv_obj_set_pos(s_caption,22+s_turn_direction*offset[s_turn_step],161);
        lv_obj_set_style_text_opa(s_quote,(lv_opa_t)(153+s_turn_step*102/8),0);
        lv_obj_set_style_text_opa(s_caption,(lv_opa_t)(153+s_turn_step*102/8),0);
        if(s_turn_step==8) { reset_turn(); lv_timer_set_period(s_timer,125); }
        lv_obj_invalidate(s_root);
    } else if(s_keep_step<12) {
        if(++s_keep_step==12) lv_timer_set_period(s_timer,125);
        lv_area_t area={188,120,213,154}; lv_obj_invalidate_area(s_root,&area);
    } else if(s_revealing) {
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
    s_turn_step=8; s_keep_step=12;
    lv_screen_load(s_root); return true;
}
void fortune_ui_update(const fortune_state_t *s,fortune_page_t page,int battery,const char *notice) {
    if (s_page==FORTUNE_MAIL) reset_turn();
    fortune_card_t card=is_album(page) && page!=FORTUNE_ALBUM_CONFIRM && s->favorite_count &&
        s_album_index<s->favorite_count?s->favorites[s_album_index]:page==FORTUNE_SHOWCASE?s->pinned:s->current;
    if (page==FORTUNE_RARE_ALBUM) card=fortune_rare_card(fortune_rare_at(s,s_album_index));
    if (page==FORTUNE_ALBUM && !s->favorite_count) card.quote=FORTUNE_NO_CARD;
    uint32_t id=page==FORTUNE_HOME||page==FORTUNE_TOPICS||card.quote==FORTUNE_NO_CARD?2976:card.art;
    bool navigation=page!=s_page || card.quote!=s_quote_id || id!=s_id;
    if(navigation) { reset_turn(); s_keep_step=12; lv_timer_set_period(s_timer,125); }
    s_album_count=s->favorite_count;
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
        lv_label_set_text_fmt(s_caption,"短句与连续信 / %lu 张未读",(unsigned long)fortune_remaining(s));
        lv_label_set_text(s_help,FT_HOME_HELP);
        lv_label_set_text(s_hint,notice?notice:s->rare_unlocked?FT_HOME_HINT:"十抽内必遇 · 长确定签册");
    } else if(page==FORTUNE_TOPICS) {
        lv_label_set_text(s_meta,"一签 / 选择类型");
        lv_label_set_text_fmt(s_quote,"< %s >",s->mood?FORTUNE_MOODS[s->mood]:"全库随机");
        if (s->mood==FORTUNE_MAIL_TOPIC) lv_label_set_text(s_caption,"四组连续故事 / 先看看再选定");
        else if (s->mood==FORTUNE_RARE_TOPIC) lv_label_set_text_fmt(s_caption,"已发现 %u/%u / 永久保留原版",fortune_rare_owned(s),FORTUNE_RARE_COUNT);
        else lv_label_set_text_fmt(s_caption,"%lu 张未读 / 确定才生效",(unsigned long)fortune_remaining(s));
        lv_label_set_text(s_help,FT_TOPICS_HELP);
        lv_label_set_text(s_hint,notice?notice:FT_TOPICS_HINT);
    } else if (page==FORTUNE_RARE_ALBUM && !s->rare_unlocked) {
        lv_label_set_text_fmt(s_meta,"一签 / 奇遇珍藏 0/%u",FORTUNE_RARE_COUNT);
        lv_label_set_text(s_quote,"特别的来信\n会在途中相遇");
        lv_label_set_text(s_caption,"首次十抽内必得 / 自动珍藏");
        lv_label_set_text(s_help,"确定回首页，拆一封来信");
        lv_label_set_text(s_hint,notice?notice:"之后概率 1% · 三十抽保底");
    } else if (page==FORTUNE_ALBUM && !s->favorite_count) {
        lv_label_set_text(s_meta,"一签 / 签册 0/16");
        lv_label_set_text(s_quote,FT_ALBUM_EMPTY);
        lv_label_set_text(s_caption,"每句喜欢的话，都有一个位置");
        lv_label_set_text(s_help,"确定回首页，拆一封来信");
        lv_label_set_text(s_hint,notice?notice:"长确定也可返回首页");
    } else if (page==FORTUNE_ALBUM_ACTIONS) {
        static const char *const actions[]={"展示这张","换个外观","移出签册"};
        lv_label_set_text_fmt(s_meta,"一签 / 整理 %02u/%02u",s_album_index+1,s->favorite_count);
        lv_label_set_text_fmt(s_caption,"已收藏 %u/16 / 选择一个操作",s->favorite_count);
        lv_label_set_text_fmt(s_quote,"%s%s\n%s%s\n%s%s",
            s_album_action==0?"> ":"   ",actions[0],s_album_action==1?"> ":"   ",actions[1],
            s_album_action==2?"> ":"   ",actions[2]);
        lv_label_set_text(s_help,"上/下选择 · 确定执行");
        lv_label_set_text(s_hint,notice?notice:"长确定返回签册");
    } else {
        if(!fortune_decode(card.quote,quote,sizeof(quote))) snprintf(quote,sizeof(quote),"暂无签文");
        char formatted[128]; format_quote(quote,formatted);
        unsigned rare=fortune_rare_index(card.quote);
        if (is_album(page)) {
            if (page==FORTUNE_ALBUM_CONFIRM) lv_label_set_text_fmt(s_meta,"新签 / 替换第 %02u 张？",s_album_index+1);
            else if (page==FORTUNE_ALBUM_REMOVE) lv_label_set_text_fmt(s_meta,"移出第 %02u 张？",s_album_index+1);
            else lv_label_set_text_fmt(s_meta,"%s / %02u/%02u",page==FORTUNE_ALBUM_REPLACE?"替换":"签册",s_album_index+1,s->favorite_count);
        } else if (rare<FORTUNE_RARE_COUNT)
            lv_label_set_text_fmt(s_meta,"奇遇 / %s",fortune_rare_title(rare));
        else lv_label_set_text_fmt(s_meta,"一签 / %s%s",s->mood?FORTUNE_MOODS[s->mood]:"全库随机",s_sound_enabled?"":" · 静音");
        lv_label_set_text(s_quote,rare<FORTUNE_RARE_COUNT?fortune_rare_display_text(rare):formatted);
        const char *citation = fortune_citation(card.quote);
        if (rare<FORTUNE_RARE_COUNT)
            lv_label_set_text_fmt(s_caption,"珍藏 %02u/%02u / 已发现 %u 款",rare+1,FORTUNE_RARE_COUNT,fortune_rare_owned(s));
        else if (*citation) lv_label_set_text(s_caption,citation);
        else lv_label_set_text_fmt(s_caption,"NO.%04lu  /  %s",(unsigned long)(card.quote & ~(FORTUNE_LEGACY_QUOTE | FORTUNE_PREVIOUS_QUOTE))+1,card.quote<FORTUNE_COUNT?FORTUNE_MOODS[FORTUNE_RECORDS[card.quote].mood]:"已留签名");
        if (page==FORTUNE_RARE_ALBUM) {
            lv_label_set_text(s_help,"上/下翻阅 · 确定设为签名");
            lv_label_set_text(s_hint,notice?notice:"原版永久保留 · 长确定首页");
        } else if (rare<FORTUNE_RARE_COUNT && !is_album(page)) {
            lv_label_set_text(s_help,page==FORTUNE_REVEAL?"上再抽 · 下换色 · 确定留签":FT_SHOW_HELP);
            lv_label_set_text(s_hint,notice?notice:page==FORTUNE_REVEAL?"已自动珍藏 · 类型页可找回":FT_SHOW_HINT);
        } else if (page==FORTUNE_ALBUM_REMOVE || page==FORTUNE_ALBUM_CONFIRM) {
            lv_label_set_text_fmt(s_help,"%s取消    %s%s",s_album_confirm?"  ":"> ",s_album_confirm?"> ":"  ",
                page==FORTUNE_ALBUM_REMOVE?"移出签册":"确定替换");
            lv_label_set_text(s_hint,notice?notice:"上/下选择 · 长确定返回");
        } else {
            lv_label_set_text(s_help,page==FORTUNE_ALBUM?FT_ALBUM_HELP:page==FORTUNE_ALBUM_REPLACE?
                FT_ALBUM_REPLACE_HELP:page==FORTUNE_SHOWCASE?FT_SHOW_HELP:FT_REVEAL_HELP);
            lv_label_set_text(s_hint,notice?notice:page==FORTUNE_ALBUM?FT_ALBUM_HINT:
                page==FORTUNE_ALBUM_REPLACE?FT_ALBUM_REPLACE_HINT:page==FORTUNE_SHOWCASE?FT_SHOW_HINT:FT_REVEAL_HINT);
        }
    }
    lv_obj_invalidate(s_root);
}
void fortune_ui_mail(const fortune_mail_t *s,const fortune_mail_view_t *v,int battery,const char *notice) {
    finish_reveal(false); reset_turn(); s_keep_step=12; s_wipe_step=3;
    s_volume_visible=false; s_page=FORTUNE_MAIL;
    uint32_t id=fortune_mail_art(v->story,v->page,v->style);
    if (id!=s_id) {
        s_id=id; s_frame=0; fortune_pixels(s_pixels,id,0); lv_image_cache_drop(&s_image);
    }
    s_quote_id=FORTUNE_NO_CARD;
    fortune_colors_t palette=fortune_pixel_colors(id);
    lv_timer_set_period(s_timer,125);
    lv_obj_set_style_bg_color(s_root,lv_color_hex(palette.paper),0);
    lv_obj_t *labels[]={s_meta,s_battery,s_quote,s_caption,s_help,s_hint};
    for (unsigned i=0;i<6;++i) lv_obj_set_style_text_color(labels[i],lv_color_hex(i<3?palette.ink:palette.muted),0);
    if (battery<0) lv_label_set_text(s_battery,"--%"); else lv_label_set_text_fmt(s_battery,"%d%%",battery);
    unsigned count=fortune_mail_pages(v->story);
    unsigned bookmark=v->story<FORTUNE_MAIL_STORIES?s->bookmarks[v->story]:0;
    lv_label_set_text(s_quote,fortune_mail_text(v->story,v->reading?v->page:0));
    if (!v->reading) {
        lv_label_set_text(s_meta,"一签 / 连续来信");
        lv_label_set_text_fmt(s_caption,"%s / %u 封",fortune_mail_title(v->story),count/FORTUNE_MAIL_PAGES_PER_LETTER);
        lv_label_set_text(s_help,bookmark && bookmark<=count?
            "上换组 · 下换肤 · 确定续读":"上换组 · 下换肤 · 确定读信");
        if (notice) lv_label_set_text(s_hint,notice);
        else if (bookmark>count) lv_label_set_text(s_hint,"已读完 · 确定重读 · 长确定首页");
        else if (bookmark) lv_label_set_text_fmt(s_hint,"读到第 %u 页 · 长确定首页",bookmark);
        else lv_label_set_text(s_hint,"先看看，喜欢再读 · 长确定首页");
    } else {
        lv_label_set_text_fmt(s_meta,"连续来信 / %s",fortune_mail_title(v->story));
        lv_label_set_text_fmt(s_caption,"第 %u/%u 封 / %u/%u 页",v->page/FORTUNE_MAIL_PAGES_PER_LETTER+1,
            count/FORTUNE_MAIL_PAGES_PER_LETTER,v->page%FORTUNE_MAIL_PAGES_PER_LETTER+1,FORTUNE_MAIL_PAGES_PER_LETTER);
        lv_label_set_text(s_help,v->page+1==count?"上回看 · 下换肤 · 确定读完":"上回看 · 下换肤 · 确定下一页");
        lv_label_set_text(s_hint,notice?notice:"进度自动保存 · 长确定首页");
    }
    lv_obj_invalidate(s_root);
}

void fortune_ui_volume(bool visible,unsigned percent,const char *notice) {
    bool changed=s_volume_visible!=visible;
    s_volume_visible=visible; s_volume_percent=percent;
    if(visible) {
        reset_turn(); s_keep_step=12; lv_timer_set_period(s_timer,125);
        lv_label_set_text(s_meta,"一签 / 声音");
        lv_label_set_text(s_caption,FT_VOLUME_CAPTION);
        lv_label_set_text_fmt(s_quote,"音量 / %u%%",percent);
        lv_label_set_text(s_help,FT_VOLUME_HELP);
        lv_label_set_text(s_hint,notice?notice:FT_VOLUME_HINT);
    }
    if(visible || changed) lv_obj_invalidate(s_root);
}
