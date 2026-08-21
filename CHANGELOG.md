## [Unreleased]

## [1.6.0] - 2026-08-22

Parser and regexp performance, incremental single-line reparse, and CRegExp correctness. The largest performance work comes from far2l ([#42](https://github.com/colorer/Colorer-library/pull/42)).

### Performance

- **Scheme matching.** Nodes are pre-filtered by the first character of the current position; schemes keep ordered candidate lists so inheritance and dynamic virtualization are not scanned on every character. Static inherited scheme nodes are inlined into the parent scheme.
- **Keywords.** Keyword search is faster; the lowercase copy of a line is built only when an ignore-case keyword is actually tried. Fixed a first-character index that used the original XML text instead of the stored keyword, which could miscolor some words.
- **Regular expressions.** First-character analysis skips impossible offsets during search (`positionMoves`). A match fails after a bounded number of backtracking steps (default 1 000 000) instead of hanging on pathological patterns.
- **Hot path.** Fewer `UnicodeString` copies, faster Latin case conversion and string compare (legacy), lazy `SMatches` arrays instead of full zerofill, simpler `CharacterClass`/`BitArray`.
- **Editor memory.** `LineRegion` objects are pooled and matching `rdef` storage is reused. Empty virtual tables are skipped. The start line of a block is copied only when the end RE uses backtrace (`\y`/`\Y`).
- **Editor window.** The `LineRegion` buffer is kept across small window resizes; idle cache warming does not move the visible region ring.
- **Logging.** Log macros check the current level before formatting. Custom `Logger` implementations must provide `getCurrentLogLevel()`.

### Parser and editor

- **Incremental edit.** `TextParser::tryParseLine` / `BaseEditor::modifyLineEvent` reparse only the edited line when the scheme stack (open blocks and start-RE captures) is unchanged; otherwise the rest of the file is invalidated as before.
- **Long lines.** Coloring of a line is still one `maxBlockSize` window by default (editor-safe on minified/pathological lines). `setChunkLongLines(true)` continues coloring in successive windows until the line ends.
- Parser no longer throws when `LineSource` returns a null line (for example during editor shutdown); parsing stops on that line.

### Regular expressions (CRegExp)

Refactoring and correctness ([#44](https://github.com/colorer/Colorer-library/pull/44)):

- Isolate each search offset: reset `\m`/`\M` and all capture slots so a failed attempt cannot leak into the next.
- Preserve compile errors instead of overwriting them with `EBRACKETS`; reject inverted `{n,m}` ranges and out-of-range escapes at compile time.
- Groups beyond 16 slots are non-capturing so existing HRC still compiles.
- Fix `SRegInfo` destructor (free union members by opcode); clear first-char cache on pattern replace; sanitize backtrace copy bounds.
- Parse groups in-place, grow the parse stack with `std::vector`, drop unused ops and unfinished named-match hash code.
- `CRegExp` / `SRegInfo` are not copyable or movable; getters are `const`.
- ICU ignore-case character classes fold the current character (legacy path already did); `[aX]/i` now matches `x`.

### API

- `FileType::getParamValueHex` — read a filetype parameter as a hex integer.
- `TextParser::tryParseLine`, `TextParser::setChunkLongLines` / `getChunkLongLines` (also on `BaseEditor`).
- `CRegExp::canStartWith`, `setParseStepLimit` / `getParseStepLimit`, `hasBackTrace`, `clearRegExpStack`.
- `Logger::getCurrentLogLevel` (pure virtual; required for custom loggers).

### Fixed

- Use-after-free when resolving `COLORER_HRD` in `ParserFactory` ([#40](https://github.com/colorer/Colorer-library/pull/40)).
- Crash on a transient null line and dangling temporary in `UnicodeString(int)` ([#41](https://github.com/colorer/Colorer-library/pull/41)).
- `RegionMapper` cache resize: region id 0 no longer throws.
- Windows and ICU builds.

### Tests and CI

- Catch2 v2 → v3 ([#43](https://github.com/colorer/Colorer-library/pull/43)).
- Unit tests for CRegExp, TextParser, BaseEditor, LineRegions, RegionMapper, UStr; tests enabled for Linux Release.
- GitHub Actions updated (checkout, artifacts, MSVC setup, CodeQL).

## [1.5.0] - 2025-07-07

### Added
- documentation website https://colorer.github.io  (so far only in Russian)
- Working with hrcsettings.xml added to the core. If the path to hrcsettings.xml omitted, the environment variable COLORER_HRC_SETTINGS is used.
  hrcsettings.xml is a configuration file designed to extend prototype settings defined in the basic library of HRC schematics, as well as to store user modifications of these settings.
- consoletools:
  - Added an input parameter to use hrcsettings.xml in consoletools
  - Added an input parameter to specify a folder with custom hrc files.
  - Added an input parameter to specify a folder with custom hrd files.
  - Added the ability to work with relative paths in parameter values. The search starts from the current directory.

### Changed
- The new "global" attribute of the "package" element of the hrc file. The default value is "yes".
- Separate loading of different hrc parts:
  - at the prototype loading stage, only prototypes and global packages are loaded.
  - at the type loading stage, only types and local packages are loaded.
- Exclude *.ent.hrc files from the prototype loading stage
- The -f (forward) parameter has been removed in consoletools

## [1.4.2] - 2025-05-24

### Fixed
- Fix reading non-latin string from hrc file
- Crash when open file on net symlinked folder
- In 'keywords', for the case ignorecase="yes", similar words specified in different case were incorrectly processed.
- if a substring of a word is also present in the 'keywords' list, then the substring word might not be highlighted. For example, for couples "!=" and "!".

## [1.4.1] - 2024-11-02

### Fixed
- Creation of log files when logging is disabled
- Reading a schema from files whose path contains non-Latin letters :
  - incorrect conversion between char and char16 on Windows
  - error reading external entities in xml files on Linux, feature or bug libxml2

## [1.4.0] - 2024-10-18

### Added
- Two options for working with strings are ICU and the old implementation of colorer

### Fixed
- Build on old Linux platform. A minimum of gcc 7 is required.
- Build on Linux ARM64, *BSD, macOS
- Fixed a bug/freeze when resizing the window in which a paired object is selected
 
### Changed
- The library for logging spdlog has been replaced by own implementation.
- The library for working with xml Xerces-c has been replaced by LibXml2.
- The environment variables in the path for the external entity in xml should be set in the format $ENV, 
   and the path should be preceded by 'env:' for files or 'jar:' for archives.

## [1.3.3] - 2024-03-16

### Fixed

- The length of the string used to detecting the file type takes into account the length restrictions
- Memory leak on reading xml files
- Removed unnecessary checks for keywords, which increased the speed
- Reduced memory consumption for storing schemes

## [1.3.2] - 2024-01-20

### Fixed

- Again: Work with catalog.xml on fat32 filesystem. Changed compiler version.

## [1.3.1] - 2023-04-01

### Fixed

- Work with catalog.xml on fat32 filesystem ([#26](https://github.com/colorer/Colorer-library/issues/26))

## [1.3.0] - 2021-05-12

### Added

- Add work with symlinks. If file is symlink, for example catalog.xml, we work with real file and full path to it ([#10](https://github.com/colorer/Colorer-library/issues/10))
- Add work with system environments in path to files
- Add work with zip entity. Support new type of schemes 'allpacked'

### Changed

- In base/hrc/auto folder (and same in catalog.xml) search only *.hrc files ([#20](https://github.com/colorer/Colorer-library/issues/20))

## [1.2.1] - 2021-04-03

First auto built release

## [1.2.0] - 2021-09-09

### Changed

- Work with redirect stdin as input file, detect charset if set BOM ([#8](https://github.com/colorer/Colorer-library/issues/8))

### Fixed

- Fix crash on invalid zip file name ([#15](https://github.com/colorer/Colorer-library/issues/15))
- colorertools: fix crash if did not detect file type and did not input filename

## [1.1.1] - 2021-02-28

### Fixed

- Fix output log messages to stdout if log level equal 'off' ([#14](https://github.com/colorer/Colorer-library/issues/14))

### Changed

- Set default log level value to 'off'

## [1.1.0] - 2021-02-28

### Changed

- Change license to MIT
- Change internal Unicode library to ICU
- Get dependencies for build from vcpkg

### Added

- regexp: add support ignorecase for symbol class
- add new property for TextParser - maximum block size of regexp in string line

### Fixed

- Fix error when searching for the missing type
- Fix duplicate entries in outliner https://github.com/colorer/FarColorer/issues/25

### Removed

- Remove dlmalloc library
- Remove working with different charsets. Input files only in Unicode, output only in UTF-8.

## [1.0.4] - 2019-01-29

### Changed

- Update xerces-c
- Update zlib
- Add spdlog
- Remove g3log

## [1.0.3] - 2018-12-09

### Fixed

- Fixed incorrect file type detection

## [1.0.2] - 2016-06-18

### Changed

- optimized

## [1.0.1] - 2016-02-06

### Changed

- logging default off
- add settings for logging

## [1.0.0] - 2016-01-23

### Changed

- add version info
- add asynchronous logging

