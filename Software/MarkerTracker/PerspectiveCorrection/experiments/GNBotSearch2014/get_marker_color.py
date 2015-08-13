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

