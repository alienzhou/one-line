#include "fortune_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_deck(uint32_t seed) {
    fortune_state_t s;
    fortune_defaults(&s, seed);
    bool seen[FORTUNE_COUNT] = {0};
    for (unsigned i = 0; i < FORTUNE_COUNT; ++i) {
        assert(fortune_draw(&s));
        assert(!seen[s.current.quote]); seen[s.current.quote] = true;
        assert(fortune_valid(&s));
    }
    assert(!fortune_draw(&s)); assert(fortune_remaining(&s) == 0);
    fortune_reset_deck(&s); assert(fortune_remaining(&s) == FORTUNE_COUNT);
    assert(fortune_draw(&s));
}

int main(void) {
    for (unsigned i = 0; i < 200; ++i) test_deck(i*27449);
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
    for (unsigned style = 0; style < 3; ++style) for (unsigned mood = 1; mood <= 8; ++mood) {
        s.style = style; s.mood = mood;
        while (fortune_draw(&s)) {
            unsigned id = s.current.quote;
            assert(FORTUNE_RECORDS[id].style == style && FORTUNE_RECORDS[id].mood == mood);
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
    bool visuals[FORTUNE_ART_COUNT] = {0};
    for (unsigned i = 0; i < FORTUNE_ART_COUNT; ++i) {
        fortune_remix(&s, &s.current);
        assert(!visuals[s.current.art]); visuals[s.current.art] = true;
    }
    puts("Fortune model: PASS (200 full decks, filter switching, corruption, restore, 24576 visual IDs)");
    return 0;
}
