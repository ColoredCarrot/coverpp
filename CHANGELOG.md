# Changelog

This changelog follows the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format.

## [Unreleased]

### Fixed

- The application under coverage no longer crashes when multiple threads hit a tracepoint at the same time

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
