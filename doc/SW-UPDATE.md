# PIXELIX
![PIXELIX](./images/LogoBlack.png)

- [PIXELIX](#pixelix)
- [Software Update](#software-update)
  - [Update via USB](#update-via-usb)
  - [Update over-the-air via espota](#update-over-the-air-via-espota)
  - [Update over-the-air via browser](#update-over-the-air-via-browser)

# Software Update

## Update via USB
Set the following in the _platformio.ini_ configuration file:
* Set _upload_protocol_ to _esptool_.
* Clear _upload_port_.
* Clear _upload_flags_.

Build and upload the software via _Project Tasks -> General -> Upload All_ and the filesystem via _Project Tasks -> env:esp32doit-devkit-v1 -> Platform -> Upload File System Image_.

## Update over-the-air via espota
Set the following in the _platformio.ini_ configuration file:
* Set _upload_protocol_ to _espota_.
* Set _upload_port_ to the device ip-address.
* Set _upload_flags_ to _--port=3232_ and set the password via _--auth=XXX_.

Build and upload the software via _Project Tasks -> General -> Upload All_ and the filesystem via _Project Tasks -> env:esp32doit-devkit-v1 -> Platform -> Upload File System Image_.

## Update over-the-air via browser
1. Build the software via _Project Tasks -> General -> Build All_
2. Build the filesystem via _Project Tasks -> env:esp32doit-devkit-v1 -> Platform -> Upload File System Image_.
3. Connect to the device.
4. Open browser add enter ip address of the device.
5. Jump to Update site.
6. Select firmware binary (firmware.bin) or filesystem binary (spiffs.bin) and click on upload.