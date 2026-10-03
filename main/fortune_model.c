#include "fortune_model.h"
#include <string.h>

static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b) { uint32_t r = a % b; a = b; b = r; }
    return a;
}

/* A complete affine permutation: every card exactly once, constant RAM.
 * The global seen bits also prevent duplicates when changing mood/style filters. */
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
}

static bool valid_card(fortune_card_t c) {
    return (c.quote == FORTUNE_NO_CARD || c.quote < FORTUNE_COUNT) && c.art < FORTUNE_ART_COUNT;
}

bool fortune_valid(const fortune_state_t *s) {
    return s && s->corpus_id == FORTUNE_CORPUS_ID && s->mood <= 8 &&
        s->style <= FORTUNE_ANY_STYLE && s->cursor < FORTUNE_COUNT &&
        s->art_cursor < FORTUNE_ART_COUNT && valid_card(s->current) && valid_card(s->pinned);
}

bool fortune_decode(uint32_t id, char *out, size_t cap) {
    if (!out || !cap) return false;
    out[0] = 0;
    if (id >= FORTUNE_COUNT) return false;
    const fortune_record_t *r = &FORTUNE_RECORDS[id];
    uint32_t bit = r->bit_offset;
    size_t written = 0;
    for (uint8_t i = 0; i < r->length; ++i) {
        uint32_t symbol = 0;
        for (uint8_t b = 0; b < FORTUNE_BITS; ++b, ++bit) {
            if (bit / 8 >= FORTUNE_PAYLOAD_BYTES) return false;
            symbol |= ((FORTUNE_PAYLOAD[bit / 8] >> (bit % 8)) & 1U) << b;
        }
        if (symbol >= FORTUNE_DICTIONARY_COUNT) return false;
        uint16_t cp = FORTUNE_DICTIONARY[symbol];
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

static bool matches(const fortune_state_t *s, uint32_t id) {
    const fortune_record_t *r = &FORTUNE_RECORDS[id];
    return (!s->mood || r->mood == s->mood) &&
        (s->style == FORTUNE_ANY_STYLE || r->style == s->style);
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

/* Keyed Feistel permutation, cycle-walked to 80 or 4 values. Unlike a fixed
 * affine stride, adjacent items have no repeating family/color arithmetic. */
static uint32_t shuffled(uint32_t x, uint32_t count, uint32_t key) {
    unsigned bits = count == FORTUNE_SCENE_COUNT ? 4 : 1;
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

bool fortune_draw(fortune_state_t *s) {
    for (uint32_t i = 0; i < FORTUNE_COUNT; ++i) {
        uint32_t id = permute(s->cursor, FORTUNE_COUNT, s->seed);
        s->cursor = (s->cursor + 1) % FORTUNE_COUNT;
        if (!matches(s, id) || (s->seen[id / 8] & (1U << (id % 8)))) continue;
        s->seen[id / 8] |= (uint8_t)(1U << (id % 8));
        s->current.quote = id;
        fortune_remix(s, &s->current);
        return true;
    }
    return false;
}

void fortune_reset_deck(fortune_state_t *s) {
    memset(s->seen, 0, sizeof(s->seen));
    ++s->cycle;
    /* Keep the visual permutation seed stable across text-deck resets. */
    s->cursor = (uint32_t)(((uint64_t)s->cycle * 37U) % FORTUNE_COUNT);
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
    const size_t n = 48 + FORTUNE_SEEN_BYTES;
    if (cap < n || !fortune_valid(s)) return 0;
    memset(out, 0, n);
    put32(out, 0x46544331U); put32(out+4, s->corpus_id);
    put32(out+8, s->seed); put32(out+12, s->cursor);
    put32(out+16, s->art_cursor); put32(out+20, s->cycle);
    put32(out+24, s->current.quote); put32(out+28, s->current.art);
    put32(out+32, s->pinned.quote); put32(out+36, s->pinned.art);
    out[40] = s->mood; out[41] = s->style;
    memcpy(out+44, s->seen, FORTUNE_SEEN_BYTES);
    put32(out+n-4, fortune_crc(out, n-4));
    return n;
}

bool fortune_decode_state(fortune_state_t *s, const uint8_t *data, size_t n) {
    if (!s || !data || n != 48 + FORTUNE_SEEN_BYTES || get32(data) != 0x46544331U ||
        get32(data+n-4) != fortune_crc(data, n-4)) return false;
    fortune_state_t candidate = {0};
    candidate.corpus_id = get32(data+4); candidate.seed = get32(data+8);
    candidate.cursor = get32(data+12); candidate.art_cursor = get32(data+16);
    candidate.cycle = get32(data+20);
    candidate.current = (fortune_card_t){get32(data+24), get32(data+28)};
    candidate.pinned = (fortune_card_t){get32(data+32), get32(data+36)};
    candidate.mood = data[40]; candidate.style = data[41];
    memcpy(candidate.seen, data+44, FORTUNE_SEEN_BYTES);
    if (!fortune_valid(&candidate)) return false;
    *s = candidate;
    return true;
}
