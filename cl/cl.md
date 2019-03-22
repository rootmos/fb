# OpenCL hackery

## Installation notes
```shell
aurman -S compute-runtime-bin

sudo pacman -S clinfo
clinfo
```

## TODO

* `look_at` function to compute a correctly oriented grid (also get rid of the
  unnecessary subtractions in the entry-point of the kernel)
* compare sky colliding vector with a "sun" light source and diminish intensity
  based on angle
* render video
  - https://www.ffmpeg.org/ffmpeg-formats.html#rawvideo
* try regular grid for the spatial oversampling
* construct dispersion vector for matte objects using fixed vectors on the unit
  sphere by randomly select vectors that lie above the tangent plane and mix
  them
  - essentially approximate a unit sphere with polygons: select polygon and then
    select a vector inside it
  - generate a binary search tree of unit vectors to be included in the kernel
    code
  - use angle wrt normal to determine which side of the tangent plane the vector
    is pointing
