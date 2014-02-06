PCBNEW-LibModule-V1  jue 06 feb 2014 11:55:08 CET
# encoding utf-8
Units mm
$INDEX
switch_power_custom
$EndINDEX
$MODULE switch_power_custom
Po 0 0 0 15 51BF52B2 00000000 ~~
Li switch_power_custom
Kw CONN DEV
Sc 0
AR /51B8A3AE
Op 0 0 0
T0 0 2 1.016 1.016 0 0.254 N V 21 N "SW1"
T1 0 -2.54 1.016 1.016 0 0.254 N I 21 N "PowerSwitchCustom"
DS 4 -1 -4 -1 0.381 21
DS 4 -1 4 1 0.381 21
DS 4 1 -4 1 0.381 21
DS -4 1 -4 -1 0.381 21
$PAD
Sh "1" O 1.5 2 0 0 0
Dr 0.7 0 0
At STD N 00E0FFFF
Ne 2 "N-0000015"
Po -2 0
$EndPAD
$PAD
Sh "2" O 1.5 2 0 0 0
Dr 0.7 0 0
At STD N 00E0FFFF
Ne 1 "+BATT"
Po 0 0
$EndPAD
$PAD
Sh "3" O 1.5 2 0 0 0
Dr 0.7 0 0
At STD N 00E0FFFF
Ne 0 ""
Po 2 0
$EndPAD
$PAD
Sh "" O 1.5 2.5 0 0 0
Dr 1 0 0
At STD N 00E0FFFF
Ne 0 ""
Po 4 0
$EndPAD
$PAD
Sh "" O 1.5 2.5 0 0 0
Dr 1 0 0
At STD N 00E0FFFF
Ne 0 ""
Po -4 0
$EndPAD
$SHAPE3D
Na "device/switch_slide_straight_terminal.wrl"
Sc 0.33 0.33 0.33
Of 0 0 0
Ro 0 0 0
$EndSHAPE3D
$EndMODULE switch_power_custom
$EndLIBRARY
