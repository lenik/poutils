#define _POSIX_C_SOURCE 200809L

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/tsvformat.h"

static int match_lang(const row_t *row, void *userdata) {
    const char *want = (const char *)userdata;
    const char *got = row_get(row, 0);
    return got && strcmp(got, want) == 0;
}

static int cmp_row_by_col0(const row_t *a, const row_t *b) {
    const char *av = row_get(a, 0);
    const char *bv = row_get(b, 0);
    if (!av) av = "";
    if (!bv) bv = "";
    return strcmp(av, bv);
}

static int cmp_row_by_col1(const row_t *a, const row_t *b) {
    const char *av = row_get(a, 1);
    const char *bv = row_get(b, 1);
    if (!av) av = "";
    if (!bv) bv = "";
    return strcmp(av, bv);
}

static int cmp_desc(const char *a, const char *b) {
    if (!a) a = "";
    if (!b) b = "";
    return -strcmp(a, b);
}

START_TEST(test_table_parse_line_plain_cells) {
    row_t *row = row_parse("a\tb\tc");
    ck_assert_ptr_nonnull(row);
    ck_assert_uint_eq(row_col_count(row), 3);
    ck_assert_str_eq(row_get(row, 0), "a");
    ck_assert_str_eq(row_get(row, 1), "b");
    ck_assert_str_eq(row_get(row, 2), "c");
    row_free(row);
}
END_TEST

START_TEST(test_table_parse_line_with_quotes_and_tab) {
    row_t *row = row_parse("\"lang\"\t\"msg\\tid\"\t\"msg\\\"str\"");
    ck_assert_ptr_nonnull(row);
    ck_assert_uint_eq(row_col_count(row), 3);
    ck_assert_str_eq(row_get(row, 0), "lang");
    ck_assert_str_eq(row_get(row, 1), "msg\\tid");
    ck_assert_str_eq(row_get(row, 2), "msg\\\"str");
    row_free(row);
}
END_TEST

START_TEST(test_table_read_stream_skips_comments_and_blank) {
    const char *src = "# comment\n\nzh_CN\thello\tworld\n";
    FILE *f = tmpfile();
    ck_assert_ptr_nonnull(f);
    ck_assert_uint_eq(fwrite(src, 1, strlen(src), f), strlen(src));
    rewind(f);

    table_t *table = table_parse(f, "<mem>");
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    ck_assert_uint_eq(table->len, 1);
    ck_assert_str_eq(row_get(&table->rows[0], 0), "zh_CN");
    ck_assert_str_eq(row_get(&table->rows[0], 1), "hello");
    ck_assert_str_eq(row_get(&table->rows[0], 2), "world");
    table_free(table);
    fclose(f);
}
END_TEST

START_TEST(test_row_dup_and_table_insert_row) {
    row_t *row = row_parse("a\tb");
    ck_assert_ptr_nonnull(row);
    row_t *dup = row_dup(row);
    ck_assert_ptr_nonnull(dup);
    ck_assert_str_eq(row_get(dup, 0), "a");
    ck_assert_str_eq(row_get(dup, 1), "b");

    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    ck_assert_int_eq(table_append_row(table, row, "s1", 1), 0);
    ck_assert_int_eq(table_insert_row(table, 0, dup, "s0", 7), 0);
    ck_assert_uint_eq(table->len, 2);
    ck_assert_str_eq(row_get(&table->rows[0], 0), "a");
    ck_assert_int_eq(table->rows[0].line_no, 7);
    ck_assert_str_eq(table->rows[0].source_name, "s0");

    row_free(row);
    row_free(dup);
    table_free(table);
}
END_TEST

START_TEST(test_table_insert_respects_n_columns) {
    row_t *row = row_parse("onlyone");
    ck_assert_ptr_nonnull(row);
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    ck_assert_int_ne(table_append_row(table, row, "src", 1), 0);
    row_free(row);
    table_free(table);
}
END_TEST

START_TEST(test_table_find_and_find_rows_and_remove) {
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;

    row_t *r1 = row_parse("zh_CN\thello");
    row_t *r2 = row_parse("ja\tkonnichiwa");
    row_t *r3 = row_parse("zh_CN\tworld");
    ck_assert_ptr_nonnull(r1);
    ck_assert_ptr_nonnull(r2);
    ck_assert_ptr_nonnull(r3);

    ck_assert_int_eq(table_append_row(table, r1, "s", 1), 0);
    ck_assert_int_eq(table_append_row(table, r2, "s", 2), 0);
    ck_assert_int_eq(table_append_row(table, r3, "s", 3), 0);

    long first_zh = table_find_row(table, match_lang, "zh_CN");
    ck_assert_int_eq((int)first_zh, 0);

    table_t *zh_rows = table_find_rows(table, match_lang, "zh_CN");
    ck_assert_ptr_nonnull(zh_rows);
    ck_assert_uint_eq(zh_rows->len, 2);
    ck_assert_str_eq(row_get(&zh_rows->rows[0], 1), "hello");
    ck_assert_str_eq(row_get(&zh_rows->rows[1], 1), "world");

    ck_assert_int_eq(table_remove_row(table, 1), 0);
    ck_assert_uint_eq(table->len, 2);
    ck_assert_str_eq(row_get(&table->rows[1], 0), "zh_CN");

    row_free(r1);
    row_free(r2);
    row_free(r3);
    table_free(zh_rows);
    table_free(table);
}
END_TEST

START_TEST(test_row_resize_append_insert) {
    row_t *row = row_new();
    ck_assert_ptr_nonnull(row);
    ck_assert_int_eq(row_resize(row, 2), 0);
    ck_assert_uint_eq(row->n_cells, 2);
    ck_assert_str_eq(row_get(row, 0), "");
    ck_assert_str_eq(row_get(row, 1), "");
    ck_assert_int_eq(row_append(row, "tail"), 0);
    ck_assert_int_eq(row_insert(row, 1, "mid"), 0);
    ck_assert_uint_eq(row->n_cells, 4);
    ck_assert_str_eq(row_get(row, 0), "");
    ck_assert_str_eq(row_get(row, 1), "mid");
    ck_assert_str_eq(row_get(row, 2), "");
    ck_assert_str_eq(row_get(row, 3), "tail");
    row_free(row);
}
END_TEST

START_TEST(test_table_sort_rows_and_sort_rows2) {
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    row_t *r1 = row_parse("zh\tb");
    row_t *r2 = row_parse("ja\ta");
    row_t *r3 = row_parse("zh\ta");
    ck_assert_ptr_nonnull(r1);
    ck_assert_ptr_nonnull(r2);
    ck_assert_ptr_nonnull(r3);
    ck_assert_int_eq(table_append_row(table, r1, "s", 1), 0);
    ck_assert_int_eq(table_append_row(table, r2, "s", 2), 0);
    ck_assert_int_eq(table_append_row(table, r3, "s", 3), 0);

    row_cmp_t chain1[] = {cmp_row_by_col0, cmp_row_by_col1, NULL};
    ck_assert_int_eq(table_sort_rows(table, chain1), 0);
    ck_assert_str_eq(row_get(&table->rows[0], 0), "ja");
    ck_assert_str_eq(row_get(&table->rows[1], 0), "zh");
    ck_assert_str_eq(row_get(&table->rows[1], 1), "a");

    int cols[] = {1, 0, -1};
    cell_cmp_t chain2[] = {NULL, NULL, NULL};
    ck_assert_int_eq(table_sort_rows2(table, cols, chain2), 0);
    ck_assert_str_eq(row_get(&table->rows[0], 1), "a");
    ck_assert_str_eq(row_get(&table->rows[1], 1), "a");
    ck_assert_str_eq(row_get(&table->rows[2], 1), "b");

    row_free(r1);
    row_free(r2);
    row_free(r3);
    table_free(table);
}
END_TEST

START_TEST(test_table_sort_rows2_with_custom_cell_cmp) {
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    row_t *r1 = row_parse("zh\ta");
    row_t *r2 = row_parse("ja\tc");
    row_t *r3 = row_parse("en\tb");
    ck_assert_ptr_nonnull(r1);
    ck_assert_ptr_nonnull(r2);
    ck_assert_ptr_nonnull(r3);
    ck_assert_int_eq(table_append_row(table, r1, "s", 1), 0);
    ck_assert_int_eq(table_append_row(table, r2, "s", 2), 0);
    ck_assert_int_eq(table_append_row(table, r3, "s", 3), 0);

    int cols[] = {1, -1};
    cell_cmp_t cmps[] = {cmp_desc, NULL};
    ck_assert_int_eq(table_sort_rows2(table, cols, cmps), 0);
    ck_assert_str_eq(row_get(&table->rows[0], 1), "c");
    ck_assert_str_eq(row_get(&table->rows[1], 1), "b");
    ck_assert_str_eq(row_get(&table->rows[2], 1), "a");

    row_free(r1);
    row_free(r2);
    row_free(r3);
    table_free(table);
}
END_TEST

START_TEST(test_table_find_row_not_found_and_remove_invalid) {
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 1;
    row_t *r = row_parse("zh_CN");
    ck_assert_ptr_nonnull(r);
    ck_assert_int_eq(table_append_row(table, r, "s", 1), 0);
    ck_assert_int_eq((int)table_find_row(table, match_lang, "fr"), -1);
    ck_assert_int_ne(table_remove_row(table, 5), 0);
    ck_assert_int_eq(table_remove_row(table, 0), 0);
    ck_assert_uint_eq(table->len, 0);
    row_free(r);
    table_free(table);
}
END_TEST

START_TEST(test_row_parse_invalid_quote_returns_null) {
    row_t *row = row_parse("\"unterminated\tvalue");
    ck_assert_ptr_null(row);
}
END_TEST

static Suite *tsvformat_suite(void) {
    Suite *s = suite_create("tsvformat");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_table_parse_line_plain_cells);
    tcase_add_test(tc, test_table_parse_line_with_quotes_and_tab);
    tcase_add_test(tc, test_table_read_stream_skips_comments_and_blank);
    tcase_add_test(tc, test_row_dup_and_table_insert_row);
    tcase_add_test(tc, test_table_insert_respects_n_columns);
    tcase_add_test(tc, test_table_find_and_find_rows_and_remove);
    tcase_add_test(tc, test_row_resize_append_insert);
    tcase_add_test(tc, test_table_sort_rows_and_sort_rows2);
    tcase_add_test(tc, test_table_sort_rows2_with_custom_cell_cmp);
    tcase_add_test(tc, test_table_find_row_not_found_and_remove_invalid);
    tcase_add_test(tc, test_row_parse_invalid_quote_returns_null);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(tsvformat_suite());
    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return failed == 0 ? 0 : 1;
}
