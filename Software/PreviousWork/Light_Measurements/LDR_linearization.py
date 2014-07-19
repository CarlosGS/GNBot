#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import sys
from pprint import pprint

import numpy as np
import matplotlib.pyplot as plt

from scipy.optimize import curve_fit

from helper import *
import math
# End modules

def LDRtoResistance(v_adc):
    Vc = 5.
    Rl = 33000.
    if v_adc <= 0: v_adc = 0.00001
    Vout = mapVals(v_adc,0,1023., 0.,Vc)
    Rs = Rl*Vout/(Vc-Vout)
    return Rs

# Function fitting implementation from http://stackoverflow.com/questions/3433486/how-to-do-exponential-and-logarithmic-curve-fitting-in-python-i-found-only-poly
def fit_func(x, a, b, c): # Exponential decay function, to be fitted to the measurements
    return a * np.exp(-b * x) + c
def fit_func_inv(y, a, b, c): # Inverse function, analitically determined
    #return y
    res = np.copy(y)
    for i in range(len(y)):
        if y[i] - c <= 0:
            res[i] = 600*3
        else:
            res[i] =  (-1/b) * np.log((y[i] - c) / a)
    return res
    #return (-1/b) * np.log((y - c) / a)

# Load the data (LDR values and angle, indexed by the distance to the light source)
data = loadFromFile("./","logged_data_LDR.p")
#pprint(data)

for dist in sorted(data.iterkeys()):
    for sensor_i in data[dist]['vals'].iterkeys():
        newVals = [1023.-val for val in data[dist]['vals'][sensor_i]]
        data[dist]['vals'][sensor_i] = newVals
        #newAngles = [math.radians(val) for val in data[dist]['angles'][sensor_i]]
        #data[dist]['angles'][sensor_i] = newAngles

distances = sorted(data.iterkeys())
print("Loaded data for distances [cm]:")
for dist in sorted(data.iterkeys()):
    print(dist)

print("===== Calibrating each sensor with three points =====")
DIST_NEAR = 100
DIST_MIDDLE = 200
DIST_FAR = 500
print("Near: "+str(DIST_NEAR)+"[cm] Middle: "+str(DIST_MIDDLE)+"[cm] Far: "+str(DIST_FAR)+"[cm]")

fit_func_coefs = {} # Stores the fitted coefficients for all the sensors
for sensor_i in data[DIST_NEAR]['vals'].iterkeys(): # Run for each sensor (usually 4)
    print("----> Sensor " + str(sensor_i+1))
    
    # Fetch max value for every distance
    max_val = {} # magnitude[distance], as a dictionary
    x_measured = [] # Distances
    y_measured = [] # Light magnitude
    for dist in sorted(data.iterkeys()):
        max_val[dist] = max(data[dist]['vals'][sensor_i]) # The maximum value is the one pointing to the light source
        x_measured.append(dist)
        y_measured.append(max_val[dist])
    
    # Get measured curve data ready to plot
    x_measured = np.array(x_measured)
    y_measured = np.array(y_measured)
    
    # Get value at the three distances
    val_near = max_val[DIST_NEAR]
    val_middle = max_val[DIST_MIDDLE]
    val_far = max_val[DIST_FAR]
    print("Near: "+str(val_near)+", Middle: "+str(val_middle)+", Far: "+str(val_far))
    
    # Three point curve fitting
    x = np.array([DIST_NEAR,DIST_MIDDLE,DIST_FAR])
    y = np.array([val_near,val_middle,val_far])
    fit_func_coefs[sensor_i], pcov = curve_fit(fit_func, x, y,[1023.,0,0],maxfev = 1000000) # Very important to give reasonable initial coefficients
    print("Calculated coefficients: "+str(fit_func_coefs[sensor_i]))
    # NOTE for fit_func_coefs[sensor_i]
    # The coefficients can be passed to the chosen fitting functions using an * before, which expands them as arguments
    # Example: y = fit_func(x, *fit_func_coefs[sensor_i])
    # Same as: y = fit_func(x, fit_func_coefs[0], fit_func_coefs[1], fit_func_coefs[2])
    
    PLOT_CURVE_FITTING = True
    if PLOT_CURVE_FITTING:
        # Plot the resulting curve and the measurements
        xlong = np.linspace(0,DIST_FAR,50)
        plt.figure()
        plt.plot(x_measured, y_measured, 'ko', label="Original data")
        plt.plot(xlong, fit_func(xlong, *fit_func_coefs[sensor_i]), 'r-', label="Fitted curve")
        plt.xlabel("Distance [cm]")
        plt.ylabel("Intensity [0-1023]")
        plt.title("y = light_intensity(distance)")
        plt.legend(loc='upper right')
        plt.show()
        
        # Plot the accuracy of the estimated distance values
        distance_estimation = fit_func_inv(y_measured, *fit_func_coefs[sensor_i])
        distance_real = x_measured
        plt.figure()
        plt.plot(distance_estimation, distance_real, 'ko', label="Estimation")
        plt.plot([0,distance_real[-1]], [0,distance_real[-1]], 'r-', label="y = x")
        plt.xlabel("Distance estimation [cm]")
        plt.ylabel("Real distance [cm]")
        plt.title("Accuracy of the estimation")
        plt.legend(loc='lower right')
        plt.show()
    
    # Calibrate all measurements of current sensor with the calculated parameters
    for dist in data.iterkeys():
        if not 'vals_cal' in data[dist].keys(): data[dist]['vals_cal'] = {}
        print("Sensor:",sensor_i)
        data[dist]['vals_cal'][sensor_i] = fit_func_inv(data[dist]['vals'][sensor_i], *fit_func_coefs[sensor_i])
#print(fit_func_coefs[0])
#print(data[600]['vals'][0],fit_func_coefs[0])

#fig = plt.figure()
#ax = fig.add_subplot(111)
#plt.plot(data[600]['vals'][0])
#plt.plot(fit_func_inv(data[600]['vals'][0], *fit_func_coefs[0]))
#plt.show()
#plt.close(fig)

saveToFile_noBak(fit_func_coefs,"./","fit_func_coefs.p")

# Polar plot with the calibrated measurements
PLOT_CALIBRATED_MEASUREMENTS = False
if PLOT_CALIBRATED_MEASUREMENTS:
    for dist in sorted(data.iterkeys()):
        fig = plt.figure()
        ax = fig.add_subplot(111,polar=True)
        for sensor_i in data[dist]['vals_cal'].iterkeys():
            angle = np.radians(data[dist]['angles'][sensor_i])
            value = np.array(data[dist]['vals_cal'][sensor_i])
            plt.plot(angle,value, marker='.', markersize=3, ls='', label="Sensor "+str(sensor_i+1))
        ax.set_rmax(DIST_FAR*3) # Set larger range, since sensors not pointing directly to the light source will output longer distances
        ax.set_theta_direction('clockwise')
        ax.set_theta_zero_location('N')
        ax.legend(loc='upper left')
        ax.grid(True)
        plt.suptitle("Estimated distance D(phi) of each sensor to a light (with lamp at phi=0, D=" + str(dist) + "cm)")
        #mySaveFig(plt,"graphs/","LDR_response_at_" + str(dist) + "cm.png")
        plt.show()
        plt.close(fig)


ANALYTICAL_LIGHT_MODEL = False
if ANALYTICAL_LIGHT_MODEL:
    DIST = 100.
    DIST_MAX = max(data[DIST]['vals_cal'][0])
    DIST_MIN = DIST

    N_samples = 360
    angles = np.linspace(0,2.*np.pi,num=N_samples,endpoint=False)
    model = np.copy(angles)
    for n in range(N_samples):
        angle = angles[n]
        if angle < np.pi/2. or angle > 3.*np.pi/2:
            model[n] = DIST_MAX-(DIST_MAX-DIST_MIN)*np.abs(np.cos(angle))
        else:
            model[n] = DIST_MAX
        #model[n] = DIST_MIN+(DIST_MAX-DIST_MIN)*0.5+(DIST_MAX-DIST_MIN)*0.5*np.cos(angle*1.-np.pi)
    #model = DIST_MIN*np.abs(np.cos(angle/2.)) + DIST_MAX*np.abs(np.cos(angle/2.+np.pi/2.))
    #model1 = np.linspace(DIST_MIN,DIST_MAX,num=360/2,endpoint=False)
    #model2 = np.linspace(DIST_MAX,DIST_MIN,num=360/2,endpoint=False)
    #model = np.append(model1,model2)

    plt.figure()
    angles_ = np.radians(data[DIST]['angles'][0])
    values = data[DIST]['vals_cal'][0]
    plt.plot(angles_, values, marker='.', markersize=3, ls='')
    plt.plot(angles_+angles_[-1], values, marker='.', markersize=3, ls='')
    plt.plot(angles, model)
    plt.plot(angles+angles[-1], model)
    plt.show()


    fig = plt.figure()
    ax = fig.add_subplot(111,polar=True)
    plt.plot(angles_,values, marker='.', markersize=3, ls='', label="Measured")
    plt.plot(angles,model, label="Model")
    ax.set_rmax(DIST_FAR*3)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend(loc='upper left')
    ax.grid(True)
    plt.suptitle("Light model at " + str(DIST) + "cm")
    #mySaveFig(plt,"graphs/","LDR_response_at_" + str(dist) + "cm.png")
    plt.show()
    plt.close(fig)


# Numerical light model
NUMERICAL_LIGHT_MODEL = True
if NUMERICAL_LIGHT_MODEL:
    N_resample = 360
    N_samples_fft = 360
    for DIST in [DIST_NEAR,DIST_MIDDLE,DIST_FAR]:
        distance_model_sum = np.zeros(N_samples_fft)
        data[DIST]['angles_model'] = {}
        data[DIST]['distance_model'] = {}
        for sensor_i in data[DIST]['vals_cal'].iterkeys():
            # First step: uniformly resampling the intensity values
            angles_measured = data[DIST]['angles'][sensor_i]
            distance_measured = np.array(data[DIST]['vals_cal'][sensor_i])
            
            (distance_model,angles_model) = discretizePolar(angles_measured, distance_measured, N_resample)
            
            # Second step: smoothing the data using the FFT
            FFT_SMOOTHING = True
            if FFT_SMOOTHING:
                distance_model_fft = np.fft.rfft(distance_model)
                 # Store the first coefficients of the FFT, this acts as a low-pass filter
                distance_model_fft = distance_model_fft[:10]
                distance_model = np.fft.irfft(distance_model_fft,N_samples_fft)*N_samples_fft/N_resample
                #distance_model = [evalIFFTpoint(angle,distance_model_fft,Nsamples) for angle in range(Nsamples)]
                angles_model = np.linspace(0.,2.*np.pi,num=N_samples_fft,endpoint=False)
            else:
                angles_model = np.radians(angles_model)
            angles_measured = np.radians(angles_measured)
            
            #distance_model_sum += distance_model
            distance_model_sum = distance_model
            
            data[DIST]['angles_model'][sensor_i] = np.degrees(angles_model)
            data[DIST]['distance_model'][sensor_i] = distance_model
            
            PLOT_LIGHT_MODEL = False
            if PLOT_LIGHT_MODEL:
                fig = plt.figure()
                ax = fig.add_subplot(111,polar=True)
                plt.plot(angles_measured,distance_measured, marker='.', markersize=3, ls='', label="Measured")
                plt.plot(angles_model,distance_model, label="Model")
                ax.set_rmax(DIST_FAR*3)
                ax.set_theta_direction('clockwise')
                ax.set_theta_zero_location('N')
                ax.legend(loc='upper left')
                ax.grid(True)
                plt.suptitle("Light model for sensor " + str(sensor_i+1) + " at " + str(DIST) + "cm")
                #mySaveFig(plt,"graphs/","LDR_response_at_" + str(dist) + "cm.png")
                plt.show()
                plt.close(fig)
        
        #distance_model_sum /= len(data[DIST]['vals_cal'])
#        data[DIST]['distance_model'] = np.copy(distance_model_sum)
#        data[DIST]['angles_model'] = np.copy(angles_model)
#    
#    angles_model = data[DIST_NEAR]['angles_model']
#    for dist in sorted(data.iterkeys()):
#        # Linear interpolation of the model
#        t = mapVals(dist, DIST_NEAR,DIST_FAR, 0.,1.)
#        distance_model = (1-t)*data[DIST_NEAR]['distance_model'] + t*data[DIST_FAR]['distance_model']
#        PLOT_LIGHT_MODEL = True
#        if PLOT_LIGHT_MODEL:
#            fig = plt.figure()
#            ax = fig.add_subplot(111,polar=True)
#            for sensor_i in data[dist]['vals_cal'].iterkeys():
#                angle = np.radians(data[dist]['angles'][sensor_i])
#                value = np.array(data[dist]['vals_cal'][sensor_i])
#                plt.plot(angle,value, marker='.', markersize=3, ls='', label="Sensor "+str(sensor_i+1))
#            plt.plot(angles_model,distance_model, label="Model")
#            ax.set_rmax(DIST_FAR*3) # Set larger range, since sensors not pointing directly to the light source will output longer distances
#            ax.set_theta_direction('clockwise')
#            ax.set_theta_zero_location('N')
#            ax.legend(loc='upper left')
#            ax.grid(True)
#            plt.suptitle("Estimated distance D(phi) of each sensor to a light (with lamp at phi=0, D=" + str(dist) + "cm)")
#            #mySaveFig(plt,"graphs/model/","LDR_response_at_" + str(dist) + "cm.png")
#            plt.show()
#            plt.close(fig)

#print(data)
saveToFile_noBak(data,"./","LDR_data_light_model.p")

