@echo off

set ADDR2LINE=%userprofile%\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-addr2line.exe
set FIRMWARE=.pio\build\adafruit_feather_esp32_v2-LED-32x8\firmware.elf

if "" == "%1" (
    echo "Usage: %0 <addresses>"
    goto end
)

%ADDR2LINE% -a -e %FIRMWARE% -p -f -C -a %*

:end
