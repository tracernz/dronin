source "%APPVEYOR_BUILD_FOLDER%\make\winx86\bash_profile" 
cd "%APPVEYOR_BUILD_FOLDER%"

export IGNORE_MISSING_TOOLCHAIN=yes

make uavobjects
