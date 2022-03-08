
# x-scroll-acceleration <img alt="logo" align="left" height="150" width="150" src="https://user-images.githubusercontent.com/85039141/129837132-4ef7b2dd-664f-4225-9879-00c42839d71d.png">

X event daemon that simulates trackpad scroll acceleration as felt on other operating systems, e.g. macOS. Most apps need to implement scroll momentum through XInput2 for just themselves, but this application allows for scroll acceleration globally in all X applications, regardless if they have implemented their own scrolling momentum.

### **Important Usage Notes**:
~This daemon works best with the trackpad scrolling distance set to be very small. The smaller distance allows each mouse-button event to provide more granular movement. If the scroll distance is too large, this tool is more harm than good as you will likely immediately scroll to the top or bottom of the window.~

The above used to be true, but the scroll acceleration has been empirically demonstrated to work well with large scroll distances. I have found that large scroll distances with less sensitive configuration of this X scroll event daemon produces a far less jittery/jumpy experience compared to small scroll distances with a sensitive configuration of this daemon. Your (scrolling) mileage may vary, so just experiment with different combinations of your scroll input sensitivity and the various CLI flags of this daemon.

I use libinput as my trackpad driver, and in recent versions of the driver I can achieve the largest scroll distance with this `xinput` command:
```sh
xinput set-prop 'DLL075B:01 06CB:76AE Touchpad' 'libinput Scrolling Pixel Distance' 10
```

Your touchpad name (second positional argument) will likely be different. For this xinput property, ***larger values result in smaller scroll distance***. In my case, 50 is the maximum value and 10 is the minimum value xinput will allow me to set.

---

### Install
- `make && make install` and execute `x-scroll-acceleration` as part of your login initialization

Alternatively, to create a debugging version of the binary which prints verbose output detailing the scroll aggregation calculations and event values execute `make debug` instead of `make` or `make x-scroll-acceleration`.


### Usage
See `x-scroll-acceleration --help`

Example: `x-scroll-acceleration --scroll-threshold 0.1 --exponent 1.5 --scalar 0.035 --speed-threshold 10.0 -v`


### About
This uses the XInput2 API to get raw pointer events indepenent of any X window, which gives access to instantenous scroll velocity per scroll event. This uses the XTest API to send artificial XEvents with mouse buttons 4 and 5 (which are discrete scroll-up and scroll-down inputs respectively) to the root X window, thus passing the input to whichever X window has mouse focus. We simply "integrate" the instantenous velocities with an acceleration factor by adding up these expontially scaled velocities to give the illusion of momentum. This aggregate sum represents the "position" or distance scrolled. This resulting distance is divided into a quantity of mouse-button presses in the appropriate direction. Thus, if more "distance" is scrolled per event, more mouse-button presses are sent to the X-server, which creates the illusion of momentum. This quantity is self-promoting via exponentiation, so that faster one scrolls the more scroll distance is produced.


### Dev notes:
https://who-t.blogspot.com/2011/09/whats-new-in-xi-21-raw-events.html
Raw events mean they don't get swallowed or transformed
