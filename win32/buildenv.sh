#!/bin/bash

#このスクリプトの置き場所
SCRIPT_DIR=$(dirname $(readlink -f ${BASH_SOURCE:-$0}))

#基本設定
source $SCRIPT_DIR/MSYS2Private/common/common.sh
commonSetup

#ツール類のインストール
pacman "${PACMAN_INSTALL_OPTS[@]}" \
$MINGW_PACKAGE_PREFIX-SDL2 \
$MINGW_PACKAGE_PREFIX-asciidoctor \
$MINGW_PACKAGE_PREFIX-qt-creator \
2>/dev/null

#Qt6をビルド
#3DにDirectXを有効化したビルド
bash $SCRIPT_DIR/MSYS2Private/qt6-static-private/qt.sh
exitOnError

