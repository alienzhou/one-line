#include "fortune_model.h"
#include "fortune_ui_host/rare_v3_save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void roundtrip(fortune_state_t *s) {
    uint8_t bytes[FORTUNE_STATE_BYTES]; fortune_state_t next;
    assert(fortune_encode_state(s,bytes,sizeof(bytes))==sizeof(bytes));
    assert(fortune_decode_state(&next,bytes,sizeof(bytes)));
    assert(!memcmp(s,&next,sizeof(next)));
    *s=next;
}
static void checksum(uint8_t *bytes,size_t n) {
    uint32_t crc=fortune_crc(bytes,n-4);
    for(unsigned i=0;i<4;++i) bytes[n-4+i]=(uint8_t)(crc>>(i*8));
}
int main(void) {
    unsigned early=0,at_ten=0,max_gap=0;
    for(unsigned seed=0;seed<512;++seed) {
        fortune_state_t s; fortune_defaults(&s,seed*193U);
        unsigned since=0,hits=0,last=FORTUNE_RARE_NONE; uint32_t discovered=0;
        while(hits<FORTUNE_RARE_COUNT+1) {
            unsigned unread=fortune_remaining(&s); ++since;
            assert(fortune_draw_surprise(&s));
            unsigned rare=fortune_rare_index(s.current.quote);
            if(rare<FORTUNE_RARE_COUNT) {
                assert(since<=(hits?30:10));
                if(!hits) { early+=since<10; at_ten+=since==10; }
                else if(since>max_gap) max_gap=since;
                if(hits<FORTUNE_RARE_COUNT) assert(!(discovered & (1U<<rare)));
                assert(rare!=last); last=rare;
                discovered|=(1U<<rare); ++hits; since=0;
                assert(s.rare_misses==0 && unread==fortune_remaining(&s));
                assert(s.favorite_count==0);
            } else assert(s.rare_misses==since && fortune_remaining(&s)==unread-1);
            /* Reboot every draw; discovery and pity must actually survive. */
            roundtrip(&s);
        }
        assert(discovered==FORTUNE_RARE_MASK && fortune_rare_owned(&s)==FORTUNE_RARE_COUNT);
        for(unsigned i=0;i<FORTUNE_RARE_COUNT;++i) assert(fortune_rare_at(&s,i)==i);
        assert(fortune_rare_at(&s,FORTUNE_RARE_COUNT)==FORTUNE_RARE_NONE);
    }
    assert(early && at_ten && max_gap==30);
    fortune_state_t s; fortune_defaults(&s,71);
    s.rare_misses=9; assert(fortune_draw_surprise(&s));
    unsigned first=fortune_rare_index(s.current.quote); assert(first<FORTUNE_RARE_COUNT);
    s.pinned=s.current; fortune_card_t original=s.current;
    unsigned rng=s.rare_random;
    for(unsigned i=0;i<3;++i) { fortune_remix(&s,&s.current); roundtrip(&s); }
    assert(!memcmp(&original,&s.current,sizeof(original)) && s.rare_random==rng);
    fortune_reset_deck(&s); fortune_select_topic(&s,2);
    assert(s.rare_random==rng && s.rare_unlocked==(1U<<first));
    s.rare_misses=29; assert(fortune_draw_surprise(&s)); assert(s.rare_last!=first);
    memset(s.seen,0xFF,sizeof(s.seen)); fortune_state_t before=s;
    assert(!fortune_draw_surprise(&s) && !memcmp(&before,&s,sizeof(s)));
    /* Common skin operations and filter navigation must not reroll an encounter. */
    fortune_defaults(&s,811); fortune_state_t other=s;
    for(unsigned i=0;i<200;++i) {
        fortune_card_t card={0,0}; fortune_remix(&other,&card); fortune_reset_deck(&other);
        fortune_select_topic(&other,2); fortune_select_topic(&other,0);
        assert(fortune_draw_surprise(&s) && fortune_draw_surprise(&other));
        assert(s.rare_random==other.rare_random && s.rare_misses==other.rare_misses && s.rare_unlocked==other.rare_unlocked);
        if(fortune_rare_index(s.current.quote)<FORTUNE_RARE_COUNT) assert(s.current.quote==other.current.quote);
    }
    /* Measure the base roll independently of the deliberate pity boost. */
    fortune_defaults(&s,98231); s.rare_unlocked=FORTUNE_RARE_MASK; s.rare_last=0;
    unsigned random_hits=0;
    for(unsigned i=0;i<50000;++i) {
        s.rare_misses=0;
        if(!fortune_remaining(&s)) fortune_reset_deck(&s);
        assert(fortune_draw_surprise(&s));
        random_hits+=fortune_rare_index(s.current.quote)<FORTUNE_RARE_COUNT;
    }
    assert(random_hits>400 && random_hits<600);
    /* Upgrade an actual V2-shaped record without touching any existing fields. */
    fortune_defaults(&s,992); for(unsigned i=0;i<16;++i) {
        assert(fortune_draw(&s)); assert(fortune_favorite_save(&s,s.current,i));
    }
    s.pinned=s.current;
    uint8_t bytes[FORTUNE_STATE_BYTES]; assert(fortune_encode_state(&s,bytes,sizeof(bytes)));
    bytes[0]=0x32; checksum(bytes,FORTUNE_V2_STATE_BYTES);
    assert(fortune_decode_state(&other,bytes,FORTUNE_V2_STATE_BYTES));
    assert(!memcmp(&s,&other,sizeof(s)) && other.rare_unlocked==0);
    other.rare_misses=9; assert(fortune_draw_surprise(&other)); assert(fortune_rare_owned(&other)==1);
    assert(other.favorite_count==16 && !memcmp(other.favorites,s.favorites,sizeof(s.favorites)));
    assert(fortune_encode_state(&other,bytes,sizeof(bytes)));
    for(unsigned i=0;i<sizeof(bytes);++i) {
        bytes[i]^=1; assert(!fortune_decode_state(&s,bytes,sizeof(bytes))); bytes[i]^=1;
    }
    /* Import the actual previous six-card save: every old palette ID is stable,
     * the full ordinary album survives, and already-earned pity is not reset. */
    assert(fortune_decode_state(&s,RARE_V3_SAVE,sizeof(RARE_V3_SAVE)));
    assert(s.favorite_count==16 && s.rare_unlocked==63 && s.rare_last==5);
    assert(s.rare_misses==29 && s.rare_random==(82173U^0xB5297A4DU));
    assert(s.pinned.quote==FORTUNE_RARE_QUOTE+5 && s.pinned.art==FORTUNE_RARE_ART_BASE+17);
    fortune_card_t prior_pin=s.pinned; fortune_card_t prior_album[16];
    memcpy(prior_album,s.favorites,sizeof(prior_album));
    roundtrip(&s); assert(fortune_draw_surprise(&s));
    assert(s.rare_last>=6 && s.rare_last<FORTUNE_RARE_COUNT && s.rare_misses==0);
    assert(!memcmp(&prior_pin,&s.pinned,sizeof(prior_pin)) && !memcmp(prior_album,s.favorites,sizeof(prior_album)));
    uint8_t old[sizeof(RARE_V3_SAVE)];
    for(unsigned misses=0;misses<60;++misses) {
        memcpy(old,RARE_V3_SAVE,sizeof(old)); old[sizeof(old)-8]=(uint8_t)misses;
        checksum(old,sizeof(old)); assert(fortune_decode_state(&s,old,sizeof(old)));
        assert(s.rare_misses==(misses>=30?29:misses));
    }
    memcpy(old,RARE_V3_SAVE,sizeof(old)); old[sizeof(old)-7]=64;
    checksum(old,sizeof(old)); assert(!fortune_decode_state(&s,old,sizeof(old)));
    for(unsigned i=0;i<FORTUNE_RARE_COUNT;++i) {
        char text[128],small[2],joined[128]; unsigned n=0;
        const char *lines=fortune_rare_display_text(i);
        for(unsigned j=0;lines[j];++j) if(lines[j]!='\n') joined[n++]=lines[j];
        joined[n]=0;
        assert(fortune_decode(FORTUNE_RARE_QUOTE+i,text,sizeof(text)) && !strcmp(text,joined));
        assert(!fortune_decode(FORTUNE_RARE_QUOTE+i,small,sizeof(small)) && !small[0]);
        for(unsigned tone=0;tone<FORTUNE_RARE_PALETTES;++tone) {
            uint32_t art=fortune_rare_art_id(i,tone);
            assert(fortune_rare_art_index(art)==i && fortune_rare_art_tone(art)==tone);
            if(i<6) assert(art==FORTUNE_RARE_ART_BASE+i+tone*6);
        }
    }
    /* Even with a recomputed checksum, impossible pity/unlock states are rejected. */
    unsigned offsets[]={sizeof(bytes)-8,sizeof(bytes)-7,sizeof(bytes)-6,sizeof(bytes)-5,sizeof(bytes)-9};
    for(unsigned i=0;i<5;++i) {
        fortune_encode_state(&other,bytes,sizeof(bytes)); bytes[offsets[i]]=255;
        checksum(bytes,sizeof(bytes)); assert(!fortune_decode_state(&s,bytes,sizeof(bytes)));
    }
    printf("Rare encounters: PASS (512 rebooting users; first <=10, later <=30; %u unique discoveries; base %u/50000; V2/V3 migration, full album, CRC, remix and shuffle isolation)\n",FORTUNE_RARE_COUNT,random_hits);
}
