#include "EncodingReader.h"

#include "aconf.h"
#include "gfile.h"
#include "GDir.h"
#include "GlobalParams.h"
#include "GString.h"
#include <string.h>

/*
# code from file name
cidToUnicode code path_to/code.cidToUnicode
# if code.cidToUnicode exists and directory exists
cMapDir code path_to/CMap
# if code.cidToUnicode exists
displayCIDFont(TT|T1) code path_to/font.(ttf|ttc|...)

# entry if CMap directory exists:
toUnicodeDir path_to/CMap
# code from file name
unicodeMap code path_to/code.unicodeMap
#
nameToUnicode path_to/\*.nameToUnicode
*/

static GBool endsWith(GString* string, const char* suffix) {
	const char* stringPtr = string->getCString();
	int stringLength = string->getLength();
	int suffixLength = strlen(suffix);
	if (stringLength < suffixLength) {
		return gFalse;
	}

	return strcmp(suffix, &stringPtr[stringLength - suffixLength]) == 0;
}

static int indexOf(GString* string, const char ch) {
	const char* position = strchr(string->getCString(), ch);
	if (position == NULL) {
		return -1;
	} else {
		return position - string->getCString();
	}
}

static GString* extractEncoding(GString* string) {
	int index = indexOf(string, '.');
	if (index != 0) {
		return new GString(string, 0, index);
	}
	return NULL;
}

static const char* makePath(GString& string, const char* directory, const char* name) {
	string.clear();
	string.append(directory);
	appendToPath(&string, (char*)name);
	return string.getCString();
}

static void setLine(const char* type, const char* arg1, const char* arg2) {
	GString line;
	line.append(type);
	line.append(" ");
	line.append(arg1);
	if (arg2 != NULL) {
		line.append(" ");
		line.append(arg2);
	}
	// fprintf(stderr, "%s\n", line.getCString());
	GString name("BePDF Encoding Reader");
	globalParams->parseLine(line.getCString(), &name, 0);
}

static void addNameToUnicode(GlobalParams* globalParams, const char* file) {
	// fprintf(stderr, "add nameToUnicode %s\n", file);
	setLine("nameToUnicode", file, NULL);
}

static void addUnicodeMap(GlobalParams* globalParams, const char* encoding, const char* file) {
	// fprintf(stderr, "add unicodeMap %s: %s\n", encoding, file);
	setLine("unicodeMap", encoding, file);
}

static void addToUnicodeDir(GlobalParams* globalParams, const char* directory) {
	// fprintf(stderr, "add toUnicodeDir %s\n", directory);
	setLine("toUnicodeDir", directory, NULL);
}

static void addCMapDir(GlobalParams* globalParams, const char* encoding, const char* directory) {
	// fprintf(stderr, "add cMapDir %s: %s\n", encoding, directory);
	setLine("cMapDir", encoding, directory);
}

static void addCidToUnicode(GlobalParams* globalParams, const char* encoding, const char* directory) {
	// fprintf(stderr, "add cidToUnicode %s: %s\n", encoding, directory);
	setLine("cidToUnicode", encoding, directory);
}

static void readFromDirectory(GlobalParams* globalParams, const char* directoryName) {
	GBool hasCMapDirectory = gFalse;
	GString* collection = NULL;

	GDir directory((char*)directoryName);
	GDirEntry* entry;
	GString string; // used to create temporary file or directory path

	// Read nameToUnicode and unicodeMap,
	// record cidToUnicode collection name and whether there is a CMap directory
	while ((entry = directory.getNextEntry())) {

		GString* name = entry->getName();

		if (entry->isDir()) {

			if (name->cmp("CMap") == 0) {
				hasCMapDirectory = gTrue;
			}

		} else if (endsWith(name, ".cidToUnicode")) {

			if (collection == NULL) {
				collection = extractEncoding(name);
			} else {
				// warning: more than one cidToUnicode file!
			}

		} else if (endsWith(name, ".nameToUnicode")) {

			addNameToUnicode(globalParams,
				makePath(string, directoryName, name->getCString()));

		} else if (endsWith(name, ".unicodeMap")) {

			GString* encoding = extractEncoding(name);
			if (encoding == NULL) {
				// warning: missing encoding in .unicodeMap file name
				continue;
			}

			addUnicodeMap(globalParams,
				encoding->getCString(),
				makePath(string, directoryName, name->getCString()));

			delete encoding;

		}
		delete entry;
	}

	// add cidToUnicode, cMapDir and toUnicodeDir
	if (hasCMapDirectory) {
		addToUnicodeDir(globalParams, makePath(string, directoryName, "CMap"));

		if (collection != NULL) {
			// still string contains CMap directory
			addCMapDir(globalParams, collection->getCString(), string.getCString());

			makePath(string, directoryName, collection->getCString());
			string.append(".cidToUnicode");
			addCidToUnicode(globalParams, collection->getCString(), string.getCString());
		} else {
			// warning: CMap directory without .cidToUnicode file!
		}
	}

	delete collection;
}

void readEncodings(GlobalParams* globalParams, const char* encodingDirectory) {
	if (encodingDirectory == NULL) {
		return;
	}

	GDir baseDirectory((char*)encodingDirectory);
	GDirEntry* entry;
	GString subDirectory;
	while ((entry = baseDirectory.getNextEntry())) {
		if (entry->isDir()) {
			subDirectory.append(encodingDirectory);
			appendToPath(&subDirectory, entry->getName()->getCString());
			readFromDirectory(globalParams, subDirectory.getCString());
			subDirectory.clear();
		}
		delete entry;
	}
}
