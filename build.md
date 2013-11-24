Building BePDF from Source Code
=================================

## Requirements
Haiku with Package Management (at the time of writing, only nightly builds had this)


## Required Packages
freetype2(+devel), liblayout(+devel), htmldoc.

## Building
Use the bash script `build.sh` to build BePDF.

Usage:
  build.sh [option]

If option is omitted BePDF and its required libraries are built.

Option can be one of
  clean ... to remove generated files and folders.
  debug ... to build libraries with debug information turned on.
  bepdf ... to build files in folder 'bepdf' only.

Make is used to build BePDF, `libsanta.a`, and `libxpdf.a`.

The temporary build files are created inside the project directories:
```
generated/libsanta.a
generated/libxpdf.a
```

The application directory contents is created in

  generated/BePDF

its contents is mainly copied from folder bepdf:

  Add BePDF to Deskbar       shell script that adds an icon to 
                             launch BePDF to the Deskbar
  BePDF                      the executable file
  bookmarks                  the bookmark files  
  docs                       the folder with documenation
  encodings                  the enconding files and folders
  fonts                      the PS and TT fonts
  lib                        the folder for shared libraries
  license                    the folder containing license text of 3rd 
                             party libraries 
  locale                     the localization files
  Remove BePDF from Deskbar  shell script to remove BePDF icon from Deskbar

The application folder is created only the first time
the build script is run.
To regenerate the application directory either do a "clean" build followed
by a regular build or delete the directory generated/x86 and do a regular
build.

At the end of the build the BePDF binary is copied into 
the application folder and the version information is updated in the file.

See build script for details.

## Documentation
The build script also generates the documenation (see bepdf/docs/make.sh)
when the application directory is set up the first time the
build script is run.

Note: htmldoc (http://htmldoc.org) is required to generate PDF files from
the HTML documentation.
