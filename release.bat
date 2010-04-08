:: This batch file generates a release for the PST File Format SDK. It must
:: be run from the root of the trunk/branch you want to generate a release 
:: for, or unexpected things will happen. "svn up" will be called.
::
:: To use it, type "release x_y_z" where x, y, and z form the version number
::
:: 7zip must be in your path
:: sfk must be in your path
:: svn must be in your path
:: doxygen must be in your path

::
:: Update everyone, regenerate documentation
::

svn up
doxygen Doxyfile
cd ..

::
:: Export to target dir
::

svn export fairport pstsdk_%1

::
:: Remove unwanted files
::

del pstsdk_%1\Doxyfile
del pstsdk_%1\release.bat
del /q pstsdk_%1\doc\*.pdf
del pstsdk_%1\pstsdk\ndb\context.h

::
:: Convert everything to windows line endings
::

sfk addcr .h .cpp .txt

::
:: Create release files
::

:: nodoc zip
7za a pstsdk_%1_nodoc.zip pstsdk_%1 -r

:: copy over docs
copy fairport\doc\*.pdf pstsdk_%1\doc
mkdir pstsdk_%1\doc\html
copy fairport\doc\html pstsdk_%1\doc\html

:: doc zip
7za a pstsdk_%1.zip pstsdk_%1 -r