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
## Update Grub and reboot
``` sudo update-grub ```
``` sudo reboot ```

# Compile
``` g++ -o vga2scope vga2scope.cpp `pkg-config --cflags --libs opencv4` ```

# Set screen settings

# Explanation of Commands for Custom Display Mode

```bash
# Generate a new modeline for 1600x700 resolution
cvt -r 1336 768

# Define a new display mode called "scope" with specified timing parameters
# These parameters define screen refresh rates, sync pulses, and pixel clock.
# Normally, this command should be used with values generated by `cvt`.
xrandr --newmode "scope" 32.50 1600 1648 1680 1760 300 303 313 319 +hsync -vsync

# Add the newly created mode "scope" to the VGA-1 output
xrandr --addmode VGA-1 "scope"

# Set VGA-1 output to use the "scope" mode and position it to the right of the LVDS1 display (or whatever your standard display is) 
xrandr --output VGA-1 --mode "scope" --right-of LVDS1

# Another way to position the VGA-1 output using absolute coordinates (placing it at X:0, Y:700)
xrandr --output VGA-1 --mode "scope" --pos 0x700
```

# Setup
Connect pins 1 (Red) to X (Channel 1) and 2 (Green) to Y (Channel 2) of the oscilloscope.
Also connect GND to the oscilloscope using the outer metal layer of the port. Turn on ```Hor. Ext. ``` on the oscilloscope so Channel 2 turns vertical. Twist the knobs until a picture forms.

# Run
``` ./vga2scope example.mp4 0 0 500 500 ```
After the display runs move the windows "Out" to the other screen which sends to the oscilloscope

![VGA to Oscilloscope Setup](https://github.com/ands/vga2scope/raw/master/setup.jpg)

# Further possible improvements
- Find a way to use a HDMI to VGA Adapter to make this work with HDMI-Ports
- ``` xrandr --output VGA-1 --mode "scope" --pos 0x700 ``` this command can throw errors when the mode validation rejects the custom mode. There should be a way to turn off mode validation

# C-Implementation
[Makefile](makefile) and [vga2scope.c](vga2scope.c) are deprecated as OpenCV4 does no longer support C-Libs.

# Related projects 
Check out:
- https://osci-render.com/
- https://www.warpzone.ms/perma/vga-vector-graphics-adapter/1859/
- https://wiki.warpzone.ms/projekte:vector_graphics_adapter
- https://code.nerd2nerd.org/pfoetchen/scopotron/-/tree/master?ref_type=heads
