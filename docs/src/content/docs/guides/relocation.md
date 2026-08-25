---
title: Relocation
description: A guide on how to relocate a coverage report file.
---

PDBs often contain absolute paths to source files.
This can cause issues when moving the source files to a different location or viewing a coverage report on a different machine.
Cover++ provides a way to fix-up a report after moving it by remapping source file locations.

For example,
say you generated `report.coverpp` on a build agent where the source files are at `C:\BuildAgent\work`.
Now, you want to view the report on your own machine,
where the sources live at `D:\Dev\sources`.
You can use the following command:

```cmd
cover++ remap report.coverpp --to D:\Dev\sources
```

:::note
In most cases, Cover++ is able to infer the original source root.
If this does not succeed, you can specify it manually with `--from`.
:::
