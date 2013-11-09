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
META_LANGUAGE="es"

# the name of the language
HTML_LANGUAGE="Espa\&ntilde;ol; Castellano"

# HTMLdoc properties
HTMLDOC_PDF_FILE="Español.pdf"
HTMLDOC_TOC_TITLE="Tabla de Contenidos"
HTMLDOC_TITLE="Documentaci\&oacute;n de BePDF"

# URI encoded properties 
# US-ASCII with %hh (2 hex-digits) for non-ASCII characters
# UTF-8 for % encoded characters
URI_DIRECTORY="Espa%C3%B1ol"
URI_PDF_FILE="$URI_DIRECTORY.pdf"

# HTML table of contents
TOC_TITLE="Tabla de Contenidos"
TOC_ABOUT="Acerca de BePDF"
TOC_REQUIREMENTS="Requerimientos del Programa"
TOC_INSTALLATION="Instalaci\&oacute;n"
TOC_GUI="Interfaz Gr\&aacute;fica de Usuario"
TOC_PREFERENCES="Preferencias"
TOC_KEYBOARD="Comandos de Teclado"
TOC_MOUSE="Controles del Rat\&oacute;n"
TOC_ATTRIBUTES="Atributos del Tracker"
TOC_ANNOTATIONS="Crear y Editar Anotaciones"
TOC_PRINTING="Imprimir"
TOC_LOCALIZATION="Traducir"
TOC_GET="Obtener BePDF para Su Sitio Web"
TOC_BUGS="Errores Conocidos"
TOC_UPCOMING_FEATURES="Caracter\&iacute;sticas Futuras"
TOC_ACKNOWLEDGEMENTS="Reconocimientos"
TOC_HISTORY="Historial de Versiones"

# HTML navigation bar
NAV_CONTENTS="Contenidos"
NAV_PREV="Anterior"
NAV_NEXT="Siguiente"
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

