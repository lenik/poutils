#ifndef POFORMAT_H
#define POFORMAT_H

#include <stddef.h>

typedef struct {
    char *msgid;
    char *msgstr;
} translation_pair_t;

typedef struct {
    translation_pair_t *pairs;
    size_t len;
    size_t cap;
} translation_map_t;

typedef struct {
    int changed;
    int updated_entries;
    int removed_fuzzy;
} po_update_result_t;

void translation_map_init(translation_map_t *map);
void translation_map_free(translation_map_t *map);
int translation_map_add(translation_map_t *map, const char *msgid, const char *msgstr);
const char *translation_map_get(const translation_map_t *map, const char *msgid);

char *po_unescape_c_string(const char *src);
int load_translation_file(const char *path, translation_map_t *map);
po_update_result_t update_po_file(const char *path, const translation_map_t *map);

#endif
