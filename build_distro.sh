#!/usr/bin/env bash

echo "Building Distro ..."
cp -f ./build/Desktop_Qt_6_9_1-Release/CPP17_Qt6_Linux_LxQt_MTP_Mounter ./distro/
cp -f *.ts ./distro/
cp -f *.qm ./distro/
cp -f *.conf ./distro/

cp -f *.svg ./distro/
cp -f *.png ./distro/


