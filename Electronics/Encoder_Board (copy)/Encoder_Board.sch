EESchema Schematic File Version 2  date Mon 17 Jun 2013 16:27:06 CEST
LIBS:/home/carlosgs/GitRepos/GNBot/Electronics/KicadLibs/CNY0_HERDproject/cny70,power,device,transistors,conn,linear,regul,74xx,cmos4000,adc-dac,memory,xilinx,special,microcontrollers,dsp,microchip,analog_switches,motorola,texas,intel,audio,interface,digital-audio,philips,display,cypress,siliconi,opto,atmel,contrib,valves,./Encoder_Board.cache
EELAYER 24  0
EELAYER END
$Descr A4 11700 8267
Sheet 1 1
Title ""
Date "17 jun 2013"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	4550 1900 4550 1850
Wire Wire Line
	4550 1850 4750 1850
Wire Wire Line
	4650 1550 4650 1650
Wire Wire Line
	4650 1650 4750 1650
Wire Wire Line
	2750 1800 2650 1800
Wire Wire Line
	2650 1800 2650 2000
Wire Wire Line
	2450 2000 2450 2100
Wire Wire Line
	2650 1400 2650 1600
Wire Wire Line
	1950 1400 1950 1600
Wire Wire Line
	3150 2000 3150 2150
Wire Wire Line
	1950 1800 1950 2000
Wire Wire Line
	4750 1750 4550 1750
$Comp
L CONN_3 K1
U 1 1 51BF17EB
P 5100 1750
F 0 "K1" V 5050 1750 50  0000 C CNN
F 1 "CONN_3" V 5150 1750 40  0000 C CNN
	1    5100 1750
	1    0    0    -1  
$EndComp
Text GLabel 2750 1800 2    60   Output ~ 0
IR1_V
$Comp
L GND #PWR01
U 1 1 51BF1253
P 3150 2150
F 0 "#PWR01" H 3150 2150 30  0001 C CNN
F 1 "GND" H 3150 2080 30  0001 C CNN
	1    3150 2150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 51BF1252
P 2450 2100
F 0 "#PWR02" H 2450 2100 30  0001 C CNN
F 1 "GND" H 2450 2030 30  0001 C CNN
	1    2450 2100
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR03
U 1 1 51BF1251
P 2650 1400
F 0 "#PWR03" H 2650 1490 20  0001 C CNN
F 1 "+5V" H 2650 1490 30  0000 C CNN
	1    2650 1400
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR04
U 1 1 51BF1250
P 1950 1400
F 0 "#PWR04" H 1950 1490 20  0001 C CNN
F 1 "+5V" H 1950 1490 30  0000 C CNN
	1    1950 1400
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 51BF124F
P 2200 2000
F 0 "R1" V 2280 2000 50  0000 C CNN
F 1 "R_led_s1" V 2200 2000 50  0000 C CNN
	1    2200 2000
	0    1    1    0   
$EndComp
$Comp
L R R3
U 1 1 51BF124E
P 2900 2000
F 0 "R3" V 2980 2000 50  0000 C CNN
F 1 "R_led_s1" V 2900 2000 50  0000 C CNN
	1    2900 2000
	0    1    1    0   
$EndComp
$Comp
L CNY70 ?
U 1 1 51BF124D
P 2350 1700
AR Path="/51BEF15D" Ref="?"  Part="1" 
AR Path="/51BF124D" Ref="11"  Part="1" 
F 0 "11" H 2300 1550 60  0000 C CNN
F 1 "CNY70_s1" H 2300 1850 60  0000 C CNN
	1    2350 1700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 51BF1143
P 4550 1900
F 0 "#PWR05" H 4550 1900 30  0001 C CNN
F 1 "GND" H 4550 1830 30  0001 C CNN
	1    4550 1900
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR06
U 1 1 51BF113E
P 4550 1750
F 0 "#PWR06" H 4550 1840 20  0001 C CNN
F 1 "+5V" H 4550 1840 30  0000 C CNN
	1    4550 1750
	1    0    0    -1  
$EndComp
Text GLabel 4650 1550 0    60   Input ~ 0
IR1_V
$EndSCHEMATC
