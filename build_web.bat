@echo off
set EMSDK=C:\Users\Elior\source\repos\emsdk
set EMSDK_PYTHON=%EMSDK%\python\3.13.3_64bit\python.exe
set EMSDK_NODE=%EMSDK%\node\22.16.0_64bit\bin\node.exe
set PATH=%EMSDK%\upstream\emscripten;%EMSDK%\node\22.16.0_64bit\bin;%PATH%

set RAYLIB_SRC=C:\Users\Elior\source\repos\raylib\src
set RAYLIB_LIB_DIR=C:\Users\Elior\source\repos\raylib\build-wasm\raylib

%EMSDK_PYTHON% %EMSDK%\upstream\emscripten\emcc.py ^
  Runtime\Main.c ^
  ConsoleApplication1\WzGuiCore.c ^
  ConsoleApplication1\WzLayoutScalar.c ^
  ConsoleApplication1\Actions.c ^
  ConsoleApplication1\Animation.c ^
  ConsoleApplication1\ArrayList.c ^
  ConsoleApplication1\FileName.c ^
  ConsoleApplication1\GameState.c ^
  ConsoleApplication1\Handle.c ^
  ConsoleApplication1\HashTable.c ^
  ConsoleApplication1\Inventory.c ^
  ConsoleApplication1\Parser.c ^
  ConsoleApplication1\Pathfinding.c ^
  ConsoleApplication1\Renderer.c ^
  ConsoleApplication1\Serialization.c ^
  ConsoleApplication1\Strings.c ^
  ConsoleApplication1\Timer.c ^
  ConsoleApplication1\file_handling.c ^
  -O2 -Wall -msimd128 ^
  -I. -IRuntime -IConsoleApplication1 -I"ConsoleApplication1 - Copy" -I%RAYLIB_SRC% ^
  -L. -L%RAYLIB_LIB_DIR% -lraylib ^
  -s USE_GLFW=3 ^
  -s FULL_ES3=1 ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s INITIAL_MEMORY=67108864 ^
  --preload-file web\fonts@/fonts ^
  --preload-file web\data@/data ^
  --shell-file %RAYLIB_SRC%\minshell.html ^
  -DPLATFORM_WEB ^
  -o web\index.html

if %ERRORLEVEL% == 0 (
  echo Build successful.
) else (
  echo Build failed.
)
