#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np
import time

data_file_name = "2014-09-25-191650"

capture = cv2.VideoCapture(data_file_name+".webm")


data = loadFromFile("./","perspective_correction_data.p")
perspective_correction_matrix = data['perspective_correction_matrix']
warpSize = data['warpSize']
cm_per_pixel = data['cm_per_pixel']
dst_points = [(int(x),int(y)) for (x,y) in data['dst_points']]

map_points = []
frame = []

def mouse_callback(event,x,y,flags,param):
    global map_points, frame
    if event == cv2.cv.CV_EVENT_LBUTTONUP:
        src = np.array([[[x,y]]],np.float32)
        dst = cv2.perspectiveTransform(src,perspective_correction_matrix)
        while len(dst.shape) > 1:
            dst = dst[0]
        dst[1] *= -1
        dst *= cm_per_pixel
        map_points.append(dst.tolist())
        cv2.circle(frame, (x,y), 3, (0,255,0), -1)
        cv2.imshow(selectionWindow,frame)

selectionWindow = "Raw footage. Click to select map points. Finish with 'q'"
cv2.namedWindow(selectionWindow, cv2.CV_WINDOW_AUTOSIZE)
cv2.setMouseCallback(selectionWindow, mouse_callback)

_,frame = capture.read()
cv2.imshow(selectionWindow,frame)

while 1:
    key = cv2.waitKey(100) & 0xFF
    if key == 27 or key == ord('q'):
        break

map_points = np.array(map_points)

saveToFile(map_points,"./",data_file_name+"map_data.p")

