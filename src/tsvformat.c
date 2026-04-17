#define _POSIX_C_SOURCE 200809L

#include "tsvformat.h"

#include <stdbool.h>
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

static void trim_eol(char *line) {
    size_t n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

row_t *row_new(void) {
    row_t *row = calloc(1, sizeof(*row));
    return row;
}

row_t *row_dup(const row_t *src) {
    if (!src) {
        return NULL;
    }
    row_t *out = row_new();
    if (!out) {
        return NULL;
    }
    out->cells = calloc(src->n_cells, sizeof(char *));
    if (!out->cells) {
        free(out);
        return NULL;
    }
    out->n_cells = src->n_cells;
    out->line_no = src->line_no;
    out->source_name = xstrdup(src->source_name ? src->source_name : "");
    if (!out->source_name) {
        row_clear(out);
        free(out);
        return NULL;
    }
    for (size_t i = 0; i < src->n_cells; i++) {
        out->cells[i] = xstrdup(src->cells[i] ? src->cells[i] : "");
        if (!out->cells[i]) {
            row_clear(out);
            free(out);
            return NULL;
        }
    }
    return out;
}

int row_resize(row_t *row, size_t n_cells) {
    if (!row) {
        return -1;
    }
    if (n_cells == row->n_cells) {
        return 0;
    }
    if (n_cells < row->n_cells) {
        for (size_t i = n_cells; i < row->n_cells; i++) {
            free(row->cells[i]);
        }
        if (n_cells == 0) {
            free(row->cells);
            row->cells = NULL;
        } else {
            char **shrunk = realloc(row->cells, n_cells * sizeof(*shrunk));
            if (shrunk) {
                row->cells = shrunk;
            }
        }
        row->n_cells = n_cells;
        return 0;
    }

    char **grown = realloc(row->cells, n_cells * sizeof(*grown));
    if (!grown) {
        return -1;
    }
    row->cells = grown;
    for (size_t i = row->n_cells; i < n_cells; i++) {
        row->cells[i] = xstrdup("");
        if (!row->cells[i]) {
            for (size_t j = row->n_cells; j < i; j++) {
                free(row->cells[j]);
                row->cells[j] = NULL;
            }
            return -1;
        }
    }
    row->n_cells = n_cells;
    return 0;
}

int row_append(row_t *row, const char *cell) {
    if (!row) {
        return -1;
    }
    size_t old = row->n_cells;
    if (row_resize(row, old + 1) != 0) {
        return -1;
    }
    free(row->cells[old]);
    row->cells[old] = xstrdup(cell ? cell : "");
    if (!row->cells[old]) {
        return -1;
    }
    return 0;
}

int row_insert(row_t *row, size_t index, const char *cell) {
    if (!row || index > row->n_cells) {
        return -1;
    }
    size_t old = row->n_cells;
    if (row_resize(row, old + 1) != 0) {
        return -1;
    }
    if (index < old) {
        char *tail = row->cells[old];
        memmove(&row->cells[index + 1], &row->cells[index], (old - index) * sizeof(char *));
        row->cells[index] = tail;
    }
    free(row->cells[index]);
    row->cells[index] = xstrdup(cell ? cell : "");
    if (!row->cells[index]) {
        return -1;
    }
    return 0;
}

void row_clear(row_t *row) {
    if (!row) {
        return;
    }
    for (size_t i = 0; i < row->n_cells; i++) free(row->cells[i]);
    free(row->cells);
    free(row->source_name);
    row->cells = NULL;
    row->n_cells = 0;
    row->line_no = 0;
    row->source_name = NULL;
}

void row_free(row_t *row) {
    if (!row) {
        return;
    }
    row_clear(row);
    free(row);
}

const char *row_get(const row_t *row, size_t idx) { return (!row || idx >= row->n_cells) ? NULL : row->cells[idx]; }
size_t row_col_count(const row_t *row) { return row ? row->n_cells : 0; }

table_t *table_new(void) {
    return calloc(1, sizeof(table_t));
}

void table_clear(table_t *table) {
    if (!table) {
        return;
    }
    for (size_t i = 0; i < table->len; i++) row_clear(&table->rows[i]);
    free(table->rows);
    table->rows = NULL;
    table->len = 0;
    table->cap = 0;
}

void table_free(table_t *table) {
    if (!table) {
        return;
    }
    table_clear(table);
    free(table);
}

static int table_parse_line(const char *line, row_t *out_row) {
    memset(out_row, 0, sizeof(*out_row));
    char **cells = NULL;
    size_t cells_len = 0, cells_cap = 0;
    char *cur = NULL;
    size_t cur_len = 0, cur_cap = 0;
    bool in_quotes = false, escaped = false;

    for (const char *p = line;; p++) {
        char c = *p;
        bool end = (c == '\0');
        bool split = (!end && !in_quotes && c == '\t');
        if (end || split) {
            if (!cur) {
                cur = xstrdup("");
                if (!cur) goto fail;
            } else {
                cur[cur_len] = '\0';
            }
            if (cells_len == cells_cap) {
                size_t next = cells_cap ? cells_cap * 2 : 4;
                char **grown = realloc(cells, next * sizeof(*grown));
                if (!grown) goto fail;
                cells = grown;
                cells_cap = next;
            }
            cells[cells_len++] = cur;
            cur = NULL;
            cur_len = 0;
            cur_cap = 0;
            if (end) break;
            continue;
        }
        if (!escaped && c == '"') { in_quotes = !in_quotes; continue; }
        escaped = (!escaped && c == '\\');
        if (cur_len + 2 > cur_cap) {
            size_t next = cur_cap ? cur_cap * 2 : 32;
            char *grown = realloc(cur, next);
            if (!grown) goto fail;
            cur = grown;
            cur_cap = next;
        }
        cur[cur_len++] = c;
    }
    if (in_quotes) goto fail;
    out_row->cells = cells;
    out_row->n_cells = cells_len;
    return 0;
fail:
    for (size_t i = 0; i < cells_len; i++) free(cells[i]);
    free(cells);
    free(cur);
    return -1;
}

row_t *row_parse(const char *line) {
    row_t *row = row_new();
    if (!row) {
        return NULL;
    }
    if (table_parse_line(line, row) != 0) {
        row_free(row);
        return NULL;
    }
    return row;
}

int table_insert_row(table_t *table, size_t index, const row_t *row, const char *source_name, int line_no) {
    if (!table || !row) {
        return -1;
    }
    if (index > table->len) {
        return -1;
    }
    if (table->n_columns > 0 && row->n_cells < table->n_columns) {
        return -1;
    }
    if (table->len == table->cap) {
        size_t next = table->cap ? table->cap * 2 : 64;
        row_t *grown = realloc(table->rows, next * sizeof(*grown));
        if (!grown) return -1;
        table->rows = grown;
        table->cap = next;
    }
    if (index < table->len) {
        memmove(&table->rows[index + 1], &table->rows[index], (table->len - index) * sizeof(row_t));
    }
    memset(&table->rows[index], 0, sizeof(row_t));

    row_t *dup = row_dup(row);
    if (!dup) {
        if (index < table->len) {
            memmove(&table->rows[index], &table->rows[index + 1], (table->len - index) * sizeof(row_t));
        }
        return -1;
    }
    table->rows[index] = *dup;
    free(dup);
    table->rows[index].line_no = line_no;
    free(table->rows[index].source_name);
    table->rows[index].source_name = xstrdup(source_name ? source_name : "");
    if (!table->rows[index].source_name) {
        row_clear(&table->rows[index]);
        if (index < table->len) {
            memmove(&table->rows[index], &table->rows[index + 1], (table->len - index) * sizeof(row_t));
        }
        return -1;
    }
    table->len++;
    return 0;
}

int table_append_row(table_t *table, const row_t *row, const char *source_name, int line_no) {
    if (!table) {
        return -1;
    }
    return table_insert_row(table, table->len, row, source_name, line_no);
}

int table_remove_row(table_t *table, size_t index) {
    if (!table || index >= table->len) {
        return -1;
    }
    row_clear(&table->rows[index]);
    if (index + 1 < table->len) {
        memmove(&table->rows[index], &table->rows[index + 1], (table->len - index - 1) * sizeof(row_t));
    }
    table->len--;
    memset(&table->rows[table->len], 0, sizeof(row_t));
    return 0;
}

long table_find_row(const table_t *table, table_row_predicate_t predicate, void *userdata) {
    if (!table || !predicate) {
        return -1;
    }
    for (size_t i = 0; i < table->len; i++) {
        if (predicate(&table->rows[i], userdata)) {
            return (long)i;
        }
    }
    return -1;
}

table_t *table_find_rows(const table_t *table, table_row_predicate_t predicate, void *userdata) {
    if (!table || !predicate) {
        return NULL;
    }
    table_t *out = table_new();
    if (!out) {
        return NULL;
    }
    out->n_columns = table->n_columns;
    for (size_t i = 0; i < table->len; i++) {
        const row_t *row = &table->rows[i];
        if (!predicate(row, userdata)) {
            continue;
        }
        if (table_append_row(out, row, row->source_name, row->line_no) != 0) {
            table_free(out);
            return NULL;
        }
    }
    return out;
}

static row_cmp_t *g_row_cmp_chain = NULL;
static const int *g_sort_columns = NULL;
static cell_cmp_t *g_cell_cmp_chain = NULL;

static int qsort_row_cmp(const void *a, const void *b) {
    const row_t *ra = (const row_t *)a;
    const row_t *rb = (const row_t *)b;
    if (!g_row_cmp_chain) {
        return 0;
    }
    for (size_t i = 0; g_row_cmp_chain[i] != NULL; i++) {
        int r = g_row_cmp_chain[i](ra, rb);
        if (r != 0) {
            return r;
        }
    }
    return 0;
}

int table_sort_rows(table_t *table, row_cmp_t comparators[]) {
    if (!table || !comparators) {
        return -1;
    }
    g_row_cmp_chain = comparators;
    qsort(table->rows, table->len, sizeof(row_t), qsort_row_cmp);
    g_row_cmp_chain = NULL;
    return 0;
}

static int qsort_cell_chain_cmp(const void *a, const void *b) {
    const row_t *ra = (const row_t *)a;
    const row_t *rb = (const row_t *)b;
    for (size_t i = 0; g_sort_columns && g_sort_columns[i] >= 0; i++) {
        int col = g_sort_columns[i];
        const char *ca = (col < (int)ra->n_cells) ? ra->cells[col] : "";
        const char *cb = (col < (int)rb->n_cells) ? rb->cells[col] : "";
        cell_cmp_t cmp = g_cell_cmp_chain ? g_cell_cmp_chain[i] : NULL;
        int r = cmp ? cmp(ca, cb) : strcmp(ca, cb);
        if (r != 0) {
            return r;
        }
    }
    return 0;
}

int table_sort_rows2(table_t *table, const int columns[], cell_cmp_t comparators[]) {
    if (!table || !columns) {
        return -1;
    }
    g_sort_columns = columns;
    g_cell_cmp_chain = comparators;
    qsort(table->rows, table->len, sizeof(row_t), qsort_cell_chain_cmp);
    g_sort_columns = NULL;
    g_cell_cmp_chain = NULL;
    return 0;
}

table_t *table_parse(FILE *in, const char *source_name) {
    if (!in) {
        return NULL;
    }
    table_t *table = table_new();
    if (!table) {
        return NULL;
    }
    char line[16384];
    int line_no = 0;
    while (fgets(line, sizeof(line), in)) {
        line_no++;
        trim_eol(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        row_t row;
        if (table_parse_line(line, &row) != 0) {
            table_free(table);
            return NULL;
        }
        if (table_append_row(table, &row, source_name, line_no) != 0) {
            row_clear(&row);
            table_free(table);
            return NULL;
        }
        row_clear(&row);
    }
    return table;
}
