#!/usr/bin/python

# Light model implementation:
# In order to model an intensity value I(A,D) at A (angle) D (distance). Given M pointcloud measurements at various distances
# -> for each M: 1 robot spin, 1 sensor (num4), lerp (nearest neighbour -angle- values) -> radial pattern model with N samples (N angles).
# -> generate .p file with the generated model M sorted curves (at incremental distances) each with N points
# -> given A (angle) and D (distance): lerp intensity between nearest neighbour D values -> radial + distance model generates I(A,D)

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
import time

import numpy as np
import matplotlib.pyplot as plt

import scipy
import scipy.interpolate

from helper import *
# End modules

# Begin configuration.
if len(sys.argv) > 1:
    LDR_DF_PATH = sys.argv[1]
else:
    LDR_DF_PATH = "./Light_Measurements/dataSingleLight_longRange_noHood_linearAngle_7nov2013/"
# End configuration

print("Processing file: " + LDR_DF_PATH)


POLAR_DISTANCE_RANGE = 600 # This sets the scale of distance of the lights shown in the polar plot

max_LDR_vals = []
lightDistance_vals = []

for LDR_DF in getFilesInDir(LDR_DF_PATH):
    print("\n=====================================")
    print("File: " + LDR_DF)
    print("=====================================")
    # Variable to store all the data for each experiment
    LDR_data = {}
    LDR_data = loadFromFile(LDR_DF_PATH,LDR_DF)
    
    # This is the assignation of the sensors
    #IR1 = 1
    #IR2 = 2
    #IR3 = 4
    #IR4 = 3
    
    offset = 45 # Sensors are separated 45 degrees
    LDR_data["LDR_real_pos"] = []
    LDR_data["LDR_real_pos"].append(offset+90*0)
    LDR_data["LDR_real_pos"].append(offset+90*1)
    LDR_data["LDR_real_pos"].append(offset+90*3)
    LDR_data["LDR_real_pos"].append(offset+90*2)
    
    # Remove escaping characters from the description
    LDR_data["description"] = LDR_data["description"].replace("\n","")
    LDR_data["description"] = LDR_data["description"].replace("\r","")
    
    # Get the number of lights present in the data
    Nlights = len(LDR_data["lights"])
    
    # Show the parameters of the data that is going to be processed
    for key in LDR_data.keys():
        if key.find("raw") == -1: # Hide the raw datapoints
            print(str(key)+": "+str(LDR_data[key]))
        else: # Instead of showing all the values, show the count
            print(str(key)+": Nvals=" + str(len(LDR_data[key])))
    
    # Load the data as an array
    LDR_raw = np.array(LDR_data["LDR_raw"])
    time_raw = np.array(LDR_data["time_raw"])
    BattVoltage_raw = np.array(LDR_data["BattVoltage_raw"])
    MagAngle_raw = np.array(LDR_data["MagAngle_raw"])
    
    N_circles = LDR_data["Ncircles"]
    
    # Force linear compass angle (Overwrites the compass measurement!)
    MagAngle_raw = np.linspace(0,360.0*N_circles,num=len(MagAngle_raw))
    
    # Invert LDR sensor values from [0,1023] to [1023,0] (to have 0 -> dark, 1023 -> bright)
    for i in range(len(LDR_raw[0,:])):
        LDR_raw[:,i] = 1023.0-LDR_raw[:,i]
    
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"]) # Angle offset of the sensors
    anglesIn = (MagAngle_raw+float(LDR_real_pos[3])).copy()
    intensitiesIn = np.array(LDR_raw[:,3]).copy()
    (intensitiesOut,anglesOut) = discretizePolar(anglesIn, intensitiesIn, 360)
    
    intensitiesOut_fft = np.fft.rfft(intensitiesOut)
    intensitiesOut_fft[4:] = 0.
    intensitiesOut = np.fft.irfft(intensitiesOut_fft)
    
    # Normalize sensor values
    # NOTE: Here we are using all the values, the robot only has an incremental sequence
    #for i in range(len(LDR_raw[0,:])):
    #    LDR_raw[:,i] = mapVals(LDR_raw[:,i], LDR_raw[:,i].min(), LDR_raw[:,i].max(), 0.0, 1023.0)
    
    
    # Polar plot
    fig = plt.figure(LDR_data["beginDate"] + " " + LDR_data["description"])
    ax = fig.add_subplot(111,polar=True)
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
    for i in range(len(LDR_real_pos)):
        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
        offset = float(LDR_real_pos[i])
        ax.plot(np.radians(MagAngle_raw+offset), LDR_raw[:,i], marker='.', markersize=2, ls='', label="Sensor "+str(i+1))
    
    ax.plot(np.radians(anglesOut), intensitiesOut, marker='o', markersize=1, ls='-', label="Model")
    
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/POLAR_DISTANCE_RANGE, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
    #plt.ylim((0,2*1050))
    ax.set_rmax(2*1050)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend()
    ax.grid(True)
    
    lightDistance = LDR_data["lights"][0]["distance"]
    
    plt.suptitle("Light intensity [0,1023]" + " (Dist: " + str(lightDistance) + "cm)")
    #mySaveFig(plt,LDR_DF_PATH + "png/2/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_2.png")
    plt.show()
    




exit()

# Linear plot
fig = plt.figure()
ax = fig.add_subplot(111)

lightDistance_vals = np.array(lightDistance_vals)
max_LDR_vals = np.array(max_LDR_vals)

sortedIndices = np.argsort(lightDistance_vals)
lightDistance_vals = lightDistance_vals[sortedIndices]
max_LDR_vals = max_LDR_vals[sortedIndices]

ax.plot(lightDistance_vals, max_LDR_vals, marker='x', markersize=10, ls='-')

plt.ylabel('Light intensity [0-1024]')
plt.xlabel('Distance [cm]')

plt.suptitle("Max intensity of light vs Distance")
mySaveFig(plt,LDR_DF_PATH + "png/","Max_intensity_vs_distance.png")

plt.ylim((0,1024))
plt.xlim((0,650))
mySaveFig(plt,LDR_DF_PATH + "png/","Max_intensity_vs_distance_range.png")

