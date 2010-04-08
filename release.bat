::
:: Update Everything
::
cd fairport
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
del /q pstsdk_%1\doc\*.pdf
del pstsdk_%1\pstsdk\ndb\context.h

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