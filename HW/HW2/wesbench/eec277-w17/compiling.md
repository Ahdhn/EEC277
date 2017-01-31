# Compiling wesBench

## Prereqs

You must install OpenGL (probably included), GLEW (to manage GL versions), and GLFW (for windowing). GLEW and GLFW are almost certainly available in any reasonable package manager. On my Mac, they are both available in MacPorts (and probably homebrew as well).

http://glew.sourceforge.net/
http://www.glfw.org/

## Compiler command line

Here's the compiler settings on my Mac:

```
cc -I/opt/local/include -L/opt/local/lib -framework OpenGL -lglew -lglfw wesBench-instructional.c -o wesBench-instructional
```

On non-Macs, it's more likely going to look something like:

```
cc -I/opt/local/include -L/opt/local/lib -lgl -lglew -lglfw wesBench-instructional.c -o wesBench-instructional
```

I welcome, very much, contributions of command lines for other operating systems.

## What it looks like when you run it

```
$ ./wesBench-instructional
--------------------------------------------------
Vendor:      NVIDIA Corporation
Renderer:    NVIDIA GeForce GT 750M OpenGL Engine
Version:     2.1 NVIDIA-10.14.20 355.10.05.15f03
Visual:      RGBA=<8,8,8,135172>  Z=<135173>  double=1
Geometry:    1024x1024+768+137
Screen:      2560x1440
--------------------------------------------------

./wesBench-instructional Setup parameters
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
nframes = 79307
 WesBench: area=128.0 px, tri rate = 0.00 Mtri/sec, vertex rate=0.00 Mverts/sec, fill rate = 0.00 Mpix/sec, verts/bucket=6144, indices/bucket=0
```

Note that it gives no meaningful performance statistics. That's part of your assignment.
