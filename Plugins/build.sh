#!/bin/bash
CURDIR=`pwd`
if [ -d /tmp/AssetsBridgeBuilder ]; then
    rm -rf /tmp/AssetsBridgeBuilder/
fi
mkdir -p /tmp/AssetsBridgeBuilder/AssetsBridge
rsync -av --exclude=Binaries/ --exclude=Intermediate/ ./AssetsBridge/ /tmp/AssetsBridgeBuilder/AssetsBridge/
cd /tmp/AssetsBridgeBuilder/
zip -r AssetsBridge.zip AssetsBridge
cp -f /tmp/AssetsBridgeBuilder/AssetsBridge.zip "$CURDIR/AssetsBridge.zip"