qolibri - EPWING Dictionary/Book Viewer
---------------------------------------

qolibri is a viewer application of EPWING-Formatted Dictionary/Book.
EPWING is a format of Dictionary that regulated by JIS(Japanese Industrial
Standard).

This application written in C++ around the Qt4 toolkit and using the library of
epwing dictionary processing - eblib.
This development environment allows to deploy application for several platforms
which are available Qt toolkit, such as Linux, Windows, MacOS X

Home Page :  http://sourceforge.jp/projects/qolibri/
License   : GNU Public License (GPL)



::: Requirement :::

This software requires following libraries for building and runnning.

Qt4   - C++ class library and tools for crossplatform development and
        internationalization
        http://www.trolltech.org/products/qt/

eblib - Library for EPWING formatted dictionary.
        http://www.sra.co.jp/people/m-kasahr/



::: Installation :::

Linux : 
  qmake
  make
  make install     | This requires root privilage.
                   | Currently, binary file will be copied to "/usr/bin" and
                   | translation files will be copied to applopriate path that
                   | Qt4 had settled.

MacOS X :
  qmake
  make
  make install      | This doesn't require root privilage.
                    | Simply, translation files(./src/i18n/) will be copied to
                    | following directory.
                    | "./bin/qolibri.app/Contents/MaxOSX."

Windows :
  qmake 
  mingw32-make
  # please copy following directory to "./bin" manually.
  #    ./src/i18n
  # then "./bin" directory contains "qolibri.exe",  "i18n/"

