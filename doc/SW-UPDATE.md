# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Update The Software <!-- omit in toc -->

- [Purpose](#purpose)
- [Update via USB](#update-via-usb)
- [Update via OTA (over-the-air)](#update-via-ota-over-the-air)
- [Update via browser](#update-via-browser)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Purpose
The software can be uploaded/updated in three different ways.
Note, replace the ```<choose-your-board>``` below in the description with your board.

If your board is not available out of the box, have a look for the [supported boards by platformio](https://docs.platformio.org/en/latest/platforms/espressif32.html#boards). Copy a board configuration block in the ```platformio.ini``` and overwrite the board configuration (```board = ...```) by using the right board id from [supported boards by platformio](https://docs.platformio.org/en/latest/platforms/espressif32.html#boards).

The update consists of two parts:
* The software.
* The filesystem.

# Update via USB
Steps:
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar (click on the head of the ant in the left column).
3. Software:
   1. _Project Tasks -> env:```<choose-your-board>```-**usb** -> General -> Build_
   2. _Project Tasks -> env:```<choose-your-board>```-**usb** -> General -> Upload_
4. Filesystem:
   1. _Project Tasks -> env:```<choose-your-board>```-**usb** -> Platform -> Build Filesystem Image_
   2. _Project Tasks -> env:```<choose-your-board>```-**usb** -> Platform -> Upload Filesystem Image_

Example:

![VSCodeUpdate](./images/VSCodeUpdate.png)

# Update via OTA (over-the-air)
Preconditions:
* PIXELIX runs already on the target.
* No major version update.

Steps:
1. Load workspace in VSCode.
2. If necessary, change the following parameters in the ```platform.ini``` configuration file:
   1. Set _upload_port_ to the device ip-address.
   2. Set _upload_flags_ to ```--port=3232``` and set the password via ```--auth=XXX```.
3. Change to PlatformIO toolbar (click on the head of the ant in the left column).
4. Software:
   1. _Project Tasks -> env:```<choose-your-board>```-**ota** -> General -> Build_
   2. _Project Tasks -> env:```<choose-your-board>```-**ota** -> General -> Upload_
   3. Upload Utility appears in the task bar, use it to perform the OTA.
5. Filesystem:
   1. _Project Tasks -> env:```<choose-your-board>```-**ota** -> Platform -> Build Filesystem Image_
   2. _Project Tasks -> env:```<choose-your-board>```-**ota** -> Platform -> Upload Filesystem Image_
   3. Upload Utility appears in the task bar, use it to perform the OTA.

Example:

![VSCodeUpdateOta](./images/VSCodeUpdateOta.png)
![UploadUtility](./images/UploadUtility.png)

# Update via browser
Preconditions:
* PIXELIX runs already on the target.
* No major version update.

Steps:
1. Build the software via _Project Tasks -> General -> Build All_
2. Build the filesystem via _Project Tasks -> env:```<choose-your-board>```-**usb** -> Platform -> Build File System Image_.
3. Now in the ```.pio/build/<choose-your-board>```-**usb** folder there are two important files:
   1. The software for the device: ```firmware.bin```
   2. The prebuilt filesystem for the device: ```littlefs.bin``` (or ```spiffs.bin``` for Pixelix &lt; 6.x.x)
4. Open browser add enter ip address of the device.
5. Jump to Update site.
6. Select firmware binary (```firmware.bin```) or filesystem binary (```spiffs.bin```/```littlefs.bin```) and click on upload button.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
