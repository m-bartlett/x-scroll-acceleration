#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <stdbool.h>
  // xinput test-xi2 --root
  // xinput list --long|grep Label -B 1w


enum { ScrollAxisX = 0, ScrollAxisY = 1 };

enum {
  ScrollButtonUp    = 4,
  ScrollButtonDown  = 5,
  ScrollButtonLeft  = 6,
  ScrollButtonRight = 7
};

const unsigned char button_pairs[2][2] = {
  [ScrollAxisX] = { ScrollButtonRight, ScrollButtonLeft },
  [ScrollAxisY] = { ScrollButtonDown, ScrollButtonUp }
};

double accumulator[] = { 0.0, 0.0 };
double scroll_threshold=0.1;
double scroll_exponent=1.25;
double scroll_scalar = 0.1;


void handleScroll(Display *display, double intensity, unsigned char axis) {

  double sign = intensity < 0.0 ? -1 : 1;
  double magnitude = fabs(intensity);

  intensity = sign * pow(magnitude * scroll_scalar, scroll_exponent);

  accumulator[axis] += intensity;

  unsigned char button=0;
  if      (accumulator[axis] >  scroll_threshold) button = button_pairs[axis][0];
  else if (accumulator[axis] < -scroll_threshold) button = button_pairs[axis][1];
  else return;

  double frac_part, int_part;
  frac_part = modf(accumulator[axis], &int_part);
  int clicks = fabs(int_part);
  for (unsigned int i=0; i < clicks; i++) {
    XTestFakeButtonEvent(display, button, True,  0);
    XTestFakeButtonEvent(display, button, False, 0);
  }
  XFlush(display);
  accumulator[axis] = frac_part;

}


void processEvents(Display *display, Window window, int xi_opcode, Atom *wm_delete_window) {
  while (True) {
    XEvent event;
    XGenericEventCookie *cookie = &event.xcookie;
    XNextEvent(display, &event);

    if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == *wm_delete_window) {
      XFreeEventData(display, cookie);
      return;
    }

    if (XGetEventData(display, cookie) && cookie->type == GenericEvent && cookie->extension == xi_opcode) {
      XIDeviceEvent *device_event = cookie->data;

      switch (device_event->evtype) {
        case XI_RawMotion: {

          XIRawEvent *raw_event = (XIRawEvent *) device_event;
          // double delta = raw_event->raw_values[0];

          XIValuatorState *states = &raw_event->valuators;
          double delta = states->values[0];

          if (delta != delta || 1+delta == delta) // test for inf and nan
            break;

          bool horizontal = XIMaskIsSet(raw_event->valuators.mask, 2);
          bool vertical   = XIMaskIsSet(raw_event->valuators.mask, 3);

          unsigned char scroll_event = vertical + (horizontal<<1);

          switch(scroll_event) {
            case 3: { // horizontal && vertical
              double delta2 = states->values[1];
              if (delta2 == delta2 && 1+delta2 != delta2) {
                handleScroll(display, delta2, ScrollAxisY);
              }
              handleScroll(display, delta, ScrollAxisX);
              break;
            }

            case 2:  // horizontal
              handleScroll(display, delta, ScrollAxisX);
              break;

            case 1: // vertical
              handleScroll(display, delta, ScrollAxisY);
              break;

            default:
          }

          break;
        }
      }
    }
    XFreeEventData(display, cookie);
  }
}


void selectRawMotionEvents(Display *display) {
  unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
  XISetMask(mask, XI_RawMotion);

  XIEventMask eventmask;
  eventmask.deviceid = XIAllMasterDevices;
  eventmask.mask_len = sizeof(mask);
  eventmask.mask = mask;

  Window window = DefaultRootWindow(display);

  XISelectEvents(display, window, &eventmask, 1);
}

static int print_usage(char *argv[]) {
  fprintf(stderr, "%s [--help|-h] [--scalar|-s *.f] [--exponent|-p *.f] [--threshold|-t *.f]\n", argv[0]);
  return EXIT_FAILURE;
}


int main(int argc, char *argv[]) {

  static const struct option long_options[] = {
    {"threshold", /*has_arg=*/1, NULL, 't'},
    {"exponent",  /*has_arg=*/1, NULL, 'p'},
    {"scalar",    /*has_arg=*/1, NULL, 's'},
    {"help",      /*has_arg=*/0, NULL, 'h'},
    {NULL,     0, NULL, 0}
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "-hs:p:", long_options, NULL) ) != -1) {
    switch (opt) {
      case 'h': // help
        return print_usage(argv);
      case 's': // scalar
        scroll_scalar = atof(optarg);
        break;
      case 't': // threshold
        scroll_threshold = atof(optarg);
        break;
      case 'p': // exponent, "p"ower
        scroll_exponent = atof(optarg);
        break;
      default:
        return print_usage(argv);
    }
  }


  Display *display = XOpenDisplay(0);
  if (!display) {fprintf(stderr, "Error opening display\n"); return -1; }

  int xi_opcode, xi_firstevent, xi_firsterror;

  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &xi_firstevent, &xi_firsterror)) {
    fprintf(stderr, "XInput extension is not supported by server.\n");
    return -1;
  }

  int major = 2, minor = 2;
  int result = XIQueryVersion(display, &major, &minor);

  if (result == BadRequest) {
    fprintf(stderr, "XInput 2.0+ required, supported by server: %d.%d\n", major, minor);
    return -1;
  } else if (result != Success) {
    fprintf(stderr, "Error reading XInput version.\n");
    return -1;
  }

  int screen = XDefaultScreen(display);
  Window window = RootWindow(display, screen);

  if (!window) {fprintf(stderr, "Error creating windows.\n"); return -1; }

  selectRawMotionEvents(display);

  // XStoreName(display, window, "Scrolling Logger");

  Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom atoms[1] = { None };
  atoms[0] = XInternAtom (display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  XChangeProperty(display, window, net_wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1);

  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_delete_window, 1);

  XMapWindow(display, window);
  XFlush(display);

  processEvents(display, window, xi_opcode, &wm_delete_window);
  XCloseDisplay(display);

  return 0;
}
