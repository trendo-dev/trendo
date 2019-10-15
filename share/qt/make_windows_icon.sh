#!/bin/bash
# create multiresolution windows icon
#mainnet
ICON_SRC=../../src/qt/res/icons/trendo.png
ICON_DST=../../src/qt/res/icons/trendo.ico
convert ${ICON_SRC} -resize 16x16 trendo-16.png
convert ${ICON_SRC} -resize 32x32 trendo-32.png
convert ${ICON_SRC} -resize 48x48 trendo-48.png
convert trendo-16.png trendo-32.png trendo-48.png ${ICON_DST}
#testnet
ICON_SRC=../../src/qt/res/icons/trendo_testnet.png
ICON_DST=../../src/qt/res/icons/trendo_testnet.ico
convert ${ICON_SRC} -resize 16x16 trendo-16.png
convert ${ICON_SRC} -resize 32x32 trendo-32.png
convert ${ICON_SRC} -resize 48x48 trendo-48.png
convert trendo-16.png trendo-32.png trendo-48.png ${ICON_DST}
rm trendo-16.png trendo-32.png trendo-48.png
