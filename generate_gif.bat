echo Demo script to convert a video demo.mp4 to an animated gif demo.gif

REM SET "ffmpeg=D:\dev\opensource\ffmpeg_build\bin\ffmpeg"
REM %ffmpeg% -version

REM get mp4 file information
REM %ffmpeg% -i testIn.mp4 2>&1 | findstr /C:Stream 

REM Video: h264 (High) (avc1 / 0x31637661), yuv420p(progressive), 1280x720, 1048 kb/s, 24 fps, 24 tbr, 12288 tbn (default)

REM crop and resize and rescale video to smaller size
REM %ffmpeg% -i testIn.mp4 -an -vf "fps=24,crop=1080:576:100:72,scale=480:360:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=32[p];[s1][p]paletteuse=dither=bayer" test.mp4


SET "mp4gif=D:\dev\ed-wares\Mp4gif\distrib\mp4gif\mp4gif.exe"
%mp4gif% Demo.mp4 Demo.gif "fps=30,scale=480:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=32[p];[s1][p]paletteuse=dither=bayer"

echo ERRORLEVEL: %ERRORLEVEL%
REM -1073741515, which is hex 0xC0000135 specifically means STATUS_DLL_NOT_FOUND
