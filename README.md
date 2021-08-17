# x-scroll-acceleration
X event daemon that simulates trackpad scroll acceleration as felt on other operating systems, e.g. macOS.


---
### **Important Usage Notes**:
This daemon works best with the trackpad scrolling distance set to be very very small (as to be nearly useless without this daemon activated). The smaller distance allows each mouse-button event to provide more granular movement. If the scroll distance is too large, this tool is more harm than good as you will likely immediately scroll to the top or bottom of the window.

I use libinput as my trackpad driver, and in recent versions of the driver I can achieve the smallest scroll distance with this `xinput` command:
```sh
xinput set-prop 'DLL075B:01 06CB:76AE Touchpad' 'libinput Scrolling Pixel Distance' 50
```

Your touchpad name (second positional argument) will likely be different. For this xinput property, ***larger values result in smaller scroll distance***. In my case, 50 is the maximum value xinput will allow me to set.

---

### Install
- `make && make install` and execute `x-scroll-acceleration` as part of your login initialization


### Usage
See `x-scroll-acceleration --help`

Example: `x-scroll-acceleration --velocity-exponent 1.5 --scroll-threshold 0.05`


### About
This uses the XInput2 API to get raw pointer events indepenent of any X window, which gives access to instantenous scroll velocity per scroll event. This uses the XTest API to send artificial XEvents with mouse buttons 4 and 5 (which are discrete scroll-up and scroll-down inputs respectively) to the root X window, thus passing the input to whichever X window has mouse focus. We simply "integrate" the instantenous velocities with an acceleration factor by adding up these expontially scaled velocities to give the illusion of momentum. This aggregate sum represents the "position" or distance scrolled. This resulting distance is divided into a quantity of mouse-button presses in the appropriate direction. Thus, if more "distance" is scrolled per event, more mouse-button presses are sent to the X-server, which creates the illusion of momentum. This quantity is self-promoting via exponentiation, so that faster one scrolls the more scroll distance is produced.


### Dev notes:
https://who-t.blogspot.com/2011/09/whats-new-in-xi-21-raw-events.html
Raw events mean they don't get swallowed or transformed