EESchema Schematic File Version 2  date vie 14 jun 2013 17:25:18 CEST
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
EELAYER 24 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "14 jun 2013"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L CONN_5 P5
U 1 1 51BB3492
P 3700 1850
F 0 "P5" V 3650 1850 50  0000 C CNN
F 1 "LDR_conn" V 3750 1850 50  0000 C CNN
F 2 "~" H 3700 1850 60  0000 C CNN
F 3 "~" H 3700 1850 60  0000 C CNN
	1    3700 1850
	1    0    0    1   
$EndComp
$Comp
L CONN_2 P1
U 1 1 51BB34A1
P 900 1750
F 0 "P1" V 850 1750 40  0000 C CNN
F 1 "LDR1" V 950 1750 40  0000 C CNN
F 2 "~" H 900 1750 60  0000 C CNN
F 3 "~" H 900 1750 60  0000 C CNN
	1    900  1750
	-1   0    0    1   
$EndComp
$Comp
L CONN_2 P2
U 1 1 51BB34E2
P 1500 1850
F 0 "P2" V 1450 1850 40  0000 C CNN
F 1 "LDR2" V 1550 1850 40  0000 C CNN
F 2 "~" H 1500 1850 60  0000 C CNN
F 3 "~" H 1500 1850 60  0000 C CNN
	1    1500 1850
	-1   0    0    1   
$EndComp
$Comp
L CONN_2 P3
U 1 1 51BB34E8
P 2100 1950
F 0 "P3" V 2050 1950 40  0000 C CNN
F 1 "LDR3" V 2150 1950 40  0000 C CNN
F 2 "~" H 2100 1950 60  0000 C CNN
F 3 "~" H 2100 1950 60  0000 C CNN
	1    2100 1950
	-1   0    0    1   
$EndComp
$Comp
L CONN_2 P4
U 1 1 51BB34EE
P 2700 2050
F 0 "P4" V 2650 2050 40  0000 C CNN
F 1 "LDR4" V 2750 2050 40  0000 C CNN
F 2 "~" H 2700 2050 60  0000 C CNN
F 3 "~" H 2700 2050 60  0000 C CNN
	1    2700 2050
	-1   0    0    1   
$EndComp
Wire Wire Line
	3300 1950 3050 1950
Wire Wire Line
	3300 1850 2450 1850
Wire Wire Line
	3300 1750 1850 1750
Wire Wire Line
	3300 1650 1250 1650
Wire Wire Line
	1250 1850 1250 2400
Wire Wire Line
	1250 2400 3300 2400
Wire Wire Line
	3300 2400 3300 2050
Wire Wire Line
	1850 1950 1850 2400
Connection ~ 1850 2400
Wire Wire Line
	2450 2050 2450 2400
Connection ~ 2450 2400
Wire Wire Line
	3050 2150 3050 2400
Connection ~ 3050 2400
$Comp
L GND #PWR1
U 1 1 51BB3546
P 2700 2600
F 0 "#PWR1" H 2700 2600 30  0001 C CNN
F 1 "GND" H 2700 2530 30  0001 C CNN
F 2 "" H 2700 2600 60  0000 C CNN
F 3 "" H 2700 2600 60  0000 C CNN
	1    2700 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 2400 2700 2600
Connection ~ 2700 2400
$EndSCHEMATC
