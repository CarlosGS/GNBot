#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlosgs (http://carlosgs.es)
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
    LDR_DF_PATH = "./data/"
# End configuration

#plt.ion() # IMPORTANT: Enable real-time plotting

def diff(vals):
    #return np.diff(vals,axis=0)
    #return np.abs(np.fft.fft(vals))
    result = vals
    for i in range(len(vals)):
        result[i] = vals[i]-vals[i-1]
    return result


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
    
    IR1 = 1
    IR2 = 2
    IR3 = 4
    IR4 = 3
    
    offset = 45
    LDR_data["LDR_real_pos"] = []
    LDR_data["LDR_real_pos"].append(offset+90*0)
    LDR_data["LDR_real_pos"].append(offset+90*1)
    LDR_data["LDR_real_pos"].append(offset+90*3)
    LDR_data["LDR_real_pos"].append(offset+90*2)
    
    LDR_data["description"] = LDR_data["description"].replace("\n","")
    LDR_data["description"] = LDR_data["description"].replace("\r","")
    
    Nlights = len(LDR_data["lights"])
    
    for key in LDR_data.keys():
        print(str(key)+":")
        if key.find("raw") == -1:
            print(LDR_data[key])
        else:
            print("Size: " + str(len(LDR_data[key])))
    
    LDR_raw = np.array(LDR_data["LDR_raw"])
    time_raw = np.array(LDR_data["time_raw"])
    BattVoltage_raw = np.array(LDR_data["BattVoltage_raw"])
    MagAngle_raw = np.array(LDR_data["MagAngle_raw"])
    
    N_circles = LDR_data["Ncircles"]
    
    # Force linear compass angle
    MagAngle_raw = np.linspace(0,360.0*N_circles,num=len(MagAngle_raw))
    
    # Invert LDR sensor values from [0,1023] to [1023,0] (to have 0 -> dark, 1023 -> bright)
    for i in range(len(LDR_raw[0,:])):
        LDR_raw[:,i] = 1023.0-LDR_raw[:,i]
    
    # Normalize sensor values
    # NOTE: Here we are using all the values, the robot only has an incremental sequence
    #for i in range(len(LDR_raw[0,:])):
    #    LDR_raw[:,i] = mapVals(LDR_raw[:,i], LDR_raw[:,i].min(), LDR_raw[:,i].max(), 0.0, 1023.0)
    
    
    fig = plt.figure()
    
    ax = fig.add_subplot(311)        
    ax.plot(time_raw,LDR_raw)
    ax.grid(True)
    plt.ylim((0,1050))
    ax.legend(["s1","s2","s3","s4"])
    plt.ylabel('Light intensity')
    
    ax = fig.add_subplot(312)        
    ax.plot(time_raw,MagAngle_raw)
    ax.grid(True)
    plt.ylim((0,360))
    plt.ylabel('Compass angle [Deg]')
    
    ax = fig.add_subplot(313)        
    ax.plot(time_raw,BattVoltage_raw)
    ax.grid(True)
    plt.ylim((5,10))
    plt.ylabel('Battery voltage [V]')
    plt.xlabel('Time [ms]')
    
    lightDistance = LDR_data["lights"][0]["distance"]
    
    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"] + " (Dist: " + str(lightDistance) + "cm)")
    mySaveFig(plt,LDR_DF_PATH + "png/1/", "Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_1.png")
    #plt.show()
    
    
    # Polar plot
    fig = plt.figure(LDR_data["beginDate"] + " " + LDR_data["description"])
    ax = fig.add_subplot(111,polar=True)
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
    for i in range(len(LDR_real_pos)):
        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
        offset = float(LDR_real_pos[i])
        ax.plot(np.radians(MagAngle_raw+offset), LDR_raw[:,i], marker='.', markersize=2, ls='', label="Sensor "+str(i+1))
    
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/POLAR_DISTANCE_RANGE, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
    #plt.ylim((0,2*1050))
    ax.set_rmax(2*1050)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend()
    ax.grid(True)
    
    plt.suptitle("Light intensity [0,1023]" + " (Dist: " + str(lightDistance) + "cm)")
    mySaveFig(plt,LDR_DF_PATH + "png/2/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_2.png")
    #plt.show()
    
    
    
    # Polar plot (fitted curve + points where derivate == 0)
    fig = plt.figure()
    ax = fig.add_subplot(111,polar=True)
    
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
    LDR_raw_all = np.array([])
    LDR_raw_angle_all = np.array([])
    for i in range(len(LDR_real_pos)):
        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
        offset = float(LDR_real_pos[i])
        
        angle = np.radians(MagAngle_raw+offset)
        angle = np.mod(angle, 2*np.pi)
        magnitude = LDR_raw[:,i]
        LDR_raw_all = np.hstack([LDR_raw_all,LDR_raw[:,i]])
        LDR_raw_angle_all = np.hstack([LDR_raw_angle_all,angle])
    
    #indices = np.argsort(LDR_raw_angle_all,axis=None)
    #LDR_raw_angle_all = LDR_raw_angle_all[indices]
    #LDR_raw_all = LDR_raw_all[indices]
    
    #fit_pol = scipy.interpolate.splrep(LDR_raw_angle_all, LDR_raw_all, per=1)
    fit_pol = np.poly1d(np.polyfit(LDR_raw_angle_all, LDR_raw_all, 6))
    fit_angle = np.linspace(LDR_raw_angle_all.min(), LDR_raw_angle_all.max(), 800)
    #fit_angle = np.linspace(0, 2*np.pi, 800)
    fit_magnitude = fit_pol(fit_angle)
    ax.plot(fit_angle, fit_magnitude, label="Fitted curve")
    ax.plot(LDR_raw_angle_all, LDR_raw_all, marker='.', markersize=0.5, ls='', label="Datapoints")
    
    fit_pol_der = np.polyder(fit_pol)
    fit_magnitude_der = fit_pol_der(fit_angle)
    fit_magnitude_der_min = abs(np.min(fit_magnitude_der))
    fit_magnitude_der += fit_magnitude_der_min
    
    # Find points with derivate == 0
    all_roots = np.roots(fit_pol_der)
    
    # Remove complex roots
    indices = np.where(np.imag(all_roots)==0)
    roots = np.real(all_roots[indices])
    
    # Remove roots outside [0,2pi)
    indices = (roots >= 0) & (roots < 2*np.pi)
    roots = roots[indices]
    print("Roots: "+str(roots))
    
    ax.plot(roots, fit_pol(roots), marker='x', markersize=10, ls='', label="Roots")
    
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/POLAR_DISTANCE_RANGE, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
    #plt.ylim((0,2*1050))
    ax.set_rmax(2*1050)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend()
    ax.grid(True)
    
    plt.suptitle("Light intensity [0,1023]" + " (Dist: " + str(lightDistance) + "cm)")
    mySaveFig(plt,LDR_DF_PATH + "png/3/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_3.png")
    #plt.show()
    
    
    
    # Linear plot
    fig = plt.figure()
    ax = fig.add_subplot(111)
    
    ax.plot(LDR_raw_angle_all, LDR_raw_all, marker='.', markersize=0.5, ls='', label="Datapoints")
    ax.plot(fit_angle, fit_magnitude, label="Fitted curve")
    
    ax.plot(fit_angle, fit_magnitude_der, label="Fitted curve (diff)")
    ax.plot(fit_angle, fit_magnitude_der_min + 0*fit_magnitude_der)
    
    ax.plot(roots, fit_pol(roots), marker='x', markersize=10, ls='', label="Roots")
    
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/POLAR_DISTANCE_RANGE, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
    plt.ylim((0,2*1050))
    
    ax.legend()
    ax.grid(True)
    
    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"] + " (Dist: " + str(lightDistance) + "cm)")
    mySaveFig(plt,LDR_DF_PATH + "png/4/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_4.png")
    #plt.show()
    #exit()
    
    max_LDR_vals.append(np.max(LDR_raw_all));
    lightDistance_vals.append(lightDistance);






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

