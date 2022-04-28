# Procedural Modeling of Terrain

By Weili An, Spring 2022 CS 334 Final Project

## Open Source Library used

1. Qt5
2. [Function Parser for C++](http://warp.povusers.org/FunctionParser/) under LGPL v3 license
3. [siv::PerlinNoise](https://github.com/Reputeless/PerlinNoise) under MIT license

## How to start

1. Run `qmake` for configuring QT and Makefile
2. `make -j` to compile the whole project
3. `./base_qt` to launch the application.

## User Manual

### General control

1. User coule use the `save` button on top left to export current configurations as text file, which could be read in by the `load` button.
2. User could choose different normals and shading for testing purposes.

### Add a surface

1. Click `Add a Surface` button to add a surface group to the app.
2. In each surface group, user can specify the Phong model parameters for the surface.
3. In addition, sub layer can also be added to enrich the details.
4. Support functions of layers include:
   1. `normal(x, y, sigma_x, sigma_y)`
      1. Draw a normal surface center at `(0,0)`
      2. With standard deviation of `sigma_x` and `sigma_y` respectively.
      3. Can offset the `x` and `y` by using expression like `x - 0.5` and `y + 0.3` to change the mean of the normal surface.
   2. `perlin(x, y, freq)`
      1. Classic perlin noise surface with seed specified in general control widget, built with [siv::PerlinNoise](https://github.com/Reputeless/PerlinNoise).
      2. `freq` specify the frequency of the surface on both axes.
   3. `plane(x, y, z1, z2, z3)`
      1. Draw a plane with points specified at `(-1, 1, z1)`, `(1, 1, z2)`, and `(1, -1, z3)`, 
   4. `pyramid(x, y, z1, z2, z3, z4, ax, ay, az)`
      1. Draw a pyramid with corners at `(-1, 1, z1)`, `(1, 1, z2)`, `(1, -1, z3)`, and `(-1, -1, z4)`.
      2. The apex of the pyramid is at `(ax, ay, az)`.
   5. Common math functions like `cos`, `sin`, `exp`, built with [Function Parser for C++](http://warp.povusers.org/FunctionParser/fparser.html).

### Multiple Surfaces

1. When multiple surfaces exist, the top one is always treated as the actual terrain.
2. For each subsequent surface, any part of the terrain that is lower than it will be painted with the Phong configuration of this subsequent surface.
   1. This allows different zones of terrain like snow, rock, forest, and sea
3. By checking `Enable Surface` in the surface group widget, the surface will paint the terrain.
4. By checking `Draw Surface` in the surface group widget, the app will draw the surface on top of the terrain.
   1. This can be used to simulate ocean surface.

### Prebuilt Terrain Configuration

1. User can found prebuilt terrain configuration files (`*.config`) in root folder.