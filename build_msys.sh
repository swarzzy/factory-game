#!/bin/bash

##########################################
# Build script for Msys2 x64 using clang #
##########################################

ObjOutDir=build/obj/
BinOutDir=build/

mkdir -p $BinOutDir
mkdir -p $ObjOutDir

CommonDefines="-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DPLATFORM_WINDOWS -DUNICODE -D_UNICODE"
CommonCompilerFlags="-std=c++17 -ffast-math -fno-rtti -fno-exceptions -static-libgcc -static-libstdc++ -fno-strict-aliasing -Werror -Wno-switch"
DebugCompilerFlags="-O0 -fno-inline-functions -g"
ReleaseCompilerFlags="-O2 -finline-functions -g"
SDLDependencies="-Lbuild -lSDL2 -lole32 -lsetupapi -limm32 -loleaut32 -lversion"
PlatformLinkerFlags="-lgdi32 -lopengl32 -luser32 -lwinmm"
GameLinkerFlags=""

ResourceLoaderLinkerFlags="-shared"
ResourceLoaderFlags="$CommonCompilerFlags $ReleaseCompilerFlags"

ConfigCompilerFlags=$DebugCompilerFlags

#echo "Building shader preprocessor..."
#clang++ -save-temps=obj -o $BinOutDir/shader_preprocessor.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/tools/shader_preprocessor.cpp

#echo "Preprocessing shaders..."
#build\shader_preprocessor.exe src/ShaderConfig.txt
#cp shader_preprocessor_output.h src\GENERATED_Shaders.h
#rm shader_preprocessor_output.h

echo "Building resource loader..."
clang++ -save-temps=obj -o $BinOutDir/flux_resource_loader.dll $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/ResourceLoader.cpp -shared  $GameLinkerFlags

echo "Building factory-game..."
build/hmrt.exe -save-temps=obj -DPLATFORM_CODE -o $BinOutDir/win32_pong.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/platform/SDLWin32Platform.cpp $SDLDependencies $PlatformLinkerFlags
clang++ -save-temps=obj -DPLATFORM_CODE -o $BinOutDir/win32_flux.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/Win32Platform.cpp $PlatformLinkerFlags
clang++ -save-temps=obj -o $BinOutDir/flux.dll $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/GameEntry.cpp -shared  $GameLinkerFlags
