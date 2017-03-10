# bashrc for dRonin development on Windows
# Michael Lyle - 2015-2016
# Michael Corcoran - 2017

DRONIN_ROOT=$1
if [ -z "$DRONIN_ROOT" ]; then
  echo "WARNING: first arg should be dronin root dir, assuming ./"
  DRONIN_ROOT=.
fi

if [ -z "$DRONIN_ENV" ]; then
  echo "WARNING: DRONIN_ENV not set... Defaulting to msvc, options are msvc, mingw"
  export DRONIN_ENV=msvc
fi

# vcvars*.bat just finds the newest kit by default, let's take a different approach for predictability
UCRT_VERSION="10.0.14393.0"
TARGET_PLATFORM="x86" # x86 or x64

QT_VERSION_FULL=5.8.0
QT_VERSION=`echo "$QT_VERSION_FULL" | cut -d '.' -f1-2`
# # This is really unfortunate on the part of Qt.
QT_MINGWVER="mingw53_32"
QT_MINGWVERB="mingw530_32"
QT_BASEDIR="${DRONIN_ROOT}/tools/Qt$QT_VERSION_FULL"


#--------------------------------------------

shopt -s expand_aliases

get_reg_key()
{
  cat "/proc/registry$1/$2"
}

add_win_path_to_path()
{
  UNIX_PATH=$(cygpath "$1")
  if [ ! -d "${UNIX_PATH}" ]; then
    echo "ERROR: MSVC2015 doesn't appear to be installed?"
  fi
  export PATH="${UNIX_PATH}:${PATH}"
}

test_and_add_to_path ()
{
  if which $2 >/dev/null 2>&1 ; then
    return
  fi

  if [ -f "$1/$2" ] ; then
    export PATH=$PATH\:$1
  fi
}


if [ "${DRONIN_ENV}" == "msvc" ] ; then
  if [ "${TARGET_PLATFORM}" == "x64" ]; then
    BITS=64
    QT_MSVCVER="msvc2015_64"
    AMD64_OR_BUST="\\amd64"
  else
    BITS=32
    QT_MSVCVER="msvc2015"
    AMD64_OR_BUST=""
  fi

  export VisualStudioVersion="14.0"
  export VSINSTALLDIR=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VS7/14.0")
  export VCINSTALLDIR=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/14.0")
  export UniversalCRTSdkDir=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows Kits/Installed Roots/KitsRoot10")
  export FrameworkDir=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/FrameworkDir${BITS}")
  export FrameworkVersion=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/FrameworkVer${BITS}")
  export WindowsSdkDir=$(get_reg_key 32 "HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v10.0/InstallationFolder")

  # this is used by the packing scripts
  export VCREDIST="${VCINSTALLDIR}/VC/redist/${TARGET_PLATFORM}/Microsoft.VC140.CRT"

  export UCRTVersion="${UCRT_VERSION}"
  export WindowsSDKVersion="${UCRT_VERSION}"
  export WindowsSDKLibVersion="${UCRT_VERSION}"

  add_win_path_to_path "${WindowsSdkDir}bin\\x86"

  if [ $(uname -m) == "x86_64" ]; then
    add_win_path_to_path "${WindowsSdkDir}bin\\x64"
    add_win_path_to_path "${VCINSTALLDIR}\\BIN\\amd64"
    if [ "${TARGET_PLATFORM}" == "x86" ]; then
      add_win_path_to_path "${VCINSTALLDIR}\\BIN\\amd64_x86"
    fi
  else
    add_win_path_to_path "${VCINSTALLDIR}\\BIN"
  fi
  
  export INCLUDE="${VCINSTALLDIR}INCLUDE;${VCINSTALLDIR}ATLMFC\\INCLUDE;${UniversalCRTSdkDir}include\\${UCRTVersion}\\ucrt;${WindowsSdkDir}include\\${WindowsSDKVersion}\\shared;${WindowsSdkDir}include\\${WindowsSDKVersion}\\um"
  export LIB="${VCINSTALLDIR}LIB;${VCINSTALLDIR}ATLMFC\\LIB${AMD64_OR_BUST};${UniversalCRTSdkDir}lib\\${UCRTVersion}\\ucrt\\${TARGET_PLATFORM};${WindowsSdkDir}lib\\${WindowsSDKLibVersion}\\um\\${TARGET_PLATFORM}" #;${WindowsSdkDir}UnionMetadata;${WindowsSdkDir}References"
  export LIBPATH="${VCINSTALLDIR}LIB;${VCINSTALLDIR}ATLMFC\\LIB${AMD64_OR_BUST}"

  export PATH="$QT_BASEDIR/$QT_VERSION/$QT_MSVCVER/bin"\:$PATH
  test_and_add_to_path "$QT_BASEDIR/Tools/QtCreator/bin" "jom.exe"

  echo "Configured for MSVC2015 ${TARGET_PLATFORM}"

elif [ "$DRONIN_ENV" == "mingw" ] ; then
  test_and_add_to_path "$QT_BASEDIR/Tools/$QT_MINGWVERB/bin" "gcc.exe"
  test_and_add_to_path "$QT_BASEDIR/$QT_VERSION/$QT_MINGWVER/bin" "qmltestrunner.exe"

  echo "Configured for MinGW x86"
fi

# Whether or not mingw build, we need mingw32-make from here
test_and_add_to_path "$QT_BASEDIR/Tools/$QT_MINGWVERB/bin" "mingw32-make.exe"

# Anaconda adds itself to path, so it's not required here
test_and_add_to_path "/C/Python27" "python.exe"
test_and_add_to_path "/C/Program Files/NSIS/Unicode" "makensis.exe"
test_and_add_to_path "/C/Program Files (x86)/NSIS/Unicode" "makensis.exe"

if ! which make >/dev/null 2>&1 ; then
  alias make=mingw32-make
fi

# if we see this in user's logs we know the bash script made it all the way through
echo "Run 'make' with no arguments/targets if unsure what you want to build"
