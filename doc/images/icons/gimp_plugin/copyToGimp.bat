@echo off
set "PLUGIN_DIR=%APPDATA%\GIMP\3.0\plug-ins\gimp_create_preview"
if not exist "%PLUGIN_DIR%" (
    mkdir "%PLUGIN_DIR%"
    if errorlevel 1 exit /b 1
)
copy /y "%~dp0gimp_create_preview.py" "%PLUGIN_DIR%\gimp_create_preview.py"
if errorlevel 1 exit /b 1
