#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np
import time

data_file_name = "2014-09-25-183439"

capture = cv2.VideoCapture(data_file_name+".webm")


data = loadFromFile("./","perspective_correction_data.p")
perspective_correction_matrix = data['perspective_correction_matrix']
warpSize = data['warpSize']
cm_per_pixel = data['cm_per_pixel']
dst_points = [(int(x),int(y)) for (x,y) in data['dst_points']]

map_points = []

def mouse_callback(event,x,y,flags,param):
    global map_points
    if event == cv2.cv.CV_EVENT_LBUTTONUP:
        src = np.array([[[x,y]]],np.float32)
        dst = cv2.perspectiveTransform(src,perspective_correction_matrix)
        while len(dst.shape) > 1:
            dst = dst[0]
        map_points.append(dst.tolist())

selectionWindow = "Raw footage. Click to select map points. Finish with 'q'"
cv2.namedWindow(selectionWindow, cv2.CV_WINDOW_AUTOSIZE)
cv2.setMouseCallback(selectionWindow, mouse_callback)

_,frame = capture.read()
cv2.imshow(selectionWindow,frame)

while 1:
    key = cv2.waitKey(100) & 0xFF
    if key == 27 or key == ord('q'):
        break

print(map_points)

data = {}
data['map_points'] = map_points
saveToFile(data,"./","map_data.p")

