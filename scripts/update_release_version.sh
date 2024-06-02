#!/bin/bash

sed 's/#define VERSION.*/#define VERSION '"$(date +"%y%m%d0")"/ wled00/wled.h > wled00/wled-v.h
mv wled00/wled-v.h wled00/wled.h

if [[ -z "${GITHUB_REF}" ]]; then 
	echo "No GITHUB_REF specified"
	exit 0
fi

if [[ -z "$(echo ${GITHUB_REF} | grep tag)" ]]; then 
	echo "Not a tag"
	exit 0
fi

TAG=$(echo $GITHUB_REF | cut -d / -f 3)
if [[ -z "${TAG}" ]]; then
	echo "No tag value"
	exit 0
fi

if [[ ${TAG} == v* ]]; then
	# strip the version
	TAG="${TAG:1}"
fi

echo "Updating version to ${TAG}"
sed 's/"version": "0.14.4"/"version": "'"${TAG}"'"/' package.json > package-v.json
mv package-v.json package.json
npm i --package-lock-only
