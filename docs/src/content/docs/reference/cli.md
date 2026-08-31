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

| Option                                 | Description                                                                                                                                                 |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `<program>`                            | The executable to run under coverage.                                                                                                                       |
| `<args...>`                            | Arguments passed verbatim to the program.                                                                                                                   |
| `-s`<br/>`--source <path>`             | Filter the files for which coverage is collected to this directory.<br/>**Default:** Empty                                                                  |
| `-d`<br/>`--debug-info <path>`         | Path to the PDB file.<br/>**Default:** Same as the program, with `.exe` replaced by `.pdb`                                                                  |
| `-o`<br/>`--out <path>`                | Where to generate the coverage report.<br/>**Default:** `./report.coverpp`                                                                                  |
| `--exclude-source-files-regex <regex>` | Source files matching this regex are excluded from coverage. The paths are canonical and use forward slashes.<br/>**Default:** `/(vcpkg_installed\|_deps)/` |
| `-v`<br/>`--verbose`                   | Repeatable flag to increase verbosity.                                                                                                                      |
| `--print-first-chance-seh`             | Print first-chance SEH exceptions to the console                                                                                                            |

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

| Option                     | Description                       |
| -------------------------- | --------------------------------- |
| `-o`<br/>`--output <path>` | Where to place the merged report. |
| `<inputs...>`              | One or more report files.         |

## export json

Export a coverage report to a JSON file.

### Usage

```sh
cover++ export json -o <output> <report>
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
