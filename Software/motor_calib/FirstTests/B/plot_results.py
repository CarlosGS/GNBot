#!/usr/bin/python
# encoding: utf-8

from helper import *

from pylab import *

import numpy as np


# https://klassenresearch.orbs.com/Plotting+with+Python
from matplotlib import rc
# Make use of TeX﻿
rc('text',usetex=True)
# Change all fonts to 'Computer Modern'
rc('font',**{'family':'serif','serif':['Computer Modern']})

f, ax = subplots(1)

data = loadFromFile("","motorCalibLog_forwards.p")

print(data)

fw_avgSpeed = data['avgSpeed']
fw_avgLmotorInput = data['avgLmotorInput']
fw_avgRmotorInput = data['avgRmotorInput']


data = loadFromFile("","motorCalibLog_backwards.p")

print(data)

bw_avgSpeed = data['avgSpeed']
bw_avgLmotorInput = data['avgLmotorInput']
bw_avgRmotorInput = data['avgRmotorInput']


avgSpeed = fw_avgSpeed
avgSpeed.reverse()
avgSpeed.extend(bw_avgSpeed)

avgLmotorInput = fw_avgLmotorInput
avgLmotorInput.reverse()
avgLmotorInput.extend(bw_avgLmotorInput)

avgRmotorInput = fw_avgRmotorInput
avgRmotorInput.reverse()
avgRmotorInput.extend(bw_avgRmotorInput)

ax.plot(avgSpeed,avgLmotorInput,'-xg')
ax.plot(avgSpeed,avgRmotorInput,'-xg')
#ax.plot([avgSpeed[0],avgSpeed[-1]],[avgLmotorInput[0],avgLmotorInput[-1]],"g")
#ax.plot([avgSpeed[0],avgSpeed[-1]],[avgRmotorInput[0],avgRmotorInput[-1]],"g")
#ax.legend(["Left motor","Right motor","Ideal response"],loc="right")






data = loadFromFile("","motorCalibLog_forwards_asymetricL.p")

print(data)

fw_avgSpeed = data['avgSpeed']
fw_avgLmotorInput = data['avgLmotorInput']
fw_avgRmotorInput = data['avgRmotorInput']


data = loadFromFile("","motorCalibLog_backwards_asymetricL.p")

print(data)

bw_avgSpeed = data['avgSpeed']
bw_avgLmotorInput = data['avgLmotorInput']
bw_avgRmotorInput = data['avgRmotorInput']


avgSpeed = fw_avgSpeed
avgSpeed.reverse()
avgSpeed.extend(bw_avgSpeed)

avgLmotorInput = fw_avgLmotorInput
avgLmotorInput.reverse()
avgLmotorInput.extend(bw_avgLmotorInput)

avgRmotorInput = fw_avgRmotorInput
avgRmotorInput.reverse()
avgRmotorInput.extend(bw_avgRmotorInput)

ax.plot(avgSpeed,avgLmotorInput,'-xb')
ax.plot(avgSpeed,avgRmotorInput,'-xb')









#data = loadFromFile("","motorCalibLog_forwards_asymetricR.p")

#print(data)

#fw_avgSpeed = data['avgSpeed']
#fw_avgLmotorInput = data['avgLmotorInput']
#fw_avgRmotorInput = data['avgRmotorInput']


#data = loadFromFile("","motorCalibLog_backwards_asymetricR.p")

#print(data)

#bw_avgSpeed = data['avgSpeed']
#bw_avgLmotorInput = data['avgLmotorInput']
#bw_avgRmotorInput = data['avgRmotorInput']


#avgSpeed = fw_avgSpeed
#avgSpeed.reverse()
#avgSpeed.extend(bw_avgSpeed)

#avgLmotorInput = fw_avgLmotorInput
#avgLmotorInput.reverse()
#avgLmotorInput.extend(bw_avgLmotorInput)

#avgRmotorInput = fw_avgRmotorInput
#avgRmotorInput.reverse()
#avgRmotorInput.extend(bw_avgRmotorInput)

#ax.plot(avgSpeed,avgLmotorInput,'-xr')
#ax.plot(avgSpeed,avgRmotorInput,'-xr')










#ax.plot(fw_avgSpeed,fw_avgLmotorInput,'-x')
#ax.plot(fw_avgSpeed,fw_avgRmotorInput,'-x')

#ax.plot(bw_avgSpeed,bw_avgLmotorInput,'-x')
#ax.plot(bw_avgSpeed,bw_avgRmotorInput,'-x')

#ax.legend(["fw L","fw R","bw L", "bw R"])



#ax.set_ylim([0,3.5])
#ax.set_xlim([0,200])

ax.set_ylabel('Motor input pulse width [ms]', fontsize=16)
ax.set_xlabel('Measured speed to wall (negative is towards wall) [cm/s]', fontsize=16)

ax.set_title('Measured motor response curve', fontsize=16)

tight_layout()

savefig("motors_speed_response_curve_asym_tires.pdf")
show()


