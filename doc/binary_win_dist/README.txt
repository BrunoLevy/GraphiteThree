=============== Graphite /// ================

This is a binary distribution of Graphite

To start Graphite, click on graphite.bat (nothing to "install" !)

The Graphite executable is in GraphiteThree/bin/win64/graphite.exe
(you may create a shortcut to it on the desktop)

Graphite website is on github:
   https://github.com/BrunoLevy/GraphiteThree

Note 1
------
The first time you start Graphite, Windows will complain with a (rather frightening) dialog box
from Microsoft Defender SmartScreen that starting `graphite.bat` represents a risk for your computer.
Click on `Additional information`, then it will indicate that the application is from an `Unknown editor`.
Click on the button `Execute anyway` that appeared when you clicked on `Additional information`.

*Why is this so ?* It is because we did not sign the Graphite application. Signing application requires to pay a
signing authority and means additional complexity in the build system, which we did not want to do.

Note 2
------
There is nothing to "install", Graphite runs from the files extracted from the `.zip` files, and does
not touch the registry or whatever else in the system. If you want an icon on the desktop,
you may create a shortcut to `graphite.bat` or to `GraphiteThree/bin/win64/graphite.exe`.

Troubleshooting:
----------------

If you got an error message about missing MSVCP???.dll or missing
VCRUNTIME???.dll, then you'll need to install a small package (a few
megabytes):
   Visual C++ 2022 redistribuable package (x64)
(can be obtained from www.microsoft.com, just google it)


