# DreamNote

## Building

Currently, DreamNote depends on [SDL2](https://www.libsdl.org/index.php), [PortAudio](http://portaudio.com),
[libsndfile](http://www.mega-nerd.com/libsndfile/), and [libsamplerate](http://www.mega-nerd.com/SRC/).
These must be installed on your system prior to building.


Additionally, DreamNote depends on [TicTocTimer](), which is included as a submodule. To install this library, run:

```
git submodule update
```


After these steps, you may simply run:

```
make
```
