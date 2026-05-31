@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~1"
set "OUT_DIR=%~2"

if "%PROJECT_DIR%"=="" goto :usage
if "%OUT_DIR%"=="" goto :usage

if "%PROJECT_DIR:~-1%"=="\" set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"
if "%OUT_DIR:~-1%"=="\" set "OUT_DIR=%OUT_DIR:~0,-1%"

if not exist "%OUT_DIR%\GUI" mkdir "%OUT_DIR%\GUI"
xcopy /E /I /Y /Q "%PROJECT_DIR%\GUI\*" "%OUT_DIR%\GUI\" >nul

if exist "%PROJECT_DIR%\GUI\Background\man_hinh_nen_menu.jpg" (
    copy /Y "%PROJECT_DIR%\GUI\Background\man_hinh_nen_menu.jpg" "%OUT_DIR%\menu_bg.jpg" >nul
)

if exist "%PROJECT_DIR%\fonts" (
    if not exist "%OUT_DIR%\fonts" mkdir "%OUT_DIR%\fonts"
    xcopy /E /I /Y /Q "%PROJECT_DIR%\fonts\*" "%OUT_DIR%\fonts\" >nul
)

exit /b 0

:usage
echo Usage: deploy_assets.cmd ProjectDir OutDir
exit /b 1
