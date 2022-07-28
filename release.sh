#!/bin/bash -e

VERSION=$1

if [[ -n "$(git diff)" ]]; then
    echo "ERROR: uncommited files"
    exit 1
fi

if [[ -n "$(git diff --cached)" ]]; then
    echo "ERROR: uncommited files"
    exit 1
fi

if [[ -z "${VERSION}" ]]; then
    echo "ERROR: no version to tag given"
    exit 1
fi

if [[ -n "$(git tag | grep "${VERSION}" || true)" ]]; then
    echo "ERROR: tag name already used"
    exit 1
fi

OLD_VERSION=$(git describe --tags --abbrev=0)
sed -i "s/\"version\":.*/\"version\": \"${VERSION}\",/" package.json
sed -i "s/\"version\":.*/\"version\": \"${VERSION}\",/" package-lock.json
sed -i "s/${OLD_VERSION}/${VERSION}" wled00/*.h wled00/*.cpp
#git add -u -m "Updated version in package.json to ${VERSION}"
#git tag -a "${VERSION}" -m "Created tag ${VERSION}"

echo "INFO: If everything looks good push the release with the following command, so the firmware will be build"
echo "CMD: git push -u origin main --tags"
