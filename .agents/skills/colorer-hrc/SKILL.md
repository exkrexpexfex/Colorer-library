---
name: colorer-hrc
description: HRC/HRD/catalog language and Colorer CRegExp semantics as this library implements them. Use when editing the parser, scheme nodes, CRegExp, catalog/HRD/XML load, regions, inherit/virtual, or HRC fixtures under tests/unit/data. Includes XML-load constraints: no SAX, recursive loadFileType on import/QName.
---

# HRC language (this library)

Full grammar and match semantics: [hrc-ref.md](hrc-ref.md). Read it before changing match order, region numbering, inherit/virtual, CRegExp, catalog/`jar:` load, or HRC fixtures.

Runtime coloring hot path (`CRegExp` filters, `TextParser` scheme search, `BaseEditor` window/invalidation): [core-parse.md](core-parse.md).

XML load constraints (no SAX, recursive `loadFileType` on import/QName) are in [hrc-ref.md](hrc-ref.md#xml-load-this-library).

Not for authoring new language types — that is Colorer-schemes skill `colorer-hrc-authoring`. Human HTML (legacy): https://colorer.sourceforge.net/hrc-ref/
