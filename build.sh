#! /bin/sh
cd $(dirname "$0")

shopt -s extglob

DEFAULT_DOCUMENT=English.pdf
INFO_SHORT="PDF Reader"
INFO_LONG="The PDF Reader for BeOS, Haiku and Zeta."

# The folder where the executable is installed
DESTINATION="$(pwd)/generated"
VERSION_FILE="$(pwd)/etc/VERSION"
VERSION=$(cat "$VERSION_FILE")
TOOLS_SOURCE="$(pwd)/etc/tools"
TOOLS_BIN="$DESTINATION/tools"

function copyFiles() {
	local src dst file dst2 
	src="$1"
	dst="$2"
	echo "Copy files from $src to $dst"
	for file in "$src/"!(CVS) ; do
		if [ -d "$file" ] ; then
			dst2="$dst"/$(basename "$file")
			mkdir -p "$dst2"
			copyattr -v "$file" "$dst2"
			copyFiles "$file" "$dst2" 
		else
			copyattr -v -d "$file" "$dst/"
		fi
	done	
}

# Setup directories and symbolic links
function setupBinFolder {
	arch="$1"
	folder="$DESTINATION"
	
	rm -rf "$folder/$arch"
	mkdir -p "$folder/$arch/lib"
	for file in "Add BePDF to Deskbar" "Remove BePDF from Deskbar" ; do
		copyattr -v -d "bepdf/$file" "$folder/$arch/$file"
	done
	for dir in encodings locale license ; do
		mkdir -p "$folder/$arch/$dir"
		copyFiles "bepdf/$dir" "$folder/$arch/$dir"
	done
	
	copyFiles "bepdf/Icons" "$folder/$arch/icons"

	mkdir -p "$folder/$arch/fonts/psfonts"
 	copyFiles bepdf/fonts/psfonts "$folder/$arch/fonts/psfonts"
 	
	cp layout/lib/liblayout.so "$folder/x86/lib/liblayout.so"	
}

function buildProject {
	debug="$1"
	shift
	folder="$1"
	shift

	echo "Building files in folder '$folder'..."
	current=$(pwd)
	cd "$folder"
	export DEBUGGER=$debug
	jam -q $@
	if [ $? -ne 0 ] ; then
		echo "Jam failed in folder '$folder'."
		exit 1
	fi
	cd "$current"
}

function buildDocumentation {
	arch="$1"

	( 
		cd bepdf/docs
		./make.sh -target "$DESTINATION/$arch/docs"
	)

	# htmldoc not ported to BeOS yet,
	# so simply copy pdf files from bepdf/docs
	(
		cd bepdf/docs
		for file in *.pdf ; do
			dest="$DESTINATION/$arch/docs/$file"
			# don't overwrite existing file
			if [ ! -e "$dest" ] ; then
				cp "$file" "$dest"
			fi
		done
	)
}

function buildTools {
	echo "Building tools"
	(
		cd "$TOOLS_SOURCE"
		mkdir -p "$TOOLS_BIN"
		cc CopyFileToAttribute.cpp -o "$TOOLS_BIN/CopyFileToAttribute" -lbe
	)
}


function splitVersion {
	# Split version information into components:
	# MAJOR "." MIDDLE "." MINOR POSTFIX [ "/" INTERNAL]
	MAJOR=$(cut -d . -f 1 < $VERSION_FILE)
	MIDDLE=$(cut -d . -f 2 < $VERSION_FILE)
	MINOR=$(cut -d . -f 3 < $VERSION_FILE)
	POSTFIX=${MINOR/[0123456789]/}
	POSTFIX=$(echo $POSTFIX | cut -d / -f 1)
	MINOR=${MINOR/[dabgmf]/}
	MINOR=$(echo $MINOR | cut -d / -f 1)
	INTERNAL=$(cut -d / -f 2 < $VERSION_FILE)
	# unset INTERNAL if "/ INTERNAL" is missing
	if [ "$INTERNAL" == "$VERSION" ] ; then
		unset INTERNAL
	fi
}

function updateVersion {
	path="$1"

	splitVersion

	echo setversion "$path" -app $MAJOR $MIDDLE $MINOR $POSTFIX $INTERNAL \
		-short "$INFO_SHORT" -long "$INFO_LONG"
	setversion "$path" -app $MAJOR $MIDDLE $MINOR $POSTFIX $INTERNAL \
		-short "$INFO_SHORT" -long "$INFO_LONG"
}

function setupBinary {
	file="$1"
	updateVersion "$file"
	chmod ug=rwx,o-rwx "$file"
}

function setVectorIcon {
	iconFile="$1"
	targetFile="$2"

	echo "Setting vector icon $iconFile to $targetFile"
	"$TOOLS_BIN/CopyFileToAttribute" "$iconFile" "VICN" "BEOS:ICON" "$targetFile"
}

function makePackage0 {
	arch="$1"
	separator="$2"
	suffix="$3"
	splitVersion
	version="$MAJOR.$MIDDLE.$MINOR$POSTFIX$INTERNAL"
	(
		cd "$DESTINATION"
		mv $arch BePDF
		rm -f "$DESTINATION/BePDF-$version$separator$suffix.zip"
		zip -9 -y -r "$DESTINATION/BePDF-$version$separator$suffix.zip" BePDF
		mv BePDF $arch
	)
}

function makePackage {
	arch="$1"
	makePackage0 "$arch" "." "$arch"
}

function clean {
	rm -rf $DESTINATION/x86
	rm -rf $TOOLS_BIN

	rm -rf santa/obj.X86
	rm -rf xpdf/obj.X86
	rm -rf bepdf/obj.X86
}

option="$1"
if [ "$option" == "clean" ] ; then
	clean
	exit 0
fi

if [ ! -e "$DESTINATION/x86" ] ; then
	setupBinFolder x86
fi

if [ ! -e "$DESTINATION/x86/docs/English.pdf" ] ; then
	buildDocumentation x86
fi

if [ ! -e "$TOOLSBIN/CopyFileToAttribute" ] ; then
	buildTools
fi

# Use jam file engine (from folder "etc")
export BUILDHOME=$(pwd)

debug="FALSE"
if [ "$option" == "debug" ] ; then
	debug=TRUE
	shift
fi

# Detect the build platform
BUILD_PLATFORM=BeOS
if [ "$MACHTYPE" == "i586-pc-haiku" ] ; then
	BUILD_PLATFORM=Haiku
fi
export BUILD_PLATFORM
echo BUILD_PLATFORM = $BUILD_PLATFORM

# Build projects
if [ "$option" == "bepdf" ] ; then
	buildProject $debug bepdf
else
	buildProject $debug santa libsanta.a
	buildProject $debug xpdf libxpdf.a
	buildProject $debug bepdf
fi

mv bepdf/obj.X86/BePDF "$DESTINATION/x86/"
setupBinary "$DESTINATION/x86/BePDF"
setVectorIcon "bepdf/icons/bepdf.hvif.attr" "$DESTINATION/x86/BePDF"

if [ "$option" == "package" ] ; then
	makePackage x86
fi
