# The script file generates html and pdf documentation in
# various languages from a set of html and template files.
# PDFs are generated with htmldoc [http://www.htmldoc.org]

# Directory structure of source files
# 
#   html/               common files and folders
#     index-header.html the header and footer of the index.html file
#     index-footer.html
#     BePDF-docu.css    Stylesheet for HTML output
#     images/           folder for language independant images
#     English/          language specific files for English documentation
#       images/         folder for language specific images
#       properties.sh   language specific settings specified in
#                       bash variables
#       titlepage.html  PDF-only title page with settings for PDF header/footer
#       *-body.html     language specific html files
#                      
#                       see below
#   templates           template files  
#     *-header.html     the template prepended to *-body.html file
#                       after replacing template variables
#     HTMLdoc.book      Config file for htmldoc used for generating PDFs

# Directory structure of generated files
#
#   index.html          the start page
#   English             the language specific html files
#   English.pdf         the language specific pdf files

# Language specific variables read from properties.sh used in this script file
#
# HTML_FILES_TO_COPY
# The list of files to copy from src/html/language to generated/html/language
#
# HTML_TEMPLATES
# The list of templates to copy from src/templates to generated/html/language
#
# PDF_FILES_TO_COPY
# The list of files to copy from src/html/language to generated/pdf/temp/language
#
# PDF_TEMPLATES
# The list of templates to copy from src/templates to generated/pdf/temp/language

# the directory containing this script file
# and directory html and templates
baseDirectory=$(pwd)
# the directory containing the plain html and related files
htmlDirectory="$baseDirectory/html"
# the directory containing template files
templatesDirectory="$baseDirectory/templates"

# the output directory
generatedDirectory="$baseDirectory/../docs-build"

# the base directory for the generated html files
htmlDestinationBase="$generatedDirectory"
# the base directory for the generated pdf files
pdfDestination="$generatedDirectory"
# the temporary directory used during pdf generation
pdfTemporaryDirectory="$pdfDestination/temp"

# the files and directories to copy to from html directory to
# generated html directory
htmlCommonFiles="images BePDF-docu.css"
# the files and directories to copy to from html directory to
# temporary pdf directory
pdfCommonFiles="$htmlCommonFiles"

# the name of the sed command file used to replace template values
replacementFile="commandFile"
# the name of the script file containing the template variable assignments
# the file is search in the language specific directory in $htmlDirectory
# e.g. src/html/Englisch/properties.sh
propertiesFile="properties.sh"

bodySuffix="-body.html"
headerSuffix="-header.html"

# whether to keep temporary files for testing (values true, false)
keepTemporaryFiles="false"

function createReplacementFile {
	properties="$1"
	(
	echo "# header"
	echo "source ./$properties"
	cat "$properties" | grep "=" | while read line ; do
		variable=$(echo "$line" | cut -f 1 -d "=")
		echo echo "s/$variable/\$$variable/g"
	done
	) > createCommandFile.sh
	chmod u+x createCommandFile.sh
	./createCommandFile.sh > "$replacementFile"
	if [ "$keepTemporaryFiles" == "false" ] ; then
		rm createCommandFile.sh
	fi
}

function appendToReplacementFile {
	key="$1"
	value="$2"
	echo "s/$key/$value/g" >> "$replacementFile"
}

function removeReplacementFile {
	if [ "$keepTemporaryFiles" == "false" ] ; then
		rm "$replacementFile"
	fi
}

function replaceProperties {
	input="$1"
	output="$2"
	sed -f "$replacementFile" "$input" > "$output"
}

function copyFiles {
	sourceDirectory="$1"
	files="$2"
	targetDirectory="$3"
	
	for file in $files ; do
		cp -R "$sourceDirectory/$file" "$targetDirectory"
		chmod -R g+w "$targetDirectory/$file"
	done
}

function replaceTemplateFiles {
	sourceDirectory="$1"
	files="$2"
	targetDirectory="$3"
	
	for file in $files ; do
		replaceProperties "$sourceDirectory/$file" "$targetDirectory/$file"
	done
}

function createHtml {
	language="$1"
	destination="$htmlDestinationBase/$language"
	mkdir -p "$destination"

	for file in *$bodySuffix ; do
		# remove $bodySuffix from file name
		base=${file/$bodySuffix/}
		replaceProperties "$templatesDirectory/$base$headerSuffix" $base$headerSuffix
		cat $base$headerSuffix $base$bodySuffix > "$destination/$base.html"
		rm $base$headerSuffix
	done

	# read language specific variables from file	
	source ./$propertiesFile
	
	replaceTemplateFiles "$templatesDirectory" "$HTML_TEMPLATES" "$destination"	
	copyFiles "." "$HTML_FILES_TO_COPY" "$destination"
}

function createPdf {
	language="$1"
	temporaryDirectory="$pdfTemporaryDirectory/$language"
	mkdir -p "$temporaryDirectory"
	destination="$temporaryDirectory"

	replaceProperties "$templatesDirectory/common-pdf-header.html" "$destination/common-pdf-header.html"
		
	for file in *$bodySuffix ; do
		# remove $bodySuffix from file name
		base=${file/$bodySuffix/}
		cat  "$destination/common-pdf-header.html" $base$bodySuffix > "$destination/$base.html"
	done

	# read language specific variables from file	
	source ./$propertiesFile

	replaceTemplateFiles "$templatesDirectory" "$PDF_TEMPLATES" "$destination"	
	copyFiles "." "$PDF_FILES_TO_COPY" "$destination"
	
	( 
		cd "$destination"
		htmldoc --batch HTMLdoc.book
		ps2pdf -dCompatibility=1.4 ../../$HTMLDOC_PS_FILE ../../$OUTPUT_PDF_FILE
		rm ../../$HTMLDOC_PS_FILE
	)
	
	if [ "$keepTemporaryFiles" == "false" ] ; then
		rm -rf "$destination"
	fi
}

function mergeHtmlFiles {
	for language in * ; do
		if [ -e "$language/$propertiesFile" ] ; then
			echo Generating documentation for language $language
			( 
				cd $language
				createReplacementFile "$propertiesFile"
				createHtml "$language"
				createPdf "$language"
				removeReplacementFile
			)
		fi
	done
}

function createIndexFile {
	index="$1"
	header="$2"
	footer="$3"
	item="$4"
	cp "$header" "$index"
	chmod g+w "$index"
	for language in * ; do
		if [ -e "$language/$propertiesFile" ] ; then
			createReplacementFile "$language/$propertiesFile"
			replaceProperties "$item" tempItem.html
			cat tempItem.html >> "$index"
			rm tempItem.html
			removeReplacementFile
		fi
	done
	cat "$footer" >> "$index"
}

function createIndexHtmlFile {
	createIndexFile "$htmlDestinationBase/index.html" "$htmlDirectory/index-header.html" \
		"$htmlDirectory/index-footer.html" "$templatesDirectory/index-item.html"
}

# created files can be read and written by user and group
umask 002

rm -rf "$generatedDirectory"

if [ "$1" == "clean" ] ; then
	exit 0
fi

# set up html destination directory 
mkdir -p "$htmlDestinationBase"
copyFiles "$htmlDirectory" "$htmlCommonFiles" "$htmlDestinationBase"

# set up pdf destination and temporary directory
mkdir -p "$pdfDestination"
mkdir -p "$pdfTemporaryDirectory"
copyFiles "$htmlDirectory" "$pdfCommonFiles" "$pdfTemporaryDirectory"

# generate html and pdf files for each language
cd html
mergeHtmlFiles

createIndexHtmlFile

# clean up temporary files
if [ "$keepTemporaryFiles" == "false" ] ; then
	rm -rf "$pdfTemporaryDirectory"
fi
