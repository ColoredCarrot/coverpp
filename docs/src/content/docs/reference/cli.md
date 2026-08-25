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

| Option                                 | Description                                                                                                                          |
| -------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| `-s`<br/>`--source <path>`             | Filter the files for which coverage is collected to this directory.<br/>**Default:** Empty                                           |
| `-d`<br/>`--debug-info <path>`         | Path to the PDB file.<br/>**Default:** Same as the program, with `.exe` replaced by `.pdb`                                           |
| `-o`<br/>`--out <path>`                | Where to generate the coverage report.<br/>**Default:** `./report.coverpp`                                                           |
| `--exclude-source-files-regex <regex>` | Source files matching this regex are excluded from coverage. The paths are canonical and use forward slashes.<br/>**Default:** Empty |
| `-v`<br/>`--verbose`                   | Repeatable flag to increase verbosity.                                                                                               |
| `--print-first-chance-seh`             | Print first-chance SEH exceptions to the console                                                                                     |

## view

View a coverage report in the browser.

**Alias:** `serve`

### Usage

```sh
cover++ view [options] <report>
```

| Option                   | Description                                                          |
| ------------------------ | -------------------------------------------------------------------- |
| `-p`<br/>`--port <port>` | Port to serve the report on.<br/>**Default:** `8080`                 |
| `--coverpp-install-dir`  | Installation directory of Cover++.<br/>**Default:** Auto-discovered. |
| `--no-open`              | Don't open the browser; only start the server.                       |
