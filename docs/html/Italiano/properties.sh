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
META_LANGUAGE="it"

# the name of the language
HTML_LANGUAGE="Italiano"

# HTMLdoc properties
HTMLDOC_PS_FILE="Italiano.ps"
OUTPUT_PDF_FILE="Italiano.pdf"
HTMLDOC_TOC_TITLE="Indice dei contenuti"
HTMLDOC_TITLE="Documentazione di BePDF"

# URI encoded properties 
# US-ASCII with %hh (2 hex-digits) for non-ASCII characters
# UTF-8 for % encoded characters
URI_DIRECTORY="Italiano"
URI_PDF_FILE="$URI_DIRECTORY.ps"

# HTML table of contents
TOC_TITLE="Indice dei contenuti"
TOC_ABOUT="Informazioni su BePDF"
TOC_REQUIREMENTS="Requisiti di programma"
TOC_INSTALLATION="Installazione"
TOC_GUI="Interface grafica utente"
TOC_PREFERENCES="Preferenze"
TOC_KEYBOARD="Comandi veloci da tastiera"
TOC_MOUSE="Controllo col mouse"
TOC_ATTRIBUTES="Attributi del Tracker"
TOC_ANNOTATIONS="Creare ed editare annotazioni"
TOC_PRINTING="Stampa"
TOC_LOCALIZATION="Traduzioni"
TOC_GET="Ottenga BePDF per il Suo sito Web"
TOC_BUGS="Bug conosciuti"
TOC_UPCOMING_FEATURES="Caratteristiche da venire"
TOC_ACKNOWLEDGEMENTS="Ringraziamenti"
TOC_HISTORY="Cronologia delle versione"

# HTML navigation bar
NAV_CONTENTS="Contenuti"
NAV_PREV="Precedente"
NAV_NEXT="Successivo"
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

