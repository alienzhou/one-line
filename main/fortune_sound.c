#include "fortune_sound.h"

typedef struct { uint16_t start, length; uint8_t note; uint16_t gain; } note_t;
/* The opening leaves room for the visual reveal at 1250 ms. A separate reveal
 * cue is triggered by the actual UI completion, including an early skip. */
static const note_t opening[] = {
    {0,220,0,2300},{280,220,2,2500},{530,230,4,2700},
    {735,220,5,2800},{895,225,7,2900}
};
static const note_t reveal[] = {{0,640,5,3300},{65,630,7,2900},{140,610,9,2400}};
static const note_t keep[] = {{0,300,2,2600},{90,440,5,3000}};
static const note_t skin[] = {{0,150,5,1600},{65,180,7,1700}};
/* A full arrival chord, an ascending flourish, then falling sparkle echoes.
 * Only a rare reveal gets this 1.9-second celebration; ordinary cues stay soft. */
static const note_t rare[] = {
    {0,1200,0,4400},{0,1160,2,3300},{0,1100,3,3400},{0,1000,5,3600},
    {120,450,5,4300},{240,450,7,4200},{360,480,8,4300},
    {480,520,10,4500},{600,450,11,3000},{720,900,10,4100},
    {800,950,7,2800},{880,920,5,2600},
    {1080,650,10,1600},{1200,600,7,1400},{1360,480,10,1200}
};
/* C/D/E/G/A across three octaves, as 32-bit phase increments at 16 kHz. */
static const uint32_t pitches[] = {
    70230768U,78828756U,88484379U,105226698U,118111600U,140458852U,157660196U,176966074U,210453397U,236223201U,280917704U,315320392U,353934833U,420901426U,472446402U
};
/* One sine period plus its repeated endpoint; interpolation avoids coarse
 * wavetable edges. No floating point or sample assets are needed at runtime. */
static const int16_t sine[257] = {
    0,804,1608,2410,3212,4011,4808,5602,6393,7179,7962,8739,9512,10278,11039,11793,
    12539,13279,14010,14732,15446,16151,16846,17530,18204,18868,19519,20159,20787,21403,22005,22594,
    23170,23731,24279,24811,25329,25832,26319,26790,27245,27683,28105,28510,28898,29268,29621,29956,
    30273,30571,30852,31113,31356,31580,31785,31971,32137,32285,32412,32521,32609,32678,32728,32757,
    32767,32757,32728,32678,32609,32521,32412,32285,32137,31971,31785,31580,31356,31113,30852,30571,
    30273,29956,29621,29268,28898,28510,28105,27683,27245,26790,26319,25832,25329,24811,24279,23731,
    23170,22594,22005,21403,20787,20159,19519,18868,18204,17530,16846,16151,15446,14732,14010,13279,
    12539,11793,11039,10278,9512,8739,7962,7179,6393,5602,4808,4011,3212,2410,1608,804,
    0,-804,-1608,-2410,-3212,-4011,-4808,-5602,-6393,-7179,-7962,-8739,-9512,-10278,-11039,-11793,
    -12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,-18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
    -23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,-27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,
    -30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,-32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,
    -32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,-32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,
    -30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,-27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,
    -23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,-18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
    -12539,-11793,-11039,-10278,-9512,-8739,-7962,-7179,-6393,-5602,-4808,-4011,-3212,-2410,-1608,-804,
    0,
};

static int32_t wave(uint32_t phase) {
    unsigned index=phase>>24, fraction=(phase>>16)&255;
    return sine[index]+(sine[index+1]-sine[index])*(int32_t)fraction/256;
}
uint32_t fortune_sound_samples(fortune_sound_t sound) {
    static const uint16_t ms[]={0,1150,800,580,280,1900};
    return sound>FORTUNE_SOUND_NONE && sound<FORTUNE_SOUND_COUNT ? ms[sound]*16U : 0;
}
size_t fortune_sound_render(fortune_sound_t sound, unsigned variant,
                            uint32_t offset, int16_t *pcm, size_t capacity) {
    uint32_t length=fortune_sound_samples(sound);
    if(!pcm || offset>=length) return 0;
    const note_t *notes=NULL; unsigned count=0;
    switch(sound) {
        case FORTUNE_SOUND_OPEN: notes=opening; count=5; break;
        case FORTUNE_SOUND_REVEAL: notes=reveal; count=3; break;
        case FORTUNE_SOUND_KEEP: notes=keep; count=2; break;
        case FORTUNE_SOUND_SKIN: notes=skin; count=2; break;
        case FORTUNE_SOUND_RARE: notes=rare; count=sizeof(rare)/sizeof(*rare); break;
        default: return 0;
    }
    variant%=FORTUNE_SOUND_VARIANTS;
    size_t frames=capacity<length-offset?capacity:length-offset;
    for(size_t i=0;i<frames;++i) {
        uint32_t t=offset+(uint32_t)i; int32_t mixed=0;
        for(unsigned v=0;v<count;++v) {
            uint32_t start=notes[v].start*16U, duration=notes[v].length*16U;
            if(t<start || t-start>=duration) continue;
            uint32_t age=t-start;
            int32_t envelope=(int32_t)((duration-age)*32767U/duration);
            envelope=envelope*envelope/32767;
            if(age<128) envelope=envelope*(int32_t)age/128; /* 8 ms attack. */
            uint32_t step=pitches[notes[v].note+variant];
            uint32_t phase=age*step;
            int32_t bell=(wave(phase)*8+wave(phase*3U)+wave(phase*5U)/2)/10;
            mixed+=(bell*envelope/32767)*(int32_t)notes[v].gain/32767;
        }
        /* A quiet, decaying paper texture, not a loud white-noise burst. */
        if(sound==FORTUNE_SOUND_OPEN && t<1280) {
            uint32_t noise=(t/4U+17U)*747796405U+2891336453U;
            int32_t paper=(int32_t)((noise>>20)&1023U)-512;
            int32_t envelope=(int32_t)((1280U-t)*128U/1280U);
            if(t<128) envelope=envelope*(int32_t)t/128;
            mixed+=paper*envelope/512;
        }
        /* A rounded low strike gives the chord weight; a brief shimmering
         * transient gives it a clear edge. Both start at zero and decay. */
        if(sound==FORTUNE_SOUND_RARE && t<4160) {
            int32_t envelope=(int32_t)((4160U-t)*32767U/4160U);
            envelope=envelope*envelope/32767;
            if(t<256) envelope=envelope*(int32_t)t/256;
            mixed+=(wave(t*(pitches[variant]/2U))*envelope/32767)*3000/32767;
            if(t<2400) {
                uint32_t noise=(t/2U+101U)*747796405U+2891336453U;
                int32_t sparkle=(int32_t)((noise>>20)&1023U)-512;
                int32_t fade=(int32_t)((2400U-t)*500U/2400U);
                if(t<128) fade=fade*(int32_t)t/128;
                mixed+=sparkle*fade/512;
            }
        }
        pcm[i]=(int16_t)mixed; /* Scores bound the sum well below int16 limits. */
    }
    return frames;
}
