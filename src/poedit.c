/*
 * Copyright (C) 2026 Lenik <poutils@bodz.net>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200809L

#include "config.h"

#include "poedit.h"
#include "poformat.h"
#include "tsvformat.h"

#include <bas/locale/i18n.h>
#include <bas/proc/env.h>

#include <dirent.h>
#include <getopt.h>
#include <libintl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { OPT_VERSION = 256 };

typedef struct {
    const char *input_path;
    const char *po_dir;
    const char *lang;
    bool dry_run;
    bool verbose;
    bool quiet;
} options_t;

typedef struct {
    char *lang;
    translation_map_t map;
} lang_map_t;

typedef struct {
    lang_map_t *items;
    size_t len;
    size_t cap;
} lang_map_list_t;

typedef struct {
    int total_entries;
    int entries_with_explicit_lang;
} input_scan_stats_t;

static void fprintln(FILE *out, const char *s) {
    fputs(s, out);
    fputc('\n', out);
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = malloc(n);
    if (!out) {
        return NULL;
    }
    memcpy(out, s, n);
    return out;
}

static char *path_join(const char *a, const char *b) {
    size_t alen = strlen(a), blen = strlen(b);
    bool slash = (alen > 0 && a[alen - 1] != '/');
    char *out = malloc(alen + blen + (slash ? 2 : 1));
    if (!out) {
        return NULL;
    }
    memcpy(out, a, alen);
    if (slash) {
        out[alen] = '/';
        memcpy(out + alen + 1, b, blen + 1);
    } else {
        memcpy(out + alen, b, blen + 1);
    }
    return out;
}

static char *lang_from_filename(const char *path) {
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    const char *dot = strrchr(base, '.');
    size_t n = dot ? (size_t)(dot - base) : strlen(base);
    if (n == 0) {
        return NULL;
    }
    char *lang = malloc(n + 1);
    if (!lang) {
        return NULL;
    }
    memcpy(lang, base, n);
    lang[n] = '\0';
    return lang;
}

static char *lang_from_subdir(const char *path) {
    const char *slash = strrchr(path, '/');
    if (!slash || slash == path) {
        return NULL;
    }
    const char *end = slash;
    const char *start = end;
    while (start > path && start[-1] != '/') {
        start--;
    }
    if (start == end) {
        return NULL;
    }
    size_t n = (size_t)(end - start);
    char *lang = malloc(n + 1);
    if (!lang) {
        return NULL;
    }
    memcpy(lang, start, n);
    lang[n] = '\0';
    return lang;
}

static char *resolve_fallback_lang(const options_t *opt, const char *path_hint) {
    if (opt->lang) {
        return xstrdup(opt->lang);
    }
    if (!path_hint) {
        return NULL;
    }
    char *subdir = lang_from_subdir(path_hint);
    if (subdir && subdir[0] != '\0') {
        return subdir;
    }
    free(subdir);
    return lang_from_filename(path_hint);
}

static char *resolve_target_file_lang(const options_t *opt, const char *path) {
    if (opt->lang) {
        return xstrdup(opt->lang);
    }
    char *name = lang_from_filename(path);
    if (name && name[0] != '\0') {
        return name;
    }
    free(name);
    return lang_from_subdir(path);
}

static void lang_map_list_init(lang_map_list_t *maps) {
    maps->items = NULL;
    maps->len = 0;
    maps->cap = 0;
}

static void lang_map_list_free(lang_map_list_t *maps) {
    for (size_t i = 0; i < maps->len; i++) {
        free(maps->items[i].lang);
        translation_map_free(&maps->items[i].map);
    }
    free(maps->items);
    maps->items = NULL;
    maps->len = 0;
    maps->cap = 0;
}

static translation_map_t *lang_map_get_or_add(lang_map_list_t *maps, const char *lang) {
    for (size_t i = 0; i < maps->len; i++) {
        if (strcmp(maps->items[i].lang, lang) == 0) {
            return &maps->items[i].map;
        }
    }
    if (maps->len == maps->cap) {
        size_t next = maps->cap ? maps->cap * 2 : 8;
        lang_map_t *grown = realloc(maps->items, next * sizeof(*grown));
        if (!grown) {
            return NULL;
        }
        maps->items = grown;
        maps->cap = next;
    }
    lang_map_t *slot = &maps->items[maps->len++];
    slot->lang = xstrdup(lang);
    if (!slot->lang) {
        return NULL;
    }
    translation_map_init(&slot->map);
    return &slot->map;
}

static const translation_map_t *find_lang_map(const lang_map_list_t *maps, const char *lang) {
    for (size_t i = 0; i < maps->len; i++) {
        if (strcmp(maps->items[i].lang, lang) == 0) {
            return &maps->items[i].map;
        }
    }
    return NULL;
}

static void print_design_purpose(FILE *out) {
    fprintln(out, _("Purpose:"));
    fprintln(out, _("  Efficiently apply many already-translated l10n messages into PO files."));
    fprintln(out, _("  poedit is usually used to supplement translations by providing only newly "
                    "added entries."));
    fprintln(out, _("  This tool is mainly designed for AI-assisted coding workflows."));
    fputc('\n', out);
    fprintln(out, _("AI usage requirements:"));
    fprintln(out,
             _("  1) Translate multiple messages in advance (manually) before running poedit."));
    fprintln(out, _("  2) Use poedit for batch update to avoid one-by-one tool calls."));
    fprintln(out, _("  3) Do NOT use poedit to generate placeholders or copy msgid to msgstr."));
    fprintln(out, _("     Doing so defeats the purpose of this tool."));
    fprintln(out, _("  4) AI must first identify missing entries in current translations, "));
    fprintln(out, _("     translate them manually."));
    fprintln(out, _("     then apply to PO files with this tool in one batch."));
}

void usage(FILE *out) {
    fprintln(out, _("Usage: poedit [OPTIONS]"));
    print_design_purpose(out);
    fputc('\n', out);
    fprintln(out, _("Input format: (lang can be inferred from filename or subdir)"));
    fprintln(out, _("  lang<TAB>msgid<TAB>msgstr"));
    fprintln(out, _("-or-"));
    fprintln(out, _("  msgid<TAB>msgstr"));
    fputc('\n', out);
    fprintln(out, _("  lang conflict resolution: [lang<TAB>] > [filename] > [subdir]"));
    fputc('\n', out);
    fprintln(out, _("Options:"));
    fputs("  -i, --input PATH      ", out);
    fprintln(out, _("mapping file or directory (optional: default stdin)"));
    fputs("  -l, --lang LANG       ", out);
    fprintln(out, _("language code for single mapping file"));
    fputs("  -p, --po-dir DIR      ", out);
    fprintln(out, _("PO directory (default: po)"));
    fputs("      --dry-run         ", out);
    fprintln(out, _("show plan but do not modify files"));
    fputs("  -v, --verbose         ", out);
    fprintln(out, _("repeat for more verbose loggings"));
    fputs("  -q, --quiet           ", out);
    fprintln(out, _("show less logging messages"));
    fputs("  -h, --help            ", out);
    fprintln(out, _("show this help and exit"));
    fputs("      --version         ", out);
    fprintln(out, _("show version and exit"));
}

static int parse_args(int argc, char **argv, options_t *opt) {
    memset(opt, 0, sizeof(*opt));
    opt->po_dir = "po";
    static const struct option long_opts[] = {
        {"input", required_argument, NULL, 'i'},
        {"lang", required_argument, NULL, 'l'},
        {"po-dir", required_argument, NULL, 'p'},
        {"dry-run", no_argument, NULL, 'd'},
        {"verbose", no_argument, NULL, 'v'},
        {"quiet", no_argument, NULL, 'q'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, OPT_VERSION},
        {NULL, 0, NULL, 0},
    };
    for (;;) {
        int c = getopt_long(argc, argv, "i:l:p:dvqh", long_opts, NULL);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'i':
            opt->input_path = optarg;
            break;
        case 'l':
            opt->lang = optarg;
            break;
        case 'p':
            opt->po_dir = optarg;
            break;
        case 'd':
            opt->dry_run = true;
            break;
        case 'v':
            opt->verbose = true;
            break;
        case 'q':
            opt->quiet = true;
            break;
        case 'h':
            usage(stdout);
            return 1;
        case OPT_VERSION:
            printf("poedit %s\n", PROJECT_VERSION);
            return 1;
        default:
            usage(stderr);
            return -1;
        }
    }
    return 0;
}

static int analyze_inputs(const options_t *opt, table_t *table) {
    table->n_columns = 2;
    if (!opt->input_path) {
        table_t *parsed = table_parse(stdin, "<stdin>");
        if (!parsed)
            return 1;
        parsed->n_columns = table->n_columns;
        table_clear(table);
        *table = *parsed;
        free(parsed);
        return 0;
    }
    struct stat st = {0};
    if (stat(opt->input_path, &st) != 0) {
        fprintf(stderr, _("error: input not found: %s\n"), opt->input_path);
        return 1;
    }
    if (S_ISREG(st.st_mode)) {
        FILE *f = fopen(opt->input_path, "r");
        if (!f) {
            fprintf(stderr, _("error: failed to read mapping file: %s\n"), opt->input_path);
            return 1;
        }
        table_t *parsed = table_parse(f, opt->input_path);
        fclose(f);
        if (!parsed)
            return 1;
        parsed->n_columns = table->n_columns;
        table_clear(table);
        *table = *parsed;
        free(parsed);
        return 0;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, _("error: unsupported input type: %s\n"), opt->input_path);
        return 1;
    }

    DIR *d = opendir(opt->input_path);
    if (!d) {
        fprintf(stderr, _("error: failed to open input directory: %s\n"), opt->input_path);
        return 1;
    }
    int rc = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 ||
            ent->d_name[0] == '.') {
            continue;
        }
        char *path = path_join(opt->input_path, ent->d_name);
        if (!path) {
            rc = 1;
            break;
        }
        struct stat fst = {0};
        if (stat(path, &fst) == 0 && S_ISREG(fst.st_mode)) {
            FILE *f = fopen(path, "r");
            if (!f) {
                free(path);
                rc = 1;
                break;
            }
            table_t *parsed = table_parse(f, path);
            if (!parsed) {
                rc = 1;
            } else {
                parsed->n_columns = table->n_columns;
                for (size_t i = 0; i < parsed->len; i++) {
                    if (table_append_row(table, &parsed->rows[i], parsed->rows[i].source_name,
                                         parsed->rows[i].line_no) != 0) {
                        rc = 1;
                        break;
                    }
                }
                table_free(parsed);
            }
            fclose(f);
            if (rc != 0) {
                free(path);
                break;
            }
        }
        free(path);
    }
    closedir(d);
    return rc;
}

static int build_maps_from_entries(const options_t *opt, const table_t *table,
                                   lang_map_list_t *maps) {
    input_scan_stats_t stats = {0, 0};
    for (size_t i = 0; i < table->len; i++) {
        size_t cols = row_col_count(&table->rows[i]);
        if (cols == 2 || cols == 3) {
            stats.total_entries++;
        }
        if (cols == 3 && row_get(&table->rows[i], 0) && row_get(&table->rows[i], 0)[0] != '\0') {
            stats.entries_with_explicit_lang++;
        }
    }
    if (stats.total_entries > 0 && stats.entries_with_explicit_lang == 0) {
        fprintln(stderr, _("error: input contains only msgid<TAB>msgstr entries; explicit language "
                           "is required."));
        fprintln(stderr, _("Please use [lang<TAB>]msgid<TAB>msgstr format."));
        fputc('\n', stderr);
        print_design_purpose(stderr);
        return 1;
    }
    for (size_t i = 0; i < table->len; i++) {
        const row_t *row = &table->rows[i];
        size_t cols = row_col_count(row);
        if (cols != 2 && cols != 3) {
            fprintf(stderr, _("error: invalid field count in %s:%d\n"), row->source_name,
                    row->line_no);
            return 1;
        }
        char *fallback = resolve_fallback_lang(opt, row->source_name);
        const char *inline_lang = (cols == 3) ? row_get(row, 0) : NULL;
        const char *chosen =
            (inline_lang && inline_lang[0]) ? inline_lang : (opt->lang ? opt->lang : fallback);
        if (!chosen) {
            fprintf(stderr, _("error: no language resolved for %s:%d\n"), row->source_name,
                    row->line_no);
            free(fallback);
            return 1;
        }
        const char *raw_msgid = (cols == 3) ? row_get(row, 1) : row_get(row, 0);
        const char *raw_msgstr = (cols == 3) ? row_get(row, 2) : row_get(row, 1);
        char *msgid = po_unescape_c_string(raw_msgid);
        char *msgstr = po_unescape_c_string(raw_msgstr);
        if (!msgid || !msgstr) {
            free(msgid);
            free(msgstr);
            free(fallback);
            return 1;
        }
        translation_map_t *map = lang_map_get_or_add(maps, chosen);
        if (!map || translation_map_add(map, msgid, msgstr) != 0) {
            free(msgid);
            free(msgstr);
            free(fallback);
            return 1;
        }
        free(msgid);
        free(msgstr);
        free(fallback);
    }
    return 0;
}

static int update_one_language(const options_t *opt, const char *lang, const char *po_path,
                               const translation_map_t *map) {
    po_update_result_t r = update_po_file(po_path, map);
    if (opt->dry_run && !opt->quiet) {
        printf(_("[dry-run] %s: %s (updated=%d, fuzzy_removed=%d)\n"), lang,
               r.changed ? _("would update") : _("no changes"), r.updated_entries, r.removed_fuzzy);
    } else if (!opt->quiet && (opt->verbose || r.changed)) {
        printf(_("%s: %s (updated=%d, fuzzy_removed=%d)\n"), lang,
               r.changed ? _("updated") : _("no changes"), r.updated_entries, r.removed_fuzzy);
    }
    return 0;
}

static int apply_to_default_targets(const options_t *opt, const lang_map_list_t *maps) {
    int rc = 0;
    for (size_t i = 0; i < maps->len; i++) {
        char po_name[256];
        snprintf(po_name, sizeof(po_name), "%s.po", maps->items[i].lang);
        char *po_path = path_join(opt->po_dir, po_name);
        if (!po_path) {
            rc = 1;
            continue;
        }
        struct stat st = {0};
        if (stat(po_path, &st) == 0) {
            if (update_one_language(opt, maps->items[i].lang, po_path, &maps->items[i].map) != 0) {
                rc = 1;
            }
        } else {
            fprintf(stderr, _("warn: skip missing PO file: %s\n"), po_path);
        }
        free(po_path);
    }
    return rc;
}

static int apply_to_file_targets(const options_t *opt, const lang_map_list_t *maps, int argc,
                                 char **argv) {
    int rc = 0;
    for (int i = 0; i < argc; i++) {
        char *lang = resolve_target_file_lang(opt, argv[i]);
        if (!lang) {
            fprintf(stderr, _("error: cannot infer language from target file: %s\n"), argv[i]);
            rc = 1;
            continue;
        }
        const translation_map_t *map = find_lang_map(maps, lang);
        if (!map) {
            fprintf(stderr, _("warn: no mapping found for language %s; skip %s\n"), lang, argv[i]);
            free(lang);
            continue;
        }
        if (update_one_language(opt, lang, argv[i], map) != 0) {
            rc = 1;
        }
        free(lang);
    }
    return rc;
}

int main(int argc, char **argv) {
    const char *exe = self_exe();
    init_i18n(LOCALEDIR);

    options_t opt;
    int parse_rc = parse_args(argc, argv, &opt);
    if (parse_rc != 0) {
        return parse_rc > 0 ? 0 : 1;
    }

    table_t *table = table_new();
    if (!table) {
        return 1;
    }
    if (analyze_inputs(&opt, table) != 0) {
        table_free(table);
        return 1;
    }

    lang_map_list_t maps;
    lang_map_list_init(&maps);
    if (build_maps_from_entries(&opt, table, &maps) != 0) {
        table_free(table);
        lang_map_list_free(&maps);
        return 1;
    }
    table_free(table);
    if (maps.len == 0) {
        fprintf(stderr, "%s\n", _("error: no mapping entries loaded"));
        lang_map_list_free(&maps);
        return 1;
    }

    argc -= optind;
    argv += optind;
    int rc = (argc > 0) ? apply_to_file_targets(&opt, &maps, argc, argv)
                        : apply_to_default_targets(&opt, &maps);
    lang_map_list_free(&maps);
    return rc;
}
