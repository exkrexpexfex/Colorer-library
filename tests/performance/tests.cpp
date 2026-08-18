#include "tests.h"
#include <iostream>
#include <chrono>
using namespace std;
using namespace std::chrono;

/*
 *  speed test a class ParserFactory constructor
 *  search and reading catalog.xml
 */
void TestParserFactoryConstructor(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryConstructor" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // stop timer
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test a ParserFactory->getHrcLibrary
 *  load proto.hrc
 */
void TestParserFactoryHrcLibrary(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryHrcLibrary" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto& hrcLibraryLocal = parserFactoryLocal.getHrcLibrary();
    // stop timer
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test a ParserFactory->createStyledMapper
 *  load default hrd scheme
 */
void TestParserFactoryStyledMapper(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryStyledMapper" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto console = UnicodeString("console");
    auto regionMapperLocal = parserFactoryLocal.createStyledMapper(&console, nullptr);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test load all hrc sheme
 */
void TestParserFactoryLoadAllHRCScheme(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryLoadAllHRCScheme" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    auto& hrcLibraryLocal = parserFactoryLocal.getHrcLibrary();
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int idx = 0;; idx++) {
      FileType* type = hrcLibraryLocal.enumerateFileTypes(idx);
      if (type == nullptr)
        break;
      type->getBaseScheme();
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

FileType* selectType(HrcLibrary* hrcLibrary, LineSource* lineSource, UnicodeString* testFile)
{
  FileType* type;

  UnicodeString textStart;
  int totalLength = 0;
  for (int i = 0; i < 4; i++) {
    UnicodeString* iLine = lineSource->getLine(i);
    if (iLine == nullptr)
      break;
    textStart.append(*iLine);
    textStart.append(UnicodeString("\n"));
    totalLength += iLine->length();
    if (totalLength > 500)
      break;
  }
  type = hrcLibrary->chooseFileType(testFile, &textStart, 0);

  return type;
}

/*
 *  speed test coloring file
 */
void TestColoringFile(int count, UnicodeString* catalogPath, UnicodeString* testFile)
{
  cout << "TestColoringFile" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(testFile, true);
    // Base editor to make primary parse
    BaseEditor baseEditor(&parserFactoryLocal, &textLinesStore);
    // HRD RegionMapper linking
    auto console = UnicodeString("console");
    baseEditor.setRegionMapper(&console, nullptr);
    FileType* type = selectType(&parserFactoryLocal.getHrcLibrary(), &textLinesStore, testFile);
    type->getBaseScheme();
    baseEditor.setFileType(type);

    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());
    baseEditor.validate(-1, false);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

class OverlayLines : public LineSource
{
 public:
  TextLinesStore store;
  int overlay_lno = -1;
  UnicodeString overlay;

  UnicodeString* getLine(size_t lno) override
  {
    if (static_cast<int>(lno) == overlay_lno) {
      return &overlay;
    }
    return store.getLine(lno);
  }
};

class ParseCounter : public RegionHandler
{
 public:
  int clear_lines = 0;

  void addRegion(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                 const Region* /*region*/) override
  {
  }
  void enterScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                   const Region* /*region*/, const Scheme* /*scheme*/) override
  {
  }
  void leaveScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                   const Region* /*region*/, const Scheme* /*scheme*/) override
  {
  }
  void clearLine(size_t /*lno*/, UnicodeString* /*line*/) override
  {
    clear_lines++;
  }
};

void paintWindow(BaseEditor& editor, int wStart, int wSize, int lineCount)
{
  const int last = wStart + wSize < lineCount ? wStart + wSize : lineCount;
  for (int lno = wStart; lno < last; lno++) {
    editor.getLineRegions(lno);
  }
}

void TestEditorLineEdit(int count, UnicodeString* catalogPath, UnicodeString* testFile)
{
  cout << "TestEditorLineEdit" << endl;
  if (testFile == nullptr) {
    cout << "need -f test file" << endl;
    return;
  }
  if (count < 1) {
    count = 1;
  }

  ParserFactory parserFactoryLocal;
  parserFactoryLocal.loadCatalog(catalogPath);

  OverlayLines source;
  source.store.loadFile(testFile, true);
  const int lineCount = static_cast<int>(source.store.getLineCount());
  int editLine = lineCount / 2;
  UnicodeString* sample = source.store.getLine(static_cast<size_t>(editLine));
  while (editLine + 1 < lineCount && (sample == nullptr || sample->length() < 4)) {
    editLine++;
    sample = source.store.getLine(static_cast<size_t>(editLine));
  }
  if (sample == nullptr || sample->length() < 1) {
    cout << "no editable line" << endl;
    return;
  }

  source.overlay_lno = editLine;
  source.overlay = UnicodeString(*sample, 0, sample->length() - 1);
  source.overlay.append(UnicodeString("x"));

  const int wSize = 20;
  int wStart = editLine - wSize / 2;
  if (wStart < 0) {
    wStart = 0;
  }

  BaseEditor baseEditor(&parserFactoryLocal, &source);
  auto console = UnicodeString("console");
  baseEditor.setRegionMapper(&console, nullptr);
  FileType* type = selectType(&parserFactoryLocal.getHrcLibrary(), &source, testFile);
  type->getBaseScheme();
  baseEditor.setFileType(type);
  baseEditor.lineCountEvent(lineCount);
  baseEditor.visibleTextEvent(wStart, wSize);
  baseEditor.validate(-1, false);

  ParseCounter counter;
  baseEditor.addRegionHandler(&counter);

  // validate(-1) moves the window to EOF; pin it back and fill the region ring.
  baseEditor.visibleTextEvent(wStart, wSize);
  paintWindow(baseEditor, wStart, wSize, lineCount);

  cout << "file lines=" << lineCount << " editLine=" << editLine << " window=" << wStart << "+"
       << wSize << " repeats=" << count << endl;

  counter.clear_lines = 0;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  for (int i = 0; i < count; i++) {
    baseEditor.modifyLineEvent(editLine);
    paintWindow(baseEditor, wStart, wSize, lineCount);
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  auto try_paint = duration_cast<duration<double>>(t2 - t1).count();
  const int try_paint_lines = counter.clear_lines;
  cout << "modifyLineEvent+paint  lines_parsed=" << try_paint_lines
       << " per_edit=" << (double) try_paint_lines / count << " time=" << try_paint
       << "s invalidLine=" << baseEditor.getInvalidLine() << endl;

  counter.clear_lines = 0;
  t1 = high_resolution_clock::now();
  baseEditor.modifyLineEvent(editLine);
  while (baseEditor.haveInvalidLine()) {
    baseEditor.idleJob(100);
  }
  t2 = high_resolution_clock::now();
  auto try_idle = duration_cast<duration<double>>(t2 - t1).count();
  cout << "modifyLineEvent+idle   lines_parsed=" << counter.clear_lines << " time=" << try_idle
       << "s invalidLine=" << baseEditor.getInvalidLine() << endl;

  counter.clear_lines = 0;
  t1 = high_resolution_clock::now();
  for (int i = 0; i < count; i++) {
    baseEditor.modifyEvent(editLine);
    paintWindow(baseEditor, wStart, wSize, lineCount);
  }
  t2 = high_resolution_clock::now();
  auto ev_paint = duration_cast<duration<double>>(t2 - t1).count();
  const int ev_paint_lines = counter.clear_lines;
  cout << "modifyEvent+paint      lines_parsed=" << ev_paint_lines
       << " per_edit=" << (double) ev_paint_lines / count << " time=" << ev_paint
       << "s invalidLine=" << baseEditor.getInvalidLine() << endl;

  baseEditor.validate(-1, false);
  baseEditor.visibleTextEvent(wStart, wSize);
  paintWindow(baseEditor, wStart, wSize, lineCount);
  counter.clear_lines = 0;
  t1 = high_resolution_clock::now();
  baseEditor.modifyEvent(editLine);
  while (baseEditor.haveInvalidLine()) {
    baseEditor.idleJob(100);
  }
  t2 = high_resolution_clock::now();
  auto ev_idle = duration_cast<duration<double>>(t2 - t1).count();
  cout << "modifyEvent+idle       lines_parsed=" << counter.clear_lines << " time=" << ev_idle
       << "s invalidLine=" << baseEditor.getInvalidLine() << endl;
}