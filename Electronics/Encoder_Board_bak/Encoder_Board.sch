EESchema Schematic File Version 2  date lun 17 jun 2013 16:05:49 CEST
LIBS:/home/carlosgs/GitRepos/GNBot/Electronics/KicadLibs/CNY0_HERDproject/cny70,power,device,transistors,conn,linear,regul,74xx,cmos4000,adc-dac,memory,xilinx,special,microcontrollers,dsp,microchip,analog_switches,motorola,texas,intel,audio,interface,digital-audio,philips,display,cypress,siliconi,opto,atmel,contrib,valves
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
AR Path="/51BF124D" Ref="1"  Part="1" 
F 0 "1" H 2300 1550 60  0000 C CNN
F 1 "CNY70_s1" H 2300 1850 60  0000 C CNN
	1    2350 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2750 2850 2650 2850
Wire Wire Line
	2650 2850 2650 3050
Wire Wire Line
	2450 3050 2450 3150
Wire Wire Line
	2650 2450 2650 2650
Wire Wire Line
	4750 1650 4650 1650
Wire Wire Line
	4600 1850 4750 1850
Wire Wire Line
	4750 1950 4600 1950
Wire Wire Line
	4750 1750 4650 1750
Wire Wire Line
	1950 2450 1950 2650
Wire Wire Line
	3150 3050 3150 3200
Wire Wire Line
	1950 2850 1950 3050
Text GLabel 2750 2850 2    60   Output ~ 0
IR2_V
$Comp
L GND #PWR05
U 1 1 51BF11B0
P 3150 3200
F 0 "#PWR05" H 3150 3200 30  0001 C CNN
F 1 "GND" H 3150 3130 30  0001 C CNN
	1    3150 3200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR06
U 1 1 51BF11AE
P 2450 3150
F 0 "#PWR06" H 2450 3150 30  0001 C CNN
F 1 "GND" H 2450 3080 30  0001 C CNN
	1    2450 3150
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR07
U 1 1 51BF11A7
P 2650 2450
F 0 "#PWR07" H 2650 2540 20  0001 C CNN
F 1 "+5V" H 2650 2540 30  0000 C CNN
	1    2650 2450
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR08
U 1 1 51BF1168
P 1950 2450
F 0 "#PWR08" H 1950 2540 20  0001 C CNN
F 1 "+5V" H 1950 2540 30  0000 C CNN
	1    1950 2450
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR09
U 1 1 51BF1143
P 4650 1750
F 0 "#PWR09" H 4650 1750 30  0001 C CNN
F 1 "GND" H 4650 1680 30  0001 C CNN
	1    4650 1750
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR010
U 1 1 51BF113E
P 4650 1650
F 0 "#PWR010" H 4650 1740 20  0001 C CNN
F 1 "+5V" H 4650 1740 30  0000 C CNN
	1    4650 1650
	1    0    0    -1  
$EndComp
Text GLabel 4600 1950 0    60   Input ~ 0
IR2_V
Text GLabel 4600 1850 0    60   Input ~ 0
IR1_V
$Comp
L CONN_4 P1
U 1 1 51BF10CD
P 5100 1800
F 0 "P1" V 5050 1800 50  0000 C CNN
F 1 "CONN_4" V 5150 1800 50  0000 C CNN
	1    5100 1800
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 51BEF175
P 2200 3050
F 0 "R2" V 2280 3050 50  0000 C CNN
F 1 "R_led_s2" V 2200 3050 50  0000 C CNN
	1    2200 3050
	0    1    1    0   
$EndComp
$Comp
L R R4
U 1 1 51BEF170
P 2900 3050
F 0 "R4" V 2980 3050 50  0000 C CNN
F 1 "R_led_s2" V 2900 3050 50  0000 C CNN
	1    2900 3050
	0    1    1    0   
$EndComp
$Comp
L CNY70 ??1
U 1 1 51BEF15D
P 2350 2750
F 0 "??1" H 2300 2600 60  0000 C CNN
F 1 "CNY70_s2" H 2300 2900 60  0000 C CNN
	1    2350 2750
	1    0    0    -1  
$EndComp
$EndSCHEMATC
