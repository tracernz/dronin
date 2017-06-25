source "${APPVEYOR_BUILD_FOLDER}\make\winx86\bash_profile" 
cd "${APPVEYOR_BUILD_FOLDER}"

export IGNORE_MISSING_TOOLCHAIN=yes

make arm_sdk_install
make openssl_install
make zip_install
make breakpad_install
make -j4 gcs GCS_BUILD_CONF=release
