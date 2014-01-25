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
META_DATE="2009-01-29"
META_LANGUAGE="de"

# the name of the language
HTML_LANGUAGE="Deutsch"

# HTMLdoc properties
HTMLDOC_PS_FILE="Deutsch.ps"
OUTPUT_PDF_FILE="Deutsch.pdf"
HTMLDOC_TOC_TITLE="Inhaltsverzeichnis"
HTMLDOC_TITLE="BePDF Dokumentation"

# URI encoded properties 
# US-ASCII with %hh (2 hex-digits) for non-ASCII characters
# UTF-8 for % encoded characters
URI_DIRECTORY="Deutsch"
URI_PDF_FILE="$URI_DIRECTORY.pdf"

# HTML table of contents
TOC_TITLE="Inhaltsverzeichnis"
TOC_ABOUT="\&Uuml;ber BePDF"
TOC_REQUIREMENTS="Systemanforderungen"
TOC_INSTALLATION="Installation"
TOC_GUI="Benutzeroberfl\&auml;che"
TOC_PREFERENCES="Einstellungen"
TOC_KEYBOARD="Tastaturk\&uuml;rzel"
TOC_MOUSE="Maus Bedienung"
TOC_ATTRIBUTES="Tracker Attribute"
TOC_ANNOTATIONS="Anmerkungen erstellen und \&auml;ndern"
TOC_PRINTING="Drucken"
TOC_LOCALIZATION="Lokalisierung"
TOC_GET="\"Get BePDF\" Banner"
TOC_BUGS="Bekannte Fehler"
TOC_UPCOMING_FEATURES="Zuk\&uuml;nftige Features"
TOC_ACKNOWLEDGEMENTS="Danksagungen"
TOC_HISTORY="Historie"

# HTML navigation bar
NAV_CONTENTS="Inhalt"
NAV_PREV="Zur\&uuml;ck"
NAV_NEXT="Weiter"
NAV_ABOUT="$TOC_ABOUT"
NAV_REQUIREMENTS="$TOC_REQUIREMENTS"
NAV_INSTALLATION="$TOC_INSTALLATION"
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
NAV_UPCOMING_FEATURES="$TOC_UPCOMING_FEATURES"
NAV_ACKNOWLEDGEMENTS="$TOC_ACKNOWLEDGEMENTS"
NAV_HISTORY="$TOC_HISTORY"

# HTML title in navigation bar
TITLE_PREFIX="BePDF -"
TITLE_TOC="$TITLE_PREFIX $TOC_TITLE"
TITLE_ABOUT="$TITLE_PREFIX $TOC_ABOUT"
TITLE_REQUIREMENTS="$TITLE_PREFIX $TOC_REQUIREMENTS"
TITLE_INSTALLATION="$TITLE_PREFIX $TOC_INSTALLATION"
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
TITLE_UPCOMING_FEATURES="$TITLE_PREFIX $TOC_UPCOMING_FEATURES"
TITLE_ACKNOWLEDGEMENTS="$TITLE_PREFIX $TOC_ACKNOWLEDGEMENTS"
TITLE_HISTORY="$TITLE_PREFIX $TOC_HISTORY"

