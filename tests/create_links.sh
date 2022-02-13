#!/bin/bash

R=(
tests/DynResDLL/libtestCRes.so
NOLF2/ClientRes/TO2/libCRes.so
)
S=(
tests/CShell/libCShell.so
NOLF2/ClientShellDLL/TO2/libCShell.so
)
ASSETS=${3:-../../game/NOLF2}
E=(
    ../sdk/rez/Engine.REZ
    ${ASSETS}/data/Resources/Engine.REZ
)
RES=$(for f in ${R[@]}; do [[ "${f}" =~ ${1:-test} ]] && echo $f; done)
SHELL=$(for f in ${S[@]}; do [[ "${f}" =~ ${2:-test} ]] && echo $f; done)
OBJ=NOLF2/ObjectDLL/TO2/libObject.lto
FX=NOLF2/ClientFxDLL/libClientFx.fxd
SRV_DIR=libs/ServerDir/libServerDir.so
REN=runtime/render_a/src/sys/ogl/libOGLRender.so
ENGINE=$([ -f ${E[1]} ] && echo ${E[1]} || echo ${E[0]})

links=(
    ${ASSETS}/GAME.REZ:game.rez
    ${ASSETS}/data/Resources/GAME2.REZ:game2.rez
    ${ASSETS}/data/Resources/SOUND.REZ:sound.rez
    ${ENGINE}:engine.rez
    ${REN}:libOGLRender.so
    ${RES}:libCRes.so
    ${SHELL}:libCShell.so
    ${SRV_DIR}:libServerDir.so
    ../${OBJ}:Game/libObject.lto
    ../${FX}:Game/libClientFx.fxd
)

mkdir -pv Game

for l in ${links[@]}; do
  src=${l//:*/}
  dest=${l//*:/}
  ln -sfT $src $dest
  ls -l --color=auto $dest
done

if [ -d ${ASSETS}/data/Program_Files/Profiles ]; then
  rsync -av ${ASSETS}/data/Program_Files/Profiles ./
fi
