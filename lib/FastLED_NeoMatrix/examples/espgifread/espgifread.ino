// http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system
// esp8266com/esp8266/libraries/SD/src/File.cpp
#include <FS.h>

#include "config.h"

#include "GifPlayer.h"
GifPlayer gifPlayer;

#if defined(ESP8266) || defined(ESP32)
extern "C" {
#include "user_interface.h"
}
#else
#error This code uses SPIFFS on ESPxx chips
#endif


void display_resolution() {
    matrix->setTextSize(1);
    // not wide enough;
    if (mw<16) return;
    matrix->clear();
    // Font is 5x7, if display is too small
    // 8 can only display 1 char
    // 16 can almost display 3 chars
    // 24 can display 4 chars
    // 32 can display 5 chars
    matrix->setCursor(0, 0);
    matrix->setTextColor(matrix->Color(255,0,0));
    if (mw>10) matrix->print(mw/10);
    matrix->setTextColor(matrix->Color(255,128,0)); 
    matrix->print(mw % 10);
    matrix->setTextColor(matrix->Color(0,255,0));
    matrix->print('x');
    // not wide enough to print 5 chars, go to next line
    if (mw<25) {
	if (mh==13) matrix->setCursor(6, 7);
	else if (mh>=13) {
	    matrix->setCursor(mw-12, 8);
	} else {
	    // we're not tall enough either, so we wait and display
	    // the 2nd value on top.
	    matrix->show();
	    matrix->clear();
	    matrix->setCursor(mw-11, 0);
	}   
    }
    matrix->setTextColor(matrix->Color(0,255,128)); 
    matrix->print(mh/10);
    matrix->setTextColor(matrix->Color(0,128,255));  
    matrix->print(mh % 10);
    // enough room for a 2nd line
    if ((mw>25 && mh >14) || mh>16) {
	matrix->setCursor(0, mh-7);
	matrix->setTextColor(matrix->Color(0,255,255)); 
	if (mw>16) matrix->print('*');
	matrix->setTextColor(matrix->Color(255,0,0)); 
	matrix->print('R');
	matrix->setTextColor(matrix->Color(0,255,0));
	matrix->print('G');
	matrix->setTextColor(matrix->Color(0,0,255)); 
	matrix->print("B");
	matrix->setTextColor(matrix->Color(255,255,0)); 
	// this one could be displayed off screen, but we don't care :)
	matrix->print("*");
    }
    
    matrix->show();
}


void loop() {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
	uint32_t result;

	String fileName = dir.fileName();
	//fileName = "/gifs/32anim_balls.gif";
	//fileName = "/gifs/32anim_dance.gif";
	//fileName = "/gifs/32anim_flower.gif";
	//fileName = "/gifs/32anim_photon.gif";

	Serial.print("Reading ");
	Serial.println(fileName);

	File imageFile = SPIFFS.open(fileName, "r");
	if (!imageFile) {
	    Serial.println("Failed to open");
	    return;
	}
	
	gifPlayer.setFile(imageFile);

	for (uint8_t c=0; c<10; c++) {
	    if (!gifPlayer.parseGifHeader()) {
	      imageFile.close();
	      Serial.println("No gif header");
	      return;
	    }

	    matrix->clear();
	    gifPlayer.parseLogicalScreenDescriptor();
	    gifPlayer.parseGlobalColorTable();
	    Serial.println("Processing gif");
	    do {
		gifPlayer.drawFrame();
		result = gifPlayer.drawFrame();
		matrix->show();
		delay(50);
	    } while (result != ERROR_FINISHED);
	    imageFile.seek(0);
	}

	Serial.println("Gif finished");
	imageFile.close();
	delay(1000);
    }
}

void setup() {
    Serial.begin(115200);

#ifdef ESP8266
    Serial.println();
    Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
    Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
    Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
    Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
    Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
    Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
    Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
    Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
    Serial.println();
#endif

    SPIFFS.begin();
    {
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
	    String fileName = dir.fileName();
	    size_t fileSize = dir.fileSize();
	    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
	}
	Serial.printf("\n");
    }

    FastLED.addLeds<WS2811_PORTA,3>(matrixleds, NUMMATRIX/3).setCorrection(TypicalLEDStrip);
    matrix->begin();
    matrix->setBrightness(matrix_brightness);
    display_resolution();
}

// vim:sts=4:sw=4
