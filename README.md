# mp4gif
Converts a mp4 video file to an animated gif file.  This program was written in a crossplatform way using C++.  It uses the ffmpeg libraries to do most of the heavy lifting.

### Demo

Here is a quick look at the application:

![Demo](https://github.com/Ed-Wares/Mp4gif/blob/main/DemoMp4gif.gif?raw=true)

## Usage
To convert a mp4 to gif file with default configurations open command line and run ```mp4gif.exe source.mp4 output.gif```

For more advanced options you can optionally include custom pre-filters like the following for cropping the video.
```
mp4gif input.mp4 output.gif "crop=w:h:x:y"
```

Scale the video to 800, and set frames per second to 20:
```
mp4gif input.mp4 output.gif "fps=20,scale=800:-1:flags=lanczos"
```

## Building

Build your own application binaries.

Prerequesites required for building source
-  msys2 - download the latest installer from the [MSYS2](https://github.com/msys2/msys2-installer/releases/download/2024-12-08/msys2-x86_64-20241208.exe)
- Run the installer and follow the steps of the installation wizard. Note that MSYS2 requires 64 bit Windows 8.1 or newer.
- Run Msys2 terminal and from this terminal, install the MinGW-w64 toolchain by running the following command:
```
pacman -S --needed make base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-yasm mingw-w64-ucrt-x86_64-nasm
```
- Accept the default number of packages in the toolchain group by pressing Enter (default=all).
- Enter Y when prompted whether to proceed with the installation.
- Add the path of your MinGW-w64 bin folder (C:\msys64\ucrt64\bin) to the Windows PATH environment variable.
- To check that your MinGW-w64 tools are correctly installed and available, open a new Command Prompt and type:
```
g++ --version
```
- Optionally, install Visual Studio Code IDE (with C++ extensions).  [VsCode](https://code.visualstudio.com/download)

- Build ffmpeg version 8.0 library
    - Download and extract the ffmpeg 8.0 source code.
  ```
    mkdir -p /d/dev/opensource/
    cd /d/dev/opensource/
    wget https://ffmpeg.org/releases/ffmpeg-7.1.2.tar.xz
    tar -xvf ffmpeg-7.1.2.tar.xz
    cd ffmpeg-7.1.2
  ```
    - Build ffmpeg libs by running the configure script and then make it. The most important part here is the --prefix flag, which tells FFmpeg where to install the final files. We will use a custom directory like d:/dev/opensource/ffmpeg_build to keep it separate from the MSYS2 system folders. --enable-shared: Builds the FFmpeg libraries as .dll files.
  ```
    ./configure --enable-shared --prefix=/d/dev/opensource/ffmpeg_build
    make -j$(nproc)
    make install
  ```

Build binaries by running the build.bat script or from VsCode by running the Build and Debug Task.


## Misc

