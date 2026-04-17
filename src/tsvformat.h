#ifndef TSVFORMAT_H
#define TSVFORMAT_H

#include <stddef.h>
#include <stdio.h>

typedef struct {
    char **cells;
    size_t n_cells;
    int line_no;
    char *source_name;
} row_t;

typedef struct {
    row_t *rows;
    size_t len;
    size_t cap;
    size_t n_columns;
} table_t;

typedef int (*table_row_predicate_t)(const row_t *row, void *userdata);
typedef int (*row_cmp_t)(const row_t *a, const row_t *b);
typedef int (*cell_cmp_t)(const char *a, const char *b);

row_t *row_new(void);
row_t *row_dup(const row_t *src);
int row_resize(row_t *row, size_t n_cells);
int row_append(row_t *row, const char *cell);
int row_insert(row_t *row, size_t index, const char *cell);
void row_clear(row_t *row);
void row_free(row_t *row);
const char *row_get(const row_t *row, size_t idx);
size_t row_col_count(const row_t *row);
row_t *row_parse(const char *line);

table_t *table_new(void);
void table_clear(table_t *table);
void table_free(table_t *table);

int table_insert_row(table_t *table, size_t index, const row_t *row, const char *source_name, int line_no);
int table_append_row(table_t *table, const row_t *row, const char *source_name, int line_no);
int table_remove_row(table_t *table, size_t index);
long table_find_row(const table_t *table, table_row_predicate_t predicate, void *userdata);
table_t *table_find_rows(const table_t *table, table_row_predicate_t predicate, void *userdata);
int table_sort_rows(table_t *table, row_cmp_t comparators[]);
int table_sort_rows2(table_t *table, const int columns[], cell_cmp_t comparators[]);
table_t *table_parse(FILE *in, const char *source_name);

#endif
