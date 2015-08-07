#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np


data_file_name = "2015-08-07-123434"


capture = cv2.VideoCapture(data_file_name+".webm")
#capture.set(cv2.cv.CV_CAP_PROP_FPS,60)
#print("Reading file at FPS: "+str(capture.get(cv2.cv.CV_CAP_PROP_FPS)))


data = loadFromFile("./","perspective_correction_data.p")
perspective_correction_matrix = data['perspective_correction_matrix']
warpSize = data['warpSize']
cm_per_pixel = data['cm_per_pixel']
dst_points = [(int(x),int(y)) for (x,y) in data['dst_points']]


updated = False
warp_hsv = []
MARKER_COLOR = []
def mouse_callback(event,x,y,flags,param):
    global warp_hsv, MARKER_COLOR, updated
    if event == cv2.cv.CV_EVENT_LBUTTONUP:
        MARKER_COLOR = warp_hsv[y,x]
        print("Color:")
        print(MARKER_COLOR)
        updated = True


selectionWindow = "Click marker to select color threshold"
cv2.namedWindow(selectionWindow)
cv2.setMouseCallback(selectionWindow, mouse_callback)

valid, frame = capture.read()
warp = cv2.warpPerspective(frame, perspective_correction_matrix, warpSize)
warp_hsv = cv2.cvtColor(warp,cv2.COLOR_BGR2HSV)
cv2.imshow(selectionWindow,warp)
while 1:
    key = cv2.waitKey(1) & 0xFF
    if updated:
        break

cv2.destroyAllWindows()
for i in range (1,5):
    cv2.waitKey(1)



MARKER_COLOR_MIN = np.array([MARKER_COLOR[0]-10, MARKER_COLOR[1]/2, MARKER_COLOR[2]/2],np.uint8)
MARKER_COLOR_MAX = np.array([MARKER_COLOR[0]+10, 200, 255],np.uint8)

dataLog = {}
dataLog['videoTimestamp'] = []
dataLog['pos'] = []

#cv2.namedWindow("thresh")

i = 0
while True:
    valid, frame = capture.read()
    if not valid:
        break
    
    elapsedTime = capture.get(cv2.cv.CV_CAP_PROP_POS_MSEC)/1000.
    
    warp = cv2.warpPerspective(frame, perspective_correction_matrix, warpSize)
    warp_hsv = cv2.cvtColor(warp,cv2.COLOR_BGR2HSV)
    
    thresh = cv2.inRange(warp_hsv,MARKER_COLOR_MIN,MARKER_COLOR_MAX)
    
    #cv2.imshow("thresh",thresh)
    #cv2.waitKey(1)
    
    contours,hierarchy = cv2.findContours(thresh,cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)

    # find contour with maximum area and store it as best_cnt
    max_area = 0
    best_cnt = []
    for cnt in contours:
        area = cv2.contourArea(cnt)
    
        if area > max_area:
            max_area = area
            best_cnt = cnt
    if max_area > 0:
        best_cnt = np.mean(best_cnt,axis=0)
        while len(best_cnt.shape) > 1:
            best_cnt = best_cnt[0]
        best_cnt[1] *= -1
        dataLog['videoTimestamp'].append(elapsedTime)
        dataLog['pos'].append(list(best_cnt*cm_per_pixel))
    
    if (i % 30) == 0:
        print("Processed: "+str(elapsedTime)+" sec")
        #if elapsedTime > 30:
        #    break
    i += 1

#print(dataLog)
dataLog['videoTimestamp'] = np.array(dataLog['videoTimestamp'])
dataLog['pos'] = np.array(dataLog['pos'])
saveToFile(dataLog,"",data_file_name+"_posLog.p")


