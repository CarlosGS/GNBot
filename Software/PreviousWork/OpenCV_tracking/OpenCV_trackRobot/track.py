#! /usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlos Garcia Saura (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

# Begin modules
import time
from pprint import pprint
from math import *
import datetime

from helper import *

from matplotlib import pyplot as plt
import numpy as np

import atexit
import struct

import sys

import cv
import cv2
# End modules







cap = cv2.VideoCapture(0)

width = 1920 #leave None for auto-detection
height = 1080 #leave None for auto-detection
width = 1280 #leave None for auto-detection
height = 720 #leave None for auto-detection

cap.set(cv.CV_CAP_PROP_FRAME_WIDTH,width)
cap.set(cv.CV_CAP_PROP_FRAME_HEIGHT,height)

fps_rate = 10
cap.set(cv.CV_CAP_PROP_FPS,fps_rate)

# Define the codec and create VideoWriter object
#fourcc = cv2.VideoWriter_fourcc(*'XVID') # http://answers.opencv.org/question/29648/attributeerror-module-object-has-no-attribute/
fourcc = cv2.cv.CV_FOURCC(*'MJPG') #MJPG
out = cv2.VideoWriter('output.avi',fourcc, fps_rate, (width,height))

points = []
M = None
p = None
maxWidth = 720
maxHeight = 720+200
verticalOffset = maxHeight/6
margin = 50/2;
cm_per_pixel = 25./(2.*float(margin)) # 25cm square


cursorX = 0
cursorY = 0
def my_mouse_callback(event,x,y,flags,param):
    global points, M, p, maxWidth, maxHeight, cursorX, cursorY
    cursorX = x
    cursorY = y
    #if event == cv.CV_EVENT_LBUTTONDBLCLK:
    if event == cv.CV_EVENT_LBUTTONUP:
        if len(points) < 4:
            print("Picked X: " + str(x) + " Y: " + str(y))
            points.append([x,y])
        if len(points) == 4:
            src = np.array(points,np.float32)
            #dst = np.array([[0,0],[maxWidth,0],[maxWidth,maxHeight],[0,maxHeight]],np.float32)
            #dst = np.array([[margin,margin],[maxWidth-margin,margin],[maxWidth-margin,maxHeight-margin],[margin,maxHeight-margin]],np.float32)
            dst = np.array([[maxWidth/2-margin,maxHeight/2+verticalOffset-margin],[maxWidth/2+margin,maxHeight/2+verticalOffset-margin],[maxWidth/2+margin,maxHeight/2+verticalOffset+margin],[maxWidth/2-margin,maxHeight/2+verticalOffset+margin]],np.float32)
            M = cv2.getPerspectiveTransform(src,dst)
            print(M)
            p = [(x,y) for [x,y] in points]
            points = []

#M = np.array([[5.26999545e-01,1.95787772e-01,-2.60864560e+01], [-1.38841133e-01,1.32747758e+00,2.80909185e+02], [-3.33195728e-04,7.43301834e-04,1.00000000e+00]])

if M == None:
    cv2.namedWindow("Perspective reference points")
    cv2.setMouseCallback("Perspective reference points",my_mouse_callback)
cv2.namedWindow("Corrected Perspective", cv2.CV_WINDOW_AUTOSIZE)



warp_hsv = 0
ROIsize = margin/2#(80/4)/2
ROIcenterX = maxWidth/2
ROIcenterY = maxHeight/2
good_color = (43,96,215)
good_color_angle = (30,80,215)
def my_mouse_callback_2(event,x,y,flags,param):
    global warp_hsv, ROIcenterX, ROIcenterY, good_color
    #if event == cv.CV_EVENT_LBUTTONDBLCLK:
    if event == cv.CV_EVENT_LBUTTONUP:
        good_color = warp_hsv[y,x]
        print(good_color)
        ROIcenterX = x
        ROIcenterY = y

angle_marker_X = 0
angle_marker_Y = 0
def my_mouse_callback_3(event,x,y,flags,param):
    global warp_hsv, good_color_angle, roi_big
    if event == cv.CV_EVENT_LBUTTONUP:
        angle_marker_X = x
        angle_marker_Y = y
        good_color_angle = roi_big[y,x]
        print(good_color_angle)


recording = False

prevCentroidsX = [0,0,0]
prevCentroidsY = [0,0,0]
cv2.namedWindow("Corrected Perspective")
cv2.setMouseCallback("Corrected Perspective",my_mouse_callback_2)
cv2.namedWindow("roi", cv2.CV_WINDOW_AUTOSIZE)
cv2.setMouseCallback("roi",my_mouse_callback_3)
while(1):
    _,frame = cap.read()
    if M != None:
        warp = cv2.warpPerspective(frame, M, (maxWidth, maxHeight))
        
        if p != None:
            cv2.line(frame, p[0], p[1], (255,0,0)) # (b,g,r)
            cv2.line(frame, p[1], p[2], (255,0,0))
            cv2.line(frame, p[2], p[3], (255,0,0))
            cv2.line(frame, p[3], p[0], (255,0,0))
        
        #warp = cv2.bilateralFilter(warp, d=5, sigmaColor=10, sigmaSpace=10)
        #warp = cv2.GaussianBlur(warp, ksize=np.zeros(0,0), sigmaX=5)
        
        # convert to hsv and find range of colors
        warp_hsv = cv2.cvtColor(warp,cv2.COLOR_BGR2HSV)
        
        thresh = cv2.inRange(warp_hsv,np.array((good_color[0]-10, good_color[1]-30, 80)), np.array((good_color[0]+10, good_color[1]+30, 255)))
        #element = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
        #thresh = cv2.erode(thresh,element,iterations=1)
        #########################cv2.imshow("Color threshold",thresh)
        #thresh = cv2.inRange(warp_hsv,np.array((0, 80, 80)), np.array((20, 255, 255)))
        
        # find contours in the threshold image
        contours,hierarchy = cv2.findContours(thresh,cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)#cv2.CHAIN_APPROX_SIMPLE)
        
        # finding contour with maximum area and store it as best_cnt
        max_area = 0
        best_cnt = None
        for cnt in contours:
            area = cv2.contourArea(cnt)
            
            if area > max_area:
                max_area = area
                best_cnt = cnt
        if best_cnt != None:
            #print(best_cnt)
            #print(best_cnt.shape)
            best_cnt = np.mean(best_cnt,axis=0)
            while len(best_cnt.shape) > 1:
                best_cnt = best_cnt[0]
            #print("Centroid = ", str(best_cnt))
            
            prevCentroidsX.pop(0)
            prevCentroidsY.pop(0)
            prevCentroidsX.append(best_cnt[0])
            prevCentroidsY.append(best_cnt[1])
            
            ROIcenterX = np.mean(prevCentroidsX)
            ROIcenterY = np.mean(prevCentroidsY)
            robotPosX = (ROIcenterX-maxWidth/2)*cm_per_pixel
            robotPosY = (ROIcenterY-(maxHeight/2+verticalOffset))*cm_per_pixel
            print("X= " + str(round(robotPosX)) + " Y=" + str(round(robotPosY)))
            #ROIcenterX = best_cnt[0]
            #ROIcenterY = best_cnt[1]
            cv2.circle(warp, (int(round(ROIcenterX)),int(round(ROIcenterY))), 10, (255,0,0), 2)
        cv2.imshow("Corrected Perspective",warp)
        
        src = np.array([[ROIcenterX-ROIsize,ROIcenterY-ROIsize],[ROIcenterX+ROIsize,ROIcenterY-ROIsize],[ROIcenterX+ROIsize,ROIcenterY+ROIsize]],np.float32)
        dst = np.array([[0,0],[200,0],[200,200]],np.float32)
        M_aff = cv2.getAffineTransform(src, dst)
        roi_big = cv2.warpAffine(cv2.cvtColor(warp_hsv,cv2.COLOR_HSV2BGR), M_aff, (200,200))
        #roi = warp_hsv[ROIcenterY-ROIsize:ROIcenterY+ROIsize, ROIcenterX-ROIsize:ROIcenterX+ROIsize]
        #roi_big = cv2.resize(cv2.cvtColor(roi,cv2.COLOR_HSV2BGR), (200,200))
        cv2.imshow("roi",roi_big)
    crossSize = 10
    cv2.line(frame, (cursorX-crossSize,cursorY), (cursorX+crossSize,cursorY), (0,0,255)) # (b,g,r)
    cv2.line(frame, (cursorX,cursorY-crossSize), (cursorX,cursorY+crossSize), (0,0,255)) # (b,g,r)
    cv2.putText(frame, "Corner " + str(len(points)+1), (cursorX+5,cursorY-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,255))
    if M == None:
        cv2.imshow("Perspective reference points",frame)
    
    key = cv2.waitKey(1) & 0xFF
    if key == ord('r'):
        recording = True
    if key == ord('t'):
        recording = False
    if key == ord('q'):
        break
    if key == ord('o'):# OK continue with these points
        break
    if recording:
        out.write(frame)










PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Open serial port
#ser = serial.Serial(PORT, BAUD_RATE)

def robotSetMotors_single(L,R):
    if not robot_dest_addr_long: return
    rf_data = struct.pack('b', L) + struct.pack('b', R)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)

def robotSetMotors(L,R):
    robotSetMotors_single(L,R)
    time.sleep(0.1)
    robotSetMotors_single(L,R)
    time.sleep(0.1)
    robotSetMotors_single(L,R)

def robotPlayNote(note,ms):
    if not robot_dest_addr_long or note <= 0: return
    L = 0
    R = 0
    note = note
    ms_scaled = int(round(ms/100.))
    rf_data = struct.pack('b', L) + struct.pack('b', R) + struct.pack('b', note) + struct.pack('b', ms_scaled)
    zb.send("tx", dest_addr='\xFF\xFE', dest_addr_long=robot_dest_addr_long, data=rf_data)
    time.sleep(ms/1000.)

def mapf(x, in_min, in_max, out_min, out_max):
    try:
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    except:
        return out_min

def toResistance(v_adc):
    Vc = 5.
    Rl = 10000.
    if v_adc <= 0: v_adc = 0.000001
    Vout = mapf(v_adc,0,1023, 0.,Vc)
    Rs = Rl*(Vc-Vout)/Vout
    return Rs

def distance(p1x,p1y,p2x,p2y):
    return np.sqrt((p1x-p2x)**2+(p1y-p2y)**2)

robot_dest_addr_long = None
robot_battery = 999
turn_dir = 1
backing_up = False
waiting = 0
robotPosX = 0
robotPosY = 0

pointList = []
#pointList.append((0,0))
it = 0
for x in [-20,20]:
    for y in np.linspace(-100, 100, num=200, endpoint=True):
        if it%2 == 0:
            pointList.append((x,y))
        else:
            pointList.append((x,-y))
    it += 1

print("pointList = " + str(pointList))

robotAngle = 0
def message_received(datain):
    global robot_dest_addr_long, robot_battery, current_logging, data, robot_speed, turn_dir, backing_up, waiting, robotAngle, robotPosX, robotPosY
    if not ('source_addr_long' in datain.keys()) or not ('rf_data' in datain.keys()): return
    if not robot_dest_addr_long:
        robot_dest_addr_long = datain['source_addr_long']
        print("Robot address: ")
        pprint(robot_dest_addr_long)
        robotSetMotors(0,0)
    try:
        words = datain['rf_data'].split()
        values = [float(word) for word in words]
    except:
        return
    #print(values)
    
    batt_value = values[0]
    robot_battery = mapf(batt_value,0.,1023., 0.,21.1765); # Voltage divider with 22k in series with 6k8
    #print("Battery: " + str(robot_battery) + " V")
    
    nose_value_min = values[1]
    nose_value_max = values[2]
    nose_resistance = (toResistance(nose_value_min)+toResistance(nose_value_max))/2
    #print("Nose: " + str(nose_resistance) + " Ohm")
    
    irDistance_value_min = values[3]
    irDistance_value_max = values[4]
    #print("IR distance min: " + str(irDistance_value_min))
    #print("IR distance max: " + str(irDistance_value_max))
    
    temperature = values[5]
    #print("Temperature: " + str(temperature) + " Degrees C")
    humidity = values[6]
    #print("Humidity: " + str(humidity) + " Rel %")
    #print("\n")
    
    if current_logging:
        timeStamp = datetime.datetime.now()
        print(values)
        print("PosX: " + str(robotPosX) + " cm")
        print("PosY: " + str(robotPosY) + " cm")
        print("Angle: " + str(robotAngle) + " deg")
        print("Battery: " + str(robot_battery) + " V")
        print("Nose: " + str(nose_resistance) + " Ohm")
        print("IR distance min: " + str(irDistance_value_min))
        print("IR distance max: " + str(irDistance_value_max))
        print("Temperature: " + str(temperature) + " Degrees C")
        print("Humidity: " + str(humidity) + " Rel %")
        print("\n")
        data['time'].append(timeStamp)
        data['battery'].append(robot_battery)
        data['IRdistMin'].append(irDistance_value_min)
        data['IRdistMax'].append(irDistance_value_max)
        data['noseResistance'].append(nose_resistance)
        data['noseValMin'].append(nose_value_min)
        data['noseValMax'].append(nose_value_max)
        data['humidity'].append(humidity)
        data['temperature'].append(temperature)
        
        data['robotPosX'].append(robotPosX)
        data['robotPosY'].append(robotPosY)
        data['robotAngle'].append(robotAngle)
        
        if len(pointList) > 0:
            dstX = pointList[0][0]
            dstY = pointList[0][1]
            if distance(robotPosX,robotPosY,dstX,dstY) < 10:
                robotSetMotors_single(0,0)
                pointList.pop(0)
            
            motorHeading = 0
            motorSpeed = 10
            
            dstAngle = 180.*np.arctan2(-(robotPosY-dstY), robotPosX-dstX)/np.pi
            #dstAngle = 0
            dstAngle %= 360.
            dstAngle -= 180.
            print("robotAngle: " + str(robotAngle))
            print("dstAngle: " + str(dstAngle))
            if abs(robotAngle-dstAngle) > 4:
                if robotAngle > dstAngle:
                    motorHeading = 4
                else:
                    motorHeading = -4
            else:
                motorHeading = 0
            motorSpeedL = motorSpeed+motorHeading
            motorSpeedR = motorSpeed-motorHeading
            robotSetMotors_single(int(round(motorSpeedL)),int(round(motorSpeedR)))
        else:
            current_logging = False
        #robotSetMotors_single(motorSpeedL,motorSpeedR)
        
        #data['speedL'].append(motorSpeedL)
        #data['speedR'].append(motorSpeedR)
        
        #print("Batt: "+str(robot_battery))
        #print(robot_battery,robot_IRdist,variance)
    
    


data = {}
current_logging = False
current_dist = 0

# Create API object, which spawns a new thread
#zb = ZigBee(ser, escaped = True, callback=message_received)




time.sleep(1)


data['battery'] = []
data['IRdistMin'] = []
data['IRdistMax'] = []
data['noseResistance'] = []
data['noseValMin'] = []
data['noseValMax'] = []
data['humidity'] = []
data['temperature'] = []
data['speedL'] = []
data['speedR'] = []
data['time'] = []
data['robotPosX'] = []
data['robotPosY'] = []
data['robotAngle'] = []

print("\nPosition the robot and press B! Battery: "+str(round(robot_battery,2))+"V")
#val = sys.stdin.readline()

count = 0
while 1:
    _,frame = cap.read()
    if M != None:
        warp = cv2.warpPerspective(frame, M, (maxWidth, maxHeight))
        
        #warp = cv2.bilateralFilter(warp, d=5, sigmaColor=10, sigmaSpace=10)
        #warp = cv2.GaussianBlur(warp, ksize=np.zeros(0,0), sigmaX=5)
        
        # convert to hsv and find range of colors
        warp_hsv = cv2.cvtColor(warp,cv2.COLOR_BGR2HSV)
        
        thresh = cv2.inRange(warp_hsv,np.array((good_color[0]-10, good_color[1]-30, 80)), np.array((good_color[0]+10, good_color[1]+30, 255)))
        #element = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
        #thresh = cv2.erode(thresh,element,iterations=1)
        #########################cv2.imshow("Color threshold",thresh)
        #thresh = cv2.inRange(warp_hsv,np.array((0, 80, 80)), np.array((20, 255, 255)))
        
        # find contours in the threshold image
        contours,hierarchy = cv2.findContours(thresh,cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)#cv2.CHAIN_APPROX_SIMPLE)
        
        # finding contour with maximum area and store it as best_cnt
        max_area = 0
        best_cnt = None
        for cnt in contours:
            area = cv2.contourArea(cnt)
            
            if area > max_area:
                max_area = area
                best_cnt = cnt
        if best_cnt != None:
            #print(best_cnt)
            #print(best_cnt.shape)
            best_cnt = np.mean(best_cnt,axis=0)
            while len(best_cnt.shape) > 1:
                best_cnt = best_cnt[0]
            #print("Centroid = ", str(best_cnt))
            
            prevCentroidsX.pop(0)
            prevCentroidsY.pop(0)
            prevCentroidsX.append(best_cnt[0])
            prevCentroidsY.append(best_cnt[1])
            
            ROIcenterX = np.mean(prevCentroidsX)
            ROIcenterY = np.mean(prevCentroidsY)
            robotPosX = (ROIcenterX-maxWidth/2)*cm_per_pixel
            robotPosY = (ROIcenterY-(maxHeight/2+verticalOffset))*cm_per_pixel
            #print("X= " + str(round(robotPosX)) + " Y=" + str(round(robotPosY)))
            #ROIcenterX = best_cnt[0]
            #ROIcenterY = best_cnt[1]
            cv2.circle(warp, (int(round(ROIcenterX)),int(round(ROIcenterY))), 10, (255,0,0), 2)
        
        
        src = np.array([[ROIcenterX-ROIsize,ROIcenterY-ROIsize],[ROIcenterX+ROIsize,ROIcenterY-ROIsize],[ROIcenterX+ROIsize,ROIcenterY+ROIsize]],np.float32)
        dst = np.array([[0,0],[200,0],[200,200]],np.float32)
        M_aff = cv2.getAffineTransform(src, dst)
        roi_size_px = 200
        roi_big = cv2.warpAffine(warp_hsv, M_aff, (roi_size_px,roi_size_px))
        
        roi_big_rgb = cv2.cvtColor(roi_big,cv2.COLOR_HSV2BGR)
        
        thresh_angle = cv2.inRange(roi_big,np.array((good_color_angle[0]-30, good_color_angle[1]-30, 180)), np.array((good_color_angle[0]+5, good_color_angle[1]+30, 255)))
        
        element = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
        thresh_angle = cv2.erode(thresh_angle,element,iterations=2)
        
        cv2.imshow("thresh_angle", thresh_angle)
        # find contours in the threshold image
        contours,hierarchy = cv2.findContours(thresh_angle,cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)
        
        # finding contour with maximum area and store it as best_cnt
        max_area = 0
        best_cnt = None
        for cnt in contours:
            area = cv2.contourArea(cnt)
            
            if area > max_area:
                max_area = area
                best_cnt = cnt
        if best_cnt != None:
            #print(best_cnt)
            #print(best_cnt.shape)
            best_cnt = np.mean(best_cnt,axis=0)
            while len(best_cnt.shape) > 1:
                best_cnt = best_cnt[0]
            #print("Centroid = ", str(best_cnt))
            
            angle_marker_X = float(best_cnt[0])
            angle_marker_Y = float(best_cnt[1])
            robotAngle = 180.*np.arctan2(-(angle_marker_Y-roi_size_px/2), angle_marker_X-roi_size_px/2)/np.pi
            #robotAngle += 180.
            robotAngle %= 360.
            robotAngle -= 180.
            #print("robotAngle: " + str(robotAngle))
            
            cv2.circle(roi_big_rgb, (int(round(angle_marker_X)),int(round(angle_marker_Y))), 10, (255,0,0), 2)
            timeStamp = datetime.datetime.now()
            data['time'].append(timeStamp)
            data['robotPosX'].append(robotPosX)
            data['robotPosY'].append(robotPosY)
            data['robotAngle'].append(robotAngle)
        
        cv2.imshow("roi",roi_big_rgb)
        cv2.imshow("Corrected Perspective",warp)
        
    
    key = cv2.waitKey(1) & 0xFF
    speed_M = 10
    if key == ord('w'):
        robotSetMotors_single(speed_M,speed_M)
    if key == ord('a'):
        robotSetMotors_single(-speed_M,speed_M)
    if key == ord('s'):
        robotSetMotors_single(-speed_M,-speed_M)
    if key == ord('d'):
        robotSetMotors_single(speed_M,-speed_M)
    if key == ord('e'):
        robotSetMotors_single(0,0)
    if key == ord('y'):
        ms = 1000
        robotPlayNote(3,ms/4)
        robotPlayNote(1,ms/4)
        robotPlayNote(3,ms/4)
        robotPlayNote(1,ms/4)
        robotPlayNote(3,ms/4)
        robotPlayNote(5,ms/4)
        robotPlayNote(5,ms/2)
        time.sleep(0.1)
        robotPlayNote(4,ms/4)
        robotPlayNote(2,ms/4)
        robotPlayNote(2,ms/2)
        time.sleep(0.1)
        robotPlayNote(3,ms/4)
        robotPlayNote(1,ms/4)
        robotPlayNote(1,ms/2)
        #for i in range(7*4):
        #    robotPlayNote(i+1,100)
        #    time.sleep(0.5)
    
    if key == ord('b'):
        current_logging = True
    
    if key == ord('r'):
        recording = True
    if key == ord('t'):
        recording = False
    if key == ord('q'):
        break
    if recording:
        if count > 25:
            out.write(frame)
            count = 0
        count += 1
    

current_logging = False

time.sleep(0.3)
robotSetMotors(0,0)

saveToFile(data,"./","logged_data.p")

#pprint(data)

# halt() must be called before closing the serial
# port in order to ensure proper thread shutdown
#zb.halt()
#ser.close()

# Clean up everything before leaving
cap.release()
out.release()
cv2.destroyAllWindows()

os._exit(0)

