---
title: Command Line Interface
description: Describes the CLI of Cover++.
---

Cover++ has multiple subcommands.

## run

Run a program under coverage.

### Usage

```sh
cover++ run [options] <program> [args...]
```

| Option                                 | Description                                                                                                                                                                                                                         |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `<program>`                            | The executable to run under coverage.                                                                                                                                                                                               |
| `<args...>`                            | Arguments passed verbatim to the program.                                                                                                                                                                                           |
| `-s`<br/>`--source <path>`             | Filter the files for which coverage is collected to this directory.<br/>**Default:** Computed as the root directory of the `<program>`. For example, running `cover++ run D:\foo\bar\baz.exe` will use `D:\foo` as the source root. |
| `-d`<br/>`--debug-info <path>`         | Path to the PDB file.<br/>**Default:** Same as the program, with `.exe` replaced by `.pdb`                                                                                                                                          |
| `-o`<br/>`--out <path>`                | Where to generate the coverage report.<br/>**Default:** `./report.coverpp`                                                                                                                                                          |
| `--exclude-source-files-regex <regex>` | Source files matching this regex are excluded from coverage. The paths are canonical and use forward slashes.<br/>**Default:** `/(vcpkg_installed\|_deps)/`                                                                         |
| `-v`<br/>`--verbose`                   | Repeatable flag to increase verbosity.                                                                                                                                                                                              |
| `--print-first-chance-seh`             | Print first-chance SEH exceptions to the console                                                                                                                                                                                    |

### Description

All arguments passed after `<program>` are forwarded verbatim to the program.

## view

View a coverage report in the browser.

**Alias:** `serve`

### Usage

```sh
cover++ view [options] [report]
```

| Option                   | Description                                                          |
| ------------------------ | -------------------------------------------------------------------- |
| `<report>`               | Report file to view.<br/>**Default:** `./report.coverpp`             |
| `-p`<br/>`--port <port>` | Port to serve the report on.<br/>**Default:** `8080`                 |
| `--coverpp-install-dir`  | Installation directory of Cover++.<br/>**Default:** Auto-discovered. |
| `--no-open`              | Don't open the browser; only start the server.                       |

## remap

Remap source locations in a coverage report file.

### Usage

```sh
cover++ remap [options] --to <path> [report]
```

| Option          | Description                                                                           |
| --------------- | ------------------------------------------------------------------------------------- |
| `<report>`      | Report file to modify.<br/>**Default:** `./report.coverpp`                            |
| `--to <path>`   | New source root.<br/>**Required.**                                                    |
| `--from <path>` | Old source root.<br/>**Default:** Inferred from the single source root in the report. |

Note that `--from` is required if there is more than one source root.

### Description

You can remap subdirectories of a source root.
In that case, the original root will be split in two.

It is not required that either the old or the new source root exist.
If the new source root does exist,
missing statistics which require source files will be computed.
This is useful in case the report was generated on a machine without the source files.

## merge

Merge multiple coverage reports into a single file.

### Usage

```sh
cover++ merge -o <output> <inputs...>
```

| Option                  | Description                       |
| ----------------------- | --------------------------------- |
| `-o`<br/>`--out <path>` | Where to place the merged report. |
| `<inputs...>`           | One or more report files.         |

## export json

Export a coverage report to a JSON file.

### Usage

```sh
cover++ export json [-o <output>] [report]
```

| Option                  | Description                                                                                       |
| ----------------------- | ------------------------------------------------------------------------------------------------- |
| `-o`<br/>`--out <path>` | Where to place the generated JSON file.<br/>Use `-` to print to the console.<br/>**Default:** `-` |
| `<report>`              | The report file to export.<br/>**Default:** `./report.coverpp`                                    |

### Examples

Export `foo.coverpp` to `foo.json`:

```sh
cover++ export json -o foo.json foo.coverpp
```

Get the covered lines of a specific file in the first root:

```powershell
$report = cover++ export json foo.coverpp | ConvertFrom-Json
$report.roots[0].children | where file -eq "main.cpp" | select covered
```

## export gcov

Export a coverage report to _gcov_ format.

**Alias:** `export clion`

### Usage

```sh
cover++ export gcov [-o <output>] [report]
```

| Option                  | Description                                                                                        |
| ----------------------- | -------------------------------------------------------------------------------------------------- |
| `-o`<br/>`--out <path>` | The directory in which to place the generated gcov files..<br/>**Default:** `coverage-gcov-export` |
| `<report>`              | The report file to export.<br/>**Default:** `./report.coverpp`                                     |

### Description

[Gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)'s
format is useful primarily because other tools support it.
Cover++ supports exporting reports in a format compatible with gcov
to enable integration with such tools.

One `.gcov` file per source file is created in the output directory.
They are named according to a flat numbering scheme (`001.gcov`, `002.gcov`, ...).
Each generated file contains the path to the source file and associated coverage information per line.

Starting with CLion 2026.2,
it's possible to import gcov files into the IDE[^clion-gcov-import].
To do so, open the _Coverage_ tool window and click _Import a report collected in CI from disk_.
Then, select ALL files in the exported directory.

[^clion-gcov-import]: See [CPP-48737](https://youtrack.jetbrains.com/projects/CPP/issues/CPP-48737/Allow-opening-existing-gcov-reports)

:::note
Gcov files contain the full source code,
so this exporter needs access to the source files.
:::
