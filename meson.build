project(
    'poutils',
    'c',
    'cpp',
    version: run_command(
        'sh',
        '-c', 'v=$(git describe --tags --always --dirty 2>/dev/null || true); v=${v#v}; [ -n "$v" ] && printf "%s" "$v" || printf "0.0.0"',
        check: false,
    ).stdout().strip(),
    license: 'AGPL-3.0-or-later',
    default_options: [
        'c_std=c99',
        'warning_level=2',
    ],
)
project_author = 'Lenik'
project_email = 'poutils@bodz.net'
project_year = 2026

fs = import('fs')
c = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
datadir = prefix / get_option('datadir')
localedir = prefix / get_option('localedir')
mandir = prefix / get_option('mandir')
pkgdatadir = datadir / meson.project_name()
pkgdocdir = datadir / 'doc' / meson.project_name()

config_h = configuration_data()
config_h.set_quoted('PROJECT_VERSION', meson.project_version())
config_h.set_quoted('PROJECT_AUTHOR', project_author)
config_h.set_quoted('PROJECT_EMAIL', project_email)
config_h.set('PROJECT_YEAR', project_year)

config_h.set_quoted('PREFIX', prefix)
config_h.set_quoted('DATADIR', datadir)
config_h.set_quoted('MANDIR', mandir)
config_h.set_quoted('PKGDATADIR', pkgdatadir)
config_h.set_quoted('LOCALEDIR', localedir)

configure_file(
    output: 'config.h',
    configuration: config_h,
)

bas_c_dep = dependency('bas-c', required: true)
# bas_cpp_dep = dependency('bas-cpp', required: true)

# bas_c_static_dep = dependency('bas-c-static', required: true)
# bas_cpp_static_dep = dependency('bas-cpp-static', required: true)

exe_objs = []

# Build assets.zip every time (no file dependency)
assets_dir = meson.current_source_dir() / 'assets'
assets_src = 'src/assets.s'
if fs.exists(assets_dir) and fs.exists(assets_src)
    build_dir = meson.current_build_dir()
    assets_zip = custom_target(
        'assets_zip',
        output: 'assets.zip',
        input: [],
        command: [
            'sh',
            '-c', 'dst=$(readlink -f "@1@"); cd "@0@" && zip -r "$dst" .'.format(assets_dir, '@OUTPUT@'),
        ],
        #build_always : true,
        build_by_default: true,
        install: false,
    )

    # Compile assembly file that embeds assets.zip using .incbin
    # Use configure_file to copy assets.s to build directory
    assets_s = configure_file(
        input: 'src/assets.s',
        output: 'assets.s',
        copy: true,
    )

    # Compile the assembly file (assets.zip will be in the same directory)
    as = find_program('as', required: true)
    assets_o = custom_target(
        'assets_o',
        output: 'assets.o',
        input: [assets_s, assets_zip],
        command: [as, '-o', '@OUTPUT@', '@INPUT0@'],
        depends: assets_zip,
    )

    exe_objs += [assets_o]
endif

# Disable all warnings (wx 3.0 and deps are noisy)
nowarn = cpp.get_supported_arguments(['-w', '/w'])
bas_args = ['-DLOGGER_NAME=poutils', '-DASSETS_NAME=poutils']

c_args = [nowarn, bas_args, '-DTEXT_DOMAIN=poutils']
c_pch = 'src/c_pch.h'
cpp_pch = 'src/cpp_pch.h'

exe_inc = [
    include_directories('src'),
]

exe_deps = [
    bas_c_dep,
]

check_dep = dependency('check', required: true)

executable(
    'poedit',
    [
        'src/poedit.c',
        'src/poformat.c',
        'src/tsvformat.c',
    ],
    exe_objs,
    c_args: c_args,
    c_pch: c_pch,
    include_directories: exe_inc,
    dependencies: exe_deps,
    install: true,
    install_dir: bindir,
)

unit_sources = [
    'tests/poedit_unit.c',
    'tests/poformat_unit.c',
    'tests/tsvformat_unit.c',
]

foreach unit_src : unit_sources
    unit_base = unit_src.split('/')[-1]
    unit_name = unit_base.replace('.c', '')

    unit_exe = executable(
        unit_name,
        [
            unit_src,
            'src/poformat.c',
            'src/tsvformat.c',
        ],
        c_args: c_args,
        c_pch: c_pch,
        include_directories: exe_inc,
        dependencies: exe_deps + [check_dep],
        install: false,
    )

    test(unit_name, unit_exe)
endforeach

install_data(
    'poedit.bash',
    install_dir: datadir / 'bash-completion' / 'completions',
    rename: 'poedit',
)

install_data(
    [
        'LICENSE',
        'README.md',
        'README-zh.md',
    ],
    install_dir: pkgdocdir,
)

config_man = configuration_data()
config_man.set('PROJECT_VERSION', meson.project_version())
config_man.set('PROJECT_AUTHOR', project_author)
config_man.set('PROJECT_EMAIL', project_email)
config_man.set('PROJECT_YEAR', project_year)

configure_file(
    input: 'poedit.1.in',
    output: 'poedit.1',
    configuration: config_man,
    install: true,
    install_dir: mandir / 'man1',
)

manpage_langs = [
    'de',
    'eo',
    'fr',
    'it',
    'ja',
    'ko',
    'th',
    'vi',
    'zh_CN',
    'zh_TW',
]
foreach lang : manpage_langs
    manin = 'man' / lang / 'poedit.1.in'
    if fs.is_file(meson.current_source_dir() / manin)
        man_gen = configure_file(
            input: manin,
            output: 'poedit.1.' + lang,
            configuration: config_man,
        )
        install_data(
            man_gen,
            install_dir: mandir / lang / 'man1',
            rename: 'poedit.1',
        )
    endif
endforeach

# look task: list what installed files looks like
# ninja install <tmpdir> && tree <tmpdir> && rm -fr <tmpdir>
run_target(
    'look',
    command: [
        'bash',
        '-euc', '''
            tmpdir=$(mktemp -d)
            DESTDIR=$tmpdir meson install -C "@BUILD_ROOT@"
            tree "$tmpdir"
            rm -fr "$tmpdir"
        ''',
    ],
)

subdir('po')

run_target(
    'posync',
    command: [
        'bash',
        '-euc', '''
            cd "@SOURCE_ROOT@/po"
            poflags=(--no-wrap)
            xgettext "${poflags[@]}" --from-code=UTF-8 --keyword=_ --language=C --directory=.. --output=poutils.pot --files-from=POTFILES
            while IFS= read -r lang; do
                [ -n "$lang" ] || continue
                case "$lang" in
                    '#'* ) continue ;;
                esac
                po_file="$lang.po"
                if [ ! -f "$po_file" ]; then
                    msginit "${poflags[@]}" --no-translator --input=poutils.pot --locale="$lang" --output-file="$po_file"
                fi
                msgmerge "${poflags[@]}" --update --backup=none "$po_file" poutils.pot
                msgattrib "${poflags[@]}" --no-obsolete --output-file="$po_file" "$po_file"
            done < LINGUAS
        ''',
    ],
)

# Fast local debugging against the active installation prefix:
# symlink installed paths to project/build outputs.
run_target(
    'install-symlinks',
    command: [
        'bash',
        '-euc', '''
            prefix="@0@"
            bindir="$prefix/bin"
            compdir="@1@/bash-completion/completions"
            man1dir="@2@/man1"

            mkdir -p "$bindir" "$compdir" "$man1dir"

            sudo ln -sfn "@BUILD_ROOT@/poedit" "$bindir/poedit"
            sudo ln -sfn "@BUILD_ROOT@/poedit.1" "$man1dir/poedit.1"
            sudo ln -sfn "@SOURCE_ROOT@/poedit.bash" "$compdir/poedit"

            printf "Symlinks installed under %s\n" "$prefix"
        '''.format(get_option('prefix'), datadir, mandir),
    ],
)

run_target(
    'uninstall-symlinks',
    command: [
        'bash',
        '-euc', '''
            prefix="@0@"
            bindir="$prefix/bin"
            compdir="@1@/bash-completion/completions"
            man1dir="@2@/man1"

            for p in \
                "$bindir/poedit" \
                "$man1dir/poedit.1" \
                "$compdir/poedit"
            do
                if [ -L "$p" ]; then
                    sudo rm -f "$p"
                    printf "Removed %s\n" "$p"
                else
                    printf "Skipped (not a symlink): %s\n" "$p"
                fi
            done
        '''.format(get_option('prefix'), datadir, mandir),
    ],
)