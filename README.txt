qolibri - EPWING Dictionary/Book Viewer
---------------------------------------

qolibri is a viewer application of EPWING Dictionary/Book 

EPWING is a format of Dictionary that regulated by JIS(Japanese Industrlal Standard).
This application written in C++ around the Qt4 toolkit and using the library of
dictionary processing - eblib. These development environment allows to deploy application for several platforms which are available Qt toolkit, such as Linux, Windows, MacOS X

Home Page :  http://sourceforge.jp/projects/qolibri/

License : GNU Public License (GPL)



::: Requirement :::

This software requires following libraries for building and runnning.

Qt4 - C++ class library and tools for crossplatform development and
      internationalization
      http://www.trolltech.org/products/qt/

eblib - Library for EPWIND formatted dictionary.
        http://www.sra.co.jp/people/m-kasahr/




::: Installation :::

Linux : 
  qmake
  make
  make install     | this requires root privilage
                   | currently binary file will be copied to /usr/bin and
                   | translation files to applopriate path that Qt4 settled.

MacOS X :
  qmake
  make
  make install      | this doesn't require root privilage 
                    | simply translation files(./src/i18n/) copied to
                    | following directory
                    | ./bin/qolibri.app/Contents/MaxOSX.

Windows :
  qmake 
  mingw32-make
  # please copy these directory to ./bin manually 
  # ./src/i18n
  # then ./bin directory contains qolibri.exe i18n/
