# Cover++

Code coverage engine for C/C++ on Windows.

A modern alternative to the abandoned [OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage) project.

## Usage

### Report generation

```bat
cover++ run                   ^
  -s "path\to\src\dir"        ^
  -p "path\to\program.exe"    ^
  -d "path\to\debug\info.pdb"
```

### Report visualization

```bat
cover++ view
```

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
