#!/bin/bash

clear
rm ~/work/mqtt_exe/ -rf
mkdir ~/work/mqtt_exe
cp -rf ./ ~/work/mqtt_exe
rm ~/work/mqtt_exe/install_mqtt.sh
cd ~/work/mqtt_exe
cp -rf ~/git_room/Linux_C_Project/C_MyLib/ ~/work/

make
mv ./main mqx_temp
make clean
mv ./mqx_temp main

mv mqx_run ~/myBin
