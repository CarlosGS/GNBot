
#import cv
import cv
import cv2
import numpy as np
from pprint import pprint

cap = cv2.VideoCapture(0)


width = 1920 #leave None for auto-detection
height = 1080 #leave None for auto-detection
width = 1280 #leave None for auto-detection
height = 720 #leave None for auto-detection

cap.set(cv.CV_CAP_PROP_FRAME_WIDTH,width)
cap.set(cv.CV_CAP_PROP_FRAME_HEIGHT,height)

cap.set(cv.CV_CAP_PROP_FPS,30)

points = []
M = None
p = None
maxWidth = 800
maxHeight = 800
margin = 100;
def my_mouse_callback(event,x,y,flags,param):
    global points, M, p, maxWidth, maxHeight
    #if event == cv.CV_EVENT_LBUTTONDBLCLK:
    if event == cv.CV_EVENT_LBUTTONUP:
        if len(points) < 4:
            points.append([x,y])
        if len(points) == 4:
            src = np.array(points,np.float32)
            #dst = np.array([[0,0],[maxWidth,0],[maxWidth,maxHeight],[0,maxHeight]],np.float32)
            dst = np.array([[margin,margin],[maxWidth-margin,margin],[maxWidth-margin,maxHeight-margin],[margin,maxHeight-margin]],np.float32)
            M = cv2.getPerspectiveTransform(src,dst)
            p = [(x,y) for [x,y] in points]
            points = []

#cv.NamedWindow("pick")
cv2.namedWindow("pick")
cv2.setMouseCallback("pick",my_mouse_callback)
cv2.namedWindow("result", cv2.CV_WINDOW_AUTOSIZE)
#while(1):
#    _,frame = cap.read()
#    cv2.imshow("pick",frame)
#    cv2.waitKey(1)
#    if M != None:
#        break

#p = [(x,y) for [x,y] in points]
#cv2.line(frame, p[0], p[1], (255,0,0)) # (b,g,r)
#cv2.line(frame, p[1], p[2], (255,0,0))
#cv2.line(frame, p[2], p[3], (255,0,0))
#cv2.line(frame, p[3], p[0], (255,0,0))
#cv2.imshow("pick",frame)
#cv.DestroyWindow("pick")


#http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
#http://stackoverflow.com/questions/9808601/is-getperspectivetransform-broken-in-opencv-python2-wrapper
#src = np.array([[50,50],[450,450],[70,420],[420,70]],np.float32)
#dst = np.array([[0,0],[299,299],[0,299],[299,0]],np.float32)

#maxWidth = 600
#maxHeight = 300

#sourcePoints = np.array(points,np.float32)
#print(sourcePoints)

#dst = np.array([[0,0],[maxWidth,0],[maxWidth,maxHeight],[0,maxHeight]],np.float32)
#print(dst)

#M = cv2.getPerspectiveTransform(sourcePoints,dst)
#pprint(M)


#cv.NamedWindow("result")
while(1):
    _,frame = cap.read()
    if M != None:
        warp = cv2.warpPerspective(frame, M, (maxWidth, maxHeight))
        cv2.imshow("result",warp)
        cv2.line(frame, p[0], p[1], (255,0,0)) # (b,g,r)
        cv2.line(frame, p[1], p[2], (255,0,0))
        cv2.line(frame, p[2], p[3], (255,0,0))
        cv2.line(frame, p[3], p[0], (255,0,0))
    cv2.imshow("pick",frame)
    cv2.waitKey(1)


# Clean up everything before leaving
cv2.destroyAllWindows()
cap.release()

exit()


posx=0
posy=0
lastx=0
lasty=0
global h,s,v,i,im,evente
h,s,v,i,r,g,b,j,evente=0,0,0,0,0,0,0,0,0

#    Mouse callback function    (from earlier mouse_callback.py with little modification)
def my_mouse_callback(event,x,y,flags,param):
    global evente,h,s,v,i,r,g,b,j
    evente=event
    if event==cv.CV_EVENT_LBUTTONDBLCLK:        # Here event is left mouse button double-clicked
        hsv=cv.CreateImage(cv.GetSize(frame),8,3)
        cv.CvtColor(frame,hsv,cv.CV_BGR2HSV)
        (h,s,v,i)=cv.Get2D(hsv,y,x)
        (r,g,b,j)=cv.Get2D(frame,y,x)
        print "x,y =",x,y
        print "hsv= ",cv.Get2D(hsv,y,x)        # Gives you HSV at clicked point
        print "im= ",cv.Get2D(frame,y,x)     # Gives you RGB at clicked point

#    Thresholding function    (from earlier mouse_callback.py)    
def getthresholdedimg(im):
    '''This function take RGB image.Then convert it into HSV for easy colour detection and threshold it with the given part as white and all other regions as black.Then return that image'''
    imghsv=cv.CreateImage(cv.GetSize(im),8,3)
    cv.CvtColor(im,imghsv,cv.CV_BGR2HSV)
    imgthreshold=cv.CreateImage(cv.GetSize(im),8,1)
    cv.InRangeS(imghsv,cv.Scalar(h-10,10,10),cv.Scalar(h+10,255,255),imgthreshold)
    return imgthreshold
    
def getpositions(im):
    ''' this function returns leftmost,rightmost,topmost and bottommost values of the white blob in the thresholded image'''
    leftmost=0
    rightmost=0
    topmost=0
    bottommost=0
    temp=0
    for i in range(im.width):
        col=cv.GetCol(im,i)
        if cv.Sum(col)[0]!=0.0:
            rightmost=i
            if temp==0:
                leftmost=i
                temp=1        
    for i in range(im.height):
        row=cv.GetRow(im,i)
        if cv.Sum(row)[0]!=0.0:
            bottommost=i
            if temp==1:
                topmost=i
                temp=2    
    return (leftmost,rightmost,topmost,bottommost)
    
capture=cv.CaptureFromCAM(1)


width = 1920 #leave None for auto-detection
height = 1080 #leave None for auto-detection
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



frame=cv.QueryFrame(capture)
test=cv.CreateImage(cv.GetSize(frame),8,3)

#     Now the selection of the desired color from video.( new)
cv.NamedWindow("pick")
cv.SetMouseCallback("pick",my_mouse_callback)
while(1):
    frame=cv.QueryFrame(capture)
    cv.ShowImage("pick",frame)
    cv.WaitKey(33)
    if evente==7:                    # When double-clicked(i.e. event=7), this window closes and opens next window
        break
cv.DestroyWindow("pick")

#    Drawing Part (from earlier program)
cv.NamedWindow("threshold")    
cv.NamedWindow("output")
while(1):
    frame=cv.QueryFrame(capture)
    cv.Flip(frame,frame,1)                # Horizontal flipping for synchronization, comment it to see difference.
    imdraw=cv.CreateImage(cv.GetSize(frame),8,3)    # We make all drawings on imdraw.
    thresh_img=getthresholdedimg(frame)        # We get coordinates from thresh_img
    cv.Erode(thresh_img,thresh_img,None,1)        # Eroding removes small noises
    
# find contours in the threshold image
    contours,hierarchy = cv2.findContours(numpy.asarray(thresh_img[:,:]),cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)

    # finding contour with maximum area and store it as best_cnt
    max_area = 0
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > max_area:
            max_area = area
            best_cnt = cnt

    # finding centroids of best_cnt and draw a circle there
    M = cv2.moments(best_cnt)
    cx,cy = int(M['m10']/M['m00']), int(M['m01']/M['m00'])

    if lastx!=0 and lasty!=0:
        posx = cx
        posy = cy
        cv.Line(imdraw,(posx,posy),(lastx,lasty),(b,g,r))
        cv.Circle(imdraw,(posx,posy),5,(b,g,r),-1)
    lastx=posx
    lasty=posy

    cv.Add(test,imdraw,test)            # Adding imdraw on test keeps all lines there on the test frame. If not, we don't get full drawing, instead we get only that fraction of line at the moment.
    cv.ShowImage("threshold",thresh_img)
    cv.ShowImage("output",imdraw)
    if cv.WaitKey(33)==1048603:            # Exit if Esc key is pressed
        break
cv.DestroyWindow("output")                # Releasing window
cv.DestroyWindow("threshold")

######################################################################################################

# Here selection is not real time. If you want to change the color while tracking, you have re-run the program.
# So for a real time tracking, just copy line 70 and paste it as line 82. Then completely comment out line 68-74.Copy line 74 and # paste it at very end of the program. Now run!!!! You select any color whenever you like.!!!!!!!!

