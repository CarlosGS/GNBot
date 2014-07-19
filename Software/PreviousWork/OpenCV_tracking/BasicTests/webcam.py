
# http://www.betasix.net/opencv-2-2-python-examples/

# v4l-utils.x86_64 0:1.0.0-1.fc20  v4l-utils-devel-tools.x86_64 0:1.0.0-1.fc20 v4l2ucp.x86_64 0:2.0.1-10.fc20

#                     brightness (int)    : min=0 max=255 step=1 default=128 value=128
#                       contrast (int)    : min=0 max=255 step=1 default=128 value=128
#                     saturation (int)    : min=0 max=255 step=1 default=128 value=128
# white_balance_temperature_auto (bool)   : default=1 value=1
#                           gain (int)    : min=0 max=255 step=1 default=0 value=0
#           power_line_frequency (menu)   : min=0 max=2 default=2 value=2
#      white_balance_temperature (int)    : min=2000 max=6500 step=1 default=4000 value=4100 flags=inactive
#                      sharpness (int)    : min=0 max=255 step=1 default=128 value=128
#         backlight_compensation (int)    : min=0 max=1 step=1 default=0 value=0
#                  exposure_auto (menu)   : min=0 max=3 default=3 value=3
#              exposure_absolute (int)    : min=3 max=2047 step=1 default=250 value=166 flags=inactive
#         exposure_auto_priority (bool)   : default=0 value=1
#                   pan_absolute (int)    : min=-36000 max=36000 step=3600 default=0 value=0
#                  tilt_absolute (int)    : min=-36000 max=36000 step=3600 default=0 value=0
#                 focus_absolute (int)    : min=0 max=250 step=5 default=0 value=0
#                     focus_auto (bool)   : default=1 value=0
#                  zoom_absolute (int)    : min=100 max=500 step=1 default=100 value=100

# v4l2-ctl -l
# v4l2-ctl -c zoom_absolute=500

# v4l2-ctl -c focus_auto=0 & v4l2-ctl -c focus_absolute=0 & v4l2-ctl -c exposure_auto_priority=0 & v4l2-ctl -c exposure_auto=0  & v4l2-ctl -c exposure_absolute=250  & v4l2-ctl -c backlight_compensation=0  & v4l2-ctl -c white_balance_temperature_auto=0  & v4l2-ctl -c white_balance_temperature=4100

# v4l2-ctl -c focus_auto=0 & v4l2-ctl -c focus_absolute=0 & v4l2-ctl -c exposure_auto_priority=1 & v4l2-ctl -c exposure_auto=1 & v4l2-ctl -c backlight_compensation=0  & v4l2-ctl -c white_balance_temperature_auto=1


import cv
import time

cv.NamedWindow("camera", 1)
capture = cv.CreateCameraCapture(0)

#width = 1920 #leave None for auto-detection
#height = 1080 #leave None for auto-detection

width = 1280 #leave None for auto-detection
height = 720 #leave None for auto-detection

if width is None:
    width = int(cv.GetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_WIDTH))
else:
	cv.SetCaptureProperty(capture,cv.CV_CAP_PROP_FRAME_WIDTH,width)    

if height is None:
	height = int(cv.GetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_HEIGHT))
else:
	cv.SetCaptureProperty(capture,cv.CV_CAP_PROP_FRAME_HEIGHT,height) 

result = cv.CreateImage((width,height),cv.IPL_DEPTH_8U,3)

val = 1
while True:
    img = cv.QueryFrame(capture)
    #cv.Smooth(img,result,cv.CV_GAUSSIAN,val,val)
    #cv.Dilate(img,result,None,val) #uncommet to apply affect
    #cv.Erode(img,result,None,val) #uncommet to apply affect
    #cv.Smooth(img,result,cv.CV_GAUSSIAN) #uncommet to apply affect
    result = img
    cv.ShowImage("camera", result)
    k = cv.WaitKey(1)
    #print(k)
    if k == 1048678: # f
        break
    if k == 1048679: # g
        val += 2

