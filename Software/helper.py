#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlosgs (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

import pickle # For file saving
import datetime
import os
import sys

# Misc functions:

# From http://stackoverflow.com/questions/3207219/how-to-list-all-files-of-a-directory-in-python
def getFilesInDir(path):
    onlyfiles = [ f for f in os.listdir(path) if os.path.isfile(os.path.join(path,f)) ]
    return onlyfiles

def makeDirs(path):
    try:
        os.makedirs(path)
    except:
        return -1
    return 0

def saveToFile_noBak(data,path,filename):
    with open(path+filename, 'wb') as path_file:
        ret = pickle.dump(data, path_file, protocol=2)
        path_file.close()
        return ret
    raise Exception("Could not save " + path + filename)

def getDate():
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S");

def saveToFile(data,path,filename):
    backupFilePath = path+"bak/"
    makeDirs(backupFilePath)
    saveToFile_noBak(data,backupFilePath,getDate()+" "+filename)
    saveToFile_noBak(data,path,filename)


def loadFromFile(path,filename):
    with open(path+filename, 'rb') as path_file:
        ret = pickle.load(path_file)
        path_file.close()
        return ret
    raise Exception("Could not load " + path + filename)

def strTypeMatches(string,typeReq):
    try:
        val = typeReq(string)
    except:
        return False
    return True

def getValue(description,default):
    validType = type(default)
    print("\n"+description + " ([Enter] for default: " + str(default) + "):")
    string = sys.stdin.readline()
    if len(string) == 1: # Pressed enter
        print("Using the default.\n")
        return default
    while not strTypeMatches(string, validType):
        print("Incorrect data type (Asked:" + str(validType) + "), try again:")
        string = sys.stdin.readline()
    print("Saved!\n")
    return validType(string)



def pltShowNonBlocking():
	#plt.ion() # Enable real-time plotting to avoid blocking behaviour for plt.show()
	plt.draw()
	#plt.ioff() # Disable real-time plotting

def pltNewFig():
	fig = plt.figure()
	#plt.draw()
	return fig

def pltSetFig(fig):
	plt.figure(fig.number)

def pltRefresh(fig):
	fig.canvas.draw()

def pltShow():
	#plt.ion() # IMPORTANT: Enable real-time plotting
	plt.draw()
	#plt.ioff()

def mapVals(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def mySaveFig(plt,path,file):
    makeDirs(path)
    plt.savefig(path+file)

