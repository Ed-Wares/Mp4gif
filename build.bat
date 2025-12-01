@echo off
SET prj_name=mp4gif
SET "current_dir=%~dp0"
SET "build_dir=%current_dir%build\"
SET "distrib_dir=%current_dir%distrib\%prj_name%\"

REM This is optional bat file to rebuild EXE
REM You can run it from command line or integrate into VSCode tasks

REM echo "Copying required DLLs..."
REM robocopy "d:/dev/opensource/ffmpeg_build/bin" "." *.dll /XC /NJH
REM g++ -o mp4gif.exe mp4gif.cpp -Id:/dev/opensource/ffmpeg_build/include -Ld:/dev/opensource/ffmpeg_build/lib -lavformat -lavcodec -lavutil -lswscale -lavfilter

cd /d "%current_dir%"

echo removing old build
rmdir /s /q "%build_dir%"

echo creating build directory "%build_dir%"
mkdir "%build_dir%"
mkdir "%distrib_dir%"
pushd "%build_dir%"

REM echo Install dependencies with cmd: vcpkg install glfw3 glm freetype
echo dependencies can be installed in MinGW with: pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake zip
echo Also to download ffmpeg library: wget https://ffmpeg.org/releases/ffmpeg-7.1.2.tar.xz

echo Configuring and building with CMake
cmake .. -G "MinGW Makefiles"
REM cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

if %ERRORLEVEL% geq 1 (
    echo CMake build failed!
    exit /b 1
)

echo copying binaries to the distrib folder...
copy /Y *.exe "%distrib_dir%"
copy /Y *.dll "%distrib_dir%"
popd

pushd "%distrib_dir%.." && C:\msys64\usr\bin\zip.exe -r %prj_name%.zip %prj_name% && popd
echo Created distribution file at distrib\%prj_name%.zip