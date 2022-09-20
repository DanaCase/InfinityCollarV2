# Trinket IO demo
# Welcome to CircuitPython 3.1.1 :)

import board
from digitalio import DigitalInOut, Direction, Pull
from analogio import AnalogOut, AnalogIn
import touchio
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
import adafruit_dotstar as dotstar
import time
import neopixel


# One pixel connected internally!
dot = dotstar.DotStar(board.APA102_SCK, board.APA102_MOSI, 1, brightness=0.2)

# Built in red LED
led = DigitalInOut(board.D13)
led.direction = Direction.OUTPUT

# Analog input on D0
analog1in = AnalogIn(board.D0)

# Analog output on D1
aout = AnalogOut(board.D1)

# Digital input with pullup on D2
button = DigitalInOut(board.D2)
button.direction = Direction.INPUT
button.pull = Pull.UP

# Capacitive touch on D3
touch = touchio.TouchIn(board.D3)

# NeoPixel strip (of 16 LEDs) connected on D4
NUMPIXELS = 20
neopixels = neopixel.NeoPixel(board.D4, NUMPIXELS, brightness=0.2, auto_write=False)

# Used if we do HID output, see below
kbd = Keyboard()

######################### HELPERS ##############################

# Helper to convert analog input to voltage
def getVoltage(pin):
    return (pin.value * 3.3) / 65536

# Helper to give us a nice color swirl
def wheel(pos):
    # Input a value 0 to 255 to get a color value.
    # The colours are a transition r - g - b - back to r.
    if (pos < 0):
        return (0, 0, 0)
    if (pos > 255):
        return (0, 0, 0)
    if (pos < 85):
        return (int(pos * 3), int(255 - (pos*3)), 0)
    elif (pos < 170):
        pos -= 85
        return (int(255 - pos*3), 0, int(pos*3))
    else:
        pos -= 170
        return (0, int(pos*3), int(255 - pos*3))

def colorwheel(pos):
    # Input a value 0 to 255 to get a color value.
    # The colours are a transition r - g - b - back to r.
    if pos < 0 or pos > 255:
        return (0, 0, 0)
    if pos < 85:
        return (255 - pos * 3, pos * 3, 0)
    if pos < 170:
        pos -= 85
        return (0, 255 - pos * 3, pos * 3)
    pos -= 170
    return (pos * 3, 0, 255 - pos * 3)

def color_chase(color, wait):
    for i in range(NUMPIXELS):
        neopixels[i] = color
        time.sleep(wait)
        neopixels.show()
    time.sleep(0.5)


def rainbow_cycle(wait):
    for j in range(255):
        for i in range(NUMPIXELS):
            rc_index = (i * 256 // NUMPIXELS) + j
            neopixels[i] = colorwheel(rc_index & 255)
        neopixels.show()
        time.sleep(wait)


RED = (255, 0, 0)
YELLOW = (255, 150, 0)
GREEN = (0, 255, 0)
CYAN = (0, 255, 255)
BLUE = (0, 0, 255)
PURPLE = (180, 0, 255)


######################### MAIN LOOP ##############################

i = 0
while True:
  # spin internal LED around! autoshow is on
  dot[0] = wheel(i & 255)

  # also make the neopixels swirl around
 # for p in range(NUMPIXELS):
  #    idx = int ((p * 256 / NUMPIXELS) + i)
  #    neopixels[p] = wheel(idx & 255)
  #neopixels.show()

  neopixels.fill(RED)
  neopixels.show()

  color_chase(RED, 0.1)  # Increase the number to slow down the color chase
  color_chase(YELLOW, 0.1)
  color_chase(GREEN, 0.1)
  color_chase(CYAN, 0.1)
  color_chase(BLUE, 0.1)
  color_chase(PURPLE, 0.1)

  rainbow_cycle(0)  # Increase the number to slow down the rainbow


  # set analog output to 0-3.3V (0-65535 in increments)
  aout.value = i * 256

  # Read analog voltage on D0
  print("D0: %0.2f" % getVoltage(analog1in))

  # use D3 as capacitive touch to turn on internal LED
  if touch.value:
      print("D3 touched!")
  led.value = touch.value

  if not button.value:
      print("Button on D2 pressed!")
      # optional! uncomment below & save to have it sent a keypress
      #kbd.press(Keycode.A)
      #kbd.release_all()

  i = (i+1) % 256  # run from 0 to 255
  #time.sleep(0.01) # make bigger to slow down
