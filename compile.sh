#!/bin/bash
g++ main.cpp -o m -std=c++11 -lopencv_highgui  -lopencv_core -lopencv_imgproc -lopencv_objdetect -I/usr/include/jsoncpp  -lcurl -ljsoncpp  -lssl -lcrypto
