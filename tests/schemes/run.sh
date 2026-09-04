#!/usr/bin/env bash
# Drive Colorer-schemes load/parse tests against this tree's colorer CLI.
# Schemes stay in a Colorer-schemes checkout (COLORER_SCHEMES_DIR, or
# ../Colorer-schemes when both repos share a parent) — they are not vendored here.
set -euo pipefail

LIB_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

usage() {
  cat <<'EOF'
Usage: tests/schemes/run.sh <command>

Commands:
  load                 Unpacked catalog (_build/base): ordinary HRC/HRD files.
  load packed          Packed catalog (_build/base-packed): HRC in zip, jar: URIs.
                       Required for XML-load or zip/jar InputSource changes.
  load allpacked       Whole catalog in one zip (_build/base-allpacked).
  parse [--quick] [GLOB...]
                       Golden HTML vs Colorer-schemes/tests/test/_valid.
                       Always uses unpacked _build/base (runtest.py is hardcoded).
  html [--packed|--allpacked] FILE [colorer-args...]
                       One-file -ht against the chosen catalog.

Environment:
  COLORER_SCHEMES_DIR  Colorer-schemes checkout. If unset, ../Colorer-schemes
                       is tried (example: both repos cloned under one parent).
  COLORER              colorer CLI (default: out/agent/tools/colorer/colorer)

Never points at out/build/. Packed catalogs need COLORER_USE_ZIPINPUTSOURCE=ON
and the `zip` tool on PATH.
EOF
}

abs_file() {
  (cd "$(dirname "$1")" && echo "$(pwd)/$(basename "$1")")
}

die() {
  echo "error: $*" >&2
  exit 1
}

find_schemes() {
  local dir
  if [ -n "${COLORER_SCHEMES_DIR:-}" ]; then
    dir="$COLORER_SCHEMES_DIR"
  elif [ -d "$LIB_ROOT/../Colorer-schemes" ]; then
    dir="$LIB_ROOT/../Colorer-schemes"
  else
    die "Colorer-schemes not found. Clone it next to this repo (same parent directory) or set COLORER_SCHEMES_DIR."
  fi
  [ -f "$dir/build.sh" ] && [ -d "$dir/tests" ] || die "not a Colorer-schemes tree: $dir"
  (cd "$dir" && pwd)
}

find_colorer() {
  local cand
  if [ -n "${COLORER:-}" ]; then
    cand="$COLORER"
  else
    cand="$LIB_ROOT/out/agent/tools/colorer/colorer"
  fi
  [ -x "$cand" ] || die "colorer CLI not found or not executable: $cand
Build it with:
  cmake -S . -B out/agent -G \"Unix Makefiles\" \\
    -DCOLORER_USE_VCPKG=OFF -DCOLORER_BUILD_TEST=ON -DCOLORER_USE_ZIPINPUTSOURCE=ON \\
    -DCMAKE_BUILD_TYPE=Release
  cmake --build out/agent -j\"\$(nproc)\" --target consoletools
Or set COLORER to the binary. Do not use out/build/."
  abs_file "$cand"
}

install_cli() {
  mkdir -p "$SCHEMES/bin"
  ln -sfn "$COLORER_BIN" "$SCHEMES/bin/colorer"
}

catalog_dir() {
  case "$1" in
    base) echo "base" ;;
    packed) echo "base-packed" ;;
    allpacked) echo "base-allpacked" ;;
    *) die "unknown catalog flavor: $1" ;;
  esac
}

build_catalog() {
  local flavor="$1"
  (
    cd "$SCHEMES"
    case "$flavor" in
      base) ./build.sh base ;;
      packed) command -v zip >/dev/null || die "zip is required for base.packed"; ./build.sh base.packed ;;
      allpacked) command -v zip >/dev/null || die "zip is required for base.allpacked"; ./build.sh base.allpacked ;;
    esac
  )
  local cat="$SCHEMES/_build/$(catalog_dir "$flavor")/catalog.xml"
  [ -f "$cat" ] || die "catalog not built: $cat"
}

require_python() {
  command -v python3 >/dev/null || die "python3 is required for parse tests"
}

cmd_load() {
  local flavor="base"
  case "${1:-}" in
    "" ) ;;
    packed|--packed) flavor="packed" ;;
    allpacked|--allpacked) flavor="allpacked" ;;
    *) die "load: unknown argument '$1' (expected packed or allpacked)" ;;
  esac
  build_catalog "$flavor"
  (
    cd "$SCHEMES"
    if [ "$flavor" = "base" ]; then
      ./build.sh test.load
    else
      ./build.sh test.load "$flavor"
    fi
  )
}

cmd_parse() {
  require_python
  build_catalog base
  (
    cd "$SCHEMES"
    python3 ./tests/test/runtest.py "$@"
  )
}

cmd_html() {
  local flavor="base"
  case "${1:-}" in
    --packed|packed) flavor="packed"; shift ;;
    --allpacked|allpacked) flavor="allpacked"; shift ;;
  esac
  [ "${1:-}" != "" ] || die "html: FILE required"
  local file="$1"
  shift
  [ -f "$file" ] || die "html: not a file: $file"
  file="$(abs_file "$file")"
  build_catalog "$flavor"
  local cat="$SCHEMES/_build/$(catalog_dir "$flavor")/catalog.xml"
  "$COLORER_BIN" -c "$cat" -ht "$file" "$@"
}

[ "${1:-}" != "" ] || { usage; exit 1; }
case "$1" in
  -h|--help) usage; exit 0 ;;
esac

SCHEMES="$(find_schemes)"
COLORER_BIN="$(find_colorer)"
install_cli

cmd="$1"
shift
case "$cmd" in
  load) cmd_load "$@" ;;
  parse) cmd_parse "$@" ;;
  html) cmd_html "$@" ;;
  *) usage; die "unknown command: $cmd" ;;
esac
