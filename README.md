batch-resource-updater
======================

The BatchResourceUpdater (BRU) program automates extracting, updating, and removing of resources from [PE][PE] and [RES][RES] files. It uses an XML file to describe all the actions it should perform, manipulates the files, saves them, and updates the checksum of the files. This program is not fully tested so backup all files first. However, I have seen no problems yet.

This is meant to complement, not replace, programs such as [ResourceHacker][ResourceHacker]. BatchResourceUpdater provides you with no way of finding out which resources already exist and you can only view resources by extracting them. It does nothing special for string, menu, and dialog resource types. Instead you should use ResourceHacker to modify the resources, extract them, and then use BRU. Where BRU really shines is the automation - allowing you to 'instantly' apply resource changes if the binary changes (e.g. because of a Windows Update).

It is written in C++/CLR and requires .NET 2.0 and Visual C++ 2010 SP1 runtimes (download [x86][VC2010_32] / [x64][VC2010_64]). The PE/RES file-modifying code and ICO/CUR manipulators are written in C++ and do not require any libraries.


Features
--------

* Extracts, adds, and removes resources from [PE][PE] (EXE, DLL, OCX, SYS, DRV, MUI, ...) and [RES][RES] files
* XML file describes batch resource extracting / updating
* Uses a custom made PE/RES file writer (haven't seen this fail yet, even when [ResourceHacker][ResourceHacker] fails)
* Smart extracting/updating of:
  * BITMAP to/from BMP
  * ICON and GROUP_ICON to/from ICO
  * CURSOR and GROUP_CURSOR to/from CUR
* Other resource types are copied as binary (works well for PNG, RCDATA, IMAGE, ANICURSOR, ...)
* Automatically updates checksum on updated files


Features it will never have
---------------------------

* Any filetype conversion (e.g. you must use a BMP image for BITMAP resources)
* Be MUI aware
* Support for files >2GB (have you ever seen an EXE/DLL that is >2GB? - actually it isn't allowed!)
* Support for [resource script (RC)](http://msdn.microsoft.com/en-us/library/aa380599%28VS.85%29.aspx) files ([windres](http://sourceware.org/binutils/docs/binutils/windres.html) can be used to convert between RES and RC)
* Support for [.NET resource formats](http://msdn.microsoft.com/en-us/library/xbx3z216%28VS.80%29.aspx) ([RESX](http://msdn.microsoft.com/en-us/library/ekyft91f%28VS.80%29.aspx), [RESOURCES](http://msdn.microsoft.com/en-us/library/zew6azb7%28VS.80%29.aspx), or [text](http://msdn.microsoft.com/en-us/library/s9eey0h7%28VS.80%29.aspx))


[PE]: http://en.wikipedia.org/wiki/Portable_Executable
[RES]: http://msdn.microsoft.com/en-us/library/ms648007%28VS.85%29.aspx
[ResourceHacker] http://angusj.com/resourcehacker/
[VC2010_32] http://www.microsoft.com/download/en/details.aspx?id=8328
[VC2010_64] http://www.microsoft.com/download/en/details.aspx?id=13523
