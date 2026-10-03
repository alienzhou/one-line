#pragma once
#include "fortune_data.h"
#include "fortune_art.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FORTUNE_NO_CARD UINT32_MAX
#define FORTUNE_SEEN_BYTES ((FORTUNE_COUNT + 7U) / 8U)
#define FORTUNE_ANY_STYLE 3U

typedef enum { FORTUNE_HOME, FORTUNE_REVEAL, FORTUNE_SHOWCASE } fortune_page_t;
typedef struct { uint32_t quote, art; } fortune_card_t;
/* Stable disk record: explicit encoding in fortune_store, not raw enum/struct bytes. */
typedef struct {
    uint32_t corpus_id, seed, cursor, art_cursor, cycle;
    fortune_card_t current, pinned;
    uint8_t mood, style;
    uint8_t seen[FORTUNE_SEEN_BYTES];
} fortune_state_t;

void fortune_defaults(fortune_state_t *state, uint32_t seed);
bool fortune_valid(const fortune_state_t *state);
bool fortune_decode(uint32_t id, char *out, size_t capacity);
bool fortune_draw(fortune_state_t *state);
void fortune_remix(fortune_state_t *state, fortune_card_t *card);
void fortune_reset_deck(fortune_state_t *state);
uint32_t fortune_remaining(const fortune_state_t *state);
uint32_t fortune_crc(const uint8_t *data, size_t size);
size_t fortune_encode_state(const fortune_state_t *state, uint8_t *out, size_t capacity);
bool fortune_decode_state(fortune_state_t *state, const uint8_t *data, size_t size);
