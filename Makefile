all:
	$(CC) -O2 -o vga2scope *.c -I/usr/include/opencv -lm -lopencv_core -lopencv_highgui -lopencv_imgproc

display:
	#cvt -r 1600 300
	xrandr --newmode "1600x300R"   32.50  1600 1648 1680 1760  300 303 313 319 +hsync -vsync
	xrandr --addmode VGA1 "1600x300R"
	#xrandr --output VGA1 --mode "1600x300R" --right-of LVDS1
	xrandr --output VGA1 --mode "1600x300R" --pos 0x700
