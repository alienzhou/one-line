#include "fortune_model.h"
#include <string.h>

static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b) { uint32_t r = a % b; a = b; b = r; }
    return a;
}

/* Palette permutation only; text uses a keyed whole-bank shuffle below. */
static uint32_t permute(uint32_t position, uint32_t count, uint32_t seed) {
    uint32_t stride = seed % count;
    if (!stride) stride = 1;
    while (gcd(stride, count) != 1) stride = (stride + 1) % count;
    return (uint32_t)(((uint64_t)position * stride + (seed >> 16) % count) % count);
}

void fortune_defaults(fortune_state_t *s, uint32_t seed) {
    memset(s, 0, sizeof(*s));
    s->corpus_id = FORTUNE_CORPUS_ID;
    s->seed = seed;
    s->style = FORTUNE_ANY_STYLE;
    s->art_cursor = FORTUNE_LEGACY_ART_COUNT;
    s->current = s->pinned = (fortune_card_t){FORTUNE_NO_CARD, 0};
    s->rare_random = seed ^ 0xB5297A4DU;
    if (!s->rare_random) s->rare_random = 1;
    s->rare_last = FORTUNE_RARE_NONE;
}

static bool valid_quote(uint32_t quote) {
    if (quote==FORTUNE_NO_CARD || quote<FORTUNE_COUNT) return true;
    if (fortune_rare_index(quote)<FORTUNE_RARE_COUNT) return true;
    if ((quote & 0xC0000000U)==FORTUNE_LEGACY_QUOTE)
        return (quote & ~FORTUNE_LEGACY_QUOTE)<FORTUNE_LEGACY_COUNT;
    if ((quote & 0xC0000000U)==FORTUNE_PREVIOUS_QUOTE)
        return (quote & ~FORTUNE_PREVIOUS_QUOTE)<FORTUNE_PREVIOUS_COUNT;
    return false;
}

static bool valid_card(fortune_card_t c) {
    unsigned rare=fortune_rare_index(c.quote);
    if (rare<FORTUNE_RARE_COUNT)
        return fortune_rare_art_index(c.art)==rare;
    return valid_quote(c.quote) && c.art < FORTUNE_COMMON_ART_COUNT;
}

bool fortune_valid(const fortune_state_t *s) {
    if (!(s && s->corpus_id == FORTUNE_CORPUS_ID && s->mood <= 8 &&
        s->style <= FORTUNE_ANY_STYLE && s->cursor < FORTUNE_COUNT &&
        s->art_cursor < FORTUNE_COMMON_ART_COUNT && valid_card(s->current) && valid_card(s->pinned) &&
        s->favorite_count <= FORTUNE_FAVORITE_CAPACITY)) return false;
    if (!s->rare_random || (s->rare_unlocked & ~FORTUNE_RARE_MASK) ||
        s->rare_misses >= (s->rare_unlocked ? FORTUNE_RARE_PITY : FORTUNE_RARE_FIRST_PITY)) return false;
    if (s->rare_unlocked ? (s->rare_last>=FORTUNE_RARE_COUNT ||
        !(s->rare_unlocked & (1U<<s->rare_last))) : s->rare_last!=FORTUNE_RARE_NONE) return false;
    fortune_card_t displayed[]={s->current,s->pinned};
    for (unsigned i=0;i<2;++i) {
        unsigned rare=fortune_rare_index(displayed[i].quote);
        if (rare<FORTUNE_RARE_COUNT && !(s->rare_unlocked & (1U<<rare))) return false;
    }
    for (unsigned i=0; i<s->favorite_count; ++i) {
        if (!valid_card(s->favorites[i]) || s->favorites[i].quote==FORTUNE_NO_CARD) return false;
        unsigned rare=fortune_rare_index(s->favorites[i].quote);
        if (rare<FORTUNE_RARE_COUNT && !(s->rare_unlocked & (1U<<rare))) return false;
        for (unsigned j=0; j<i; ++j)
            if (s->favorites[i].quote==s->favorites[j].quote) return false;
    }
    return true;
}

int fortune_favorite_find(const fortune_state_t *s, uint32_t quote) {
    if (!s || quote==FORTUNE_NO_CARD || s->favorite_count>FORTUNE_FAVORITE_CAPACITY) return -1;
    for (unsigned i=0; i<s->favorite_count; ++i)
        if (s->favorites[i].quote==quote) return (int)i;
    return -1;
}

bool fortune_favorite_save(fortune_state_t *s, fortune_card_t card, unsigned slot) {
    if (!s || !valid_card(card) || card.quote==FORTUNE_NO_CARD ||
        s->favorite_count>FORTUNE_FAVORITE_CAPACITY) return false;
    int existing=fortune_favorite_find(s,card.quote);
    if (existing>=0) { s->favorites[existing]=card; return true; }
    if (slot>s->favorite_count || slot>=FORTUNE_FAVORITE_CAPACITY) return false;
    s->favorites[slot]=card;
    if (slot==s->favorite_count) ++s->favorite_count;
    return true;
}

bool fortune_favorite_remove(fortune_state_t *s, unsigned slot) {
    if (!s || slot>=s->favorite_count || s->favorite_count>FORTUNE_FAVORITE_CAPACITY) return false;
    memmove(s->favorites+slot,s->favorites+slot+1,
        (s->favorite_count-slot-1)*sizeof(s->favorites[0]));
    s->favorites[--s->favorite_count]=(fortune_card_t){0,0};
    /* Removing a bookmark does not remove the independently displayed signature. */
    return true;
}

bool fortune_decode(uint32_t id, char *out, size_t cap) {
    if (!out || !cap) return false;
    out[0] = 0;
    if (!valid_quote(id) || id==FORTUNE_NO_CARD) return false;
    unsigned rare=fortune_rare_index(id);
    if (rare<FORTUNE_RARE_COUNT) {
        const char *text=fortune_rare_text(rare);
        if (strlen(text)>=cap) return false;
        memcpy(out,text,strlen(text)+1); return true;
    }
    bool legacy = (id & FORTUNE_LEGACY_QUOTE) != 0;
    bool previous = (id & FORTUNE_PREVIOUS_QUOTE) != 0;
    id &= ~(FORTUNE_LEGACY_QUOTE | FORTUNE_PREVIOUS_QUOTE);
    const fortune_record_t *r = &(previous ? FORTUNE_PREVIOUS_RECORDS : legacy ? FORTUNE_LEGACY_RECORDS : FORTUNE_RECORDS)[id];
    const uint8_t *payload = previous ? FORTUNE_PREVIOUS_PAYLOAD : legacy ? FORTUNE_LEGACY_PAYLOAD : FORTUNE_PAYLOAD;
    const uint16_t *dictionary = previous ? FORTUNE_PREVIOUS_DICTIONARY : legacy ? FORTUNE_LEGACY_DICTIONARY : FORTUNE_DICTIONARY;
    unsigned bits = previous ? FORTUNE_PREVIOUS_BITS : legacy ? FORTUNE_LEGACY_BITS : FORTUNE_BITS;
    unsigned payload_size = previous ? FORTUNE_PREVIOUS_PAYLOAD_BYTES : legacy ? FORTUNE_LEGACY_PAYLOAD_BYTES : FORTUNE_PAYLOAD_BYTES;
    unsigned dictionary_size = previous ? FORTUNE_PREVIOUS_DICTIONARY_COUNT : legacy ? FORTUNE_LEGACY_DICTIONARY_COUNT : FORTUNE_DICTIONARY_COUNT;
    uint32_t bit = r->bit_offset;
    size_t written = 0;
    for (uint8_t i = 0; i < r->length; ++i) {
        uint32_t symbol = 0;
        for (uint8_t b = 0; b < bits; ++b, ++bit) {
            if (bit / 8 >= payload_size) return false;
            symbol |= ((payload[bit / 8] >> (bit % 8)) & 1U) << b;
        }
        if (symbol >= dictionary_size) return false;
        uint16_t cp = dictionary[symbol];
        size_t n = cp < 128 ? 1 : cp < 2048 ? 2 : 3;
        if (written + n >= cap) { out[0] = 0; return false; }
        if (n == 1) out[written++] = (char)cp;
        else if (n == 2) {
            out[written++] = (char)(0xC0 | (cp >> 6));
            out[written++] = (char)(0x80 | (cp & 63));
        } else {
            out[written++] = (char)(0xE0 | (cp >> 12));
            out[written++] = (char)(0x80 | ((cp >> 6) & 63));
            out[written++] = (char)(0x80 | (cp & 63));
        }
    }
    out[written] = 0;
    return true;
}

const char *fortune_citation(uint32_t id) {
    return id < FORTUNE_COUNT ? FORTUNE_CITATIONS[FORTUNE_RECORDS[id].citation] : "";
}

static bool matches(const fortune_state_t *s, uint32_t id) {
    const fortune_record_t *r = &FORTUNE_RECORDS[id];
    return !s->mood || r->mood == s->mood;
}

void fortune_select_topic(fortune_state_t *s, uint8_t topic) {
    if (topic > 8) return;
    s->mood = topic;
    s->style = FORTUNE_ANY_STYLE; /* Retired preference must never hide a theme. */
}

uint32_t fortune_remaining(const fortune_state_t *s) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < FORTUNE_COUNT; ++i)
        if (matches(s, i) && !(s->seen[i / 8] & (1U << (i % 8)))) ++n;
    return n;
}

static uint32_t art_mix(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU;
    return x ^ (x >> 16);
}

/* Keyed Feistel permutation, cycle-walked to the requested count. Unlike a fixed
 * affine stride, adjacent items have no repeating family/color arithmetic. */
static uint32_t shuffled(uint32_t x, uint32_t count, uint32_t key) {
    unsigned bits = 1;
    while ((1U << (bits*2)) < count) ++bits;
    uint32_t mask = (1U << bits) - 1;
    do {
        uint32_t left = x >> bits, right = x & mask;
        for (unsigned round = 0; round < 6; ++round) {
            uint32_t next = left ^ (art_mix(key ^ (round * 0x9E3779B9U) ^ right) & mask);
            left = right; right = next;
        }
        x = (left << bits) | right;
    } while (x >= count);
    return x;
}

void fortune_remix(fortune_state_t *s, fortune_card_t *c) {
    unsigned rare=fortune_rare_index(c->quote);
    if (rare<FORTUNE_RARE_COUNT) {
        unsigned tone=fortune_rare_art_tone(c->art);
        c->art=fortune_rare_art_id(rare,(tone+1)%FORTUNE_RARE_PALETTES);
        return;
    }
    /* Upgrade the old cursor lazily; quote history, current card and pin survive. */
    uint32_t cursor = s->art_cursor < FORTUNE_LEGACY_ART_COUNT ? 0 :
        s->art_cursor - FORTUNE_LEGACY_ART_COUNT;
    uint32_t block = cursor / FORTUNE_SCENE_COUNT, position = cursor % FORTUNE_SCENE_COUNT;
    uint32_t key = s->seed ^ art_mix(block + 719U);
    uint32_t previous = (block + FORTUNE_NEW_ART_COUNT / FORTUNE_SCENE_COUNT - 1) %
        (FORTUNE_NEW_ART_COUNT / FORTUNE_SCENE_COUNT);
    uint32_t first = shuffled(0, FORTUNE_SCENE_COUNT, key);
    uint32_t last = shuffled(FORTUNE_SCENE_COUNT-1, FORTUNE_SCENE_COUNT, s->seed ^ art_mix(previous + 719U));
    /* A boundary swap keeps adjacent scenes different across blocks and wrap. */
    if (first == last && position < 2) position = 1 - position;
    uint32_t scene = shuffled(position, FORTUNE_SCENE_COUNT, key);
    uint32_t batch = cursor / FORTUNE_SKIN_COUNT;
    uint32_t subject = shuffled(block % 4, 4, s->seed ^ art_mix(scene + batch * 137U));
    uint32_t skin = scene + subject * FORTUNE_SCENE_COUNT;
    uint32_t palette = permute(batch, FORTUNE_PALETTE_COUNT, s->seed ^ art_mix(skin + 83U));
    c->art = FORTUNE_LEGACY_ART_COUNT + palette * FORTUNE_SKIN_COUNT + skin;
    s->art_cursor = FORTUNE_LEGACY_ART_COUNT + (cursor + 1) % FORTUNE_NEW_ART_COUNT;
}

/* Shuffle the entire bank. Category is metadata, not a scheduling quota.
 * The persistent seen bitmap takes precedence across optional topic draws,
 * migration and this change of ordering, without discarding saved cards. */
static uint32_t text_id(const fortune_state_t *s, uint32_t position) {
    uint32_t key = s->seed ^ art_mix(s->cycle + 0xA123U);
    return shuffled(position,FORTUNE_COUNT,key);
}

bool fortune_draw(fortune_state_t *s) {
    for (uint32_t i = 0; i < FORTUNE_COUNT; ++i) {
        uint32_t id = text_id(s, s->cursor);
        s->cursor = (s->cursor + 1) % FORTUNE_COUNT;
        if (!matches(s, id) || (s->seen[id / 8] & (1U << (id % 8)))) continue;
        s->seen[id / 8] |= (uint8_t)(1U << (id % 8));
        s->current.quote = id;
        fortune_remix(s, &s->current);
        return true;
    }
    return false;
}

fortune_card_t fortune_rare_card(unsigned index) {
    return index<FORTUNE_RARE_COUNT ?
        (fortune_card_t){FORTUNE_RARE_QUOTE+index,fortune_rare_art_id(index,0)} :
        (fortune_card_t){FORTUNE_NO_CARD,0};
}
unsigned fortune_rare_owned(const fortune_state_t *s) {
    unsigned count=0;
    for (unsigned i=0;i<FORTUNE_RARE_COUNT;++i) count+=(s->rare_unlocked>>i)&1U;
    return count;
}
unsigned fortune_rare_at(const fortune_state_t *s,unsigned position) {
    for (unsigned i=0;i<FORTUNE_RARE_COUNT;++i)
        if ((s->rare_unlocked & (1U<<i)) && position--==0) return i;
    return FORTUNE_RARE_NONE;
}
/* Separate, persisted PRNG: skin changes and text shuffles cannot reroll pity.
 * Rejection sampling removes modulo bias from the nonzero xorshift domain. */
static uint32_t rare_below(fortune_state_t *s,uint32_t bound) {
    uint32_t x;
    do {
        x=s->rare_random; x^=x<<13; x^=x>>17; x^=x<<5;
        s->rare_random=x;
    } while (x>UINT32_MAX-UINT32_MAX%bound);
    return (x-1)%bound;
}
bool fortune_draw_surprise(fortune_state_t *s) {
    if (!s || !fortune_remaining(s)) return false;
    unsigned limit=s->rare_unlocked?FORTUNE_RARE_PITY:FORTUNE_RARE_FIRST_PITY;
    bool hit=rare_below(s,100)<FORTUNE_RARE_PERCENT;
    if (++s->rare_misses>=limit || hit) {
        uint32_t choices=FORTUNE_RARE_MASK & ~s->rare_unlocked;
        if (!choices) choices=FORTUNE_RARE_MASK & ~(1U<<s->rare_last);
        unsigned count=0;
        for (unsigned i=0;i<FORTUNE_RARE_COUNT;++i) count+=(choices>>i)&1U;
        unsigned slot=rare_below(s,count),index=0;
        for (;index<FORTUNE_RARE_COUNT;++index)
            if ((choices & (1U<<index)) && slot--==0) break;
        s->rare_last=(uint8_t)index; s->rare_unlocked|=1U<<index;
        s->rare_misses=0; s->current=fortune_rare_card(index);
        return true;
    }
    return fortune_draw(s);
}

void fortune_reset_deck(fortune_state_t *s) {
    memset(s->seen, 0, sizeof(s->seen));
    ++s->cycle;
    /* Keep the visual permutation seed stable across text-deck resets. */
    s->cursor = 0;
}

uint32_t fortune_crc(const uint8_t *data, size_t n) {
    uint32_t crc = UINT32_MAX;
    for (size_t i = 0; i < n; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) crc = (crc >> 1) ^ (0xEDB88320U & (0U - (crc & 1U)));
    }
    return ~crc;
}

static void put32(uint8_t *p, uint32_t v) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8*i));
}
static uint32_t get32(const uint8_t *p) {
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i) v |= (uint32_t)p[i] << (8*i);
    return v;
}

size_t fortune_encode_state(const fortune_state_t *s, uint8_t *out, size_t cap) {
    const size_t n = FORTUNE_STATE_BYTES;
    if (cap < n || !fortune_valid(s)) return 0;
    memset(out, 0, n);
    put32(out, 0x46544334U); put32(out+4, s->corpus_id);
    put32(out+8, s->seed); put32(out+12, s->cursor);
    put32(out+16, s->art_cursor); put32(out+20, s->cycle);
    put32(out+24, s->current.quote); put32(out+28, s->current.art);
    put32(out+32, s->pinned.quote); put32(out+36, s->pinned.art);
    out[40] = s->mood; out[41] = s->style;
    memcpy(out+44, s->seen, FORTUNE_SEEN_BYTES);
    size_t album=44+FORTUNE_SEEN_BYTES;
    out[album]=s->favorite_count;
    for (unsigned i=0; i<s->favorite_count; ++i) {
        put32(out+album+4+8*i,s->favorites[i].quote);
        put32(out+album+8+8*i,s->favorites[i].art);
    }
    put32(out+n-16,s->rare_random); put32(out+n-12,s->rare_unlocked);
    out[n-8]=s->rare_misses; out[n-7]=s->rare_last;
    put32(out+n-4, fortune_crc(out, n-4));
    return n;
}

bool fortune_decode_state(fortune_state_t *s, const uint8_t *data, size_t n) {
    if (!s || !data || n < 48 ||
        (get32(data)!=0x46544331U && get32(data)!=0x46544332U &&
         get32(data)!=0x46544333U && get32(data)!=0x46544334U) ||
        get32(data+n-4) != fortune_crc(data, n-4)) return false;
    bool expanded_format=get32(data)==0x46544334U;
    bool rare_format=expanded_format || get32(data)==0x46544333U;
    bool album_format=rare_format || get32(data)==0x46544332U;
    size_t expected=expanded_format?FORTUNE_STATE_BYTES:rare_format?FORTUNE_V3_STATE_BYTES:FORTUNE_V2_STATE_BYTES;
    if (album_format && (n!=expected ||
        get32(data+4)!=FORTUNE_CORPUS_ID)) return false;
    if (get32(data+4) == FORTUNE_LEGACY_CORPUS_ID &&
        FORTUNE_LEGACY_CORPUS_ID != FORTUNE_CORPUS_ID) {
        if (n != 48 + (FORTUNE_LEGACY_COUNT+7)/8 || get32(data+12)>=FORTUNE_LEGACY_COUNT ||
            get32(data+16)>=FORTUNE_COMMON_ART_COUNT || data[40]>8 || data[41]>FORTUNE_ANY_STYLE) return false;
        uint32_t current = get32(data+24), pinned = get32(data+32);
        if ((current!=FORTUNE_NO_CARD && current>=FORTUNE_LEGACY_COUNT) ||
            (pinned!=FORTUNE_NO_CARD && pinned>=FORTUNE_LEGACY_COUNT) ||
            get32(data+28)>=FORTUNE_COMMON_ART_COUNT || get32(data+36)>=FORTUNE_COMMON_ART_COUNT) return false;
        fortune_state_t migrated;
        fortune_defaults(&migrated, get32(data+8));
        migrated.art_cursor = get32(data+16); migrated.cycle = get32(data+20);
        migrated.current = (fortune_card_t){current==FORTUNE_NO_CARD?current:current|FORTUNE_LEGACY_QUOTE, get32(data+28)};
        migrated.pinned = (fortune_card_t){pinned==FORTUNE_NO_CARD?pinned:pinned|FORTUNE_LEGACY_QUOTE, get32(data+36)};
        fortune_select_topic(&migrated, data[40] ? 1 : 0);
        for (uint32_t i=0; i<FORTUNE_COUNT; ++i) {
            uint32_t old_id = FORTUNE_RETAINED_IDS[i];
            if (old_id < FORTUNE_LEGACY_COUNT && (data[44+old_id/8] & (1U << (old_id%8))))
                migrated.seen[i/8] |= (uint8_t)(1U << (i%8));
        }
        if (migrated.pinned.quote!=FORTUNE_NO_CARD)
            fortune_favorite_save(&migrated,migrated.pinned,0);
        *s = migrated;
        return true;
    }
    if (get32(data+4) == FORTUNE_PREVIOUS_CORPUS_ID) {
        if (n != 48+(FORTUNE_PREVIOUS_SOURCE_COUNT+7)/8 ||
            get32(data+12)>=FORTUNE_PREVIOUS_SOURCE_COUNT || data[40]>8 || data[41]>FORTUNE_ANY_STYLE ||
            get32(data+16)>=FORTUNE_COMMON_ART_COUNT || get32(data+28)>=FORTUNE_COMMON_ART_COUNT ||
            get32(data+36)>=FORTUNE_COMMON_ART_COUNT) return false;
        uint32_t quotes[2] = {get32(data+24),get32(data+32)};
        for (unsigned i=0;i<2;++i) {
            if (quotes[i]==FORTUNE_NO_CARD) continue;
            if (quotes[i]<FORTUNE_PREVIOUS_SOURCE_COUNT) quotes[i]=FORTUNE_PREVIOUS_MAP[quotes[i]];
            else if ((quotes[i]&0xC0000000U)!=FORTUNE_LEGACY_QUOTE ||
                     (quotes[i]&~FORTUNE_LEGACY_QUOTE)>=FORTUNE_LEGACY_COUNT) return false;
        }
        fortune_state_t migrated;
        fortune_defaults(&migrated,get32(data+8));
        migrated.art_cursor=get32(data+16); migrated.cycle=get32(data+20);
        migrated.current=(fortune_card_t){quotes[0],get32(data+28)};
        migrated.pinned=(fortune_card_t){quotes[1],get32(data+36)};
        fortune_select_topic(&migrated,data[40]);
        for (uint32_t old=0;old<FORTUNE_PREVIOUS_SOURCE_COUNT;++old) {
            uint32_t id=FORTUNE_PREVIOUS_MAP[old];
            if (id<FORTUNE_COUNT && (data[44+old/8]&(1U<<(old%8))))
                migrated.seen[id/8] |= (uint8_t)(1U<<(id%8));
        }
        if (migrated.pinned.quote!=FORTUNE_NO_CARD)
            fortune_favorite_save(&migrated,migrated.pinned,0);
        *s=migrated;
        return true;
    }
    if (n != (album_format ? expected : 48 + FORTUNE_SEEN_BYTES)) return false;
    fortune_state_t candidate;
    fortune_defaults(&candidate,get32(data+8));
    candidate.corpus_id = get32(data+4); candidate.seed = get32(data+8);
    candidate.cursor = get32(data+12); candidate.art_cursor = get32(data+16);
    candidate.cycle = get32(data+20);
    candidate.current = (fortune_card_t){get32(data+24), get32(data+28)};
    candidate.pinned = (fortune_card_t){get32(data+32), get32(data+36)};
    candidate.mood = data[40]; candidate.style = data[41];
    memcpy(candidate.seen, data+44, FORTUNE_SEEN_BYTES);
    if (album_format) {
        size_t album=44+FORTUNE_SEEN_BYTES;
        candidate.favorite_count=data[album];
        if (candidate.favorite_count>FORTUNE_FAVORITE_CAPACITY || data[album+1] ||
            data[album+2] || data[album+3]) return false;
        for (unsigned i=0; i<candidate.favorite_count; ++i)
            candidate.favorites[i]=(fortune_card_t){get32(data+album+4+8*i),get32(data+album+8+8*i)};
    }
    if (expanded_format) {
        candidate.rare_random=get32(data+n-16); candidate.rare_unlocked=get32(data+n-12);
        candidate.rare_misses=data[n-8]; candidate.rare_last=data[n-7];
        if (data[n-6] || data[n-5]) return false;
    } else if (rare_format) {
        candidate.rare_random=get32(data+n-12);
        candidate.rare_misses=data[n-8]; candidate.rare_unlocked=data[n-7]; candidate.rare_last=data[n-6];
        if (data[n-5] || candidate.rare_unlocked>=1U<<FORTUNE_FIRST_RARE_COUNT ||
            candidate.rare_misses >= (candidate.rare_unlocked?60U:FORTUNE_RARE_FIRST_PITY)) return false;
        /* A previously accrued 30..59 misses earns the next draw immediately. */
        if (candidate.rare_misses>=FORTUNE_RARE_PITY) candidate.rare_misses=FORTUNE_RARE_PITY-1;
    }
    if (!fortune_valid(&candidate)) return false;
    candidate.style=FORTUNE_ANY_STYLE;
    if (!album_format && candidate.pinned.quote!=FORTUNE_NO_CARD)
        fortune_favorite_save(&candidate,candidate.pinned,0);
    *s = candidate;
    return true;
}
