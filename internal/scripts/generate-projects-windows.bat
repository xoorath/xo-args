@ECHO OFF
pushd "%~dp0.."
.\tools\windows\premake5.exe vs2022 --file="%~dp0premake5.lua"
popd