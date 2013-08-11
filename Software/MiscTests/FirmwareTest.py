# Simple bluetooth terminal
# Based on http://stackoverflow.com/questions/15486570/bluetooth-communication-between-arduino-and-pybluez

# Begin modules
import sys
import atexit
import time

import GNBot
# End modules

# Begin configuration.
BT_ADDRESS = "00:11:09:18:02:34"
# End configuration

robot = GNBot.GNBot(BT_ADDRESS)

if robot.connect() == -1:
    print("Connect failed.")
    exit()

# Callback to close the connection when force-quitting
def exitCallback():
    print("Closing connection...")
    robot.send("Init:ByeBye!\n") # This will stop the robot
    robot.disconnect()

atexit.register(exitCallback)

print("Press enter for 0...")
val = sys.stdin.readline()
robot.send("GoToAngle:90\n")

print("Press enter for 0...")
val = sys.stdin.readline()
robot.send("GoToAngle:0\n")

print("Press enter for 1...")
val = sys.stdin.readline()
robot.send("Motor:90,0\n")

print("Press enter for 2...")
val = sys.stdin.readline()
robot.send("Motor:0,0\n")

print("Press enter for 3...")
val = sys.stdin.readline()
robot.send("Motor:0,90\n")

print("Press enter for 4...")
val = sys.stdin.readline()
robot.send("Motor:0,0\n")


count = 0
while 1:
    line = robot.recvLine()
    if line != -1 and len(line) > 0:
        print(line)
    
    if count > 100:
        count = 0
        #robot.send("Buzz:3,300,50\n")
        robot.send("LedRGB:20,0,0\n")
        robot.send("Log:ON\n")
    elif count == 50:
        print("SENDING!");
        robot.send("Log:OFF\n")
        robot.send("LedRGB:0,20,0\n")
        #robot.send("Buzz:3,200,50\n")
    time.sleep(0.05)
    count += 1
    
