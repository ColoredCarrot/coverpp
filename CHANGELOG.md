# Changelog

This changelog follows the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format.

## [Unreleased]

### Changed

- The default value for `--exclude-source-files-regex` is now `/(vcpkg_installed|_deps)/`.

  This heuristic excludes files in locations commonly containing dependency sources (vcpkg and CMake FetchContent).

- Breakpoints not set by Cover++ are now handled the same as other SEH exceptions.

  In practice, this means that Cover++ will correctly report them according to `--print-first-chance-seh`.
  This might affect you if your app uses `DebugBreak()` and a `__try`/`__except` block for control flow.

- Cover++ can now decode more SEH exceptions to generate useful descriptions. For example, the legacy exception used to set a thread's name is now decoded.

### Fixed

- The application under coverage no longer crashes when multiple threads hit a tracepoint at the same time.

## [26.0.2] - 2026-08-31

### Added

- `-v`, `--version` to display the version number

### Changed

- Consolidated CLI options and defaults

### Fixed

- Fixed a crash when a single virtual address resolves to multiple tracepoints

## [26.0.1] - 2026-08-30

### Added

- Support more entrypoints (and make finding it optional): `wmain`, `WinMain`, `wWinMain`

### Fixed

- Initialize DIA session's base address
- Proper use of virtual addresses throughout

## [26] - 2026-08-29

Initial release.
