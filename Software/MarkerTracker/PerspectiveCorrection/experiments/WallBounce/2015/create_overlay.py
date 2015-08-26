#!/usr/bin/python
# encoding: utf-8

from helper import *
import cv2
import numpy as np



data_file_name = "2015-08-07-183814"
paths = loadFromFile("",data_file_name+"_posLog.p")


ts = paths['timestamp']
N_markers = len(paths.keys())-1
N_points = len(paths[0][:,0])

# Low pass filtering:
for r in xrange(N_markers):
    first = True
    for i in xrange(N_points):
        if not first:
            paths[r][i,:] = paths[r][i-1,:]*0.9 + paths[r][i,:]*0.1
        else:
            first = False




data = loadFromFile("./","perspective_correction_data.p")
perspective_correction_matrix = data['perspective_correction_matrix']
warpSize = data['warpSize']
cm_per_pixel = data['cm_per_pixel']

src_points = np.array(data['src_points'],np.float32)
dst_points = np.array(data['dst_points'],np.float32)

inv_perspective_correction_matrix = cv2.getPerspectiveTransform(dst_points,src_points)

print inv_perspective_correction_matrix

# Apply inverse transform to every point
for r in xrange(N_markers):
    for i in xrange(N_points):
        src = np.array([[paths[r][i,:]]],np.float32)
        src[0][0][1] *= -1
        src /= cm_per_pixel
        dst = cv2.perspectiveTransform(src,inv_perspective_correction_matrix)
        while len(dst.shape) > 1:
            dst = dst[0]
        paths[r][i,:] = dst


img = cv2.imread("Screenshot from 2015-08-07-183814.webm.png")

#colors = [(255,0,0), (0,255,0), (0,0,255), (255,255,255),(0,0,0)]
colors = [(0,0,255), (0,255,0), (255,255,0), (0,0,0)]

def tuple_lerp(start,end,l):
    start = np.array(start)
    end = np.array(end)
    res = start*(1-l) + end*l
    return tuple(res)


first = True
for i in xrange(N_points):
    if not first:
        for r in xrange(N_markers):
            progress = min(5*float(i)/float(N_points),1)
            prev = paths[r][i-1,:]
            prev = (int(prev[0]),int(prev[1]))
            new = paths[r][i,:]
            new = (int(new[0]),int(new[1]))
            cv2.line(img, prev, new, tuple_lerp((128,128,128),colors[r],progress), thickness=2, lineType=cv2.cv.CV_AA)
            #col = (colors[r][0],colors[r][1],colors[r][2],10)
            #cv2.line(img, prev, new, col, thickness=2, lineType=cv2.cv.CV_AA)
    else:
        first = False


cv2.imwrite("overlay.png",img)


