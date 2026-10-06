#include "fortune_mail.h"
#include "fortune_art.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void roundtrip(const fortune_mail_t *s) {
    uint8_t bytes[FORTUNE_MAIL_BYTES]; fortune_mail_t restored;
    assert(fortune_mail_encode(s,bytes,sizeof(bytes))==sizeof(bytes));
    assert(fortune_mail_decode(&restored,bytes,sizeof(bytes)));
    assert(!memcmp(s,&restored,sizeof(*s)));
    for (unsigned i=0;i<sizeof(bytes);++i) for (unsigned b=0;b<8;++b) {
        bytes[i]^=1U<<b; assert(!fortune_mail_decode(&restored,bytes,sizeof(bytes)));
        assert(!memcmp(s,&restored,sizeof(*s))); bytes[i]^=1U<<b;
    }
    assert(!fortune_mail_decode(&restored,bytes,sizeof(bytes)-1));
}
static void short_text(const char *text) {
    unsigned lines=1,width=0,total=0;
    for (const unsigned char *p=(const unsigned char *)text;*p;++p) {
        if (*p=='\n') { assert(width<=9); width=0; ++lines; }
        else if ((*p&0xc0)!=0x80) { ++width; ++total; }
    }
    assert(lines<=2 && width<=9 && total<=18 && total>0);
}
static void checksum(uint8_t *bytes) {
    uint32_t h=2166136261U;
    for (unsigned i=0;i<12;++i) h=(h^bytes[i])*16777619U;
    for (unsigned i=0;i<4;++i) bytes[12+i]=(uint8_t)(h>>(i*8));
}
int main(void) {
    fortune_mail_t s; fortune_mail_defaults(&s); roundtrip(&s);
    assert(!s.active && s.selected==FORTUNE_MAIL_NONE);
    unsigned total=0;
    const unsigned covers[]={68,30,21,10};
    for (unsigned story=0;story<FORTUNE_MAIL_STORIES;++story) {
        assert((fortune_mail_art(story,0,0)-FORTUNE_LEGACY_ART_COUNT)%FORTUNE_SCENE_COUNT==covers[story]);
        assert(fortune_mail_select(&s,story));
        assert(s.selected==story && s.bookmarks[story]==1 && s.active);
        unsigned count=fortune_mail_pages(story); assert(count>=24 && count%6==0); total+=count;
        assert(!fortune_mail_step(&s,-1));
        for (unsigned page=0;page<count;++page) {
            roundtrip(&s); short_text(fortune_mail_text(story,page));
            assert(s.bookmarks[story]==page+1);
            fortune_mail_view_t v; fortune_mail_restore_view(&s,&v);
            assert(v.reading && v.story==story && v.page==page);
            if (page) { assert(fortune_mail_step(&s,-1)); assert(fortune_mail_step(&s,1)); }
            for (unsigned style=0;style<24;++style) {
                uint32_t art=fortune_mail_art(story,page,style);
                assert(art>=FORTUNE_LEGACY_ART_COUNT && art<FORTUNE_ART_COUNT);
            }
            assert(fortune_mail_step(&s,1));
        }
        assert(!s.active && s.bookmarks[story]==count+1); roundtrip(&s);
        assert(!fortune_mail_step(&s,1));
        for (unsigned previous=0;previous<story;++previous)
            assert(s.bookmarks[previous]==fortune_mail_pages(previous)+1);
    }
    assert(total==120);
    assert(fortune_mail_select(&s,1) && s.bookmarks[1]==1); /* Finished stories can be reread. */
    assert(fortune_mail_step(&s,1)); s.active=false;
    assert(fortune_mail_select(&s,2)); assert(fortune_mail_select(&s,1));
    assert(s.bookmarks[1]==2); /* Changing stories keeps independent bookmarks. */
    for (unsigned seed=0;seed<1000;++seed) {
        uint8_t bag=0,last=FORTUNE_MAIL_NONE;
        for (unsigned cycle=0;cycle<10;++cycle) {
            unsigned seen=0;
            for (unsigned i=0;i<FORTUNE_MAIL_STORIES;++i) {
                uint8_t next=fortune_mail_draw(&bag,last,seed*65537U+i*113+cycle*77);
                assert(next<FORTUNE_MAIL_STORIES && next!=last && !(seen&(1U<<next)));
                seen|=1U<<next; last=next;
            }
            assert(seen==15 && !bag);
        }
    }
    /* Migrate every valid old reading/receipt checkpoint without touching a fortune. */
    for (unsigned delivered=0;delivered<=8;++delivered) for (unsigned sent=0;sent<2;++sent)
        for (unsigned page=0;page<6;++page) for (unsigned active=0;active<2;++active) {
            if ((sent && (!delivered || page)) || (delivered==8 && !sent)) continue;
            uint8_t old[16]={'M','A','I','L',1,delivered,page,0,sent,active,0,0}; checksum(old);
            assert(fortune_mail_decode(&s,old,sizeof(old)));
            assert(s.selected==0 && s.bookmarks[0]==delivered*6+page+1);
            assert(s.active==(active && delivered<8)); roundtrip(&s);
        }
    uint8_t bad[16]; fortune_mail_defaults(&s); fortune_mail_encode(&s,bad,sizeof(bad));
    bad[7]=50; checksum(bad); assert(!fortune_mail_decode(&s,bad,sizeof(bad)));
    assert(s.selected==FORTUNE_MAIL_NONE);
    bad[7]=0; bad[6]=1; checksum(bad); assert(!fortune_mail_decode(&s,bad,sizeof(bad)));
    assert(!*fortune_mail_text(4,0) && !*fortune_mail_text(0,48));
    assert(!fortune_mail_select(&s,4) && !fortune_mail_step(&s,0));
    puts("Continuous letters: PASS (four complete stories, 120 short illustrated beats, shuffle bags, independent bookmarks, completion/reread, every v1 checkpoint migration, corruption rejection)");
}
