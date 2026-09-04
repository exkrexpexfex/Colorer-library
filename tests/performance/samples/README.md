Large-file corpus for `perftest` (`-t5` coloring a file, `-t6` line-edit). Not Catch2 fixtures.

```bash
./out/agent/tests/perftest -t5 -c5 \
  -b <catalog.xml> \
  -f tests/performance/samples/sqlite3.c
```

`catalog.xml` comes from a Colorer-schemes build (`_build/base/` after `./build.sh base`). See `AGENTS.md`.
