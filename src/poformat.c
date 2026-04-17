#define _POSIX_C_SOURCE 200809L

#include "poformat.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = malloc(n);
    if (!out) {
        return NULL;
    }
    memcpy(out, s, n);
    return out;
}

static bool has_prefix(const char *s, const char *prefix) {
    size_t p = strlen(prefix);
    return strncmp(s, prefix, p) == 0;
}

static void trim_eol(char *line) {
    size_t n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

char *po_unescape_c_string(const char *src) {
    size_t n = strlen(src);
    char *out = malloc(n + 1);
    if (!out) {
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        char c = src[i];
        if (c != '\\') {
            out[j++] = c;
            continue;
        }
        if (i + 1 >= n) {
            out[j++] = '\\';
            break;
        }
        i++;
        switch (src[i]) {
        case 'n':
            out[j++] = '\n';
            break;
        case 't':
            out[j++] = '\t';
            break;
        case 'r':
            out[j++] = '\r';
            break;
        case '\\':
            out[j++] = '\\';
            break;
        case '"':
            out[j++] = '"';
            break;
        default:
            out[j++] = src[i];
            break;
        }
    }
    out[j] = '\0';
    return out;
}

static char *escape_po_string(const char *src) {
    size_t n = 0;
    for (const char *p = src; *p; p++) {
        switch (*p) {
        case '\\':
        case '"':
        case '\n':
            n += 2;
            break;
        default:
            n++;
            break;
        }
    }
    char *out = malloc(n + 1);
    if (!out) {
        return NULL;
    }
    size_t j = 0;
    for (const char *p = src; *p; p++) {
        switch (*p) {
        case '\\':
            out[j++] = '\\';
            out[j++] = '\\';
            break;
        case '"':
            out[j++] = '\\';
            out[j++] = '"';
            break;
        case '\n':
            out[j++] = '\\';
            out[j++] = 'n';
            break;
        default:
            out[j++] = *p;
            break;
        }
    }
    out[j] = '\0';
    return out;
}

static char *extract_po_quoted(const char *line) {
    const char *start = strchr(line, '"');
    if (!start) {
        return xstrdup("");
    }
    start++;
    const char *end = strrchr(start, '"');
    if (!end || end < start) {
        return xstrdup("");
    }
    size_t n = (size_t)(end - start);
    char *raw = malloc(n + 1);
    if (!raw) {
        return NULL;
    }
    memcpy(raw, start, n);
    raw[n] = '\0';
    char *out = po_unescape_c_string(raw);
    free(raw);
    return out;
}

void translation_map_init(translation_map_t *map) {
    map->pairs = NULL;
    map->len = 0;
    map->cap = 0;
}

void translation_map_free(translation_map_t *map) {
    for (size_t i = 0; i < map->len; i++) {
        free(map->pairs[i].msgid);
        free(map->pairs[i].msgstr);
    }
    free(map->pairs);
    map->pairs = NULL;
    map->len = 0;
    map->cap = 0;
}

int translation_map_add(translation_map_t *map, const char *msgid, const char *msgstr) {
    for (size_t i = 0; i < map->len; i++) {
        if (strcmp(map->pairs[i].msgid, msgid) == 0) {
            char *dup = xstrdup(msgstr);
            if (!dup) {
                return -1;
            }
            free(map->pairs[i].msgstr);
            map->pairs[i].msgstr = dup;
            return 0;
        }
    }
    if (map->len == map->cap) {
        size_t next = map->cap ? map->cap * 2 : 16;
        translation_pair_t *pairs = realloc(map->pairs, next * sizeof(*pairs));
        if (!pairs) {
            return -1;
        }
        map->pairs = pairs;
        map->cap = next;
    }
    map->pairs[map->len].msgid = xstrdup(msgid);
    map->pairs[map->len].msgstr = xstrdup(msgstr);
    if (!map->pairs[map->len].msgid || !map->pairs[map->len].msgstr) {
        free(map->pairs[map->len].msgid);
        free(map->pairs[map->len].msgstr);
        return -1;
    }
    map->len++;
    return 0;
}

const char *translation_map_get(const translation_map_t *map, const char *msgid) {
    for (size_t i = 0; i < map->len; i++) {
        if (strcmp(map->pairs[i].msgid, msgid) == 0) {
            return map->pairs[i].msgstr;
        }
    }
    return NULL;
}

int load_translation_file(const char *path, translation_map_t *map) {
    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }
    char line[16384];
    int rc = 0;
    while (fgets(line, sizeof(line), f)) {
        trim_eol(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        char *tab = strchr(line, '\t');
        if (!tab) {
            rc = -2;
            break;
        }
        *tab = '\0';
        char *msgid = po_unescape_c_string(line);
        char *msgstr = po_unescape_c_string(tab + 1);
        if (!msgid || !msgstr || translation_map_add(map, msgid, msgstr) != 0) {
            free(msgid);
            free(msgstr);
            rc = -1;
            break;
        }
        free(msgid);
        free(msgstr);
    }
    fclose(f);
    return rc;
}

static int append_buf(char **buf, size_t *len, size_t *cap, const char *s) {
    size_t n = strlen(s);
    if (*len + n + 1 > *cap) {
        size_t next = *cap ? *cap : 4096;
        while (*len + n + 1 > next) {
            next *= 2;
        }
        char *grown = realloc(*buf, next);
        if (!grown) {
            return -1;
        }
        *buf = grown;
        *cap = next;
    }
    memcpy(*buf + *len, s, n);
    *len += n;
    (*buf)[*len] = '\0';
    return 0;
}

static int append_quoted_payload(char **dst, size_t *len, size_t *cap, const char *line) {
    char *part = extract_po_quoted(line);
    if (!part) {
        return -1;
    }
    int rc = append_buf(dst, len, cap, part);
    free(part);
    return rc;
}

po_update_result_t update_po_file(const char *path, const translation_map_t *map) {
    po_update_result_t out = {0, 0, 0};
    FILE *in = fopen(path, "r");
    if (!in) {
        return out;
    }
    char *new_text = NULL;
    size_t new_len = 0, new_cap = 0;
    char line[16384];
    bool in_fuzzy = false, seen_msgid = false, seen_msgstr = false, updated_this_entry = false;
    char *msgid = NULL, *msgstr = NULL;
    size_t msgid_len = 0, msgid_cap = 0, msgstr_len = 0, msgstr_cap = 0;

    while (fgets(line, sizeof(line), in)) {
        if (strcmp(line, "\n") == 0) {
            in_fuzzy = seen_msgid = seen_msgstr = updated_this_entry = false;
            msgid_len = msgstr_len = 0;
            if (append_buf(&new_text, &new_len, &new_cap, line) != 0) {
                goto done;
            }
            continue;
        }
        if (!seen_msgid && has_prefix(line, "#, fuzzy")) {
            in_fuzzy = true;
            out.removed_fuzzy++;
            out.changed = 1;
            continue;
        }
        if (!seen_msgid && has_prefix(line, "msgid ")) {
            seen_msgid = true;
            msgid_len = 0;
            if (append_quoted_payload(&msgid, &msgid_len, &msgid_cap, line) != 0 ||
                append_buf(&new_text, &new_len, &new_cap, line) != 0) {
                goto done;
            }
            continue;
        }
        if (seen_msgid && !seen_msgstr && has_prefix(line, "\"")) {
            if (append_quoted_payload(&msgid, &msgid_len, &msgid_cap, line) != 0 ||
                append_buf(&new_text, &new_len, &new_cap, line) != 0) {
                goto done;
            }
            continue;
        }
        if (seen_msgid && has_prefix(line, "msgstr ")) {
            seen_msgstr = true;
            msgstr_len = 0;
            if (append_quoted_payload(&msgstr, &msgstr_len, &msgstr_cap, line) != 0) {
                goto done;
            }
            const char *replacement = translation_map_get(map, msgid ? msgid : "");
            if (replacement) {
                char *escaped = escape_po_string(replacement);
                if (!escaped) {
                    goto done;
                }
                char *entry = malloc(strlen(escaped) + 12);
                if (!entry) {
                    free(escaped);
                    goto done;
                }
                sprintf(entry, "msgstr \"%s\"\n", escaped);
                if (append_buf(&new_text, &new_len, &new_cap, entry) != 0) {
                    free(entry);
                    free(escaped);
                    goto done;
                }
                free(entry);
                free(escaped);
                if (strcmp(replacement, msgstr ? msgstr : "") != 0 || in_fuzzy) {
                    out.changed = 1;
                    out.updated_entries++;
                }
                updated_this_entry = true;
            } else if (append_buf(&new_text, &new_len, &new_cap, line) != 0) {
                goto done;
            }
            continue;
        }
        if (seen_msgstr && has_prefix(line, "\"")) {
            if (updated_this_entry) {
                out.changed = 1;
                continue;
            }
            if (append_quoted_payload(&msgstr, &msgstr_len, &msgstr_cap, line) != 0 ||
                append_buf(&new_text, &new_len, &new_cap, line) != 0) {
                goto done;
            }
            continue;
        }
        if (append_buf(&new_text, &new_len, &new_cap, line) != 0) {
            goto done;
        }
    }

    if (out.changed) {
        FILE *w = fopen(path, "w");
        if (!w) {
            out.changed = 0;
            goto done;
        }
        fwrite(new_text, 1, new_len, w);
        fclose(w);
    }

done:
    free(new_text);
    free(msgid);
    free(msgstr);
    fclose(in);
    return out;
}
