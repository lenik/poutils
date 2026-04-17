#define _POSIX_C_SOURCE 200809L

#include <check.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/poformat.h"

static void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    ck_assert_ptr_nonnull(f);
    ck_assert_uint_eq(fwrite(content, 1, strlen(content), f), strlen(content));
    fclose(f);
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    ck_assert_ptr_nonnull(f);
    ck_assert_int_eq(fseek(f, 0, SEEK_END), 0);
    long n = ftell(f);
    ck_assert_int_ne(n, -1);
    ck_assert_int_eq(fseek(f, 0, SEEK_SET), 0);
    char *buf = malloc((size_t)n + 1);
    ck_assert_ptr_nonnull(buf);
    ck_assert_uint_eq(fread(buf, 1, (size_t)n, f), (size_t)n);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

START_TEST(test_load_translation_file_parses_escapes) {
    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "/tmp/poformat-map-%ld-%d.txt", (long)getpid(), rand());
    write_file(tmpl, "hello\\nworld\tnihao\\nshijie\nalpha\tbeta\n");

    translation_map_t map;
    translation_map_init(&map);
    ck_assert_int_eq(load_translation_file(tmpl, &map), 0);
    ck_assert_str_eq(translation_map_get(&map, "hello\nworld"), "nihao\nshijie");
    ck_assert_str_eq(translation_map_get(&map, "alpha"), "beta");
    translation_map_free(&map);
    unlink(tmpl);
}
END_TEST

START_TEST(test_update_po_file_rewrite_and_drop_fuzzy) {
    char po_tmpl[PATH_MAX];
    snprintf(po_tmpl, sizeof(po_tmpl), "/tmp/poformat-po-%ld-%d.po", (long)getpid(), rand());
    write_file(po_tmpl,
               "#: src/demo.c:1\n"
               "#, fuzzy\n"
               "msgid \"hello\\n\"\n"
               "msgstr \"old\"\n");

    translation_map_t map;
    translation_map_init(&map);
    ck_assert_int_eq(translation_map_add(&map, "hello\n", "new\ntext"), 0);
    po_update_result_t r = update_po_file(po_tmpl, &map);
    ck_assert_int_eq(r.changed, 1);
    ck_assert_int_eq(r.updated_entries, 1);
    ck_assert_int_eq(r.removed_fuzzy, 1);

    char *updated = read_file(po_tmpl);
    ck_assert_ptr_null(strstr(updated, "#, fuzzy"));
    ck_assert_ptr_nonnull(strstr(updated, "msgstr \"new\\ntext\""));
    free(updated);
    translation_map_free(&map);
    unlink(po_tmpl);
}
END_TEST

START_TEST(test_po_unescape_c_string_edge_cases) {
    char *s1 = po_unescape_c_string("a\\\\b\\nc\\t\\r\\\"z");
    ck_assert_ptr_nonnull(s1);
    ck_assert_str_eq(s1, "a\\b\nc\t\r\"z");
    free(s1);

    char *s2 = po_unescape_c_string("trail\\");
    ck_assert_ptr_nonnull(s2);
    ck_assert_str_eq(s2, "trail\\");
    free(s2);
}
END_TEST

START_TEST(test_translation_map_add_overwrites_existing_key) {
    translation_map_t map;
    translation_map_init(&map);
    ck_assert_int_eq(translation_map_add(&map, "k", "v1"), 0);
    ck_assert_int_eq(translation_map_add(&map, "k", "v2"), 0);
    ck_assert_str_eq(translation_map_get(&map, "k"), "v2");
    translation_map_free(&map);
}
END_TEST

START_TEST(test_load_translation_file_invalid_line_returns_minus2) {
    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "/tmp/poformat-bad-%ld-%d.txt", (long)getpid(), rand());
    write_file(tmpl, "valid\tok\ninvalid_without_tab\n");

    translation_map_t map;
    translation_map_init(&map);
    ck_assert_int_eq(load_translation_file(tmpl, &map), -2);
    translation_map_free(&map);
    unlink(tmpl);
}
END_TEST

START_TEST(test_update_po_file_no_changes_returns_zero_flags) {
    char po_tmpl[PATH_MAX];
    snprintf(po_tmpl, sizeof(po_tmpl), "/tmp/poformat-nochange-%ld-%d.po", (long)getpid(), rand());
    write_file(po_tmpl,
               "msgid \"hello\"\n"
               "msgstr \"world\"\n");

    translation_map_t map;
    translation_map_init(&map);
    ck_assert_int_eq(translation_map_add(&map, "missing", "x"), 0);
    po_update_result_t r = update_po_file(po_tmpl, &map);
    ck_assert_int_eq(r.changed, 0);
    ck_assert_int_eq(r.updated_entries, 0);
    translation_map_free(&map);
    unlink(po_tmpl);
}
END_TEST

static Suite *poformat_suite(void) {
    Suite *s = suite_create("poformat");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_po_unescape_c_string_edge_cases);
    tcase_add_test(tc, test_translation_map_add_overwrites_existing_key);
    tcase_add_test(tc, test_load_translation_file_parses_escapes);
    tcase_add_test(tc, test_load_translation_file_invalid_line_returns_minus2);
    tcase_add_test(tc, test_update_po_file_rewrite_and_drop_fuzzy);
    tcase_add_test(tc, test_update_po_file_no_changes_returns_zero_flags);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(poformat_suite());
    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return failed == 0 ? 0 : 1;
}
