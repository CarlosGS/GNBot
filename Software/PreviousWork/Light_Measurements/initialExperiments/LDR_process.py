#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
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

# Data from the measurement with one light and no ambient light
Light1_LDR_curve = np.array([232.56363636363633, 235.3166666666666, 248.2173913043478, 259.352380952381, 276.0769230769231, 292.81000000000006, 305.46399999999994, 316.01599999999996, 338.83333333333337, 396.3083333333334, 495.16999999999996, 567.0560000000002, 613.44, 654.4173913043478, 685.4782608695652, 706.5826086956522, 720.7652173913043, 731.6260869565218, 740.1043478260871, 747.162962962963, 745.96, 747.0999999999999, 743.0380952380954, 739.9000000000001, 727.3636363636365, 719.4952380952379, 702.2166666666667, 678.904347826087, 651.1047619047619, 596.5923076923076, 552.2700000000001, 471.1461538461538, 388.39200000000005, 313.46666666666664, 274.425, 254.23999999999995, 247.07199999999995, 227.47000000000003, 210.89565217391302, 204.33043478260868, 193.295652173913, 185.50434782608693, 184.19130434782608, 187.96521739130438, 191.02222222222224, 193.92, 198.03846153846155, 204.21904761904764, 208.32500000000002, 217.4090909090909])
Light1_angle = 144.73
Light1_distance = 106.0
Light1_intensity = 60.0

# Data from the measurement with two lights, no ambient light and removing the effect of the first light
Light2_LDR_curve = np.array([596.897902097902, 603.4659420289854, 599.0608695652172, 592.361904761905, 580.7412587412585, 566.2780000000001, 553.7110000000002, 542.8080000000001, 516.7484848484846, 458.61166666666657, 358.87000000000023, 283.6856666666665, 231.40761904761916, 184.34782608695684, 144.30269151138725, 111.28405797101448, 79.26811594202911, 53.42608695652166, 38.747826086956366, 31.321652421652402, 33.22999999999979, 27.97407407407411, 28.088220551378413, 25.363999999999805, 30.584189723320037, 25.88737060041433, 26.21811594202893, 34.06086956521722, 29.885714285714243, 61.99860139860152, 60.40199999999993, 87.66217948717957, 110.62399999999991, 157.67878787878794, 174.79899999999992, 198.84000000000015, 208.19466666666676, 228.15857142857135, 250.4347826086956, 262.6505175983437, 283.54601449275367, 300.07065217391306, 318.0086956521739, 338.6695652173913, 391.7777777777777, 445.4200000000001, 508.46524216524233, 555.3809523809524, 578.851, 594.1996047430829])
Light2_angle = 41.62
Light2_distance = 63.0
Light2_intensity = 60.0

# Data from the measurement with three lights, no ambient light and removing the effect of the first and second lights
Light3_LDR_curve = np.array([-1.3015384615382573, -5.018972332016006, -3.5502608695649087, -2.287012987013213, -2.887747035572829, -3.342545454545757, -1.490384615384869, -1.7360000000002174, 0.37272727272750217, 1.4163636363635987, 2.4183333333331802, 4.206159420289623, 5.670562770562469, 7.060869565217104, 9.152380952380781, 14.975000000000023, 23.1575757575755, 28.8144927536232, 31.909730848861386, 34.21538461538478, 36.07363636363664, 40.80774410774427, 41.691866028707864, 44.12830769230766, 44.49427917620153, 53.300724637681014, 60.7470355731225, 63.62678260869575, 78.94588744588748, 81.00039525691693, 104.90981818181808, 128.64551282051298, 155.18400000000008, 156.2636363636363, 162.69418181818185, 144.29500000000002, 122.15942028985495, 111.47142857142853, 94.2434782608697, 90.78095238095244, 81.39999999999986, 75.87954545454551, 68.11666666666673, 61.90807453416153, 40.24285714285736, 25.20545454545436, 10.632659932659976, -1.481818181818312, -2.037538461538702, -2.387643020594737])
Light3_angle = 176.26
Light3_distance = 95.0
Light3_intensity = 30.0

# Data from the measurement with four lights, no ambient light and removing the effect of the first, second and third lights
Light4_LDR_curve = np.array([24.101538461538325, 23.49636363636398, 16.755333333333283, 16.67672727272725, 13.351383399209453, 14.236363636363876, 11.49938461538477, 11.75200000000018, 11.681818181818016, 10.943636363636415, 11.157051282051384, 10.191304347826303, 11.61225296442717, 11.873913043478183, 12.823188405797168, 12.732246376811645, 13.904329004329156, 19.637681159420367, 22.063095238095002, 21.48333333333312, 19.79723320158098, 20.68181818181813, 23.86007905138365, 25.299358974359052, 30.07607655502386, 31.664814814814918, 42.298181818181774, 55.87466666666671, 74.15163636363616, 95.1814229249012, 118.30000000000018, 147.6501538461539, 182.4240000000001, 210.7727272727273, 228.91181818181843, 242.62500000000006, 263.69565217391283, 274.6565217391304, 285.10942028985505, 281.48157349896474, 277.80181159420306, 269.2502164502164, 259.11811594202914, 236.6404761904762, 201.1821428571426, 161.72411067193684, 112.03636363636338, 76.54268774703553, 55.13653846153875, 34.84258373205728])
Light4_angle = 263.02
Light4_distance = 65.0
Light4_intensity = 30.0

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
    
    offset = 45;
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
    
    # Invert LDR sensor values from [0,1023] to [1023,0] (to have 0 -> dark, 1023 -> bright)
    LDR_raw = 1023.0-LDR_raw
    
    # Normalize sensor values
    # NOTE: Here we are using all the values, the robot only has an incremental sequence
    #for i in range(len(LDR_raw[0,:])):
    #    LDR_raw[:,i] = mapVals(LDR_raw[:,i], LDR_raw[:,i].min(), LDR_raw[:,i].max(), 0.0, 1023.0)
    
    # Plot the raw data
#    fig = plt.figure()
#    
#    ax = fig.add_subplot(311)        
#    ax.plot(time_raw,LDR_raw)
#    ax.grid(True)
#    plt.ylim((0,1050))
#    ax.legend(["1","2","3","4"])
#    plt.ylabel('Light intensity')
#    
#    ax = fig.add_subplot(312)        
#    ax.plot(time_raw,MagAngle_raw)
#    ax.grid(True)
#    plt.ylim((0,360))
#    plt.ylabel('Compass angle [Deg]')
#    
#    ax = fig.add_subplot(313)        
#    ax.plot(time_raw,BattVoltage_raw)
#    ax.grid(True)
#    plt.ylim((5,10))
#    plt.ylabel('Battery voltage [V]')
#    plt.xlabel('Time [ms]')
#    
#    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"])
    #mySaveFig(plt,LDR_DF_PATH + "png/", "Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_1.png")
    #plt.show()
    
    
    # Polar plot
#    fig = plt.figure()
#    ax = fig.add_subplot(111,polar=True)
#    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
#    for i in range(len(LDR_real_pos)):
#        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
#        offset = float(LDR_real_pos[i])
#        ax.plot(np.radians(MagAngle_raw+offset), LDR_raw[:,i], marker='.', markersize=2, ls='', label=str(i+1))
#    
#    for light in LDR_data["lights"]:
#        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/150.0, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
#    #plt.ylim((0,2*1050))
#    ax.set_rmax(2*1050)
#    ax.set_theta_direction('clockwise')
#    ax.set_theta_zero_location('N')
#    ax.legend()
#    ax.grid(True)
#    
#    plt.suptitle(LDR_data["beginDate"] + " " + LDR_data["description"])
    #mySaveFig(plt,LDR_DF_PATH + "png/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_2.png")
    #plt.show()
    
    
    # Merges the data from all the sensors as a datapoint cloud
    LDR_real_pos = np.array(LDR_data["LDR_real_pos"])
    LDR_raw_all = np.array([])
    LDR_raw_angle_all = np.array([])
    for i in range(len(LDR_real_pos)):
        # NOTE: here we are incorporating the knowledge of the orientation of the sensors
        offset = float(LDR_real_pos[i])
        
        angle = MagAngle_raw+offset
        angle = np.mod(angle, 360.);
        magnitude = LDR_raw[:,i]
        LDR_raw_all = np.hstack([LDR_raw_all,LDR_raw[:,i]])
        LDR_raw_angle_all = np.hstack([LDR_raw_angle_all,angle])
    
    
    # Discretizes the measurements as a linear plot
    Nsamples = 50
    LDR_curve = discretizePolar(LDR_raw_angle_all, LDR_raw_all, Nsamples)
    LDR_angle = np.arange(0., 360., 360./float(Nsamples))
    print("Measurement:")
    print("LDR_curve="+str(LDR_curve.tolist()))
    print("LDR_angle="+str(LDR_angle.tolist()))
    
#    if Nlights >= 1:
#        LDR_curve -= Light1_LDR_curve
#    if Nlights >= 2:
#        LDR_curve -= Light2_LDR_curve
#    if Nlights >= 3:
#        LDR_curve -= Light3_LDR_curve
#    if Nlights >= 4:
#        LDR_curve -= Light4_LDR_curve
    
    print("Removed offset:")
    print("LDR_curve="+str(LDR_curve.tolist()))
    print("LDR_angle="+str(LDR_angle.tolist()))
    
    # Polar plot (fitted curve + points where derivate == 0)
    fig = plt.figure(LDR_data["beginDate"] + " " + LDR_data["description"])
    ax = fig.add_subplot(111,polar=True)
    
    plotClosedLine(ax, np.radians(LDR_angle), LDR_curve, "Curve")
    ax.plot(np.radians(LDR_raw_angle_all), LDR_raw_all, marker='.', markersize=0.5, ls='', label="Datapoints")
    
    LDR_result_curve = np.zeros(Nsamples)
    for light in LDR_data["lights"]:
        ax.plot(np.radians(light["angle"]), 1024+1050.0*light["distance"]/150.0, marker='o', markersize=50.0*(light["intensity"]/100.0), ls='')
        if isSameAngle(Light1_angle, light["angle"]):
            print("Light 1")
            intens_front = eval_intensity(Light1_LDR_curve, light["angle"])
            intens_back = eval_intensity(Light1_LDR_curve, 180.+light["angle"])
        elif isSameAngle(Light2_angle, light["angle"]):
            print("Light 2")
            intens_front = eval_intensity(Light2_LDR_curve, light["angle"])
            intens_back = eval_intensity(Light2_LDR_curve, 180.+light["angle"])
        elif isSameAngle(Light3_angle, light["angle"]):
            print("Light 3")
            intens_front = eval_intensity(Light3_LDR_curve, light["angle"])
            intens_back = eval_intensity(Light3_LDR_curve, 180.+light["angle"])
        elif isSameAngle(Light4_angle, light["angle"]):
            print("Light 4")
            intens_front = eval_intensity(Light4_LDR_curve, light["angle"])
            intens_back = eval_intensity(Light4_LDR_curve, 180.+light["angle"])
        intens_back = eval_intensity(Light1_LDR_curve, 180.+Light1_angle)
        if intens_back < 0:
            intens_back = 0
        intens_back /= float(Nlights)
        if light["intensity"] == 60:
            intens = 1.*(intens_front - intens_back)*((light["distance"]/100.)**1.)
        else:
            intens = 1.*(intens_front - intens_back)*((light["distance"]/100.)**1.)
        #intens_back += 250./float(Nlights)
        print("Back="+str(intens_back))
        print("Front="+str(intens))
        (LDR_model_angle, LDR_model_curve) = lightModelCurve(Nsamples, angle=light["angle"], distance=light["distance"]/100., intensity=intens, ambient=intens_back)
        #plotClosedLine(ax, np.radians(LDR_angle), LDR_model_curve)
        LDR_result_curve += LDR_model_curve
    
    plotClosedLine(ax, np.radians(LDR_angle), LDR_result_curve, "Model")
    
    #plt.ylim((0,2*1050))
    ax.set_rmax(2*1050)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend()
    ax.grid(True)
    
    plt.suptitle("Intensity [0,1023]")
    #mySaveFig(plt,LDR_DF_PATH + "png/","Nlights:" + str(Nlights) + " " + LDR_data["beginDate"] + "_2.png")
    plt.show()
    

