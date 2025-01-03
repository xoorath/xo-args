@echo off
REM This script assumes a windows environment with WSL, msvc, clang, gcc and 
REM make. It will build all supported targets for windows and linux and run 
REM all tests.

setlocal enabledelayedexpansion
set LF=^& echo.

GOTO Start

:WriteRed
set txt=%*
set txt=%txt:"=%
echo [31m%txt%[0m
exit /b


:Start

:: Check for MSBuild using vswhere
set VSWHERE_PATH=%~dp0..\tools\windows\vswhere.exe

echo Looking for msbuild
for /f "tokens=*" %%i in ('"%VSWHERE_PATH%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe') do set MSBUILD_PATH=%%i
if not defined MSBUILD_PATH (
    call :WriteRed "Error: MSBuild is not installed or not found."
    exit /b 1
)
echo %MSBUILD_PATH%

:: Run Windows project generation script
call %~dp0generate-projects-windows.bat
if %errorlevel% neq 0 (
    call :WriteRed "Error: Failed to run generate-projects-windows.bat."
    exit /b 1
)

pushd %~dp0
wsl ./generate-projects-linux.sh
popd
if %errorlevel% neq 0 (
    call :WriteRed "Error: Failed to run generate-projects-linux.sh."
    exit /b 1
)

set "SUMMARY=Build Summary:!LF!"
set ERROR_COUNT=0

pushd %~dp0..\build\vs2022 || (call :WriteRed "Error: Failed to navigate to build/vs2022" && exit /b 1)
for %%f in (*.sln) do (
    for %%c in (Debug Release) do (
        for %%p in (x86 x64) do (
            @REM translate x86 to Win32 for msbuild
            if "%%p"=="x86" (
                "%MSBUILD_PATH%" "%%f" /p:Configuration=%%c /p:Platform=Win32 /t:Rebuild
            ) else (
                "%MSBUILD_PATH%" "%%f" /p:Configuration=%%c /p:Platform=x64 /t:Rebuild
            )
            if !errorlevel! neq 0 (
                set "SUMMARY=!SUMMARY! !LF![31m%%f - Windows %%c %%p: FAILED to build[0m"
                set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!%~dp0/generate-projects-windows.bat!LF!^"%MSBUILD_PATH%^" ^"%~dp0..\build\vs2022\%%f^" /p:Configuration=%%c /p:Platform=Win32 /t:Rebuild"
                set /a ERROR_COUNT=!ERROR_COUNT!+1

            ) else (
                set "SUMMARY=!SUMMARY! !LF![32m%%f - Windows %%c %%p: built[0m"
                pushd %~dp0..\build\vs2022\bin\%%c_%%p
                for %%e in (*.exe) do (
                    "%%e"
                    if !errorlevel! neq 0 (
                        set "SUMMARY=!SUMMARY! !LF![31m%%e (Windows %%c %%p) Exited with code !errorlevel![0m"
                        set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!^"%~dp0..\build\vs2022\bin\%%c_%%p\%%e^""
                        set /a ERROR_COUNT=!ERROR_COUNT!+1
                    ) else (
                        set "SUMMARY=!SUMMARY! !LF![32m%%e (Windows %%c %%p) Exited with code !errorlevel![0m"
                    )
                    ver >nul
                )
                popd
            )
        )
    )
)
popd

pushd %~dp0..\build\gmake2 || (call :WriteRed "Error: Failed to navigate to build/gmake2" && exit /b 1)

for %%c in (debug release) do (
    for %%p in (x86 x64) do (
        wsl make clean config=%%c_%%p
        wsl make config=%%c_%%p

        if !errorlevel! neq 0 (
            set "SUMMARY=!SUMMARY! !LF![31mgmake2 - Linux gcc %%c %%p: FAILED to build[0m"
            set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!pushd %~dp0!LF!wsl ./generate-projects-linux.sh!LF!popd!LF!pushd %~dp0..\build\gmake2!LF!wsl make config=%%c_%%p!LF!popd"
            set /a ERROR_COUNT=!ERROR_COUNT!+1
        ) else (
            set "SUMMARY=!SUMMARY! !LF![32mgmake2 - Linux gcc %%c %%p: built[0m"
                pushd %~dp0..\build\gmake2\bin\%%c_%%p
                for %%e in (*) do (
                    wsl "./%%e"
                    if !errorlevel! neq 0 (
                        set "SUMMARY=!SUMMARY! !LF![31m%%e (Linux gcc %%c %%p) Exited with code !errorlevel![0m"
                        set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!pushd %~dp0..\build\gmake2\bin\%%c_%%p!LF!wsl ^"%%e^"!LF!popd"
                        set /a ERROR_COUNT=!ERROR_COUNT!+1
                    ) else (
                        set "SUMMARY=!SUMMARY! !LF![32m%%e (Linux gcc %%c %%p) Exited with code !errorlevel![0m"
                    )
                    ver >nul
                )
                popd
        )
    )
)
popd

pushd %~dp0
wsl ./generate-projects-linux.sh --clang
popd
if %errorlevel% neq 0 (
    call :WriteRed "Error: Failed to run generate-projects-linux.sh --clang."
    exit /b 1
)

pushd %~dp0..\build\gmake2 || (call :WriteRed "Error: Failed to navigate to build/gmake2" && exit /b 1)

for %%c in (debug release) do (
    for %%p in (x86 x64) do (
        wsl make clean config=%%c_%%p
        wsl make config=%%c_%%p
        if !errorlevel! neq 0 (
            set "SUMMARY=!SUMMARY! !LF![31mgmake2 - Linux clang %%c %%p: FAILED to build[0m"
            set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!pushd %~dp0!LF!wsl ./generate-projects-linux.sh --clang!LF!popd!LF!pushd %~dp0..\build\gmake2!LF!wsl make config=%%c_%%p!LF!popd"
            set /a ERROR_COUNT=!ERROR_COUNT!+1
        ) else (
            set "SUMMARY=!SUMMARY! !LF![32mgmake2 - Linux clang %%c %%p: built[0m"
                pushd %~dp0..\build\gmake2\bin\%%c_%%p
                for %%e in (*) do (
                    wsl "./%%e"
                    if !errorlevel! neq 0 (
                        set "SUMMARY=!SUMMARY! !LF![31m%%e (Linux clang %%c %%p) Exited with code !errorlevel![0m"
                        set "SUMMARY=!SUMMARY! !LF![36mcommands:[0m!LF!pushd %~dp0..\build\gmake2\bin\%%c_%%p!LF!wsl ^"%%e^"!LF!popd"
                        set /a ERROR_COUNT=!ERROR_COUNT!+1
                    ) else (
                        set "SUMMARY=!SUMMARY! !LF![32m%%e (Linux clang %%c %%p) Exited with code !errorlevel![0m"
                    )
                    ver >nul
                )
                popd
        )
    )
)
popd

@echo.
@echo %SUMMARY%
if %ERROR_COUNT% neq 0 (
    echo [31m%ERROR_COUNT% errors encountered[0m
) else (
    echo [32m%ERROR_COUNT% errors encountered[0m
)