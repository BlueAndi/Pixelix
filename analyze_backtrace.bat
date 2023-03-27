@echo off

set ADDR2LINE=%userprofile%\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-addr2line.exe
set FIRMWARE=.pio\build\esp32doit-devkit-v1\firmware.elf

if "" == "%1" (
    echo "Usage: %0 <addresses>"
    goto end
)

%ADDR2LINE% -a -e %FIRMWARE% -p -f -C -a %*

:end
