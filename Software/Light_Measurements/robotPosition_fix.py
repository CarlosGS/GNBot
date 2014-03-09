#! /usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
from xbee import ZigBee
import time
import serial
from pprint import pprint
from math import *

from matplotlib import pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons
import numpy as np

from helper import *

import struct


WORLD_SIZE = (600.,600.)
LANDMARK_POS = (-90.,-90.)

robotPosition = (30.,100.)
robotAngle = radians(45.)

# Load the sensor calibration coefficients
fit_func_coefs = loadFromFile("./","fit_func_coefs.p")

# Load our data model
data = loadFromFile("./","LDR_data_light_model.p")
model = {}
model_distances = sorted(data.iterkeys())
dist_close = float(model_distances[0])
dist_middle = float(model_distances[1])
dist_far = float(model_distances[2])
for sensor_i in range(4):
    model[sensor_i] = {}
    model[sensor_i]['angles'] = data[dist_close]['angles_model'][sensor_i]
    model[sensor_i]['intensities'] = {}
    for distances in model_distances:
        model[sensor_i]['intensities'][distances] = data[distances]['distance_model'][sensor_i]

offsetAngle = 0 # average initial angle value for the three measurements
for dist in data.keys():
    print(data[dist]['iniAngle'])
    offsetAngle += data[dist]['iniAngle']
offsetAngle /= float(len(data.keys()))
#offsetAngle = 86
L_R_x = LANDMARK_POS[0]-dist_close
L_R_y = LANDMARK_POS[1]-dist_close
angleFromLight = -degrees(atan2(L_R_y,L_R_x))
offsetAngle -= angleFromLight
offsetAngle = offsetAngle % 360.
print("Offset angle: " + str(offsetAngle))

def sampleModelPoint(sensor_i,distance,angle):
    angles = model[sensor_i]['angles']
    if distance < dist_middle:
        t = (distance-dist_close)/(dist_middle-dist_close)
        intensities = (1.-t)*model[sensor_i]['intensities'][dist_close] + t*model[sensor_i]['intensities'][dist_middle]
    else:
        t = (distance-dist_middle)/(dist_far-dist_middle)
        intensities = (1.-t)*model[sensor_i]['intensities'][dist_middle] + t*model[sensor_i]['intensities'][dist_far]
    return sample_point(angle, angles, intensities)

SHOW_LIGHT_MODEL = True
if SHOW_LIGHT_MODEL:
    currentLDR = 0
    angles_model = np.linspace(0.,360.,num=50,endpoint=False)
    angles_model_radians = np.radians(angles_model)
    distance_model = [sampleModelPoint(currentLDR,400,angle) for angle in angles_model]

    fig = plt.figure()
    ax = fig.add_subplot(111,polar=True)
    l, = plt.plot(angles_model_radians,distance_model, label="Model")
    ax.set_rmax(model_distances[-1]*4)
    ax.set_theta_direction('clockwise')
    ax.set_theta_zero_location('N')
    ax.legend(loc='upper left')
    ax.grid(True)
    #plt.axis([0, 1, -10, 10])

    axcolor = 'lightgoldenrodyellow'
    axdist = plt.axes([0.25, 0.1, 0.65, 0.03], axisbg=axcolor)
    axangle  = plt.axes([0.25, 0.15, 0.65, 0.03], axisbg=axcolor)

    sdist = Slider(axdist, 'Distance', 0., 700., valinit=model_distances[0])
    sangle = Slider(axangle, 'Angle', 0., 360., valinit=0.)

    def update(val):
        distance_model = [sampleModelPoint(currentLDR,sdist.val,angle) for angle in angles_model]
        l.set_ydata(distance_model)
        fig.canvas.draw_idle()
    sdist.on_changed(update)
    sangle.on_changed(update)

    resetax = plt.axes([0.8, 0.025, 0.1, 0.04])
    button = Button(resetax, 'Reset', color=axcolor, hovercolor='0.975')
    def reset(event):
        sdist.reset()
        sangle.reset()
    button.on_clicked(reset)

    rax = plt.axes([0.025, 0.5, 0.15, 0.15], axisbg=axcolor)
    radio = RadioButtons(rax, (0,1,2,3), active=0)
    def colorfunc(label):
        global currentLDR
        currentLDR = int(label)
        update(0)
    radio.on_clicked(colorfunc)

    plt.show()
    plt.close(fig)


def evaluateError(angle,distance,m1,m2,m3,m4):
    offset = 45.
    angle += offset
    s1 = sampleModelPoint(0,distance,angle+90*2)
    s2 = sampleModelPoint(1,distance,angle+90*3)
    s3 = sampleModelPoint(2,distance,angle+90*0)
    s4 = sampleModelPoint(3,distance,angle+90*1)
    return (s1-m1)**2+(s2-m2)**2+(s3-m3)**2+(s4-m4)**2




plt.ion() # Real time plotting
fig_world = plt.figure(figsize=(15.0, 9.0))
ax_world = fig_world.add_subplot( 111 )

def redrawRobotPosition():
    global ax_world, robotPosition, robotAngle, robot_world
    robotSize = 15./2.
    x = robotPosition[0]
    y = robotPosition[1]
    dx = robotSize*cos(robotAngle)
    dy = robotSize*-sin(robotAngle)
    if robot_world:
        robot_world.remove()
        robot_world = None
    if not robot_world:
        robot_world = ax_world.arrow(x,y,dx,dy, shape='full', lw=1, length_includes_head=True, width=robotSize/2., head_width=robotSize)
    else:
        robot_world.set_x(x)
        robot_world.set_y(y)
        robot_world.set_dx(dx)
        robot_world.set_dy(dy)
        


landmark_world, = ax_world.plot(LANDMARK_POS[0],LANDMARK_POS[1], 'yo', markersize=50)
robot_world = None
redrawRobotPosition()

plt.xlim([LANDMARK_POS[0],WORLD_SIZE[0]])
plt.ylim([LANDMARK_POS[1],WORLD_SIZE[1]])
ax_world.set_aspect('equal')
plt.tight_layout()
plt.show()

# check http://www.lebsanft.org/?p=48
WINDOW_LEN = 500#20*60*1000/100
ydata1 = [0] * WINDOW_LEN
ydata2 = [0] * WINDOW_LEN

plt.ion()
fig = plt.figure(figsize=(15.0, 9.0))

ax1 = fig.add_subplot( 211 )
#ax1.set_yscale('log')
line1, = plt.plot(ydata1, linewidth=0.0001)
line1.set_antialiased(False)
plt.ylim([0,360])

ax2 = fig.add_subplot( 212 )
line2, = plt.plot(ydata2, linewidth=0.0001)
line2.set_antialiased(False)
plt.ylim([0,25])

plt.tight_layout()
plt.show()




updated = 0

PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Open serial port
ser = serial.Serial(PORT, BAUD_RATE)

magnetometerCalibration = {'xmax': -620.0, 'xmin': -1153.0, 'ymax': 391.0, 'ymin': -152.0} # Manually calibrated
CALIBRATE_MAGNETOMETER = False
def readCompassAngle(x,y):
    global magnetometerCalibration
    compassX = float(x)
    compassY = float(y)
    if CALIBRATE_MAGNETOMETER:
        if magnetometerCalibration['xmin'] > compassX: magnetometerCalibration['xmin'] = compassX
        if magnetometerCalibration['xmax'] < compassX: magnetometerCalibration['xmax'] = compassX
        if magnetometerCalibration['ymin'] > compassY: magnetometerCalibration['ymin'] = compassY
        if magnetometerCalibration['ymax'] < compassY: magnetometerCalibration['ymax'] = compassY
        pprint(magnetometerCalibration)
    compassX = mapVals(compassX, magnetometerCalibration['xmin'],magnetometerCalibration['xmax'], -1.,1.)
    compassY = mapVals(compassY, magnetometerCalibration['ymin'],magnetometerCalibration['ymax'], -1.,1.)
    angle = degrees(atan2(compassY,compassX))
    return angle

def fit_func_inv(y, a, b, c): # Inverse function, analitically determined
    return (-1/b) * np.log((y - c) / a)

robot_dest_addr_long = None
def message_received(data):
    global updated, robotAngle, robot_dest_addr_long, robotPosition
    #print(data)
    if not ('source_addr_long' in data.keys()) or not ('rf_data' in data.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = data['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
    if 'rf_data' in data.keys():
        rf_data = data['rf_data']
        #print(rf_data)
        
        data_vals = rf_data.split()
        angle = readCompassAngle(data_vals[1],data_vals[2])
        angle -= offsetAngle
        angle = angle % 360.
        #print(angle)
        ydata1.append(angle)
        del ydata1[0]
        
        robot_LDR_vals = {}
        for sensor_i in range(4):
           data_vals_i = sensor_i + 4 # items 4,5,6,7
           robot_LDR_vals[sensor_i] = 1023.-float(data_vals[data_vals_i])
           robot_LDR_vals[sensor_i] = fit_func_inv(robot_LDR_vals[sensor_i], *fit_func_coefs[sensor_i])
        
        m1 = robot_LDR_vals[0]
        m2 = robot_LDR_vals[1]
        m3 = robot_LDR_vals[2]
        m4 = robot_LDR_vals[3]
        
        minErr = 9000000.
        newPos = (0,0)
        for x in np.linspace(10.,200.,num=20):
            for y in np.linspace(10.,200.,num=20):
                L_R_x = LANDMARK_POS[0]-x
                L_R_y = LANDMARK_POS[1]-y
                absolute_angleToLight = (-degrees(atan2(L_R_y,L_R_x))) % 360.
                angleToLight = angle-absolute_angleToLight
                distanceToLight = np.sqrt((x-LANDMARK_POS[0])**2+(y+LANDMARK_POS[1])**2)
                #print()
                angleToLight = angleToLight % 360.
                err = evaluateError(angleToLight,distanceToLight,m1,m2,m3,m4)
                if minErr > err:
                    minErr = err
                    newPos = (x,y)
        robotPosition = newPos
        robotAngle = radians(angle)
        redrawRobotPosition()
        
        batt_ADC = data_vals[0]
        batt_ADC = float(batt_ADC[1:])
        battV = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
        ydata2.append(battV)
        del ydata2[0]
        updated = 0

def robotSetMotors(L,R):
    if not robot_dest_addr_long: return
    rf_data = struct.pack('b', L) + struct.pack('b', R)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)

# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        if not updated:
            updated = 1
            #ymin = float(min(ydata))-1
            #ymax = float(max(ydata))+1
            #plt.ylim([ymin,ymax])
            line1.set_xdata(range(WINDOW_LEN))
            line1.set_ydata(ydata1)  # update the data
            
            #fftdata = np.abs(np.fft.rfft(ydata))[1:]
            
            #ymin = float(min(fftdata))-1
            #ymax = float(max(fftdata))+1
            #ax2.set_ylim([ymin,ymax])
            line2.set_xdata(range(WINDOW_LEN))
            line2.set_ydata(ydata2)
            
            fig.canvas.draw() # redraw the plot
            fig_world.canvas.draw()
        else: time.sleep(0.01)
    except KeyboardInterrupt:
        break

robotSetMotors(0,0)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

