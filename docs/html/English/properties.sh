# Before the following characters inside the quotes a backslash has to be prepended:
# "/&,".
# e.g. \/ instead of /

# Files (spaces in file names are not supported)
# The list of files to copy from src/html/language to generated/html/language
HTML_FILES_TO_COPY=""
# The list of templates to copy from src/templates to generated/html/language
HTML_TEMPLATES="table_of_contents.html"
# The list of files to copy from src/html/language to generated/pdf/temp/language
PDF_FILES_TO_COPY="titlepage.html"
# The list of templates to copy from src/templates to generated/pdf/temp/language
PDF_TEMPLATES="HTMLdoc.book"

# Meta data
META_AUTHOR="Celerick Stephens\, Michael Pfeiffer"
META_DATE="2013-11-10"
META_LANGUAGE="en"

# the name of the language
HTML_LANGUAGE="English"

# HTMLdoc properties
HTMLDOC_PS_FILE="English.ps"
OUTPUT_PDF_FILE="English.pdf"
HTMLDOC_TOC_TITLE="Table of Contents"
HTMLDOC_TITLE="BePDF Documentation"

# URI encoded properties 
# US-ASCII with %hh (2 hex-digits) for non-ASCII characters
# UTF-8 for % encoded characters
URI_DIRECTORY="English"
URI_PDF_FILE="$URI_DIRECTORY.pdf"

# HTML table of contents
TOC_TITLE="Table of Contents"
TOC_ABOUT="About BePDF"
TOC_REQUIREMENTS="Program Requirements"
TOC_GUI="Graphical User Interface"
TOC_PREFERENCES="Preferences"
TOC_KEYBOARD="Keyboard Commands"
TOC_MOUSE="Mouse Controls"
TOC_ATTRIBUTES="Tracker Attributes"
TOC_ANNOTATIONS="Creating and Editing Annotations"
TOC_PRINTING="Printing"
TOC_LOCALIZATION="Localization"
TOC_GET="Get BePDF for your Site"
TOC_BUGS="Known Bugs"
TOC_ACKNOWLEDGEMENTS="Acknowledgements"
TOC_HISTORY="Version History"

# HTML navigation bar
NAV_CONTENTS="Contents"
NAV_PREV="Previous"
NAV_NEXT="Next"
NAV_ABOUT="$TOC_ABOUT"
NAV_REQUIREMENTS="$TOC_REQUIREMENTS"
NAV_GUI="$TOC_GUI"
NAV_PREFERENCES="$TOC_PREFERENCES"
NAV_KEYBOARD="$TOC_KEYBOARD"
NAV_MOUSE="$TOC_MOUSE"
NAV_ATTRIBUTES="$TOC_ATTRIBUTES"
NAV_ANNOTATIONS="$TOC_ANNOTATIONS"
NAV_PRINTING="$TOC_PRINTING"
NAV_LOCALIZATION="$TOC_LOCALIZATION"
NAV_GET="$TOC_GET"
NAV_BUGS="$TOC_BUGS"
NAV_ACKNOWLEDGEMENTS="$TOC_ACKNOWLEDGEMENTS"
NAV_HISTORY="$TOC_HISTORY"

# HTML title in navigation bar
TITLE_PREFIX="BePDF -"
TITLE_TOC="$TITLE_PREFIX $TOC_TITLE"
TITLE_ABOUT="$TITLE_PREFIX $TOC_ABOUT"
TITLE_REQUIREMENTS="$TITLE_PREFIX $TOC_REQUIREMENTS"
TITLE_GUI="$TITLE_PREFIX $TOC_GUI"
TITLE_PREFERENCES="$TITLE_PREFIX $TOC_PREFERENCES"
TITLE_KEYBOARD="$TITLE_PREFIX $TOC_KEYBOARD"
TITLE_MOUSE="$TITLE_PREFIX $TOC_MOUSE"
TITLE_ATTRIBUTES="$TITLE_PREFIX $TOC_ATTRIBUTES"
TITLE_ANNOTATIONS="$TITLE_PREFIX $TOC_ANNOTATIONS"
TITLE_PRINTING="$TITLE_PREFIX $TOC_PRINTING"
TITLE_LOCALIZATION="$TITLE_PREFIX $TOC_LOCALIZATION"
TITLE_GET="$TITLE_PREFIX $TOC_GET"
TITLE_BUGS="$TITLE_PREFIX $TOC_BUGS"
TITLE_ACKNOWLEDGEMENTS="$TITLE_PREFIX $TOC_ACKNOWLEDGEMENTS"
TITLE_HISTORY="$TITLE_PREFIX $TOC_HISTORY"

