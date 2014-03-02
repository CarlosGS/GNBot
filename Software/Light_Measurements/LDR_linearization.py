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
# End modules

def LDRtoResistance(v_adc):
    Vc = 5.
    Rl = 33000.
    if v_adc <= 0: v_adc = 0.000001
    Vout = mapVals(v_adc,0,1023, 0.,Vc)
    Rs = Rl*Vout/(Vc-Vout)
    return Rs

# Function fitting implementation from http://stackoverflow.com/questions/3433486/how-to-do-exponential-and-logarithmic-curve-fitting-in-python-i-found-only-poly
def fit_func(x, a, b, c): # Exponential decay function, to be fitted to the measurements
    return a * np.exp(-b * x) + c
def fit_func_inv(y, a, b, c): # Inverse function, analitically determined
    return (-1/b) * np.log((y - c) / a)

# Load the data (LDR values and angle, indexed by the distance to the light source)
data = loadFromFile("./","data_all_measurements.p")
#pprint(data)

distances = sorted(data.iterkeys())
print("Loaded data for distances [cm]:")
for dist in sorted(data.iterkeys()):
    print(dist)

print("===== Calibrating each sensor with three points =====")
DIST_NEAR = 100
DIST_MIDDLE = 200
DIST_FAR = 600
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
    fit_func_coefs[sensor_i], pcov = curve_fit(fit_func, x, y,[1023.,0,0]) # Very important to give reasonable initial coefficients
    print("Calculated coefficients: "+str(fit_func_coefs[sensor_i]))
    # NOTE for fit_func_coefs[sensor_i]
    # The coefficients can be passed to the chosen fitting functions using an * before, which expands them as arguments
    # Example: y = fit_func(x, *fit_func_coefs[sensor_i])
    # Same as: y = fit_func(x, fit_func_coefs[0], fit_func_coefs[1], fit_func_coefs[2])
    
    PLOT_FITTING = False
    if PLOT_FITTING:
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
        data[dist]['vals_cal'][sensor_i] = fit_func_inv(data[dist]['vals'][sensor_i], *fit_func_coefs[sensor_i])

# Polar plot with the calibrated measurements
PLOT_FITTING = False
if PLOT_FITTING:
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


# Light model
DIST_MAX = 1200
DIST_MIN = 600

angle = np.linspace(0,360,)
model = 

fig = plt.figure()
ax = fig.add_subplot(111,polar=True)
plt.plot(np.radians(angle),model, marker='.', markersize=3, ls='', label="Model")
ax.set_rmax(DIST_MAX) # Set larger range, since sensors not pointing directly to the light source will output longer distances
ax.set_theta_direction('clockwise')
ax.set_theta_zero_location('N')
ax.legend(loc='upper left')
ax.grid(True)
plt.suptitle("Light model at " + str(DIST_MIN) + "cm)")
#mySaveFig(plt,"graphs/","LDR_response_at_" + str(dist) + "cm.png")
plt.show()
plt.close(fig)

