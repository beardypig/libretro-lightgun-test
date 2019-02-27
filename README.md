# Lightgun Testing Core

This is a simple `libretro` core for testing out the lightgun API with absolute position mice. An absolute mouse is a mouse that reports its absolute position relative to its movement surface rather than its relative movement. For example a touchscreen, or real lightgun would report its absolute position rather than relative movement. 

The plan is for the core draw a cursor, and display the button presses on screen as well as show a message when the gun is pointed off screen.

I have been using `python-uinput` in conjunction with this core to generate absolute mouse inputs.

This short `python` recipe can be used in an interactive python shell to simulate an absolute mouse. Use the `move` method to move the cursor around the screen.

```python
import uinput

events = (
    uinput.ABS_X + (0, 320, 0, 0), # absolute ranges (min, max, fuzz, flat)
    uinput.ABS_Y + (0, 240, 0, 0),
    uinput.BTN_LEFT, uinput.BTN_RIGHT
)

device = uinput.Device(events, name="fake-abs-mouse")

def move(x, y):
    device.emit(uinput.ABS_X, x, syn=False)
    device.emit(uinput.ABS_Y, y)
```

The `min` and `max` ranges for `ABS_X` and `ABS_Y` do not make any difference as they are normalised to between `-0x7fff` and `0x8000` by `libretro` and then must be normalised to screen position by the core. However, a larger range in the `ABS_X/Y` axis will allow for higher resolution movement if the screen is large enough. 

If you start a `python` shell and create the `fake-abs-mouse` device, you should also be able to monitor the events using `evtest`.

For example, using `move(320//2, 240//2)` will result in the follow `ABS_X/Y` messages.
```
Input device name: "fake-abs-mouse"
Supported events:
  Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 272 (BTN_LEFT)
    Event code 273 (BTN_RIGHT)
  Event type 3 (EV_ABS)
    Event code 0 (ABS_X)
      Value      0
      Min        0
      Max      320
    Event code 1 (ABS_Y)
      Value      0
      Min        0
      Max      240
Properties:
Testing ... (interrupt to exit)
Event: time 1551224310.202565, type 3 (EV_ABS), code 0 (ABS_X), value 160
Event: time 1551224310.202565, type 3 (EV_ABS), code 1 (ABS_Y), value 120
Event: time 1551224310.202565, -------------- SYN_REPORT ------------
```

