#!/bin/bash

# based on original script by Jeroen de Bruijn

set -e

git clone --depth=1 -b gh-pages "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" code_docs > /dev/null 2>&1
if [ $? -ne 0 ]; then
     echo 'Error: git clone failed (output hidden to protect credentials)' >&2
     exit 1
fi

cd code_docs

git config --global push.default simple
git config user.name "Travis CI"
git config user.email "travis@travis-ci.org"

function build_dox {
    DOX=$1

    if [ -n "${DOX}" ]; then
        echo "Building ${DOX} Doxygen documentation..."

        rm -rf ./${DOX}
        mkdir ./${DOX}

        DOXYFILE=${TRAVIS_BUILD_DIR}/Doxygen/${DOX}_doxygen.cfg

        sed -i s#[=\ \t]\\\./#${TRAVIS_BUILD_DIR}/# ${DOXYFILE}
        sed -i s#OUTPUT_DIRECTORY[\t\ =].*#OUTPUT_DIRECTORY=${DOX}# ${DOXYFILE}

        doxygen ${DOXYFILE} 2>&1 | tee ./${DOX}/doxygen.log
    else
        echo "Error: no arg to build_dox" >&2
        exit 1
    fi
}

build_dox ground
build_dox flight

if [ -n "${GH_REPO_TOKEN}" ] && \
   [ -d "./ground/html" ] && [ -d "./flight/html" ] && \
   [ -f "./ground/html/index.html" ] && [ -f "./flight/html/index.html" ]; then

    echo 'Uploading documentation to the gh-pages branch...'
    git add --all
    git commit -m "Auto-generated Doxygen docs from travis-ci build: ${TRAVIS_BUILD_NUMBER}" -m "Commit: ${TRAVIS_COMMIT}"
    git push origin gh-pages > /dev/null 2>&1
    if [ $? -ne 0 ]; then
         echo 'Error: git push failed (output hidden to protect credentials)' >&2
         exit 1
    fi
else
    echo '' >&2
    echo 'Warning: No documentation (html) files have been found, or github credentials missing!' >&2
    echo 'Warning: Not going to push the documentation to GitHub!' >&2
    exit 1
fi
