@echo off
set PLUGIN_FOLDER=..\..\..\..\lib\OpenWeatherPlugin

del %PLUGIN_FOLDER%\web\icon8x8\*.* /q
xcopy icon8x8\*.* %PLUGIN_FOLDER%\web\icon8x8 /y

rem del %PLUGIN_FOLDER%\web\icon16x16\*.* /q
rem xcopy icon16x16\*.* %PLUGIN_FOLDER%\web\icon16x16 /y
