# vga2scope
A Video/Camera image (over VGA) to oscilloscope X/Y mode streaming tool.

# Dependencies
## Install OpenCV
``` sudo apt install libopencv-dev ```

# Configure Grub

The newer VGA Version uses the so called Data Display Channel (DDC) which checks if a screen is connected. As we do not connect a screen in this case we need to keep the VGA-Port constantly activated so we can keep sending signals over it.
This step was necessary as in my setup as the VGA-Port wouldn't show up using ```xrandr``` when no screen is connected.

## Edit GRUB 
``` sudo nano /etc/default/grub ``` 
## Edit Parameters
Change line ``` GRUB_CMDLINE_LINUX_DEFAULT="quiet splash" ``` to ``` GRUB_CMDLINE_LINUX_DEFAULT="quiet splash video=VGA-1:e"``` 

# Compile
``` g++ -o vga2scope vga2scope.cpp `pkg-config --cflags --libs opencv4` ```

# Set screen settings
``` #cvt -r 1336 768 ``` \
``` xrandr --newmode "scope"   32.50  1600 1648 1680 1760  300 303 313 319 +hsync -vsync ``` \
``` xrandr --addmode VGA-1 "scope" ``` \
``` xrandr --output VGA-1 --mode "scope" --right-of LVDS1 ``` \
``` xrandr --output VGA-1 --mode "scope" --pos 0x700 ``` \

# Setup
Connect pins 1 (Red) to X (Channel 1) and 2 (Green) to Y (Channel 2) of the VGA output to X and Y of the oscilloscope.
Also connect GND to the oscilloscope using the outer metal layer of the port.

![VGA to Oscilloscope Setup](https://github.com/ands/vga2scope/raw/master/setup.jpg)
