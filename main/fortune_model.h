#pragma once
#include "fortune_data.h"
#include "fortune_legacy_data.h"
#include "fortune_previous_data.h"
#include "fortune_art.h"
#include "fortune_rare.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FORTUNE_NO_CARD UINT32_MAX
#define FORTUNE_SEEN_BYTES ((FORTUNE_COUNT + 7U) / 8U)
#define FORTUNE_ANY_STYLE 3U
#define FORTUNE_PREVIOUS_QUOTE 0x40000000U
#define FORTUNE_LEGACY_QUOTE 0x80000000U
#define FORTUNE_FAVORITE_CAPACITY 16U
#define FORTUNE_V2_STATE_BYTES (52U + FORTUNE_SEEN_BYTES + 8U * FORTUNE_FAVORITE_CAPACITY)
#define FORTUNE_V3_STATE_BYTES (FORTUNE_V2_STATE_BYTES + 8U)
#define FORTUNE_STATE_BYTES (FORTUNE_V2_STATE_BYTES + 12U)

typedef enum {
    FORTUNE_HOME, FORTUNE_REVEAL, FORTUNE_SHOWCASE, FORTUNE_TOPICS,
    FORTUNE_ALBUM, FORTUNE_ALBUM_ACTIONS, FORTUNE_ALBUM_REMOVE,
    FORTUNE_ALBUM_REPLACE, FORTUNE_ALBUM_CONFIRM, FORTUNE_MAIL, FORTUNE_RARE_ALBUM
} fortune_page_t;
typedef struct { uint32_t quote, art; } fortune_card_t;
/* Stable disk record: explicit encoding in fortune_store, not raw enum/struct bytes. */
typedef struct {
    uint32_t corpus_id, seed, cursor, art_cursor, cycle;
    fortune_card_t current, pinned;
    uint8_t mood, style;
    uint8_t seen[FORTUNE_SEEN_BYTES];
    uint8_t favorite_count;
    fortune_card_t favorites[FORTUNE_FAVORITE_CAPACITY];
    uint32_t rare_random, rare_unlocked;
    uint8_t rare_misses, rare_last;
} fortune_state_t;

void fortune_defaults(fortune_state_t *state, uint32_t seed);
bool fortune_valid(const fortune_state_t *state);
bool fortune_decode(uint32_t id, char *out, size_t capacity);
const char *fortune_citation(uint32_t id);
void fortune_select_topic(fortune_state_t *state, uint8_t topic);
bool fortune_draw(fortune_state_t *state);
/* Product draw: rare overlay, then the unchanged nonrepeating common deck.
 * Only successful new draws advance pity; the caller commits before reveal. */
bool fortune_draw_surprise(fortune_state_t *state);
unsigned fortune_rare_owned(const fortune_state_t *state);
unsigned fortune_rare_at(const fortune_state_t *state, unsigned position);
fortune_card_t fortune_rare_card(unsigned index);
void fortune_remix(fortune_state_t *state, fortune_card_t *card);
void fortune_reset_deck(fortune_state_t *state);
int fortune_favorite_find(const fortune_state_t *state, uint32_t quote);
/* Existing quotes update in place; a new quote appends or replaces an explicit slot. */
bool fortune_favorite_save(fortune_state_t *state, fortune_card_t card, unsigned slot);
bool fortune_favorite_remove(fortune_state_t *state, unsigned slot);
uint32_t fortune_remaining(const fortune_state_t *state);
uint32_t fortune_crc(const uint8_t *data, size_t size);
size_t fortune_encode_state(const fortune_state_t *state, uint8_t *out, size_t capacity);
bool fortune_decode_state(fortune_state_t *state, const uint8_t *data, size_t size);
