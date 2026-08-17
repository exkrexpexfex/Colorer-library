#include <colorer/parsers/ParserFactoryImpl.h>
#include <colorer/editor/BaseEditor.h>
#include <colorer/viewer/TextLinesStore.h>
#include <colorer/RegionHandler.h>

void TestParserFactoryConstructor(int count, UnicodeString* catalogPath);
void TestParserFactoryHrcLibrary(int count, UnicodeString* catalogPath);
void TestParserFactoryStyledMapper(int count, UnicodeString* catalogPath);
void TestParserFactoryLoadAllHRCScheme(int count, UnicodeString* catalogPath);
void TestColoringFile(int count, UnicodeString* catalogPath, UnicodeString* testFile);
void TestEditorLineEdit(int count, UnicodeString* catalogPath, UnicodeString* testFile);