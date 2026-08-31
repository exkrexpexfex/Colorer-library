---
name: colorer-schemes-tests
description: Run Colorer-schemes catalog load and golden parse tests against a freshly built colorer CLI. Use when changing HRC/HRD loading, XML, zip/jar InputSource, ParserFactory, TextParser coloring, ConsoleTools HTML output, or when verifying full-catalog regressions. Not for authoring HRC (that skill lives in Colorer-schemes).
---

# Colorer-schemes tests (library)

External regression for this library. Schemes, goldens, and `ignored_error.txt` live in a **sibling** [Colorer-schemes](https://github.com/colorer/Colorer-schemes) checkout. Do not copy them here. Do not rewrite them as Catch2.

Always go through `tests/schemes/run.sh`. It finds `../Colorer-schemes` (or `$COLORER_SCHEMES_DIR`), links **this** tree's CLI into `Colorer-schemes/bin/colorer`, and builds the catalog. CLI default: `out/agent/tools/colorer/colorer`. Never `out/build/`.

## Catalog flavors

`./build.sh base` and `./build.sh base.packed` write **different directories**:

| Build | Output | What it is |
|-------|--------|------------|
| `base` | `_build/base/` | Ordinary HRC/HRD files on disk |
| `base.packed` | `_build/base-packed/` | HRC packed into zip; `jar:` URIs (`COLORER_USE_ZIPINPUTSOURCE=ON`) |
| `base.allpacked` | `_build/base-allpacked/` | Whole catalog in one zip |

Golden parse (`runtest.py`) is hardcoded to unpacked `_build/base`. Packed/allpacked are load (and one-file `-ht`) checks.

**XML load or zip/jar InputSource changes must run `load packed`.** Unpacked `load` is not a substitute. Rebuild the CLI with `-DCOLORER_USE_ZIPINPUTSOURCE=ON` if needed. `zip` must be on PATH.

## What to run

| Change | Command |
|--------|---------|
| XML, catalog, `XmlInputSource`, zip/jar | `./tests/schemes/run.sh load packed` (required). Also `load` and, if relevant, `load allpacked`. |
| Matcher / regions / `-ht` HTML | `parse --quick` or `parse '*/lang/*'`. Full `parse` only if asked or before claiming no coloring regression. |
| ConsoleTools flags / HTML generator | one-file `html` (add `--packed` if zip paths are involved) |
| Unrelated (cregexp unit, strings, …) | do not run schemes tests |

```bash
./tests/schemes/run.sh load
./tests/schemes/run.sh load packed
./tests/schemes/run.sh parse --quick
./tests/schemes/run.sh parse '*/cpp/*'
./tests/schemes/run.sh html --packed path/to/sample.cpp -t cpp -o /tmp/out.html
```

Missing schemes tree: clone next to this repo or set `COLORER_SCHEMES_DIR`. Missing CLI: build `consoletools` into `out/agent/` (see `AGENTS.md`).

## Rules

- Catch2 in `tests/unit` stays the default loop. This is extra.
- Do not update Colorer-schemes `_valid/` goldens from a library-only change unless the HTML CLI contract changed **and** the user asked.
- Do not extend `ignored_error.txt` to hide a library loader bug.
- Do not `rm -rf` Colorer-schemes `_build`/`_test`, and never `out/build/` or `rm -rf out/`.
- Authoring HRC: Colorer-schemes skill `colorer-hrc-authoring`, not this one.
