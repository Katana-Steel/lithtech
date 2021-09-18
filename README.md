Travis[![Build Status](https://travis-ci.com/Katana-Steel/lithtech.svg?branch=linux-x86_64)](https://travis-ci.org/Katana-Steel/lithtech)
GL[![pipeline status](https://gitlab.com/Katana-Steel/lithtech/badges/linux-x86_64/pipeline.svg)](https://gitlab.com/Katana-Steel/lithtech/pipelines)

lithtech
========

The engine code is from the apparent GPL release of the Jupiter Ex engine before Touchdown Entertainment disappeared. The GPL release was of the PC Enterprise Edition Build 69. Included were the game code and assets from NOLF2 and the game code from Tron 2.0. The code from the other games are from their respective public releases.

Goal
----
I found the GPL release of the Jupiter Engine and couldn't find anyone else who had really seemed to do anything with it. I wanted to see how hard it would be to adapt it into a modern, multi-platform source port. I also wanted to see if it would be possible to port the other Lithtech release game code to the GPL release of the engine. Both have turned out to be much more work than I originally envisioned. Even though the effort is initially *extremely* rough, I thought I would put it up on GitHub so that I have a backup of my work and that no matter how far (or not) I get, my work will not disappear with me.

Status
------

Currently the engine and NOLF2 game code will build with CMake to generate the project.

Windows screenshot:

![nolf2 windows](https://cdn.discordapp.com/attachments/270406768750886912/642060349767680012/nolf2.jpg)


![debug nolf2 windows](https://cdn.discordapp.com/attachments/270406768750886912/642412574725636167/nolf2_vs.png)

Linux screenshot:

![NOLF2 using dxvk-native](https://i.imgur.com/b1hV8Fb.png)
![LT on Linux](https://imgur.com/LOpCNfa.png)

Requirements
------------

- cmake
- gcc or clang (Linux)
- ms vs2017 (windows only)
- SDL2
- SDL2 image
- SDL2 mixer
- dx9 sdk (windows only)
- dxvk-native (linux only)
- meson (linux for dxvk)
- ninja (optional)



building [dxvk-native](https://github.com/Joshua-Ashton/dxvk-native)
--------------------------------------------------------------------

```bash
#this will build and install libraries and headers under /usr/local
git clone https://github.com/Joshua-Ashton/dxvk-native
cd dxvk-native
TOP=`pwd`
meson --buildtype "release" --prefix /usr/local/ -Dbuild_id=false build
cd build
sudo ninja install
LIB_DIR=$(meson introspect --installed | python3 -c 'import sys,json; o = list(json.loads(sys.stdin.read()).values()); print("/".join(o[0].split("/")[:-1]))')
#install headers
cd $TOP;
INC_DIR=/usr/local/include/dxvk-native
sudo mkdir -p ${INC_DIR}
sudo cp -rv include/* ${INC_DIR}/
sed -i -e "/includedir=/ s%.+%includedir=${INC_DIR}%" -e "/libdir=/ s%.+%libdir=${LIB_DIR}%" /usr/share/pkgconfig/dxvk-native.pc
```

Quick how to get building
-------------------------

```bash
mkdir build
cd build
cmake -DJSON_BuildTests=off -G 'Ninja' ..
cmake --build .
```
Ninja above can be replaced with what makes sense on your platform. 

TODO
----
* Fix the secure standard library exceptions so that a Debug build under VS2010 will run.
* Port/adapt all the game code to run on the GPL engine. Each game was written to a different version of the engine. So, theoretically this is possible, but it could take a substantial amount of work.
* Remove the Windows dependencies. The engine is heavily oriented towards Windows (even though there is a bit of Linux code).
    * Replace the usage of MFC with wxWidgets in the tools. (MFC and wxWidgets are close enough that it shouldn't be too painful.)
    * Replace DirectInput and other Windows specific code with SDL.
    * Create a new audio driver based on SDL and/or OpenAL.
    * ~~Create a new OpenGL or Vulkan renderer.~~
* Long term, it might be cool to bring back something similar to the renderer DLLs that Shogo used. In this version of the engine, the renderer is statically linked to the engine.

Games I want to support and the order to work on them
-----------------------------------------------------
1. No One Lives Forever 2: A Spy in H.A.R.M.'s Way
2. Tron 2.0 (if I can find a copy)
3. Shogo
4. The Operative: No One Lives Forever
5. F.E.A.R.
6. Blood II: The Chosen (if I can find a copy)

These are all the Lithtech I am aware of that had a public source release. If there are more out there, I want to eventually support them too.

Recent Developments
===================
Thanks to Rohit Nirmal for his time in enabling DXVK-native and getting NOLF2 working with
the retail assets.

The travis-ci builds have become stagnant however I still maintain a mirror on gitlab
where the pipelines now build and run tests against them in 2 stages, meaning the 
produced binaries could be downloaded from a successful run.

I've been setting up testing, build env in docker and cleaning up the code

- Katana Steel

After trying things out, I've managed to setup an automated build test with Travis-CI
from github, and I keep a mirror on gitlab which also automatically builds Lithtech
engine and and supporting libraries.

On GitLab two builds are happening 1 for gcc and 1 for clang, to get a diversity in
build tools used. Each are done with a docker build env which has a stable cmake version (currently 3.17.5),
and up to date GCC and Clang, at the time of writing.

- Katana-Steel

Builds on Windows
-----------------

There are a few reasons why I am implementing a new build system:
* All code releases were either using VC++ 6 or Visual Studio 2003 (VC++ 7.1). Neither type of project file converted that well in VS2010.
* Since the ultimate goal is a source port, I would like to be able to build the games on Linux and OS X as well. There was a homegrown converter for VC++ 7.1 project files to makefiles, but given then problems converting between versions of Visual Studio, I decided against it.
* I don't want to require someone to use Visual Studio on Windows. It should at least be buildable by MinGW as well (if not OpenWatcom too).

I can get NOLF2 running. Interestingly enough, a release build seems to run okay, but a debug build will trigger exceptions in the standard library. All the problem seem to have to do with Microsoft's secure CRT. Either things had slightly different semantics between compilers or certain iterator usages were technically not correct in the original code. I fixed maybe two of these. There are still probably another handful left. There also seems to be some differences in the NOLF2 retail data files and those supplied with the GPL release. I am not 100% certain of this, but I need to investigate more.

I also started an effort to port the Shogo source (originally from a really old version of the engine) to the engine in the GPL release. I have made a *ton* of changes, but it still doesn't compile completely. I decided to focus on getting a single tree build and debug version of NOLF2 running first.


Builds on linux
---------------
Since Linux doesn't have a global registry systems, the RegMgr was reimplemented with a JSON file as the backend.

The CRes.dll or Client Resources was replaced by libDynRes which currently only supports compiling strings(stringtable) from a Windows style rc file.
The RC file is parsed by a python script which generates a single source C++ file that inturn is compiled into a dynamic library.

and generally stubbed out every function/class needed to compile the engine.

DXVK has become a viable DirectX 9 implementation with dxvk-native enabling that will now build LIB_D3DRender.a static lib.

Past work
=========
Hai, MWisBest here.

It turns out that the NOLF2 assets included with the "GPL" release are indeed different than the retail NOLF2 assets. This code is technically working OK in its current state, but only with those "GPL" assets; the problem is that the "GPL" assets aren't complete. They really only include the first part of the first level (world) of the game for the campaign. The old (retail) versions of the WORLD files do not work directly with this newer Lithtech engine build.

There's a few options to maybe get the entirety of NOLF2 working with this code:

1. Figure out the difference between formats and rework the engine to support both. (very unlikely)
2. Locate an old version of Maya or 3ds Max (version 3 or 4 of either) to use the retail game's import plugin for, and then this release's export plugin, to "convert" them to the new format. (haven't been able to find an old Maya or 3ds Max so far)
3. Recreate the levels from scratch. Really give the entire game a proper remake. (beyond *my* capabilities)



