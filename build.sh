#! /bin/sh
cd $(dirname "$0")

shopt -s extglob

DEFAULT_DOCUMENT=English.pdf
INFO_SHORT="PDF Reader"
INFO_LONG="The PDF Reader for BeOS, Haiku and Zeta."

# The folder where the executable is installed
DESTINATION="$(pwd)/generated"
TOOLS_SOURCE="$(pwd)/etc/tools"
TOOLS_BIN="$DESTINATION/tools"
OBJDIR="objects.x86-gcc2-release"

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
	make
	if [ $? -ne 0 ] ; then
		echo "Make failed in folder '$folder'."
		exit 1
	else
		if [ "$folder" == 'xpdf' ] ; then
			cp "$OBJDIR/libxpdf.a" "../generated/"
		elif [ "$folder" == 'santa' ] ; then
			cp "$OBJDIR/libsanta.a" "../generated/"
		fi
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

function clean {
	rm -rf $DESTINATION/BePDF

	rm -rf santa/$OBJDIR
	rm -rf xpdf/$OBJDIR
	rm -rf bepdf/$OBJDIR
}

option="$1"
if [ "$option" == "clean" ] ; then
	clean
	exit 0
fi

if [ ! -e "$DESTINATION/BePDF" ] ; then
	setupBinFolder BePDF
fi

if [ ! -e "$DESTINATION/BePDF/docs/English.pdf" ] ; then
	buildDocumentation BePDF
fi

debug="FALSE"
if [ "$option" == "debug" ] ; then
	debug=TRUE
	shift
fi

# Build projects
if [ "$option" == "bepdf" ] ; then
	buildProject $debug bepdf
else
	buildProject $debug santa libsanta.a
	buildProject $debug xpdf libxpdf.a
	buildProject $debug bepdf
fi

mv bepdf/$OBJDIR/BePDF "$DESTINATION/BePDF/"
