include(gcs.pri)

TEMPLATE = subdirs
# Copy Qt runtime libraries into the build directory (to run or package)
equals(copydata, 1) {
    # copy OpenSSL DLLs
    win32 {
        THIRDPARTY_PATH = $$GCS_SOURCE_TREE/../../tools
        OPENSSL_DIR = $$THIRDPARTY_PATH/win32openssl

        OPENSSL_DLLS = \
            ssleay32.dll \
            libeay32.dll

        for(dll, OPENSSL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$OPENSSL_DIR/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }
        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }

    # Copy KML libraries
    KML {
        THIRDPARTY_PATH = $$GCS_SOURCE_TREE/../../tools
        linux-g++* {
            # Copy extra binary library files
            EXTRA_BINFILES += \
                $${THIRDPARTY_PATH}/libkml/lib/libkmlbase.so.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libkmlbase.so.0.0.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libkmldom.so.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libkmldom.so.0.0.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libkmlengine.so.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libkmlengine.so.0.0.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libminizip.so.0 \
                $${THIRDPARTY_PATH}/libkml/lib/libminizip.so.0.0.0 \
                $${THIRDPARTY_PATH}/libkml/lib/liburiparser.so.1 \
                $${THIRDPARTY_PATH}/libkml/lib/liburiparser.so.1.0.5
        }

        for(FILE,EXTRA_BINFILES){
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$FILE\") $$targetPath(\"$$GCS_LIBRARY_PATH\") $$addNewline()
        }
        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }
}
