#!/usr/bin/python

import roslib; roslib.load_manifest('panasonic_blc111')
import rospy
from panasonic_blc111.msg import *
import urllib2
import traceback,sys

cam_ip = '0.0.0.0'
cam_user = 'aslteam'
cam_pwd = '123456'

# Create an OpenerDirector with support for Basic HTTP Authentication...
class MyPwdManager(urllib2.HTTPPasswordMgrWithDefaultRealm):
    def find_user_password(self,realm,uri):
        return (cam_user,cam_pwd)


auth_handler = urllib2.HTTPBasicAuthHandler(MyPwdManager())
opener = urllib2.build_opener(auth_handler)
# ...and install it globally so it can be used with urlopen.
urllib2.install_opener(opener)

def ptcmdCallback(cmd):
    commands={ PTCmd.Home: 'HomePosition', PTCmd.PanLeft: 'PanLeft', \
            PTCmd.PanRight: 'PanRight', PTCmd.TiltUp: 'TiltUp', \
            PTCmd.TiltDown: 'TiltDown', PTCmd.PanScan: 'PanScan', \
            PTCmd.TiltScan: 'TiltScan'}
    url = 'http://'+cam_ip+'/nphControlCamera?Direction='+commands[cmd.command];
    try:
        print "Sending message to url: " + url
        f = urllib2.urlopen(url)
        dump = f.read()
    except:
        traceback.print_exc(file=sys.stderr)


if __name__ == '__main__':
    home = PTCmd(command=PTCmd.Home)
    rospy.init_node('blc111_command')
    cam_ip = rospy.get_param("~cam_ip","0.0.0.0")
    listener = rospy.Subscriber("~command", PTCmd, ptcmdCallback)

    ptcmdCallback(home)
    print "Subscribed to command topic. Ready to go"
    rospy.spin()


