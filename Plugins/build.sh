#!/bin/bash
CURDIR=`pwd`
TAG=$1
if [ -z "$TAG" ]; then
  echo "No tag specified. Commit and push a tag first then reference it here."
  echo "Usage: $0 <tag>"
  exit 1
fi
HASH=`git rev-parse $TAG`
if [ -z "$HASH" ]; then
  echo "Tag not found: $TAG"
  exit 1
fi
if [ -d /tmp/AssetsBridgeBuilder ]; then
    rm -rf /tmp/AssetsBridgeBuilder/
fi
mkdir -p /tmp/AssetsBridgeBuilder/AssetsBridge
rsync -av --exclude=Binaries/ --exclude=Intermediate/ ./AssetsBridge/ /tmp/AssetsBridgeBuilder/AssetsBridge/
cd /tmp/AssetsBridgeBuilder/ || exit
zip -r AssetsBridge.zip AssetsBridge
cp -f /tmp/AssetsBridgeBuilder/AssetsBridge.zip "$CURDIR/AssetsBridge.zip"
cd "$CURDIR" || exit
aws s3 cp ./AssetsBridge.zip s3://nitecon/AssetsBridge/$TAG-$HASH/AssetsBridge.zip
echo "New location to use for store is:"
echo "https://nitecon.s3.amazonaws.com/AssetsBridge/$TAG-$HASH/AssetsBridge.zip"