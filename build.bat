@echo off

WHERE cl 2>NUL 1>NUL 0>NUL
IF %ERRORLEVEL% NEQ 0 (
  echo Error: Microsoft Visual C++ compiler ^(cl^) not found
  echo Install Visual Studio and run "vcvarsall.bat x64" to enable compiler in command line.
  echo More info: https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line
  echo Build Fail :(
  goto end
)

IF NOT EXIST build mkdir build
pushd build
echo Building...
cl -MT -wd4530 -nologo -GR- -W4 -FC -Fm -Zi /I..\include ..\main.cpp ..\include\glad\src\glad.c /link -opt:ref user32.lib Gdi32.lib msvcrt.lib shell32.lib OpenGL32.lib vcruntime.lib libcmt.lib ..\lib\glfw3.lib ..\lib\LibOVR.lib
if %ERRORLEVEL%==0 (
  echo Success :^)
  goto end
)

if not %ERRORLEVEL%==0 (
  echo Build Fail :(
  goto end
)

:end
popd