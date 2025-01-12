# vga2scope
A Video/Camera image (over VGA) to oscilloscope X/Y mode streaming tool.

# dependencies
## Install OpenCV
``` sudo apt install libopencv-dev ```
## Include the library
Edit the C/C++ Configurations (JSON) as follows
``` {
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/opencv4"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

# compile
``` g++ -o vga2scope vga2scope.cpp `pkg-config --cflags --libs opencv4` ```

# setup
Connect pins 1 and 2 (Red, Green) of the VGA output to X and Y of the oscilloscope.

Don't forget GND!

![VGA to Oscilloscope Setup](https://github.com/ands/vga2scope/raw/master/setup.jpg)
