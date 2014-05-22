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
import random


WORLD_SIZE = (600.,600.)
LANDMARK_POS = (WORLD_SIZE[0]/2.,WORLD_SIZE[1]/2.)

robotPosition = (30.,100.)
robotPositions = range(2)
robotPositions[0] = []
robotPositions[1] = []
robotAngle = radians(45.)

# Load the sensor calibration coefficients
fit_func_coefs = loadFromFile("./","fit_func_coefs.p")

# Load our data model
data = loadFromFile("./","LDR_data_light_model.p")
model = {}
DIST_NEAR = 100
DIST_MIDDLE = 200
DIST_FAR = 500
model_distances = [DIST_NEAR,DIST_MIDDLE,DIST_FAR]
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
    global ax_world, robotPosition, robotAngle, robot_world, robotPositions, robotPositions_world
    robotSize = 3*15./2.
    x = robotPosition[0]
    y = robotPosition[1]
    dx = robotSize*cos(robotAngle)
    dy = robotSize*-sin(robotAngle)
    if robot_world:
        robot_world.remove()
        robot_world = None
    robotPositions[0].append(x)
    robotPositions[1].append(y)
    if not robotPositions_world:
        robotPositions_world, = ax_world.plot(robotPositions[0],robotPositions[1], 'c')
    else:
        robotPositions_world.set_xdata(robotPositions[0])
        robotPositions_world.set_ydata(robotPositions[1])
    if not robot_world:
        robot_world = ax_world.arrow(x,y,dx,dy, shape='full', lw=1, length_includes_head=True, width=robotSize/2., head_width=robotSize, zorder=100)
    else:
        robot_world.set_x(x)
        robot_world.set_y(y)
        robot_world.set_dx(dx)
        robot_world.set_dy(dy)

N_particles = 500
particles = range(N_particles)
for i in range(N_particles):
    particles[i] = {}
    particles[i]['x'] = 0.
    particles[i]['y'] = 0.
    particles[i]['ang'] = 0.
    particles[i]['w'] = 0.

particles_plotX = [p['x'] for p in particles]
particles_plotY = [p['y'] for p in particles]

particles_world, = ax_world.plot(particles_plotX,particles_plotY, 'ro', markersize=3)
landmark_world, = ax_world.plot(LANDMARK_POS[0],LANDMARK_POS[1], 'yo', markersize=50)

robot_world = None
robotPositions_world = None
redrawRobotPosition()

plt.xlim([0,WORLD_SIZE[0]])
plt.ylim([0,WORLD_SIZE[1]])
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
    #return y
    res = 0
    if y - c <= 0:
        res = DIST_FAR*3
    else:
        res =  (-1/b) * np.log((y - c) / a)
    return res
    #return (-1/b) * np.log((y - c) / a)

robot_dest_addr_long = None
velocity = [0,0]
processing = False
def message_received(data):
    global updated, robotAngle, robot_dest_addr_long, robotPosition, particles, velocity, processing, end
    if processing or end:
        print("Skipping")
        return
    #print(data)
    if not ('source_addr_long' in data.keys()) or not ('rf_data' in data.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = data['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
    if 'rf_data' in data.keys():
        processing = True
#    if 1:
        rf_data = data['rf_data']
        #print(rf_data)
        
        data_vals = rf_data.split()
        angle = readCompassAngle(data_vals[1],data_vals[2])
        angle -= offsetAngle
        angle = angle % 360.
        #angle = angleFromLight#+time.time()*57.
        #angle = angle % 360.
        #print(angle)
        ydata1.append(angle)
        del ydata1[0]
        
        robot_LDR_vals = {}
        for sensor_i in range(4):
            data_vals_i = sensor_i + 4 # items 4,5,6,7
            robot_LDR_vals[sensor_i] = 1023.-float(data_vals[data_vals_i])
            robot_LDR_vals[sensor_i] = fit_func_inv(robot_LDR_vals[sensor_i], *fit_func_coefs[sensor_i])
        #print(robot_LDR_vals)
#        x = 300+50*cos(0.4*time.time())
#        y = 300+100*sin(0.4*time.time())
#        L_R_x = LANDMARK_POS[0]-x
#        L_R_y = LANDMARK_POS[1]-y
#        absolute_angleToLight = (-degrees(atan2(L_R_y,L_R_x))) % 360.
#        angleToLight = angle-absolute_angleToLight
#        distanceToLight = np.sqrt((x-LANDMARK_POS[0])**2+(y+LANDMARK_POS[1])**2)
#        angleToLight = angleToLight % 360.
#        offset = 45.
#        angleToLight += offset
#        robot_LDR_vals[0] = sampleModelPoint(0,distanceToLight,angleToLight+90*2)
#        robot_LDR_vals[1] = sampleModelPoint(1,distanceToLight,angleToLight+90*3)
#        robot_LDR_vals[2] = sampleModelPoint(2,distanceToLight,angleToLight+90*0)
#        robot_LDR_vals[3] = sampleModelPoint(3,distanceToLight,angleToLight+90*1)
        
        # Give more importance to the sensors pointing closest to the light
        vals = [robot_LDR_vals[key] for key in robot_LDR_vals.keys()]
        sensor_i_min1 = np.argmin(vals)
        vals[sensor_i_min1] = 9999
        sensor_i_min2 = np.argmin(vals)
        robot_LDR_pond = {}
        for sensor_i in range(4):
            if sensor_i_min1 == sensor_i or sensor_i_min2 == sensor_i:
                robot_LDR_pond[sensor_i] = 1.
            else:
                robot_LDR_pond[sensor_i] = 0.5
        #print(robot_LDR_vals)
        #print(robot_LDR_pond)
        # 3) Resample: new set of particles are chosen such that each particle survives in proportion to its weight
        for i in range(N_particles):
            if particles[i]['w'] > 0.99/N_particles and within(particles[i]['x'],0,WORLD_SIZE[0]) and within(particles[i]['y'],0,WORLD_SIZE[1]):
                continue
            if random.uniform(0,100) < 25:
                particles[i]['x'] = random.uniform(0,WORLD_SIZE[0])
                particles[i]['y'] = random.uniform(0,WORLD_SIZE[1])
                particles[i]['ang'] = angle+random.normalvariate(0,90)
                particles[i]['ang'] = particles[i]['ang'] % 360.
                particles[i]['w'] = 0.
            else:
                noise = 50
                particles[i]['x'] = random.normalvariate(robotPosition[0],noise)
                particles[i]['y'] = random.normalvariate(robotPosition[1],noise)
                particles[i]['ang'] = angle+random.normalvariate(0,2)
                particles[i]['ang'] = particles[i]['ang'] % 360.
                particles[i]['w'] = 0.
        
        # 1) Prediction: for each particle, sample and add random, noisy values from action model
        for i in range(N_particles):
            particles[i]['x'] += velocity[0]*random.uniform(0,1.5)
            particles[i]['y'] += velocity[1]*random.uniform(0,1.5)
            noise = 1
            particles[i]['x'] += random.normalvariate(0,noise)
            particles[i]['y'] += random.normalvariate(0,noise)
            noise = 1
            particles[i]['ang'] += random.normalvariate(0,noise)
            particles[i]['ang'] = particles[i]['ang'] % 360.
        
        # 2) Update: each particle's weight is the likelihood of getting the sensor readings from that particle's hypothesis
        totalWeight = 0.
        for i in range(N_particles):
            x = particles[i]['x']
            y = particles[i]['y']
            L_R_x = LANDMARK_POS[0]-x
            L_R_y = LANDMARK_POS[1]-y
            absolute_angleToLight = (-degrees(atan2(L_R_y,L_R_x))) % 360.
            angleToLight = angle-absolute_angleToLight
            distanceToLight = np.sqrt((x-LANDMARK_POS[0])**2+(y+LANDMARK_POS[1])**2)
            angleToLight = angleToLight % 360.
            
            compass_STD = 1.
            ldr_STD = 8.
            
            mean_angle = particles[i]['ang']
            particles[i]['w'] = 0.#pdf(mean_angle, compass_STD, angle)
            
            offset = 45.
            ang = angleToLight+offset
            mean_s1 = sampleModelPoint(0,distanceToLight,ang+90*2) # "true" distance
            particles[i]['w'] += pdf(mean_s1, ldr_STD, robot_LDR_vals[0])*robot_LDR_pond[0]
            
            mean_s2 = sampleModelPoint(1,distanceToLight,ang+90*3)
            particles[i]['w'] += pdf(mean_s2, ldr_STD, robot_LDR_vals[1])*robot_LDR_pond[1]
            
            mean_s3 = sampleModelPoint(2,distanceToLight,ang+90*0)
            particles[i]['w'] += pdf(mean_s3, ldr_STD, robot_LDR_vals[2])*robot_LDR_pond[2]
            
            mean_s4 = sampleModelPoint(3,distanceToLight,ang+90*1)
            particles[i]['w'] += pdf(mean_s4, ldr_STD, robot_LDR_vals[3])*robot_LDR_pond[3]
            
            totalWeight += particles[i]['w']
        
        # Renormalize weights
        maxw = 0.
        newPos = (0,0)
        for i in range(N_particles):
            if totalWeight == 0:
                particles[i]['w'] = 1./N_particles
            else:
                particles[i]['w'] /= totalWeight
            if particles[i]['w'] > maxw:
                maxw = particles[i]['w']
                newPos = (particles[i]['x'],particles[i]['y'])
        
        # Get the average of the most likely particles
        avgX = 0.
        avgY = 0.
        n = 0
        for i in range(N_particles):
            if particles[i]['w'] < 0.9/N_particles:
                continue    
            avgX += particles[i]['x']*particles[i]['w']
            avgY += particles[i]['y']*particles[i]['w']
            n += particles[i]['w']
        avgX /= n
        avgY /= n
        
        newPos = (avgX,avgY)
        
#        minErr = 9000000.
#        newPos = (0,0)
#        for x in np.linspace(50.,600.,num=30):
#            for y in np.linspace(50.,600.,num=30):
#                L_R_x = LANDMARK_POS[0]-x
#                L_R_y = LANDMARK_POS[1]-y
#                absolute_angleToLight = (-degrees(atan2(L_R_y,L_R_x))) % 360.
#                angleToLight = angle-absolute_angleToLight
#                distanceToLight = np.sqrt((x-LANDMARK_POS[0])**2+(y+LANDMARK_POS[1])**2)
#                #print()
#                angleToLight = angleToLight % 360.
#                err = evaluateError(angleToLight,distanceToLight,m1,m2,m3,m4)
#                if minErr > err:
#                    minErr = err
#                    newPos = (x,y)
        velocity[0] = newPos[0]-robotPosition[0]
        velocity[1] = newPos[1]-robotPosition[1]
        if abs(velocity[0]) > 5: velocity[0] = 0
        if abs(velocity[1]) > 5: velocity[1] = 0
        robotPosition = newPos
        robotAngle = radians(angle)
        redrawRobotPosition()
        
        spinSpeed = 3
        forwardSpeed = 15
        dstAngle = 0
        
        x = robotPosition[0]
        y = robotPosition[1]
        destx = 0
        desty = 0
        L_R_x = destx-x
        L_R_y = desty-y
        absolute_angleToLight = (-degrees(atan2(L_R_y,L_R_x))) % 360.
        angleToLight = angle-absolute_angleToLight
        distanceToLight = np.sqrt((x-destx)**2+(y+destx)**2)
        #angleToLight = angleToLight % 360.
        angle = angleToLight
        #angle = mapVals(angle,0.,360.,-180.,180.)
        #angle = angle % 360.
        #print(angle)
        if abs(angle) < 0.5:
            spinSpeed *= 0
        elif abs(angle) < 20:
            spinSpeed *= 2
        elif abs(angle) < 30:
            spinSpeed *= 3
        elif abs(angle) < 40:
            spinSpeed *= 4
        else:
            spinSpeed *= 5
        
#        if distanceToLight > 30:
#            forwardSpeed = int(mapVals(distanceToLight,100.,0.,20.,5.))
#            if forwardSpeed > 15: forwardSpeed = 15
#            spinSpeed *= mapVals(distanceToLight,100.,0.,1.,2.)
#            spinSpeed = int(spinSpeed)
#            if spinSpeed > 10: spinSpeed = 10
#        else:
#            forwardSpeed = 0
#            spinSpeed = 0
        
#        if angle < 0:
#            robotSetMotors(forwardSpeed+spinSpeed,forwardSpeed-spinSpeed)
#        else:
#            robotSetMotors(forwardSpeed-spinSpeed,forwardSpeed+spinSpeed)
        
        batt_ADC = data_vals[0]
        batt_ADC = float(batt_ADC[1:])
        battV = mapVals(batt_ADC,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
        ydata2.append(battV)
        del ydata2[0]
        updated = 0
        processing = False

def robotSetMotors(L,R):
    if not robot_dest_addr_long: return
    rf_data = struct.pack('b', L) + struct.pack('b', R)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)


end = False
# Create API object, which spawns a new thread
zb = ZigBee(ser, escaped = True, callback=message_received)

# Do other stuff in the main thread
while True:
    try:
        #message_received("ajshkjash")
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
            
            particles_plotX = [p['x'] for p in particles]
            particles_plotY = [p['y'] for p in particles]

            particles_world.set_xdata(particles_plotX)
            particles_world.set_ydata(particles_plotY)
            
            fig.canvas.draw() # redraw the plot
            fig_world.canvas.draw()
        else: time.sleep(0.01)
        #time.sleep(0.1)
    except KeyboardInterrupt:
        break

end = True
time.sleep(0.2)
robotSetMotors(0,0)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
zb.halt()
ser.close()

