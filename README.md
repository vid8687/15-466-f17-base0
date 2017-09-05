# ColorSlide
An implementation of [''ColorSlide''](http://graphics.cs.cmu.edu/courses/15-466-f17/game0-designs/rmukunda/) game design over Base0 starter code.

Base0 is the starter code for the game0 in the 15-466-f17 course. It was developed by Jim McCann, and is hereby released into the public domain.

Note that Base0 is attempting to illustrate a very straightfoward "get it done" philosophy of game code design -- I've purposefully removed some of the conveniences one typically uses to sweep things under the rug.

This is because game0 *does not need* such conveniences. It is simple because game0 is simple.

The game follows the design document, with the following minor changes:
- Sliding the base color bar is based on left/right arrow keys
- Spacebar toggles between missiles falling straight down and  with random directions(default)
- As score increases, frequency of missiles rather than their speed increases

## Requirements

 - glm
 - libSDL2
 - modern C++ compiler

On Linux or OSX these requirements should be available from your package manager without too much hassle.

## Building

### Linux
```
  g++ -g -Wall -Werror -o main main.cpp Draw.cpp `sdl2-config --cflags --libs` -lGL
```
or:
```
  make
```

### OSX
```
  clang++ -g -Wall -Werror -o main main.cpp Draw.cpp `sdl2-config --cflags --libs`
```
or:
```
  make
```

### Windows

Before building, clone [kit-libs-win](https://github.com/ixchow/kit-libs-win) into the `kit-libs-win` subdirectory:
```
  git clone https://github.com/ixchow/kit-libs-win
```
Now you can:
```
  nmake -f Makefile.win
```
or:
```
  cl.exe /EHsc /W3 /WX /MD /Ikit-libs-win\out\include /Ikit-libs-win\out\include\SDL2 main.cpp Draw.cpp gl_shims.cpp /link /SUBSYSTEM:CONSOLE /LIBPATH:kit-libs-win\out\lib SDL2main.lib SDL2.lib OpenGL32.lib
  copy kit-libs-win\out\dist\SDL2.dll .
```

