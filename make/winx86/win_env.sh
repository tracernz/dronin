# bashrc for dRonin development on Windows
# Michael Lyle - 2015-2016
# Michael Corcoran - 2017

DRONIN_ROOT=$1
if [ -z "$DRONIN_ROOT" ]; then
  echo "WARNING: first arg should be dronin root dir, assuming ./"
  DRONIN_ROOT=.
fi

QT_VERSION_FULL=5.8.0
QT_VERSION=`echo "$QT_VERSION_FULL" | cut -d '.' -f1-2`

# # This is really unfortunate on the part of Qt.
# QT_MINGWVER="mingw53_32"
QT_MINGWVERB="mingw530_32"

QT_BASEDIR="${DRONIN_ROOT}/tools/Qt$QT_VERSION_FULL"

unix_path_from_registry()
{
  KEY=$1
  cygpath "`cat "/proc/registry32/${KEY}"`"
}
win_path_from_registry()
{
  KEY=$1
  cat "/proc/registry32/${KEY}"
}

if [ "$DRONIN_ENV" == "msvc" ] ; then
  QT_MSVCVER="msvc2015"
  VS_PATH=$(unix_path_from_registry HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/14.0/ShellFolder)
  VS_INCLUDE_PATH=$(win_path_from_registry HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/14.0/ShellFolder)
  WK_PATH=$(unix_path_from_registry "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot10")
  WK_INCLUDE_PATH=$(win_path_from_registry "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot10")
  WK81_INCLUDE_PATH=$(win_path_from_registry "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot81")
  WK_CRT_INCLUDE_PATH="$WK_INCLUDE_PATH/include/10.0.10240.0/ucrt"
  WK_CRT_LIB_PATH="$WK_INCLUDE_PATH/lib/10.0.10240.0/ucrt"
fi

shopt -s expand_aliases

test_and_add_to_path ()
{
  if which $2 >/dev/null 2>&1 ; then
    return
  fi

  if [ -f "$1/$2" ] ; then
    export PATH=$PATH\:$1
  fi
}

# Whether or not mingw build, we need mingw32-make from here
test_and_add_to_path "$QT_BASEDIR/Tools/$QT_MINGWVERB/bin" "mingw32-make.exe"

# TODO: don't think this is necessary for mingw?
if [ "$DRONIN_ENV" == "mingw" ] ; then
  test_and_add_to_path "$QT_BASEDIR/Tools/$QT_MINGWVERB/bin" "gcc.exe"
  test_and_add_to_path "$QT_BASEDIR/$QT_VERSION/$QT_MINGWVER/bin" "qmltestrunner.exe"
fi
if [ "$DRONIN_ENV" == "msvc" ] ; then
  if [ $(uname -m)="x86_64" ]; then
#   VS_PLATFORM="amd64"
#   WK_PLATFORM="x64"
#   VC_PLATFORM="/amd64"
    VS_PLATFORM="amd64_x86"
    WK_PLATFORM="x86"
    VC_PLATFORM=""
  else
    VS_PLATFORM="x86"
    WK_PLATFORM="x86"
    VC_PLATFORM=""
  fi

  test_and_add_to_path "$VS_PATH/VC/bin/amd64" mspdb140.dll
  test_and_add_to_path "$VS_PATH/VC/VCPackages" "VCProject.dll"

  export PATH="$QT_BASEDIR/$QT_VERSION/$QT_MSVCVER/bin"\:$PATH
  export PATH="$VS_PATH/VC/BIN/$VS_PLATFORM"\:$PATH
  export PATH="$VS_PATH/VC/include"\:$PATH

  test_and_add_to_path "$QT_BASEDIR/Tools/QtCreator/bin" "jom.exe"
  test_and_add_to_path "$WK_PATH/bin/$WK_PLATFORM" "rc.exe"

  export INCLUDE="$VS_INCLUDE_PATH/VC/INCLUDE;$VS_INCLUDE_PATH/VC/ATLMFC/INCLUDE;$WK_CRT_INCLUDE_PATH;$WK_INCLUDE_PATH/include/shared;$WK_INCLUDE_PATH/include/um;$WK_INCLUDE_PATH/include/winrt"

  export LIB="$VS_INCLUDE_PATH/VC/LIB;$VS_INCLUDE_PATH/VC/ATLMFC/LIB;$WK_CRT_LIB_PATH/$WK_PLATFORM;$WK81_INCLUDE_PATH/lib/winv6.3/um/$WK_PLATFORM;"

  export MACHINE_TYPE=$(uname -m)
  export VCINSTALLDIR="$VS_INCLUDE_PATH"
  export VCREDIST="$VCINSTALLDIR/VC/redist/$WK_PLATFORM/Microsoft.VC140.CRT"
fi

# Anaconda adds itself to path, so it's not required here
test_and_add_to_path "/C/Python27" "python.exe"
# test_and_add_to_path "/C/OpenOCD/0.4.0/bin/" "openocd.exe"
test_and_add_to_path "/C/Program Files/NSIS/Unicode" "makensis.exe"
test_and_add_to_path "/C/Program Files (x86)/NSIS/Unicode" "makensis.exe"

if ! which make >/dev/null 2>&1 ; then
  alias make=mingw32-make
fi

