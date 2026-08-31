---
title: Getting started
description: A guide on how to get started using Cover++.
---

Cover++ is a code coverage tool for C/C++ applications on Windows.

## Installation

Grab the latest release from [GitHub](https://github.com/ColoredCarrot/coverpp/releases).
You can use the installer or the portable archive.

For details, head over to the [installation guide](/guides/installation/).

## Run an app under coverage

To run `my-app.exe` while collecting coverage, use:

```sh
cover++ run my-app.exe
```

If you need to pass arguments to your app,
you can append them to the invocation above.

This command:

- expects to find `my-app.pdb` next to `my-app.exe`
- runs `my-app.exe` under coverage
- places the coverage report at `./report.coverpp`

:::tip
The command has lots of further options.
Refer to the [CLI reference](/reference/cli/#run) for more details.
:::

Cover++ stores coverage results in a compact binary format.

## View a coverage report

To view the report you just generated, use:

```sh
cover++ view report.coverpp
```

This will start a local web server and open your browser.

To stop the server, type `q` and _Enter_ or use _Ctrl+C_.

You can find more details [here](/reference/cli/#view).
