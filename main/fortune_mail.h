#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FORTUNE_MAIL_STORIES 4U
#define FORTUNE_MAIL_PAGES_PER_LETTER 6U
#define FORTUNE_MAIL_BYTES 16U
#define FORTUNE_MAIL_TOPIC 9U
#define FORTUNE_TOPIC_COUNT 11U
#define FORTUNE_MAIL_NONE 255U

/* One bookmark per story: 0 unread, page+1 reading, count+1 finished. */
typedef struct {
    uint8_t selected, bookmarks[FORTUNE_MAIL_STORIES];
    bool active;
} fortune_mail_t;
typedef struct {
    uint8_t story, page, style;
    bool reading;
} fortune_mail_view_t;

void fortune_mail_defaults(fortune_mail_t *state);
bool fortune_mail_valid(const fortune_mail_t *state);
bool fortune_mail_select(fortune_mail_t *state, unsigned story);
bool fortune_mail_step(fortune_mail_t *state, int direction);
void fortune_mail_restore_view(const fortune_mail_t *state, fortune_mail_view_t *view);
/* A shuffled bag visits every story before refilling; no adjacent repeats. */
uint8_t fortune_mail_draw(uint8_t *bag, uint8_t previous, uint32_t random);
size_t fortune_mail_encode(const fortune_mail_t *state, uint8_t *out, size_t size);
bool fortune_mail_decode(fortune_mail_t *state, const uint8_t *data, size_t size);
unsigned fortune_mail_pages(unsigned story);
const char *fortune_mail_title(unsigned story);
const char *fortune_mail_text(unsigned story, unsigned page);
uint32_t fortune_mail_art(unsigned story, unsigned page, unsigned style);
