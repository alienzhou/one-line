#include "fortune_pixels.h"
#include "fortune_model.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N (FORTUNE_PIXEL_W*FORTUNE_PIXEL_H)
static uint16_t guarded[N+2],reference[N],phase[N];
static uint64_t hashes[FORTUNE_ART_COUNT];
static int compare(const void *a,const void *b) {
    uint64_t x=*(const uint64_t *)a,y=*(const uint64_t *)b; return x<y?-1:x>y;
}
int main(void) {
    unsigned max_change=0,min_subject_change=N;
    uint64_t legacy_hash=14695981039346656037ULL;
    for(uint32_t id=0;id<FORTUNE_ART_COUNT;++id) {
        guarded[0]=0xA55A;guarded[N+1]=0x5AA5;
        fortune_pixels(guarded+1,id,0); fortune_pixels(reference,id,0);
        assert(!memcmp(guarded+1,reference,sizeof(reference)));
        assert(guarded[0]==0xA55A && guarded[N+1]==0x5AA5);
        uint64_t h=14695981039346656037ULL;
        for(unsigned j=0;j<N;++j) { h^=reference[j]; h*=1099511628211ULL; }
        hashes[id]=h;
        /* Diverse seeds and complete animation cycles: motion must stay subtle. */
        if(id<96 || (id>=FORTUNE_LEGACY_ART_COUNT && id<FORTUNE_LEGACY_ART_COUNT+FORTUNE_SKIN_COUNT)) for(unsigned f=1;f<32;++f) {
            fortune_pixels(phase,id,f); unsigned changed=0;
            for(unsigned j=0;j<N;++j) changed+=phase[j]!=reference[j];
            if(changed>max_change) max_change=changed;
            assert(changed<N/10);
        }
        fortune_pixel_wipe(phase,(id+1)%FORTUNE_ART_COUNT,id,3);
        assert(!memcmp(phase,reference,sizeof(reference)));
        if(id<FORTUNE_LEGACY_ART_COUNT) for(unsigned f=0;f<3;++f) {
            fortune_pixels(phase,id,(unsigned[]){0,1,27}[f]);
            for(unsigned j=0;j<N;++j){legacy_hash^=phase[j];legacy_hash*=1099511628211ULL;}
        }
        if(id>=FORTUNE_LEGACY_ART_COUNT && id<FORTUNE_LEGACY_ART_COUNT+FORTUNE_SCENE_COUNT) {
            for(unsigned subject=1;subject<4;++subject) {
                fortune_pixels(phase,id+subject*FORTUNE_SCENE_COUNT,0);unsigned changed=0;
                for(unsigned j=0;j<N;++j)changed+=phase[j]!=reference[j];
                if(changed<min_subject_change)min_subject_change=changed;
            }
        }
    }
    qsort(hashes,FORTUNE_ART_COUNT,sizeof(hashes[0]),compare);
    unsigned unique=1;
    for(unsigned i=1;i<FORTUNE_ART_COUNT;++i) unique+=hashes[i]!=hashes[i-1];
    assert(unique==FORTUNE_ART_COUNT); assert(max_change>0);
    assert(legacy_hash==3579629304425850537ULL);
    assert(min_subject_change>150); /* Real silhouettes/props, not a color-only ID. */
    for(unsigned step=0;step<10;++step) {
        fortune_pixel_unwrap(guarded+1,2976,step);
        assert(guarded[0]==0xA55A && guarded[N+1]==0x5AA5);
    }
    printf("Pixel scenes: PASS (%u distinct images, %u scene/subject skins, exact legacy rendering; min subject change %u, max motion %u/%u pixels)\n",unique,FORTUNE_SKIN_COUNT,min_subject_change,max_change,N);
    return 0;
}
