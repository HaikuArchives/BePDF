#!/bin/sh
cd $(dirname "$0")

shopt -s extglob

INFO_SHORT="PDF Reader"
INFO_LONG="The PDF Reader for Haiku."

DESTINATION="$(pwd)/generated"

CC_VER=$(gcc -dumpversion | head -c 1)
OBJDIR=objects.x86-gcc$CC_VER-release
echo $OBJDIR

# Setup directories and symbolic links
function setupBinFolder {
	arch="$1"
	folder="$DESTINATION"
	
	rm -rf "$folder/$arch"
	for dir in encodings locale license ; do
		mkdir -p "$folder/$arch/$dir"
		cp -R "bepdf/$dir" "$folder/$arch/"
	done

	mkdir -p "$folder/$arch/fonts"
 	cp -R bepdf/fonts/ "$folder/$arch/"
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

function copyDocs {
	arch="$1"

	(
		mkdir "$DESTINATION/$arch/docs"
		cp bepdf/docs/*.pdf "$DESTINATION/$arch/docs/"
		
		cd "$DESTINATION/$arch/docs"
		wget http://haikuarchives.github.io/BePDF/English.pdf
		wget http://haikuarchives.github.io/BePDF/Deutsch.pdf
		wget http://haikuarchives.github.io/BePDF/Español.pdf
		wget http://haikuarchives.github.io/BePDF/Italiano.pdf
		cd -
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

if [ ! -d "$DESTINATION/BePDF" ] ; then
	setupBinFolder BePDF
fi

if [ ! -d "$DESTINATION/BePDF/docs" ] ; then
	copyDocs BePDF
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
	buildProject $debug santa
	buildProject $debug xpdf
	buildProject $debug bepdf
fi

mv bepdf/$OBJDIR/BePDF "$DESTINATION/BePDF/"
