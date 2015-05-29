
import cv2

capture = cv2.VideoCapture("video.mp4")

while True:
    key_pressed = cv2.waitKey(1)    #Escape to exit
    if key_pressed == 27:
        exit()
    if key_pressed == 65363:
        capture.set(cv2.cv.CV_CAP_PROP_POS_MSEC,299.9)
#        for i in xrange(30): # skip frames
#            flag = capture.grab()
#            if flag == False:
#                exit()
    
    flag, frame = capture.read()
    if flag == False:
        exit()
    
    cv2.imshow("Video", frame)
    #cv2.waitKey()
    
    print capture.get(cv2.cv.CV_CAP_PROP_POS_MSEC)

