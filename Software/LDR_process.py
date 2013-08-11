#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlosgs (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
import time

import numpy as np
import matplotlib.pyplot as plt

from helper import *
# End modules

# Begin configuration.
LDR_DF_PATH = "./data/"
# End configuration

#plt.ion() # IMPORTANT: Enable real-time plotting

for LDR_DF in getFilesInDir(LDR_DF_PATH):
    print("\n=====================================")
    print("File: " + LDR_DF)
    print("=====================================")
    # Variable to store all the data for each experiment
    LDR_data = {}
    LDR_data = loadFromFile(LDR_DF_PATH,LDR_DF)
    
    offset = 45;
    LDR_data["LDR_real_pos"] = []
    LDR_data["LDR_real_pos"].append(offset+90*2)
    LDR_data["LDR_real_pos"].append(offset+90*3)
    LDR_data["LDR_real_pos"].append(offset+90*1)
    LDR_data["LDR_real_pos"].append(offset+90*0)
    
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
    
    
    # Normalize sensor values
    # NOTE: Here we are using all the values, the robot only has an incremental sequence
    for i in range(len(LDR_raw[0,:])):
        LDR_raw[:,i] = mapVals(LDR_raw[:,i], LDR_raw[:,i].min(), LDR_raw[:,i].max(), 0.0, 1023.0)
    
    
    fig = plt.figure()
    
    ax = fig.add_subplot(311)        
    ax.plot(time_raw,LDR_raw)
    ax.grid(True)
    plt.ylim((0,1050))
    ax.legend(["1","2","3","4"])
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
    
    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"])
    mySaveFig(plt,LDR_DF_PATH + "png/", "Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_1.png")
    #plt.show()
    
    
    fig = plt.figure()
    ax = fig.add_subplot(111, polar=True)
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
    for i in range(len(LDR_real_pos)):
        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
        offset = float(LDR_real_pos[i])
        ax.plot(np.radians(MagAngle_raw+offset), LDR_raw[:,i], marker='.', markersize=2, ls='', label=str(i+1))
    
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1050.0*light["distance"]/150.0, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
    
    ax.set_rmax(1050)
    ax.set_theta_direction('clockwise')
    ax.legend()
    ax.grid(True)
    
    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"])
    mySaveFig(plt,LDR_DF_PATH + "png/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_2.png")
    #plt.show()
    #exit()

