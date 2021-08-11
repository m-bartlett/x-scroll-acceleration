#!/usr/bin/env bash
xinput set-prop 'DLL075B:01 06CB:76AE Touchpad' 'libinput Scrolling Pixel Distance' 50
make && ./bin/x-scroll-acceleration $@