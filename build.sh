#!/bin/sh

if [ ! -f "dist/docs/English.pdf" ]; then
	cd dist/docs/
	wget https://haikuarchives.github.io/BePDF/English.pdf
	wget https://haikuarchives.github.io/BePDF/Deutsch.pdf
	wget https://haikuarchives.github.io/BePDF/Español.pdf
	wget https://haikuarchives.github.io/BePDF/Italiano.pdf
	cd ../..
fi

cd xpdf
make $@
cd ../bepdf
make $@
cd ..
