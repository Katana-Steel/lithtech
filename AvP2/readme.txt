==========================================

ALIENS VERSUS PREDATOR 2
Source Code v1.0.9.6
July 15, 2002

==========================================


I.     SYSTEM REQUIREMENTS
II.    INSTALLATION INSTRUCTIONS
III.   DIRECTORY STRUCTURE
IV.    AVP2 VISUAL C++ WORKSPACE
V.     END USER LISCENCE AGREEMENT (EULA)

The Aliens Versus Predator 2 (AVP2) Source Code v1.0.9.6 is not
officially supported by Monolith Productions, Inc. or Fox
Interactive. If you plan to modify the AVP2 Source Code it is
assumed that you have a good understanding of the AVP2 tools and
how Monolith games work (i.e., you have worked with the NOLF, Shogo
or Blood 2 source code).

NOTE: The source code is specifically designed to work with AVP2
version 1.0.9.6. Using modifications made with it on older versions
of AVP2 may lead to unpredictable results.



I.      SYSTEM REQUIREMENTS
__________________________________

To build the AVP2 source v1.0.9.6 you will need the following:

    Visual C++ 6.0 with service pack 3 installed
    DX8.1 sdk
    400MB free disk space



II.     INSTALLATION INSTRUCTIONS
__________________________________

To install the AVP2 source code you must have a utility which can
extract from ".zip" files.

1) Extract the .zip archive to your C: drive root, making sure that
you preserve the archive's folder names when you unzip it (this may
be a setting in your extraction utility). The contents of the .zip
will extract into a folder called "proj."

2) Add this line to your autoexec.bat file:

    CALL c:\proj\avp2\source\setbui~1.bat

    [Note: if you need to install the source to a directory other
    than c:\proj, you will have to replace "proj" in the
    c:\proj\avp2\source\setbui~1.bat file with the directory path
    you will be using.]

3) Reboot your computer to use the new environment settings from
the step above.

4) Start VC++ 6.0.

5) In the Tools/Options/Directories menu, place these above all
   other listings for include files:
    C:\DXSDK\INCLUDE
        [Note: DirectX SDK install path may vary. Enter the path
        that matches your installation.]
    C:\PROJ\LT2\LITHSHARED\INCS
    C:\PROJ\LT2\LITHSHARED\STL
    C:\PROJ\LT2\LITHSHARED\WONAPI
    C:\PROJ\LT2\SDK\INC
    C:\PROJ\LT2\SDK\INC\COMPAT
  In the Tools/Options/Directories menu, place these above all other
  listings for library files:
    C:\DXSDK\LIB
        [Note: DirectX SDK install path may vary. Enter the path
        that matches your installation.]
    C:\PROJ\LT2\LITHSHARED\LIBS\RELEASE
    C:\PROJ\LT2\LITHSHARED\LIBS\DEBUG

6) Load the AVP2 project (c:\proj\AVP2\AVP2.dsw). After doing this,
if VC++ tells you that a project was not found and asks if you want
to browse to it, select "No" in the prompt window, and "No" again
if it asks whether you want to try the connection again in the
future.

7) Batch compile ClientRes, ClientShellDLL, Object, and ServerRes.

8) Take the compiled files from the 'proj\avp2\avp2\' directory
and put them in your AVP2 game folder.

9) Run the game.




III.    DIRECTORY STRUCTURE
__________________________________

The AVP2 source code extracts into the following directory
structure:

c:\proj           (Directory you extracted to)

    AVP2              (Root game code directory)

        AVP2Serv          (Dedicated server exe)
        ClientRes         (Client string resource dll)
        ClientShellDLL    (Client dll)
        ObjectDLL         (Server dll)
        ServerRes         (Server string resource dll)
        Shared            (Source code used by both the client and
                          the server)

    LT2               (Root engine code directory)

        Lithshared        (Header files for shared Monolith
                          libraries)
        Sdk               (LithTech sdk header files)



IV.     AVP2 VISUAL C++ WORKSPACE
__________________________________

The AVP2 Visual C++ Workspace (AVP2.dsw) contains the AVP2Serv,
ClientRes, ClientShellDLL, ObjectDLL and ServerRes projects (and
shared files in the Shared directory). These five projects compile
into avp2serv.exe, cres.dll, cshell.dll, object.lto and sres.dll,
respectively.

Once compiled, new versions of cres.dll, cshell.dll, object.lto and
sres.dll should be placed in the root of your AVP2 resource
directory to be used by the game (the resource directory is called
"AVP2" so if you had installed AVP2 to the default "c:\Program
Files\Fox\Aliens vs. Predator 2" then you would put cres.dll,
cshell.dll, object.lto and sres.dll in a folder called "c:\Program
Files\Fox\Aliens vs. Predator 2\AVP2"). It is recommended that
before doing this you use "Lithrez.exe" from the AVP2 Tools
(downloadable from http://avp2.sierra.com) to unrez your game's
AVP2DLL.REZ and AVP2L.REZ files. See the documentation included
with the AVP2 Tools for instructions on using Lithrez.

New versions of avp2serv.exe should go in your root AVP2 game
folder (ie, "c:\Program Files\Fox\Aliens vs. Predator 2\"
if you installed the game to the default directory).

The ClientShellDLL and Object projects contain Final and Final
Debug build targets. These build targets MUST be used when building
the game for use with .rez files. However, you can use the normal
Debug and Release builds if you are running AVP2 from a normal
directory structure (i.e., you have unrezzed AVP2DLL.rez,
AVP2L.rez, etc. and are running the game from this directory).



IV.     END-USER LICENSE AGREEMENT FOR ADD-ON COMPONENTS -- ALIENS
VERSUS PREDATOR 2 Source Code v1.0.9.6
__________________________________

THIS END-USER LICENSE AGREEMENT (THIS "EULA") IS A LEGAL AGREEMENT
BETWEEN YOU (EITHER AN INDIVIDUAL OR A SINGLE ENTITY) AND MONOLITH
PRODUCTIONS, INC. ("MONOLITH") FOR ALIENS VERSUS PREDATOR 2 Source
Code v1.0.9.6, WHICH INCLUDES COMPUTER SOFTWARE AND ASSOCIATED
MATERIALS (THE "SOFTWARE").

YOU MAY ONLY BECOME A PARTY TO THIS AGREEMENT IF YOU ARE A LICENSED
USER OF MONOLITH'S "Aliens Versus Predator 2" PRODUCT.

BY CLICKING ON THE "YES" BUTTON, YOU ARE CONSENTING TO BE BOUND BY
AND ARE BECOMING A PARTY TO THIS AGREEMENT. IF YOU DO NOT AGREE TO
ALL OF THE TERMS OF THIS AGREEMENT,  CLICK THE "NO" BUTTON AND DO
NOT USE THE SOFTWARE.

1. GRANT OF LICENSE. You may use and publicly display one copy of
the Software on one personal computer. A license for the Software
may not be shared or used concurrently on different computers.

2. LIMITATIONS. You may not reverse engineer, decompile, or
disassemble the Software. You may not rent or lease the Software.
You may make one additional copy of the Software solely for backup
or archival purposes. You may not copy any printed materials
accompanying the Software.

3. SOFTWARE TRANSFER. You may permanently transfer all of your
rights under this EULA, provided (a) you retain no copies, (b) you
transfer all of the Software (including all component parts, the
media on which the Software was delivered, all printed materials,
any upgrades, and this EULA), and (c) the recipient agrees to the
terms of this EULA in writing delivered to Monolith. Any transfer
must include all prior versions and upgrades of the Software.
Otherwise, you have no right to distribute copies of the Software.

4. TERMINATION. Without prejudice to any other rights, Monolith may
terminate this EULA if you fail to comply with the terms and
conditions of this EULA. In such event, you must destroy all copies
of the Software and all of its component parts.

5. NO SUPPORT. Because Monolith is offering this Software to you
free of charge, Monolith will not provide any telephone, online or
other support for the use of the Software.

6. NO WARRANTIES. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
LAW, MONOLITH AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER
EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
WITH REGARD TO THE SOFTWARE.

7. NO LIABILITY FOR DAMAGES. TO THE MAXIMUM EXTENT PERMITTED BY
APPLICABLE LAW, IN NO EVENT SHALL MONOLITH OR ITS SUPPLIERS BE
LIABLE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,
DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS
OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT
OF THE USE OF OR INABILITY TO USE THE SOFTWARE PRODUCT, EVEN IF
MONOLITH HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
BECAUSE SOME STATES DO NOT ALLOW SUCH EXCLUSION OR LIMITATION OF
LIABILITY, THE ABOVE LIMITATION MAY NOT APPLY TO YOU.

8. EDITOR, SOURCE CODE AND END-USER VARIATIONS.

(a)  The Software may include an "Editor."  An "Editor" is a
feature which allows you to modify the Software or to construct new
variations for use with it. These modifications and variations can
be both playable and non-playable. An Editor includes its
associated tools and utilities. An Editor is NOT shareware. You may
not freely distribute it to any BBS, CD, floppy or any other media.
You may not sell it or repackage it for sale.

(b)  The Software may include "Source Code."  "Source Code" is a
feature which allows you to modify the Software or to construct new
variations for use with it. These modifications and variations can
be both playable and non-playable. Source Code is NOT public
domain. You may not freely distribute it to any BBS, CD, floppy or
any other media. You may not sell it or repackage it for sale.

(c) Using the Editor and/or Source Code, you may create
modifications or enhancements to the Software, including the
construction of new levels (collectively referred to as
"Variations"), subject to the following restrictions:

i. Your Variations must only work with the full copy of the
Software, not independently or with any other software.

ii. Your Variations must not contain modifications to any
executable file.

iii. Your Variations must not contain any libelous, defamatory, or
other illegal material, material that is scandalous or invades the
rights of privacy or publicity of any third party, or contains any
trademarks, copyright-protected work, or other recognizable
property of third parties.

iv. At least once in every online description and with reasonable
duration on the opening screen, your Variations must prominently
identify (i) the names and email addresses of its creators, and
(ii) the words "This add-on is not made by or supported by Monolith
Productions, or any of its affiliates and subsidiaries."

v. Your Variations must be distributed solely for free. Neither you
nor any other person or party may sell them to anyone, commercially
exploit them in any way, or charge anyone for using them. You may
exchange them at no charge among other end-users.

vi. By distributing or permitting the distribution of any of your
Variations, you hereby grant back to Monolith Productions an
irrevocable royalty-free right to use and distribute them by any
means.

vii. The prohibitions and restrictions in this section apply to
anyone in possession of the Software or any of your Variations.

9. MISCELLANEOUS. This EULA is governed by the laws of the State of
Washington, U.S.A. Each of the parties hereto submits to
jurisdiction in the state and federal courts sitting in King
County, Washington. Should you have any questions concerning this
EULA, or if you desire to contact Monolith for any reason, please
write:  Monolith Productions, Inc., 10516 NE 37th Circle, Kirkland,
WA 98033.