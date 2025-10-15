REM This is optional bat file to rebuild EXE
REM You can run it from command line or integrate into VSCode tasks

echo "Copying required DLLs..."
robocopy "d:/dev/opensource/ffmpeg_build/bin" "." *.dll /XC /NJH

echo "Building mp4gif..."
g++ -o mp4gif.exe mp4gif.cpp -Id:/dev/opensource/ffmpeg_build/include -Ld:/dev/opensource/ffmpeg_build/lib -lavformat -lavcodec -lavutil -lswscale -lavfilter
