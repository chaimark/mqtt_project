#!/bin/bash

mkdir ~/work/WorkLib  ~/work/Eclipse_Paho 
cp ./WorkLib/* ~/work/WorkLib/ -rf
cp ./Eclipse_Paho/* ~/work/Eclipse_Paho/ -rf

clear
rm ~/work/mqtt_exe/ -rf
mkdir ~/work/mqtt_exe
cp -rf ./ ~/work/mqtt_exe
rm ~/work/mqtt_exe/install_mqtt.sh
cd ~/work/mqtt_exe
cp -rf ~/git_room/Linux_C_Project/WorkLib/ ~/work/

make
mv ./main mqx_temp
make clean
mv ./mqx_temp main

mv mqx_run ~/myBin
