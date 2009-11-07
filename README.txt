Hummingbird EB Viewer
---------------------

::: Requirement :::

This software requires following libraries for building and runnning.

Qt4   - C++ class library and tools for cross-platform development and
        internationalization
        http://qt.nokia.com/

EB Library - Library for EPWING formatted dictionary.
             http://www.sra.co.jp/people/m-kasahr/eb


::: Installation :::

Linux : 
  $ qmake
  $ make
  $ sudo make install

If you'd like to install to your home directory and debug build.

  $ qmake CONFIG+=debug INSTALL_PREFIX=$HOME/opt
  $ make
  $ make install
