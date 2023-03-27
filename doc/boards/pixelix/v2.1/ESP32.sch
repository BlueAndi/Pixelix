EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title "Pixelix"
Date "2022-05-27"
Rev "2.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	5500 1000 4700 1000
Wire Wire Line
	5500 1300 4700 1300
Wire Wire Line
	5500 1900 4700 1900
Wire Wire Line
	7900 1000 7100 1000
Wire Wire Line
	7900 2100 7100 2100
Wire Wire Line
	7900 2400 7100 2400
Wire Wire Line
	5300 900  5500 900 
Wire Wire Line
	4400 2700 3600 2700
Wire Wire Line
	7900 1100 7100 1100
Wire Wire Line
	5500 2300 4700 2300
Wire Wire Line
	5500 2100 4700 2100
Text Label 4700 1000 0    50   ~ 0
RESET
Text Label 4700 1300 0    50   ~ 0
LDR-ANALOG-IN
Text Label 4700 1900 0    50   ~ 0
LV-DATA-OUT
Text Label 4700 2100 0    50   ~ 0
MTDI
Text Label 4700 2300 0    50   ~ 0
MTCK
Text Label 3600 2700 0    50   ~ 0
VCC-PROTECTED
$Comp
L power:+3V3 #PWR0101
U 1 1 629189B1
P 5300 900
F 0 "#PWR0101" H 5300 750 50  0001 C CNN
F 1 "+3V3" H 5315 1073 50  0000 C CNN
F 2 "" H 5300 900 50  0001 C CNN
F 3 "" H 5300 900 50  0001 C CNN
	1    5300 900 
	1    0    0    -1  
$EndComp
Text Label 7900 2400 2    50   ~ 0
MTDO
Text Label 7900 2100 2    50   ~ 0
USER-BUTTON
Text Label 7900 1000 2    50   ~ 0
SDA
Text Label 7900 1100 2    50   ~ 0
SCL
Text Label 4700 2000 0    50   ~ 0
MTMS
Wire Wire Line
	5500 2000 4700 2000
Wire Wire Line
	5500 1800 4700 1800
Wire Wire Line
	5500 1700 4700 1700
Text Label 4700 1800 0    50   ~ 0
I2S_CLK
Text Label 4700 1700 0    50   ~ 0
I2S_WS
Wire Wire Line
	5500 1600 4700 1600
Text Label 4700 1600 0    50   ~ 0
I2S_Data_IN
NoConn ~ 5500 1100
NoConn ~ 5500 1200
NoConn ~ 5500 1400
NoConn ~ 5500 1500
NoConn ~ 5500 2400
NoConn ~ 5500 2500
NoConn ~ 5500 2600
NoConn ~ 7100 2700
NoConn ~ 7100 2600
NoConn ~ 7100 2500
NoConn ~ 7100 2300
NoConn ~ 7100 2200
NoConn ~ 7100 2000
NoConn ~ 7100 1900
NoConn ~ 7100 1800
NoConn ~ 7100 1700
NoConn ~ 7100 1600
NoConn ~ 7100 1400
NoConn ~ 7100 1300
NoConn ~ 7100 1200
$Comp
L MCU_Module_new:Adafruit_Feather_HUZZAH32_ESP32 A1
U 1 1 629189DA
P 6350 5000
F 0 "A1" H 6000 6200 50  0000 C CNN
F 1 "Adafruit_Feather_HUZZAH32_ESP32" H 7400 6200 50  0000 C CNN
F 2 "Module:Adafruit_Feather" H 6450 3650 50  0001 L CNN
F 3 "https://cdn-learn.adafruit.com/downloads/pdf/adafruit-huzzah32-esp32-feather.pdf" H 6350 3800 50  0001 C CNN
	1    6350 5000
	1    0    0    -1  
$EndComp
Text HLabel 1450 1100 0    50   Input ~ 0
RESET
Text HLabel 1450 1200 0    50   BiDi ~ 0
LDR-ANALOG-IN
Text HLabel 1450 1300 0    50   BiDi ~ 0
I2S_Data_IN
Text HLabel 1450 1400 0    50   BiDi ~ 0
I2S_WS
Text HLabel 1450 1500 0    50   BiDi ~ 0
I2S_CLK
Text HLabel 1450 1600 0    50   BiDi ~ 0
LV-DATA-OUT
Text HLabel 1450 1700 0    50   BiDi ~ 0
MTMS
Text HLabel 1450 1800 0    50   BiDi ~ 0
MTDI
Text HLabel 1450 1900 0    50   BiDi ~ 0
MTCK
Text HLabel 1450 2000 0    50   UnSpc ~ 0
VCC-PROTECTED
Text HLabel 1450 2100 0    50   BiDi ~ 0
SDA
Text HLabel 1450 2200 0    50   BiDi ~ 0
SCL
Text HLabel 1450 2300 0    50   Input ~ 0
USER-BUTTON
Text HLabel 1450 2400 0    50   BiDi ~ 0
MTDO
Wire Wire Line
	2250 1100 1450 1100
Wire Wire Line
	2250 1200 1450 1200
Wire Wire Line
	2250 1600 1450 1600
Wire Wire Line
	2250 2000 1450 2000
Wire Wire Line
	2250 1900 1450 1900
Wire Wire Line
	2250 1800 1450 1800
Text Label 2250 1100 2    50   ~ 0
RESET
Text Label 2250 1200 2    50   ~ 0
LDR-ANALOG-IN
Text Label 2250 1600 2    50   ~ 0
LV-DATA-OUT
Text Label 2250 1800 2    50   ~ 0
MTDI
Text Label 2250 1900 2    50   ~ 0
MTCK
Text Label 2250 2000 2    50   ~ 0
VCC-PROTECTED
Text Label 2250 1700 2    50   ~ 0
MTMS
Wire Wire Line
	2250 1700 1450 1700
Wire Wire Line
	2250 1500 1450 1500
Wire Wire Line
	2250 1400 1450 1400
Text Label 2250 1500 2    50   ~ 0
I2S_CLK
Text Label 2250 1400 2    50   ~ 0
I2S_WS
Wire Wire Line
	2250 1300 1450 1300
Text Label 2250 1300 2    50   ~ 0
I2S_Data_IN
Wire Wire Line
	2250 2100 1450 2100
Wire Wire Line
	2250 2300 1450 2300
Wire Wire Line
	2250 2400 1450 2400
Wire Wire Line
	2250 2200 1450 2200
Text Label 2250 2400 2    50   ~ 0
MTDO
Text Label 2250 2300 2    50   ~ 0
USER-BUTTON
Text Label 2250 2100 2    50   ~ 0
SDA
Text Label 2250 2200 2    50   ~ 0
SCL
Wire Wire Line
	6450 3700 6450 3800
$Comp
L power:+3V3 #PWR0102
U 1 1 6296A234
P 6450 3700
F 0 "#PWR0102" H 6450 3550 50  0001 C CNN
F 1 "+3V3" H 6465 3873 50  0000 C CNN
F 2 "" H 6450 3700 50  0001 C CNN
F 3 "" H 6450 3700 50  0001 C CNN
	1    6450 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	7650 4500 6850 4500
Text Label 7650 4500 2    50   ~ 0
RESET
Wire Wire Line
	6850 5000 7650 5000
Text Label 7650 5000 2    50   ~ 0
LDR-ANALOG-IN
Wire Wire Line
	5850 4500 5050 4500
Text Label 5050 4500 0    50   ~ 0
I2S_Data_IN
Wire Wire Line
	5050 4600 5850 4600
Wire Wire Line
	3600 3850 4400 3850
Wire Wire Line
	5050 4800 5850 4800
Wire Wire Line
	5050 4700 5850 4700
Text Label 5050 4600 0    50   ~ 0
LV-DATA-OUT
Text Label 5050 4700 0    50   ~ 0
MTDI
Text Label 5050 4800 0    50   ~ 0
MTCK
Text Label 3600 3850 0    50   ~ 0
VCC-PROTECTED
Text Label 5050 4200 0    50   ~ 0
MTMS
Wire Wire Line
	5050 4200 5850 4200
Wire Wire Line
	7650 5200 6850 5200
Wire Wire Line
	7650 5100 6850 5100
Text Label 7650 5200 2    50   ~ 0
I2S_CLK
Text Label 7650 5100 2    50   ~ 0
I2S_WS
Wire Wire Line
	5050 5800 5850 5800
Wire Wire Line
	7650 5300 6850 5300
Wire Wire Line
	5050 4400 5850 4400
Wire Wire Line
	5050 5900 5850 5900
Text Label 5050 4400 0    50   ~ 0
MTDO
Text Label 7650 5300 2    50   ~ 0
USER-BUTTON
Text Label 5050 5800 0    50   ~ 0
SDA
Text Label 5050 5900 0    50   ~ 0
SCL
$Comp
L ESP32-DEVKITC-32D_new:ESP32-DEVKITC-32D U?
U 1 1 6291899A
P 6300 1800
AR Path="/6291899A" Ref="U?"  Part="1" 
AR Path="/62912066/6291899A" Ref="U1"  Part="1" 
F 0 "U1" H 6300 2967 50  0000 C CNN
F 1 "ESP32-DEVKITC-32D" H 6300 2876 50  0000 C CNN
F 2 "footprints:MODULE_ESP32-DEVKITC-32D" H 6300 1800 50  0001 L BNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf" H 6300 1800 50  0001 L BNN
F 4 "Espressif Systems" H 6300 1800 50  0001 C CNN "Manufacturer"
	1    6300 1800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 629954B3
P 8250 1550
F 0 "#PWR0103" H 8250 1300 50  0001 C CNN
F 1 "GND" H 8255 1377 50  0000 C CNN
F 2 "" H 8250 1550 50  0001 C CNN
F 3 "" H 8250 1550 50  0001 C CNN
	1    8250 1550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 629966B5
P 8250 950
F 0 "#PWR0104" H 8250 700 50  0001 C CNN
F 1 "GND" H 8255 777 50  0000 C CNN
F 2 "" H 8250 950 50  0001 C CNN
F 3 "" H 8250 950 50  0001 C CNN
	1    8250 950 
	1    0    0    -1  
$EndComp
Wire Wire Line
	8250 950  8250 900 
Wire Wire Line
	8250 900  7100 900 
Wire Wire Line
	8250 1550 8250 1500
Wire Wire Line
	8250 1500 7100 1500
Wire Wire Line
	4300 2200 5500 2200
Wire Wire Line
	4300 2250 4300 2200
$Comp
L power:GND #PWR0105
U 1 1 62994B92
P 4300 2250
F 0 "#PWR0105" H 4300 2000 50  0001 C CNN
F 1 "GND" H 4305 2077 50  0000 C CNN
F 2 "" H 4300 2250 50  0001 C CNN
F 3 "" H 4300 2250 50  0001 C CNN
	1    4300 2250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 6299D8B6
P 6350 6450
F 0 "#PWR0106" H 6350 6200 50  0001 C CNN
F 1 "GND" H 6355 6277 50  0000 C CNN
F 2 "" H 6350 6450 50  0001 C CNN
F 3 "" H 6350 6450 50  0001 C CNN
	1    6350 6450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 6300 6350 6450
NoConn ~ 6850 4200
NoConn ~ 5850 5600
NoConn ~ 5850 5500
NoConn ~ 5850 5300
NoConn ~ 5850 5200
NoConn ~ 5850 5100
NoConn ~ 5850 4900
NoConn ~ 5850 4300
$EndSCHEMATC
