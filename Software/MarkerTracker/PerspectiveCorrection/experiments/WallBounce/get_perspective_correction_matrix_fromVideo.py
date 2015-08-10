#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np
import time

data_file_name = "2015-08-07-183814"
scale_for_selection = 4

if data_file_name == None:
    capture = cv2.VideoCapture(0)
    
    width = 1920 #leave None for auto-detection
    height = 1080 #leave None for auto-detection
    
    capture.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH,width)
    capture.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT,height)
    
    fps_rate = 10
    capture.set(cv2.cv.CV_CAP_PROP_FPS,fps_rate)
    
else:
    capture = cv2.VideoCapture(data_file_name+".webm")
    width = int(capture.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH))
    height = int(capture.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT))


scale = 1
maxWidth = 1080*scale
maxHeight = 1080*scale
verticalOffset = maxHeight/9.
margin = scale*70./2.;
cm_per_pixel = 25./(2.*float(margin)) # 25cm square
warpSize = (maxWidth, maxHeight)

src_points = []
dst_points = [(maxWidth/2-margin,maxHeight/2+verticalOffset-margin),(maxWidth/2+margin,maxHeight/2+verticalOffset-margin),(maxWidth/2+margin,maxHeight/2+verticalOffset+margin),(maxWidth/2-margin,maxHeight/2+verticalOffset+margin)]


perspective_correction_matrix = []
updated = False

def mouse_callback(event,x,y,flags,param):
    global src_points, dst_points, perspective_correction_matrix, updated
    x = float(x)/float(scale_for_selection)
    y = float(y)/float(scale_for_selection)
    if event == cv2.cv.CV_EVENT_LBUTTONUP:
        if len(src_points) < 4:
            print("Picked X: " + str(x) + " Y: " + str(y))
            src_points.append( (x,y) )
        if len(src_points) == 4:
            src = np.array(src_points,np.float32)
            dst = np.array(dst_points,np.float32)
            perspective_correction_matrix = cv2.getPerspectiveTransform(src,dst)
            print("perspective_correction_matrix = ")
            print(perspective_correction_matrix)
            updated = True

selectionWindow = "Raw footage. Click to select 4 reference points"
cv2.namedWindow(selectionWindow, cv2.CV_WINDOW_AUTOSIZE)
cv2.setMouseCallback(selectionWindow, mouse_callback)

_,frame = capture.read()

frame_large = cv2.resize(frame, (width*scale_for_selection,height*scale_for_selection))
cv2.imshow(selectionWindow,frame_large)

while 1:
    key = cv2.waitKey(1) & 0xFF
    if updated:
        break

data = {}
data['perspective_correction_matrix'] = perspective_correction_matrix
data['src_points'] = src_points
data['dst_points'] = dst_points
data['captureSize'] = (width,height)
data['warpSize'] = warpSize
data['cm_per_pixel'] = cm_per_pixel
saveToFile(data,"./","perspective_correction_data.p")

src_points = [(int(x),int(y)) for (x,y) in src_points]
cv2.line(frame, src_points[0], src_points[1], (255,0,0)) # (b,g,r)
cv2.line(frame, src_points[1], src_points[2], (255,0,0))
cv2.line(frame, src_points[2], src_points[3], (255,0,0))
cv2.line(frame, src_points[3], src_points[0], (255,0,0))

cv2.imwrite("./bak/"+getDate()+"frame.png", frame)

#cv2.putText(frame, "Press 'o' to confirm", (width/2,height/2), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,255))
cv2.imshow(selectionWindow,frame)

cv2.waitKey(1)


resultWindow = "Corrected perspective"
cv2.namedWindow(resultWindow, cv2.CV_WINDOW_AUTOSIZE)

warp = cv2.warpPerspective(frame, perspective_correction_matrix, warpSize)
dst_points = [(int(x),int(y)) for (x,y) in dst_points]
cv2.line(warp, dst_points[0], dst_points[1], (0,255,0)) # (b,g,r)
cv2.line(warp, dst_points[1], dst_points[2], (0,255,0))
cv2.line(warp, dst_points[2], dst_points[3], (0,255,0))
cv2.line(warp, dst_points[3], dst_points[0], (0,255,0))

cv2.imwrite("./bak/"+getDate()+"warp.png", warp)

cv2.imshow(resultWindow,warp)


while 1:
    key = cv2.waitKey(1) & 0xFF
    if key == ord('q') or key == 27:
        break

