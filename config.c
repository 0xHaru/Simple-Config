#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

typedef struct {
    const char *src;
    int len;
    int cur;
} Scanner;

static void
init_scanner(Scanner *s, const char *src, int src_len)
{
    s->src = src;
    s->len = src_len;
    s->cur = 0;
}

static bool
is_at_end(Scanner *s)
{
    return s->cur >= s->len;
}

static int
cur(Scanner *s)
{
    return s->cur;
}

static char
peek(Scanner *s)
{
    return s->src[s->cur];
}

static char
peek_next(Scanner *s)
{
    if (s->cur >= s->len - 1)
        return '\0';
    return s->src[s->cur + 1];
}

static char
advance(Scanner *s)
{
    return s->src[s->cur++];
}

static char
advance2(Scanner *s, int n)
{
    for (int i = 0; i < n - 1; i++)
        advance(s);
    return advance(s);
}

static void
skip_whitespace(Scanner *s)
{
    while (!is_at_end(s) && isspace(peek(s)))
        advance(s);
}

static void
skip_blank(Scanner *s)
{
    while (!is_at_end(s) && isspace(peek(s)) && peek(s) != '\n')
        advance(s);
}

static void
skip_comment(Scanner *s)
{
    while (!is_at_end(s) && peek(s) == '#') {
        do
            advance(s);
        while (!is_at_end(s) && peek(s) != '\n');
    }
}

void
skip_whitespace_and_comments(Scanner *s)
{
    while (!is_at_end(s) && (isspace(peek(s)) || peek(s) == '#')) {
        skip_whitespace(s);
        skip_comment(s);
    }
}

static void *
alloc(Cfg *cfg, int size, int align)
{
    int avail = cfg->capacity - cfg->offset;
    int padding = -cfg->offset & (align - 1);
    if (size > avail - padding)
        return NULL;
    void *p = cfg->arena + cfg->offset + padding;
    cfg->offset += padding + size;
    return memset(p, 0, size);
}

static void *
make_string(Cfg *cfg, Scanner *s, int src_off, int src_len)
{
    char *dst = alloc(cfg, src_len + 1, 1);
    if (dst != NULL)
        memcpy(dst, s->src + src_off, src_len);
    return dst;
}

static bool
match_literal(Scanner *s, int offset, const char *literal, int len)
{
    if (offset + len > s->len)
        return false;
    return !strncmp(s->src + offset, literal, len);
}

static bool
is_key(char ch)
{
    return isalpha(ch) || ch == '.' || ch == '_';
}

static bool
is_string(char ch)
{
    return isalnum(ch) || isblank(ch) || (ispunct(ch) && ch != '"');
}

static void
init_error(CfgError *err)
{
    err->off = -1;
    err->col = -1;
    err->row = -1;
    err->msg[0] = '\0';
}

static int
error(Scanner *s, CfgError *err, const char *fmt, ...)
{
    const char prefix[] = "";
    const int prefix_len = sizeof(prefix) - 1;

    err->off = cur(s);
    err->row = 1;
    err->col = 1;
    for (int i = 0; i < cur(s); i++) {
        err->col++;
        if (s->src[i] == '\n') {
            err->row++;
            err->col = 1;
        }
    }

    va_list vargs;
    va_start(vargs, fmt);
    snprintf(err->msg, CFG_MAX_ERR, prefix);
    vsnprintf(err->msg + prefix_len, CFG_MAX_ERR - prefix_len, fmt, vargs);
    va_end(vargs);
    return -1;
}

void
cfg_fprint_error(FILE *stream, CfgError *err)
{
    if (err->row == -1 && err->col == -1)
        fprintf(stream, "Error: %s\n", err->msg);
    else
        fprintf(stream, "Error at %d:%d :: %s\n", err->row, err->col, err->msg);
}

static int
parse_string(Cfg *cfg, Scanner *s, CfgEntry *entry, CfgError *err)
{
    // Consume opening '"'
    advance(s);

    // Consume string
    int val_offset = cur(s);
    while (!is_at_end(s) && is_string(peek(s)))
        advance(s);

    if (is_at_end(s) || peek(s) != '"')
        return error(s, err, "closing '\"' expected");

    int val_len = cur(s) - val_offset;

    // Consume closing '"'
    advance(s);

    entry->val.string = make_string(cfg, s, val_offset, val_len);
    if (entry->val.string == NULL)
        return error(s, err, "out of memory");
    entry->type = CFG_TYPE_STRING;
    return 0;
}

static int
consume_number(Scanner *s, float *number, bool *is_int)
{
    bool is_float = false;
    int sign = 1;
    int int_part = 0;
    float fract_part = 0;

    if (!is_at_end(s) && peek(s) == '-' && isdigit(peek_next(s))) {
        // Consume '-'
        advance(s);
        sign = -1;
    }

    if (!is_at_end(s) && !isdigit(peek(s)))
        return -1;

    while (!is_at_end(s) && isdigit(peek(s))) {
        int digit = advance(s) - '0';
        if (int_part > (INT_MAX - digit) / 10)
            return -1;  // overflow
        int_part = int_part * 10 + digit;
    }

    if (!is_at_end(s) && peek(s) == '.') {
        advance(s);
        is_float = true;
    }

    if (!is_float) {
        *is_int = true;
        *number = sign * int_part;
    } else {
        int div = 1;
        while (!is_at_end(s) && isdigit(peek(s))) {
            int digit = advance(s) - '0';
            if (digit > (INT_MAX - digit) / 10)
                return -1;  // overflow
            fract_part = fract_part * 10 + digit;
            if (div > INT_MAX / 10)
                return -1;  // overflow
            div *= 10;
        }
        float floating = int_part + (fract_part / div);
        *is_int = false;
        *number = sign * floating;
    }

    return 0;
}

static int
parse_number(Scanner *s, CfgEntry *entry, CfgError *err)
{
    bool is_int;
    float number;
    if (consume_number(s, &number, &is_int) != 0)
        return error(s, err, "number expected");

    if (is_int) {
        entry->val.integer = (int) number;
        entry->type = CFG_TYPE_INT;
    } else {
        entry->val.floating = number;
        entry->type = CFG_TYPE_FLOAT;
    }

    return 0;
}

static int
parse_rgba(Scanner *s, CfgEntry *entry, CfgError *err)
{
    if (!match_literal(s, cur(s), "rgba", 4))
        return error(s, err, "invalid literal");

    // Consume "rgba"
    advance2(s, 4);

    // Skip blank space between 'a' and '('
    skip_blank(s);

    if (is_at_end(s) || peek(s) != '(')
        return error(s, err, "'(' expected");

    // Consume '('
    advance(s);

    bool is_int;
    uint8_t rgb[3];
    for (int i = 0; i < 3; i++) {
        // Skip blank space preceding the number
        skip_blank(s);

        float number;
        if (consume_number(s, &number, &is_int) != 0)
            return error(s, err, "number expected");

        if (!is_int || number < 0 || number > 255)
            return error(s, err,
                         "red, blue and green must be "
                         "integers in range (0, 255)");

        rgb[i] = (uint8_t) number;

        // Skip blank space following the number
        skip_blank(s);

        if (is_at_end(s) || peek(s) != ',')
            return error(s, err, "',' expected");

        // Consume ','
        advance(s);
    }

    // Skip blank space preceding the number
    skip_blank(s);

    float alpha;
    if (consume_number(s, &alpha, &is_int) != 0)
        return error(s, err, "number expected");

    if (alpha < 0 || alpha > 1)
        return error(s, err, "alpha must be in range (0, 1)");

    // Skip blank space following the number
    skip_blank(s);

    if (is_at_end(s) || peek(s) != ')')
        return error(s, err, "')' expected");

    // Consume ')'
    advance(s);

    CfgColor color = {
        .r = rgb[0],
        .g = rgb[1],
        .b = rgb[2],
        .a = (uint8_t) (alpha * 255),
    };
    entry->val.color = color;
    entry->type = CFG_TYPE_COLOR;
    return 0;
}

static int
parse_true(Scanner *s, CfgEntry *entry, CfgError *err)
{
    if (!match_literal(s, cur(s), "true", 4))
        return error(s, err, "invalid literal");

    // Consume "true"
    advance2(s, 4);

    entry->val.boolean = true;
    entry->type = CFG_TYPE_BOOL;
    return 0;
}

static int
parse_false(Scanner *s, CfgEntry *entry, CfgError *err)
{
    if (!match_literal(s, cur(s), "false", 5))
        return error(s, err, "invalid literal");

    // Consume "false"
    advance2(s, 5);

    entry->val.boolean = false;
    entry->type = CFG_TYPE_BOOL;
    return 0;
}

static int
parse_literal(Scanner *s, CfgEntry *entry, CfgError *err)
{
    switch (peek(s)) {
    case 't':
        return parse_true(s, entry, err);
    case 'f':
        return parse_false(s, entry, err);
    case 'r':
        return parse_rgba(s, entry, err);
    default:
        return error(s, err, "invalid literal");
    }
}

static int
parse_value(Cfg *cfg, Scanner *s, CfgEntry *entry, CfgError *err)
{
    // Skip blank space between ':' and the value
    skip_blank(s);

    if (is_at_end(s) || peek(s) == '\n')
        return error(s, err, "missing value");

    // Consume value
    char c = peek(s);

    if (c == '"')
        return parse_string(cfg, s, entry, err);
    else if (isalpha(c))
        return parse_literal(s, entry, err);
    else if (isdigit(c) || (c == '-' && isdigit(peek_next(s))))
        return parse_number(s, entry, err);
    else
        return error(s, err, "invalid value");
}

static int
parse_key(Cfg *cfg, Scanner *s, CfgEntry *entry, CfgError *err)
{
    if (is_at_end(s) || !is_key(peek(s)))
        return error(s, err, "missing key");

    // Consume key
    int key_offset = cur(s);
    do
        advance(s);
    while (!is_at_end(s) && is_key(peek(s)));
    int key_len = cur(s) - key_offset;

    entry->key = make_string(cfg, s, key_offset, key_len);
    if (entry->key == NULL)
        return error(s, err, "out of memory");

    return 0;
}

static int
consume_colon(Scanner *s, CfgError *err)
{
    // Skip blank space between the key and ':'
    skip_blank(s);

    if (is_at_end(s) || peek(s) != ':')
        return error(s, err, "':' expected");

    // Consume ':'
    advance(s);
    return 0;
}

static int
parse_entry(Cfg *cfg, Scanner *s, CfgEntry *entry, CfgError *err)
{
    if (parse_key(cfg, s, entry, err) != 0)
        return -1;

    if (consume_colon(s, err) != 0)
        return -1;

    if (parse_value(cfg, s, entry, err) != 0)
        return -1;

    // Skip trailing blank space after the value
    skip_blank(s);

    if (!is_at_end(s) && peek(s) == '#')
        skip_comment(s);

    if (!is_at_end(s) && peek(s) != '\n')
        return error(s, err, "unexpected character '%c'", peek(s));

    // Consume '\n'
    if (!is_at_end(s))
        advance(s);

    return 0;
}

int
cfg_parse(const char *src, int src_len, Cfg *cfg, CfgError *err)
{
    Scanner s;
    init_scanner(&s, src, src_len);
    init_error(err);

    skip_whitespace_and_comments(&s);

    while (!is_at_end(&s)) {
        CfgEntry *entry = alloc(cfg, sizeof(*entry), sizeof(void *));
        if (entry == NULL)
            return error(&s, err, "out of memory");

        if (parse_entry(cfg, &s, entry, err) != 0)
            return -1;

        entry->next = cfg->entries;
        cfg->entries = entry;

        skip_whitespace_and_comments(&s);
    }

    return 0;
}

static char *
read_file(Cfg *cfg, const char *filename, int *count, char *err)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(err, CFG_MAX_ERR, "failed to open file");
        return NULL;
    }

    cfg->offset = fread(cfg->arena, sizeof(char), cfg->capacity, file);
    bool failed = ferror(file);
    fclose(file);

    if (failed) {
        snprintf(err, CFG_MAX_ERR, "failed to read file");
        return NULL;
    }

    *count = cfg->offset;
    return cfg->arena;
}

int
cfg_load(const char *filename, Cfg *cfg, CfgError *err)
{
    init_error(err);

    size_t len = strlen(filename);
    if (len < 5) {
        snprintf(err->msg, CFG_MAX_ERR, "invalid filename");
        return -1;
    }

    const char *ext = filename + (len - strlen(CFG_FILE_EXT));
    if (strcmp(ext, CFG_FILE_EXT) != 0) {
        snprintf(err->msg, CFG_MAX_ERR, "invalid file extension");
        return -1;
    }

    int src_len;
    char *src = read_file(cfg, filename, &src_len, err->msg);
    if (src == NULL)
        return -1;

    int res = cfg_parse(src, src_len, cfg, err);

    return res;
}

static void *
get_val(Cfg *cfg, const char *key, void *fallback, CfgValType type)
{
    for (CfgEntry *entry = cfg->entries; entry; entry = entry->next) {
        if (entry->type == type && !strcmp(key, entry->key))
            return &(entry->val);
    }
    return fallback;
}

char *
cfg_get_string(Cfg *cfg, const char *key, char *fallback)
{
    char **fallbackp = &fallback;
    return *(char **) get_val(cfg, key, fallbackp, CFG_TYPE_STRING);
}

bool
cfg_get_bool(Cfg *cfg, const char *key, bool fallback)
{
    return *(bool *) get_val(cfg, key, &fallback, CFG_TYPE_BOOL);
}

int
cfg_get_int(Cfg *cfg, const char *key, int fallback)
{
    return *(int *) get_val(cfg, key, &fallback, CFG_TYPE_INT);
}

int
cfg_get_int_min(Cfg *cfg, const char *key, int fallback, int min)
{
    int value = cfg_get_int(cfg, key, fallback);
    if (value < min)
        value = fallback;
    return value;
}

int
cfg_get_int_max(Cfg *cfg, const char *key, int fallback, int max)
{
    int value = cfg_get_int(cfg, key, fallback);
    if (value > max)
        value = fallback;
    return value;
}

int
cfg_get_int_range(Cfg *cfg, const char *key, int fallback, int min, int max)
{
    int value = cfg_get_int(cfg, key, fallback);
    if (value < min || value > max)
        value = fallback;
    return value;
}

float
cfg_get_float(Cfg *cfg, const char *key, float fallback)
{
    return *(float *) get_val(cfg, key, &fallback, CFG_TYPE_FLOAT);
}

float
cfg_get_float_min(Cfg *cfg, const char *key, float fallback, float min)
{
    float value = cfg_get_float(cfg, key, fallback);
    if (value < min)
        value = fallback;
    return value;
}

float
cfg_get_float_max(Cfg *cfg, const char *key, float fallback, float max)
{
    float value = cfg_get_float(cfg, key, fallback);
    if (value > max)
        value = fallback;
    return value;
}

float
cfg_get_float_range(Cfg *cfg,
                    const char *key,
                    float fallback,
                    float min,
                    float max)
{
    float value = cfg_get_float(cfg, key, fallback);
    if (value < min || value > max)
        value = fallback;
    return value;
}

CfgColor
cfg_get_color(Cfg *cfg, const char *key, CfgColor fallback)
{
    return *(CfgColor *) get_val(cfg, key, &fallback, CFG_TYPE_COLOR);
}

void
cfg_fprint(FILE *stream, Cfg *cfg)
{
    for (CfgEntry *entry = cfg->entries; entry; entry = entry->next) {
        fprintf(stream, "%s: ", entry->key);

        switch (entry->type) {
        case CFG_TYPE_STRING:
            fprintf(stream, "\"%s\"\n", entry->val.string);
            break;
        case CFG_TYPE_BOOL:
            fprintf(stream, "%s\n",
                    entry->val.boolean ? "true" : "false");
            break;
        case CFG_TYPE_INT:
            fprintf(stream, "%d\n", entry->val.integer);
            break;
        case CFG_TYPE_FLOAT:
            fprintf(stream, "%f\n", entry->val.floating);
            break;
        case CFG_TYPE_COLOR:;
            CfgColor c = entry->val.color;
            fprintf(stream, "rgba(%d, %d, %d, %d)\n", c.r, c.g, c.b, c.a);
            break;
        }
    }
}
