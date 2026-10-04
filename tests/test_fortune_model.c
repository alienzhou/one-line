#include "fortune_model.h"
#include "fortune_pixels.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fortune_ui_host/legacy_save.h"

static void test_skins(uint32_t seed) {
    fortune_state_t s;fortune_defaults(&s,seed);
    s.current.quote=100;s.pinned=(fortune_card_t){100,2976};
    bool all[FORTUNE_NEW_ART_COUNT]={0};uint32_t previous=UINT32_MAX;
    for(unsigned i=0;i<FORTUNE_NEW_ART_COUNT*2;++i) {
        if(i==FORTUNE_NEW_ART_COUNT)memset(all,0,sizeof(all));
        fortune_remix(&s,&s.current);
        assert(s.current.quote==100 && s.pinned.quote==100 && s.pinned.art==2976);
        assert(s.current.art>=FORTUNE_LEGACY_ART_COUNT && s.current.art<FORTUNE_ART_COUNT);
        unsigned offset=s.current.art-FORTUNE_LEGACY_ART_COUNT;
        assert(!all[offset]);all[offset]=true;
        uint32_t scene=offset%FORTUNE_SCENE_COUNT;
        assert(scene!=previous);previous=scene;
        assert(fortune_valid(&s));
    }
    /* Every scene and every scene/subject skin once in their own batches. */
    fortune_defaults(&s,seed);
    for(unsigned batch=0;batch<6;++batch) {
        bool skins[FORTUNE_SKIN_COUNT]={0};
        for(unsigned block=0;block<4;++block) {
            bool scenes[FORTUNE_SCENE_COUNT]={0};
            for(unsigned j=0;j<FORTUNE_SCENE_COUNT;++j) {
                fortune_remix(&s,&s.current);
                unsigned offset=s.current.art-FORTUNE_LEGACY_ART_COUNT;
                assert(!scenes[offset%FORTUNE_SCENE_COUNT] && !skins[offset%FORTUNE_SKIN_COUNT]);
                scenes[offset%FORTUNE_SCENE_COUNT]=true;skins[offset%FORTUNE_SKIN_COUNT]=true;
            }
        }
        for(unsigned i=0;i<FORTUNE_SKIN_COUNT;++i)assert(skins[i]);
    }
}

static unsigned min_poetry=20,max_poetry;
static void test_deck(uint32_t seed) {
    fortune_state_t s;
    fortune_defaults(&s, seed);
    bool seen[FORTUNE_COUNT] = {0};
    unsigned counts[9]={0};
    s.style=0; /* An old straight-talking preference must not exclude poetry. */
    for (unsigned i = 0; i < FORTUNE_COUNT; ++i) {
        assert(fortune_draw(&s));
        assert(!seen[s.current.quote]); seen[s.current.quote] = true;
        assert(fortune_valid(&s));
        ++counts[FORTUNE_RECORDS[s.current.quote].mood];
        if (i==19) {
            if(counts[2]<min_poetry) min_poetry=counts[2];
            if(counts[2]>max_poetry) max_poetry=counts[2];
        }
    }
    assert(!fortune_draw(&s)); assert(fortune_remaining(&s) == 0);
    for(unsigned theme=1;theme<=8;++theme) assert(counts[theme]==FORTUNE_THEME_COUNTS[theme]);
    fortune_reset_deck(&s); assert(fortune_remaining(&s) == FORTUNE_COUNT);
    assert(fortune_draw(&s));
}

int main(void) {
    test_deck(1); test_deck(2199);
    for (unsigned i = 0; i < 200; ++i) test_deck(i*27449);
    assert(min_poetry==0 && max_poetry>2); /* Full-bank shuffle, no fixed per-block quotas. */
    char text[128], tiny[2];
    for (unsigned id = 0; id < FORTUNE_COUNT; ++id) {
        assert(fortune_decode(id, text, sizeof(text)) && strlen(text));
        assert(!fortune_decode(id, tiny, sizeof(tiny)) && !tiny[0]);
    }
    assert(!fortune_decode(FORTUNE_COUNT, text, sizeof(text)));
    fortune_state_t s, restored;
    fortune_defaults(&s, 42);
    bool seen[FORTUNE_COUNT] = {0};
    /* Filtering never reintroduces a card drawn under another filter. */
    for (unsigned mood = 1; mood <= 8; ++mood) {
        fortune_select_topic(&s,mood);
        while (fortune_draw(&s)) {
            unsigned id = s.current.quote;
            assert(FORTUNE_RECORDS[id].mood == mood);
            assert(!seen[id]); seen[id] = true;
        }
    }
    for (unsigned i = 0; i < FORTUNE_COUNT; ++i) assert(seen[i]);
    s.pinned = s.current;
    uint8_t bytes[48+FORTUNE_SEEN_BYTES];
    size_t n = fortune_encode_state(&s, bytes, sizeof(bytes)); assert(n == sizeof(bytes));
    assert(fortune_decode_state(&restored, bytes, n));
    assert(memcmp(s.seen, restored.seen, sizeof(s.seen)) == 0);
    assert(s.pinned.quote == restored.pinned.quote && s.pinned.art == restored.pinned.art);
    assert(!fortune_decode_state(&restored, bytes, n-1));
    for (size_t i = 0; i < n; ++i) {
        bytes[i] ^= 1; assert(!fortune_decode_state(&restored, bytes, n)); bytes[i] ^= 1;
    }
    for(unsigned seed=0;seed<64;++seed)test_skins(seed*27449);
    assert(fortune_decode_state(&restored,LEGACY_SAVE,sizeof(LEGACY_SAVE)));
    fortune_card_t legacy_pin=restored.pinned;
    assert(restored.art_cursor<FORTUNE_LEGACY_ART_COUNT);
    uint8_t old_seen[FORTUNE_SEEN_BYTES];memcpy(old_seen,restored.seen,sizeof(old_seen));
    fortune_remix(&restored,&restored.current);
    assert(restored.current.art>=FORTUNE_LEGACY_ART_COUNT);
    assert(restored.pinned.quote==legacy_pin.quote && restored.pinned.art==legacy_pin.art);
    assert(!memcmp(old_seen,restored.seen,sizeof(old_seen)));
    assert(legacy_pin.quote == (FORTUNE_LEGACY_QUOTE | 387U));
    char legacy_text[128];
    assert(fortune_decode(legacy_pin.quote, legacy_text, sizeof(legacy_text)));
    assert(!strcmp(legacy_text,"欢迎沟通，拒绝精神卸货"));
    assert(!*fortune_citation(legacy_pin.quote));
    n=fortune_encode_state(&restored,bytes,sizeof(bytes));assert(n==sizeof(bytes));
    assert(fortune_decode_state(&s,bytes,n));
    assert(s.art_cursor==restored.art_cursor && s.pinned.art==legacy_pin.art);
    /* Retained comfort quotes keep their seen bits when importing an old bank. */
    for (unsigned id=0; id<FORTUNE_COUNT; ++id) {
        unsigned old=FORTUNE_RETAINED_IDS[id];
        bool expected=old<FORTUNE_LEGACY_COUNT && (LEGACY_SAVE[44+old/8]&(1U<<(old%8)));
        assert(((s.seen[id/8]>>(id%8))&1U)==expected);
    }
    fortune_state_t a,b;fortune_defaults(&a,42);fortune_defaults(&b,43);
    a.style=0; fortune_select_topic(&a,2);
    assert(a.mood==2 && a.style==FORTUNE_ANY_STYLE && fortune_remaining(&a)==220);
    fortune_select_topic(&a,9); assert(a.mood==2);
    fortune_select_topic(&a,0);
    unsigned differences=0;
    for(unsigned i=0;i<FORTUNE_SKIN_COUNT;++i){fortune_remix(&a,&a.current);fortune_remix(&b,&b.current);differences+=a.current.art!=b.current.art;}
    assert(differences>100);
    printf("Fortune model: PASS (202 whole-bank text decks, 20-card poetry counts %u..%u; 64 seeds, %u nonrepeating skins / %u appearances, different adjacent scenes, legacy save retained)\n",min_poetry,max_poetry,FORTUNE_SKIN_COUNT,FORTUNE_NEW_ART_COUNT);
    return 0;
}
