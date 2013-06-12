EESchema Schematic File Version 2  date mié 12 jun 2013 13:54:19 CEST
LIBS:/home/carlosgs/GitRepos/GNBot/Electronics/KicadLibs/VREG/vreg,/home/carlosgs/GitRepos/GNBot/Electronics/KicadLibs/Arduino_Shield_Modules_for_KiCad_V3/arduino_shieldsNCL,power,device,transistors,conn,linear,regul,74xx,cmos4000,adc-dac,memory,xilinx,special,microcontrollers,dsp,microchip,analog_switches,motorola,texas,intel,audio,interface,digital-audio,philips,display,cypress,siliconi,opto,atmel,contrib,valves,./GNBoard.cache
EELAYER 24  0
EELAYER END
$Descr A4 11700 8267
Sheet 1 1
Title ""
Date "12 jun 2013"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	6750 1700 6750 1750
Wire Wire Line
	6750 1750 6950 1750
Wire Wire Line
	6750 2000 6750 1850
Wire Wire Line
	6750 1850 6950 1850
Wire Notes Line
	4400 650  650  650 
Wire Notes Line
	4400 650  4400 2350
Wire Notes Line
	4400 2350 650  2350
Wire Notes Line
	650  2350 650  650 
Wire Wire Line
	850  2200 850  2000
Wire Wire Line
	4000 2200 4000 2000
Connection ~ 2850 1500
Wire Wire Line
	2850 1600 2850 1500
Wire Wire Line
	2000 1950 2000 2200
Wire Wire Line
	850  1050 850  1600
Wire Wire Line
	1200 1300 850  1300
Connection ~ 850  1300
Wire Wire Line
	1800 1950 1800 2200
Wire Wire Line
	2600 1500 3100 1500
Wire Wire Line
	2850 2200 2850 2000
Wire Wire Line
	4000 1600 4000 1300
Wire Wire Line
	4000 1300 2600 1300
Connection ~ 4000 1500
Wire Wire Line
	4300 1300 4300 1500
Wire Wire Line
	4300 1500 3700 1500
Wire Wire Line
	6950 1950 6750 1950
Connection ~ 6750 1950
$Comp
L GND #PWR01
U 1 1 51B85F37
P 6750 2000
F 0 "#PWR01" H 6750 2000 30  0001 C CNN
F 1 "GND" H 6750 1930 30  0001 C CNN
	1    6750 2000
	1    0    0    -1  
$EndComp
Text Notes 1150 800  0    60   ~ 0
Voltage Regulator (5V)
$Comp
L +5V #PWR02
U 1 1 51B8535B
P 4300 1300
F 0 "#PWR02" H 4300 1390 20  0001 C CNN
F 1 "+5V" H 4300 1390 30  0000 C CNN
	1    4300 1300
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR03
U 1 1 51B852D0
P 4000 2200
F 0 "#PWR03" H 4000 2200 30  0001 C CNN
F 1 "GND" H 4000 2130 30  0001 C CNN
	1    4000 2200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 51B852CE
P 2850 2200
F 0 "#PWR04" H 2850 2200 30  0001 C CNN
F 1 "GND" H 2850 2130 30  0001 C CNN
	1    2850 2200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 51B852C1
P 2000 2200
F 0 "#PWR05" H 2000 2200 30  0001 C CNN
F 1 "GND" H 2000 2130 30  0001 C CNN
	1    2000 2200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 51B852BE
P 1800 2200
F 0 "#PWR06" H 1800 2200 30  0001 C CNN
F 1 "GND" H 1800 2130 30  0001 C CNN
	1    1800 2200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 51B85293
P 850 2200
F 0 "#PWR07" H 850 2200 30  0001 C CNN
F 1 "GND" H 850 2130 30  0001 C CNN
	1    850  2200
	1    0    0    -1  
$EndComp
$Comp
L DIODESCH D1
U 1 1 51B851F9
P 2850 1800
F 0 "D1" H 2850 1900 40  0000 C CNN
F 1 "1N5822" H 2850 1700 40  0000 C CNN
	1    2850 1800
	0    -1   -1   0   
$EndComp
$Comp
L +5V #PWR08
U 1 1 51B8519F
P 6750 1700
F 0 "#PWR08" H 6750 1790 20  0001 C CNN
F 1 "+5V" H 6750 1790 30  0000 C CNN
	1    6750 1700
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR09
U 1 1 51B85188
P 850 1050
F 0 "#PWR09" H 850 1000 20  0001 C CNN
F 1 "+BATT" H 850 1150 30  0000 C CNN
	1    850  1050
	1    0    0    -1  
$EndComp
$Comp
L CP1 C2
U 1 1 51B84E73
P 4000 1800
F 0 "C2" H 4050 1900 50  0000 L CNN
F 1 "100uF" H 4050 1700 50  0000 L CNN
	1    4000 1800
	1    0    0    -1  
$EndComp
$Comp
L CP1 C1
U 1 1 51B84E6F
P 850 1800
F 0 "C1" H 900 1900 50  0000 L CNN
F 1 "100uF" H 900 1700 50  0000 L CNN
	1    850  1800
	1    0    0    -1  
$EndComp
$Comp
L LM2576 U1
U 1 1 51B84E1C
P 1900 1400
F 0 "U1" H 1550 1650 60  0000 C CNN
F 1 "LM2576" H 2150 1650 60  0000 C CNN
F 4 "Texas Instruments" H 1900 1750 60  0001 C CNN "Manufacturer"
	1    1900 1400
	1    0    0    -1  
$EndComp
$Comp
L ARDUINO_MEGA_SHIELD SHIELD1
U 1 1 51B84CE1
P 7950 3500
F 0 "SHIELD1" H 7550 6000 60  0000 C CNN
F 1 "ARDUINO_MEGA_SHIELD" H 7850 800 60  0000 C CNN
	1    7950 3500
	1    0    0    -1  
$EndComp
$Comp
L INDUCTOR L1
U 1 1 51B84C2F
P 3400 1500
F 0 "L1" V 3350 1500 40  0000 C CNN
F 1 "100uH" V 3500 1500 40  0000 C CNN
	1    3400 1500
	0    1    1    0   
$EndComp
$EndSCHEMATC
