@echo off
:: This batch file generates a release for the PST File Format SDK. It must
:: be run from the root of the trunk/branch you want to generate a release 
:: for. "svn up" will be called.
::
:: To use it, type "release x_y_z" where x, y, and z form the version number
::
:: 7zip must be in your path
:: sfk must be in your path
:: svn must be in your path
:: doxygen must be in your path

::
:: Check preconditions
::

if not exist Doxyfile goto Usage
if %1!==! goto Usage
goto Start
:Usage
echo Release.bat - Generate a release for PST File Format SDK.
echo.
echo This tool must be run from the root of a branch/trunk.
echo.
echo This tool requires that 7zip, sfk, svn, and doxygen are all in the
echo current PATH.
echo.
echo Release.bat will generate a release directory as well as the two release
echo zip files (doc and nodoc) as a sibling of the branch/trunk directory.
echo.
echo Usage: release 1_2_3 
echo        Creates a release for version 1.2.3
goto Done
:Start
::
:: Update everyone, regenerate documentation
::

svn up
doxygen Doxyfile

::
:: Export to target dir
::

svn export . ..\pstsdk_%1

::
:: Remove unwanted files
::

del ..\pstsdk_%1\Doxyfile
del ..\pstsdk_%1\release.bat
del ..\pstsdk_%1\.gitignore
del /q ..\pstsdk_%1\doc\*.pdf
del ..\pstsdk_%1\pstsdk\ndb\context.h

::
:: Convert everything to windows line endings
::

sfk addcr ..\pstsdk_%1 .h .cpp .txt

::
:: Create release files
::

:: nodoc zip
7za a ..\pstsdk_%1_nodoc.zip ..\pstsdk_%1 -r
7za a ..\pstsdk_%1_nodoc.7z ..\pstsdk_%1 -r

:: copy over docs
copy doc\*.pdf ..\pstsdk_%1\doc
mkdir ..\pstsdk_%1\doc\html
copy doc\html ..\pstsdk_%1\doc\html
mkdir ..\pstsdk_%1\doc\html\search
copy doc\html\search ..\pstsdk_%1\doc\html\search

:: doc zip
7za a ..\pstsdk_%1.zip ..\pstsdk_%1 -r
7za a ..\pstsdk_%1.7z ..\pstsdk_%1 -r

:Done
