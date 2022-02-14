#!/bin/bash
Dest=$1
Build=${2:-.}

mkdir -pv ${Dest}/Game

cp -v ${Build}/OUT/Lithtech ${Build}/NOLF2/Client{Res/TO2/libCRes,ShellDLL/TO2/libCShell}.so ${Build}/libs/ServerDir/libServerDir.so ${Dest}/
cp -v ${Build}/NOLF2/{ClientFxDLL/libClientFx.fxd,ObjectDLL/TO2/libObject.lto} ${Dest}/Game/

ln -vs Game/libObject.lto ${Dest}/
ln -vs Game/libClientFx.fxd ${Dest}/

echo "Please copy/link all *.rez files to ${Dest} along with your 'Profiles' folder, autoexec.cfg and display.cfg"
