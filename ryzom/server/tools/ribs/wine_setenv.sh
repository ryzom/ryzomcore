#!/bin/bash

COMPILER=$1
COMPILERVERSION=$2
ARCH=$3

cd $BUILD_PATH/wine/

p_info "Using VisualStudio 2019 compilers for $ARCH"

function dirdos() {
	for path in $*
	do
		echo -n "Z:$1" | sed 's/\//\\/g'
	done
}

export BASEDIR=$(pwd)
export WINDEBUG=-all
export WINEBIN=wine
export TOOLSDIR=$BASEDIR/tools
export TOOLSBINDIR=$TOOLSDIR/bin
export VCDIR=$BASEDIR/MSVC/$COMPILERVERSION
export VCBINDIR=$VCDIR/bin/Host$ARCH/$ARCH
export WINSDKDIR=$BASEDIR/winsdk
export DXSDK=$BASEDIR/dxsdk
if [ $ARCH = "x64" ]
then
	export WINSDKBINDIR=$WINSDKDIR/bin
else
	export WINSDKBINDIR=$WINSDKDIR/bin32
fi

# add wine to path
export CURRENT_CPU=amd64
export Configuration=Release
export TARGET_PLATFORM=WIN7
export APPVER=6.1
export DEBUGMSG=Release
export INCLUDE=$(dirdos "$WINSDKDIR/include/")
export INCLUDE="$INCLUDE;$(dirdos "$VCDIR/include/")"
if [ $ARCH = "x64" ]
then
	export LIB=$(dirdos "$WINSDKDIR/lib/")
else
	export LIB=$(dirdos "$WINSDKDIR/lib32/")
fi
export LIB="$LIB;$(dirdos "$VCDIR/lib/$ARCH/")"

export PATH=$PATH:$TOOLSBINDIR:$WINSDKBINDIR:$VCBINDIR
export WINEPATH="$(dirdos "$TOOLSBINDIR")"
export WINEPATH="$WINEPATH;$(dirdos "$WINSDKBINDIR")"
export WINEPATH="$WINEPATH;$(dirdos "$VCBINDIR")"
#-- Found OpenSSL: /home/nevrax/builds/wine/vc142/external/openssl/lib/libcrypto.lib (found version "1.1.1")

declare -A FLAGS
#FLAGS["openssl"]="-DOPENSSL_INCLUDE_DIR=%/include"
#FLAGS["AL"]="-DOPENAL_INCLUDE_DIR=%/include"
#FLAGS["fmod64"]="-DFMOD_INCLUDE_DIR=%/include"
FLAGS["vorbis"]="-DVORBIS_INCLUDE_DIR=%/include -DVORBISFILE_INCLUDE_DIR=%/include"
FLAGS["qt5"]="-DQTDIR=%"
CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_VS_MSBUILD_COMMAND=$VCDIR -DDXSDK_DIR=$DXSDK"

if [ "$TYPE" == "steam" ]
then
	STEAM_DIR="$(dirdos "$BASEDIR/$COMPILER/external/steam/lib/")"
	INCLUDE="$INCLUDE;$(dirdos "$BASEDIR/$COMPILER/external/steam/include/")"
	LIB="$LIB;$(dirdos "$BASEDIR/$COMPILER/external/steam/lib/")"
fi

export LIBPATH=$LIB

# launch WINE for the first time
$WINEBIN ping

# Remove all WINE warnings
export WINEDEBUG=-all

# Setting misc environment variables
#export MT_OPTIONS="-log /tmp/mt/log -verbose"
export DXSDK_DIR=Z:$BASEDIR/dxsdk
export VC_DIR=Z:$VCDIR

