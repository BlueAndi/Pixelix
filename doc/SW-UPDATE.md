# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Software Update](#software-update)
  - [Update via USB](#update-via-usb)
  - [Update via OTA (over-the-air)](#update-via-ota-over-the-air)
  - [Update via browser](#update-via-browser)

# Software Update

## Update via USB
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar (click on the head of the ant in the left column).
3. _Project Tasks -> env:esp32doit_devkit-v1-**usb** -> General -> Build_
4. _Project Tasks -> env:esp32doit_devkit-v1-**usb** -> General -> Upload_
5. _Project Tasks -> env:esp32doit_devkit-v1-**usb** -> Platform -> Build Filesystem Image_
6. _Project Tasks -> env:esp32doit_devkit-v1-**usb** -> Platform -> Upload Filesystem Image_

## Update via OTA (over-the-air)
1. Load workspace in VSCode.
2. If necessary, change the following parameters in the _platform.ini_ configuration file:
   1. Set _upload_port_ to the device ip-address.
   2. Set _upload_flags_ to _--port=3232_ and set the password via _--auth=XXX_.
3. Change to PlatformIO toolbar (click on the head of the ant in the left column).
4. _Project Tasks -> env:esp32doit_devkit-v1-**ota** -> General -> Build_
5. _Project Tasks -> env:esp32doit_devkit-v1-**ota** -> General -> Upload_
6. _Project Tasks -> env:esp32doit_devkit-v1-**ota** -> Platform -> Build Filesystem Image_
7. _Project Tasks -> env:esp32doit_devkit-v1-**ota** -> Platform -> Upload Filesystem Image_

## Update via browser
1. Build the software via _Project Tasks -> General -> Build All_
2. Build the filesystem via _Project Tasks -> env:esp32doit-devkit-v1-**usb** -> Platform -> Build File System Image_.
3. Now in the _.pio/build/esp32doit-devkit-v1-usb_ folder there are two important files:
   1. The software for the device: _firmware.bin_
   2. The prebuilt filesystem for the device: _spiffs.bin_
4. Open browser add enter ip address of the device.
5. Jump to Update site.
6. Select firmware binary (_firmware.bin_) or filesystem binary (_spiffs.bin_) and click on upload button.

