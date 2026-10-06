#include "fortune_sound.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int16_t whole[32001],chunks[32001]; uint64_t hashes[(FORTUNE_SOUND_COUNT-1)*FORTUNE_SOUND_VARIANTS]; unsigned hcount=0;
    uint64_t reveal_energy[FORTUNE_SOUND_VARIANTS]={0};
    assert(fortune_sound_samples(FORTUNE_SOUND_NONE)==0);
    assert(fortune_sound_samples((fortune_sound_t)-1)==0);
    assert(fortune_sound_render(FORTUNE_SOUND_OPEN,0,UINT32_MAX,whole,10)==0);
    for(unsigned sound=1;sound<FORTUNE_SOUND_COUNT;++sound) for(unsigned v=0;v<FORTUNE_SOUND_VARIANTS;++v) {
        uint32_t n=fortune_sound_samples((fortune_sound_t)sound); assert(n<32001);
        assert(fortune_sound_render((fortune_sound_t)sound,v,0,whole,32001)==n);
        memset(chunks,0x5A,sizeof(chunks));
        for(uint32_t offset=0;offset<n;) {
            size_t cap=1+(offset*17U%317U);
            size_t written=fortune_sound_render((fortune_sound_t)sound,v,offset,chunks+offset,cap);
            assert(written>0 && written<=cap); offset+=(uint32_t)written;
        }
        assert(!memcmp(whole,chunks,n*sizeof(*whole)));
        assert(chunks[n]==0x5A5A); /* Final partial chunk never overruns. */
        assert(whole[0]==0 && whole[n-1]==0);
        uint64_t h=14695981039346656037ULL,energy=0; int64_t dc=0; int peak=0,max_step=0;
        for(unsigned i=0;i<n;++i) {
            int a=abs(whole[i]); if(a>peak) peak=a;
            if(i && abs(whole[i]-whole[i-1])>max_step) max_step=abs(whole[i]-whole[i-1]);
            energy+=(int64_t)whole[i]*whole[i]; dc+=whole[i]; h^=(uint16_t)whole[i]; h*=1099511628211ULL;
        }
        assert(peak>500 && peak<(sound==FORTUNE_SOUND_RARE?16000:12000)); assert(energy/n>10000);
        if(sound==FORTUNE_SOUND_REVEAL) reveal_energy[v]=energy/n;
        /* The rare celebration must be clearly more present at the same volume,
         * while retaining headroom and the same click-free chunk rendering. */
        if(sound==FORTUNE_SOUND_RARE) assert(energy/n>reveal_energy[v]*3);
        assert(llabs(dc)/(int64_t)n<20); assert(max_step<peak);
        for(unsigned i=n-160;i<n;++i) assert(whole[i]==0);
        for(unsigned i=0;i<hcount;++i) assert(hashes[i]!=h);
        hashes[hcount++]=h;
        printf("cue=%u variant=%u frames=%u peak=%d max_step=%d\n",sound,v,n,peak,max_step);
    }
    puts("Fortune sound: PASS (all distinct scores including rare reveal, bounded gain/DC, zero tails, arbitrary chunk boundaries)");
    return 0;
}
