#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np


data = loadFromFile("./","perspective_correction_data.p")
perspective_correction_matrix = data['perspective_correction_matrix']
warpSize = data['warpSize']
cm_per_pixel = data['cm_per_pixel']
dst_points = [(int(x),int(y)) for (x,y) in data['dst_points']]



data_file_name = "2014-09-25-193626"

capture = cv2.VideoCapture(data_file_name+".webm")
capture.set(cv2.cv.CV_CAP_PROP_FPS,60)
print("Reading file at FPS: "+str(capture.get(cv2.cv.CV_CAP_PROP_FPS)))



N_MARKERS_TO_TRACK = 4

MARKER_COLOR = [41,79,250]

MARKER_COLOR_MIN = np.array([MARKER_COLOR[0]-15, MARKER_COLOR[1]-15, MARKER_COLOR[2]/2],np.uint8)
MARKER_COLOR_MAX = np.array([MARKER_COLOR[0]+15, 200, 255],np.uint8)

dataLog = {}
dataLog['videoTimestamp'] = []
dataLog['posX'] = []
dataLog['posY'] = []



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
    
    
    # Find N largest blobs
    contours,hierarchy = cv2.findContours(thresh,cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
    areas = [cv2.contourArea(cnt) for cnt in contours]
    indices = np.argsort(areas)
    if len(indices) >= N_MARKERS_TO_TRACK:
        largest_contours = [contours[idx] for idx in indices[-N_MARKERS_TO_TRACK:]]
        centroids_X = []
        centroids_Y = []
        for contour in largest_contours:
            centroid = np.mean(contour,axis=0)
            while len(centroid.shape) > 1:
                centroid = centroid[0]
            centroid[1] *= -1
            centroid = list(centroid*cm_per_pixel)
            centroids_X.append(centroid[0])
            centroids_Y.append(centroid[1])
        centroids_X = np.array(centroids_X)
        centroids_Y = np.array(centroids_Y)
        dataLog['videoTimestamp'].append(elapsedTime)
        dataLog['posX'].append(centroids_X)
        dataLog['posY'].append(centroids_Y)
    
    if (i % 25) == 0:
        print("Processed: "+str(elapsedTime)+" sec")
#        if elapsedTime > 40:
#            break
    i += 1

#print(dataLog)
dataLog['videoTimestamp'] = np.array(dataLog['videoTimestamp'])
dataLog['posX'] = np.array(dataLog['posX'])
dataLog['posY'] = np.array(dataLog['posY'])
saveToFile(dataLog,"",data_file_name+"_posLog_raw.p")


