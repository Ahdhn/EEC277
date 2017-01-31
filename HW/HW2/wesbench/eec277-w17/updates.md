# 20 Jan 2016 4:00 pm

Added error-checking in main loop. This, oddly, makes it work on Linux, but no one knows why.

# 20 Jan 2016 2:20 pm

Added support for Linux builds on X11 (this required a lot of extra library linking). This works on Mac, hopefully will also work on Windows.

# 20 Jan 2016 1:30 pm

Windows builds must not call glLinkProgram. MacOS builds must call glLinkProgram. Crazy, huh? It compiles that way now. Linux builds will call glLinkProgram and if that's wrong, please tell me.

# 20 Jan 2016 10:55 am

Added cmake support. This should solve a lot of build issues. Just run `cmake .` then `make` (not sure yet how this will look on Windows, it might make Visual Studio build files, let me know).

Here's how it looks on my Mac:

```
$ cmake .
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/jowens/Box Sync/class/277-w17/resources/wesBench/eec277-w17
$ make
[ 50%] Building C object CMakeFiles/wesbench-instructional.dir/wesbench-instructional.c.o
[100%] Linking C executable wesbench-instructional
[100%] Built target wesbench-instructional
$ ./wesbench-instructional
--------------------------------------------------
Vendor:      NVIDIA Corporation
Renderer:    NVIDIA GeForce GT 750M OpenGL Engine
Version:     2.1 NVIDIA-10.14.20 355.10.05.15f03
Visual:      RGBA=<8,8,8,135172>  Z=<135173>  double=1
Geometry:    1024x1024+768+137
Screen:      2560x1440
--------------------------------------------------

./wesbench-instructional Setup parameters
Triangle area   128.000 (pixels^2)
Test duration   5.000000(s)
Screen W/H  (1024,1024)
Triangle limit  1073741824
VertexBuf limit 1073741824
Texture size    (0 x 0)
Lighting is     disabled
ColorMaterial   disabled
Triangle type   0
Retained mode   disabled
verts/frame = 6144
nframes = 80604
 WesBench: area=128.0 px, tri rate = 0.00 Mtri/sec, vertex rate=0.00 Mverts/sec, fill rate = 0.00 Mpix/sec, verts/bucket=6144, indices/bucket=0
```
