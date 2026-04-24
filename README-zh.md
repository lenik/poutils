# poutils

`poutils` 是一个基于 Meson 的小型 C/C++ 命令行应用项目模板。  
`poedit` 是此模板中的一个**示例应用**；同一仓库中可以继续添加更多应用。

## 仓库结构

- `src/` - 应用与共享模块源码
- `tests/` - 使用 Check 框架的单元测试（`*_unit.c`）
- `debian/` - Debian 打包元数据
- `meson.build` - 顶层构建定义与辅助目标

## 示例应用：`poedit`

`poedit` 从映射输入批量更新 `po/<语言>.po`（TSV/文本行：`[lang<制表符>]msgid<制表符>msgstr` 等，详见 `poedit --help`）。

```bash
poedit [选项] [PO 文件...]
```

常见用法与英文版 `README` 的 “Example app: poedit” 一节一致。`--help` 中说明了面向 AI 的工作流：先人工完成翻译/准备好映射，再用本工具一次写入，勿用其生成占位或简单复制 `msgid` 到 `msgstr`。

维护词条的典型循环：`ninja -C <构建目录> posync` 从源码刷新模板与各语言 `.po`，在目录中补全/修正 `msgstr` 后，用 `poedit --input …` 批量写回（或整文件编辑后再 `posync` 校验）。

## 构建与测试

### 构建依赖

```bash
sudo apt install meson ninja-build gcc pkg-config check
```

### 配置并构建

使用绝对构建目录 `/build`：

```bash
meson setup /build
ninja -C /build
```

### 运行测试

```bash
meson test -C /build
```

Meson 会自动发现 `tests/*_unit.c` 中的单元测试并完成注册。

## i18n（gettext）

`poedit` 使用 `po/` 下的 gettext 翻译，文本域为 **poutils**（与构建中的 `TEXT_DOMAIN` 及 `po/LINGUAS` 一致）。

- **已安装**到前缀下的二进制，从该前缀下的 `localedir` 加载（`meson install` 后生效）。
- **未安装/开发**运行：若可执行文件同目录下存在 `po` 子目录（例如 `ninja` 之后在 `<构建目录>/po`），Bas-C 的 `init_i18n(LOCALEDIR)` 会优先从该树加载，便于在不动 `/usr` 的情况下验证新生成的 `*.mo`；否则回退为编译时配置的 `localedir`。装到系统后若仍见旧翻译，需更新系统内 `.mo` 或改用带 `<build>/po` 的开发构建。

### 同步翻译词条

使用 `posync` 从当前源码字符串同步词条：

```bash
ninja -C /build posync
```

`posync` 会：

- 为 `po/LINGUAS` 中每种语言补齐缺失消息
- 移除源码中已不再使用的废弃消息

### 构建翻译文件

```bash
ninja -C /build
```

### 手册页

英文与各语言 `poedit(1)` 分别来自
`poedit.1.in` 与
`man/<语言>/poedit.1.in`，由 Meson
`configure_file` 用
`@PROJECT_VERSION@`、
`@PROJECT_YEAR@`、
`@PROJECT_AUTHOR@`、
`@PROJECT_EMAIL@` 在构建时替换；安装到
`$PREFIX/share/man/<语言>/man1/poedit.1`。
要单独看某一语言，把 `MANPATH` 指到
`$PREFIX/share/man/<语言>` 后执行
`man poedit`。

### 快速测试语言

建议优先使用 `LANGUAGE=<lang>`，在开发环境中选择更稳定：

```bash
LANGUAGE=ja /build/poedit -h
LANGUAGE=zh_CN /build/poedit -h
```

`LANG=<lang>.<encoding>` 是否生效取决于系统是否已生成对应 locale。

## 安装 / 符号链接辅助命令

常规安装：

```bash
meson install -C /build
```

调试符号链接工作流（在已配置的安装前缀下）：

```bash
ninja -C /build install-symlinks
ninja -C /build uninstall-symlinks
```

## Debian 打包

```bash
dpkg-buildpackage -us -uc
```

## 许可证

Copyright (C) 2026 Lenik <poutils@bodz.net>

采用 **AGPL-3.0-or-later** 许可。  
本项目明确反对 AI 剥削与 AI 霸权，反对无脑 MIT 式许可证和政治愚蠢的 BSD 式许可证。  
完整文本及项目补充条款见 `LICENSE`。
