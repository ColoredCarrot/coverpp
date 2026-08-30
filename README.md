# Cover++

Code coverage engine for C/C++ on Windows.

A modern alternative to the abandoned [OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage) project.

**Go to:** [**Documentation**](https://coloredcarrot.github.io/coverpp/)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/src/assets/view-main-dark.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/src/assets/view-main-light.png">
  <img alt="Screenshot of main view" src="docs/src/assets/view-main-dark.png">
</picture>

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/src/assets/view-file-dark.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/src/assets/view-file-light.png">
  <img alt="Screenshot of file view" src="docs/src/assets/view-file-dark.png">
</picture>

## Usage

### Report generation

You don't need to compile or link your program with any specific options.
Having an `.exe` and a `.pdb` is enough.

```bat
cover++ run -s path\to\sources my-program.exe
```

### Report visualization

```bat
cover++ view report.coverpp
```

### Export

```bat
cover++ export json report.coverpp -o report.json
```

For a complete list of commands and options,
see the [documentation](https://coloredcarrot.github.io/coverpp/reference/cli).

## Development

### Running tests

Cover++ integrates its various test suites with CTest,
allowing them to be conveniently run with one unified command.

```bat
mkdir build      # We want an out-of-source build
cd build
cmake ..         # Configure with default options (includes testing)
cmake --build .  # Build
ctest -C Debug   # Run all test suites
```

### Packaging

Cover++ uses CPack to generate installers.

```bat
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cpack
```

### Frontend

To develop the frontend or the documentation,
you need Node and pnpm:

```bat
winget install OpenJS.NodeJS.LTS
winget install pnpm.pnpm
```

You can then run development servers in the `frontend` and `docs` directories:

```bat
pnpm dev
```

### Terminology

A process consists of one or more _modules_
(most commonly one `.exe` and a few `.dll`s).
Each module needs its own PDB.

A process owns one _Virtual Address (VA)_ space.
Each module owns a disjoint, contiguous region of this space.

A module is loaded at a _base address_.
A module itself, and its PDB, contain _Relative Virtual Addresses (RVA)_.
These are relative to the base address:
_VA = base + RVA_.

The DIA SDK provides methods using VAs as well as RVAs.
Before using any method that deals with VAs,
you need to inform the SDK of the base address of the module via `IDiaSession::put_loadAddress`.
We consistently use VAs throughout the codebase;
RVAs should not be used.
