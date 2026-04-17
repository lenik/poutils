#define _POSIX_C_SOURCE 200809L

#include <check.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define main main_renamed
#include "../src/poedit.c"
#undef main

static void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    ck_assert_ptr_nonnull(f);
    ck_assert_uint_eq(fwrite(content, 1, strlen(content), f), strlen(content));
    fclose(f);
}

START_TEST(test_resolve_target_file_lang_by_filename) {
    options_t opt = {0};
    char *lang = resolve_target_file_lang(&opt, "po/zh_CN.po");
    ck_assert_ptr_nonnull(lang);
    ck_assert_str_eq(lang, "zh_CN");
    free(lang);
}
END_TEST

START_TEST(test_resolve_target_file_lang_with_override) {
    options_t opt = {0};
    opt.lang = "ja";
    char *lang = resolve_target_file_lang(&opt, "po/zh_CN.po");
    ck_assert_ptr_nonnull(lang);
    ck_assert_str_eq(lang, "ja");
    free(lang);
}
END_TEST

START_TEST(test_resolve_fallback_lang_prefers_subdir) {
    options_t opt = {0};
    char *lang = resolve_fallback_lang(&opt, "inputs/ko/map.tsv");
    ck_assert_ptr_nonnull(lang);
    ck_assert_str_eq(lang, "ko");
    free(lang);
}
END_TEST

START_TEST(test_resolve_fallback_lang_uses_filename) {
    options_t opt = {0};
    char *lang = resolve_fallback_lang(&opt, "zh_TW.tsv");
    ck_assert_ptr_nonnull(lang);
    ck_assert_str_eq(lang, "zh_TW");
    free(lang);
}
END_TEST

START_TEST(test_build_maps_from_entries_rejects_missing_explicit_lang) {
    options_t opt = {0};
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    row_t *r = row_parse("hello\tworld");
    ck_assert_ptr_nonnull(r);
    ck_assert_int_eq(table_append_row(table, r, "maps/zh_CN.tsv", 1), 0);

    lang_map_list_t maps;
    lang_map_list_init(&maps);
    ck_assert_int_ne(build_maps_from_entries(&opt, table, &maps), 0);

    lang_map_list_free(&maps);
    row_free(r);
    table_free(table);
}
END_TEST

START_TEST(test_build_maps_from_entries_accepts_explicit_lang_and_updates_map) {
    options_t opt = {0};
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    row_t *r = row_parse("zh_CN\thello\\n\tworld");
    ck_assert_ptr_nonnull(r);
    ck_assert_int_eq(table_append_row(table, r, "maps/any.tsv", 3), 0);

    lang_map_list_t maps;
    lang_map_list_init(&maps);
    ck_assert_int_eq(build_maps_from_entries(&opt, table, &maps), 0);
    ck_assert_uint_eq(maps.len, 1);
    ck_assert_str_eq(maps.items[0].lang, "zh_CN");
    ck_assert_str_eq(translation_map_get(&maps.items[0].map, "hello\n"), "world");

    lang_map_list_free(&maps);
    row_free(r);
    table_free(table);
}
END_TEST

START_TEST(test_build_maps_from_entries_invalid_column_count) {
    options_t opt = {0};
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    table->n_columns = 2;
    row_t *r = row_parse("a\tb\tc\td");
    ck_assert_ptr_nonnull(r);
    ck_assert_int_eq(table_append_row(table, r, "maps/any.tsv", 4), 0);

    lang_map_list_t maps;
    lang_map_list_init(&maps);
    ck_assert_int_ne(build_maps_from_entries(&opt, table, &maps), 0);

    lang_map_list_free(&maps);
    row_free(r);
    table_free(table);
}
END_TEST

START_TEST(test_analyze_inputs_nonexistent_file_returns_error) {
    options_t opt = {0};
    opt.input_path = "/definitely/not/exist.tsv";
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    ck_assert_int_ne(analyze_inputs(&opt, table), 0);
    table_free(table);
}
END_TEST

START_TEST(test_analyze_inputs_directory_merges_multiple_files) {
    char dir[PATH_MAX];
    snprintf(dir, sizeof(dir), "/tmp/poedit-unit-dir-%ld-%d", (long)getpid(), rand());
    ck_assert_int_eq(mkdir(dir, 0700), 0);

    char f1[PATH_MAX], f2[PATH_MAX];
    snprintf(f1, sizeof(f1), "%s/zh_CN.tsv", dir);
    snprintf(f2, sizeof(f2), "%s/ja.tsv", dir);
    write_file(f1, "zh_CN\thello\tworld\n");
    write_file(f2, "ja\tcat\tneko\n");

    options_t opt = {0};
    opt.input_path = dir;
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    ck_assert_int_eq(analyze_inputs(&opt, table), 0);
    ck_assert_uint_eq(table->len, 2);

    lang_map_list_t maps;
    lang_map_list_init(&maps);
    ck_assert_int_eq(build_maps_from_entries(&opt, table, &maps), 0);
    ck_assert_uint_eq(maps.len, 2);

    const translation_map_t *zh = find_lang_map(&maps, "zh_CN");
    const translation_map_t *ja = find_lang_map(&maps, "ja");
    ck_assert_ptr_nonnull(zh);
    ck_assert_ptr_nonnull(ja);
    ck_assert_str_eq(translation_map_get(zh, "hello"), "world");
    ck_assert_str_eq(translation_map_get(ja, "cat"), "neko");

    lang_map_list_free(&maps);
    table_free(table);
    ck_assert_int_eq(unlink(f1), 0);
    ck_assert_int_eq(unlink(f2), 0);
    ck_assert_int_eq(rmdir(dir), 0);
}
END_TEST

START_TEST(test_analyze_inputs_directory_invalid_file_fails) {
    char dir[PATH_MAX];
    snprintf(dir, sizeof(dir), "/tmp/poedit-unit-baddir-%ld-%d", (long)getpid(), rand());
    ck_assert_int_eq(mkdir(dir, 0700), 0);

    char bad[PATH_MAX];
    snprintf(bad, sizeof(bad), "%s/bad.tsv", dir);
    write_file(bad, "\"unterminated\tline\n");

    options_t opt = {0};
    opt.input_path = dir;
    table_t *table = table_new();
    ck_assert_ptr_nonnull(table);
    ck_assert_int_ne(analyze_inputs(&opt, table), 0);

    table_free(table);
    ck_assert_int_eq(unlink(bad), 0);
    ck_assert_int_eq(rmdir(dir), 0);
}
END_TEST

static Suite *poedit_unit_suite(void) {
    Suite *s = suite_create("poedit");
    TCase *tc_core = tcase_create("core");

    tcase_add_test(tc_core, test_resolve_target_file_lang_by_filename);
    tcase_add_test(tc_core, test_resolve_target_file_lang_with_override);
    tcase_add_test(tc_core, test_resolve_fallback_lang_prefers_subdir);
    tcase_add_test(tc_core, test_resolve_fallback_lang_uses_filename);
    tcase_add_test(tc_core, test_build_maps_from_entries_rejects_missing_explicit_lang);
    tcase_add_test(tc_core, test_build_maps_from_entries_accepts_explicit_lang_and_updates_map);
    tcase_add_test(tc_core, test_build_maps_from_entries_invalid_column_count);
    tcase_add_test(tc_core, test_analyze_inputs_nonexistent_file_returns_error);
    tcase_add_test(tc_core, test_analyze_inputs_directory_merges_multiple_files);
    tcase_add_test(tc_core, test_analyze_inputs_directory_invalid_file_fails);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    Suite *s = poedit_unit_suite();
    SRunner *sr = srunner_create(s);
    int failed;

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return failed == 0 ? 0 : 1;
}
