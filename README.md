# Video Player

An OpenGL based video player application.

## Dependencies

* scythe-thirdparty
* scythe
* ffmpeg

## Build

*ffmpeg* should be built separately as a number of static libraries: avformat, avdevice, etc.
Example of _ffmpeg_ configuration:
```bash
./configure --target-os=win64 --arch=x86_64 --toolchain=msvc --disable-ffplay --disable-ffprobe --disable-ffmpeg --enable-static
```