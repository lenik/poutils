# poutils

`poutils` is a Meson-based project template for small C/C++ command-line apps.
`poedit` is one **example app** in this template; more apps can be added in the same repository.

## Repository layout

- `src/` - source code for apps and shared pieces
- `tests/` - unit tests (`*_unit.c`) using the Check framework
- `debian/` - Debian packaging metadata
- `meson.build` - top-level build definition and helper targets

## Example app: `poedit`

`poedit` updates `po/*.po` from translation mapping inputs.

```bash
poedit [OPTIONS] [FILE...]
```

Common usage:

```bash
# 1) Read mappings from stdin, update po/<lang>.po
poedit < mappings.tsv

# 2) Read mappings from one file
poedit --input mappings.tsv

# 3) Read mappings from a directory (batch)
poedit --input mappings-dir/

# 4) Update only selected PO files
poedit --input mappings-dir/ po/zh_CN.po po/ja.po
```

Input source auto-detection:

- no `--input`: read mapping lines from `stdin`
- `--input FILE`: read one mapping file
- `--input DIR`: read all regular files in that directory

Mapping line format (UTF-8):

```text
[lang<TAB>]msgid<TAB>msgstr
```

- `lang` column is optional and has highest priority.
- Use C-style escapes in fields when needed (`\n`, `\t`, `\"`, `\\`).

Language discovery priority (highest to lowest):

1. `[lang<TAB>]` in each input line
2. `--lang`
3. subdir name
4. filename

Notes:

- When `FILE...` args are omitted, `poedit` updates `po/<lang>.po` under `--po-dir`.
- When `FILE...` args are provided, `poedit` updates those PO files only, and language for each target is inferred using the same priority chain (except line-level language, which already belongs to input parsing).

### AI- and l10n-friendly workflow

`poedit --help` documents how to work with AIs: translate strings yourself (or produce TSV/inputs outside the app), use `poedit` only to batch them into the right `po/<lang>.po` files, and do not use it to fabricate `msgstr` from `msgid` alone. A typical loop is: run `ninja -C <build> posync` to pull new `msgid`s from source, list still-empty or wrong `msgstr`s in a catalog, add translations, then `poedit --input …` to apply in one go.

## Build and test

### Build dependencies

```bash
sudo apt install meson ninja-build gcc pkg-config check
```

### Configure and build

Use the absolute build directory `/build`:

```bash
meson setup /build
ninja -C /build
```

### Run tests

```bash
meson test -C /build
```

Unit tests are auto-discovered from `tests/*_unit.c` and registered in Meson.

## i18n (gettext)

`poedit` uses gettext translations under `po/` (`*.po` + generated `.mo` files), domain **poutils** (see `TEXT_DOMAIN` in the build and `po/LINGUAS`).

- **Installed** binaries load from the configured `localedir` (under your Meson `prefix` after `meson install`).
- **Uninstalled / dev** runs: if a `po` directory exists next to the `poedit` executable (e.g. `<build>/po` after `ninja`), the Bas-C `init_i18n(LOCALEDIR)` path prefers that tree so you see freshly built `*.mo` without overwriting `/usr`. Otherwise it falls back to the same `localedir` the binary was built with. After installing to `/usr`, update system locale files (or use the dev binary + `<build>/po`) to test new strings.

### Sync translation catalogs

Use `posync` to update catalogs from current source strings:

```bash
ninja -C /build posync
```

`posync` will:

- add missing messages into each language from `po/LINGUAS`
- remove obsolete messages no longer used in source

### Build translation files

```bash
ninja -C /build
```

### Quick locale testing

Prefer `LANGUAGE=<lang>` for predictable gettext selection in dev shells:

```bash
LANGUAGE=ja /build/poedit -h
LANGUAGE=zh_CN /build/poedit -h
```

`LANG=<lang>.<encoding>` may depend on whether that locale is generated on your system.

## Install / symlink helpers

Normal install:

```bash
meson install -C /build
```

Debug symlink workflow (under configured prefix):

```bash
ninja -C /build install-symlinks
ninja -C /build uninstall-symlinks
```

## Debian package

```bash
dpkg-buildpackage -us -uc
```

## License

Copyright (C) 2026 Lenik <poutils@bodz.net>

Licensed under **AGPL-3.0-or-later**.  
This project explicitly opposes AI exploitation and AI hegemony, and rejects
mindless MIT-style licensing and politically naive BSD-style licensing.  
See `LICENSE` for the full text and supplemental project terms.
