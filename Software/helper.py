#!/usr/bin/python

# This file is part of GNBot (https://github.com/carlosgs/GNBot)
# by Carlosgs (http://carlosgs.es)
# CC-BY-SA license (http://creativecommons.org/licenses/by-sa/3.0/)

import pickle # For file saving
import datetime
import os
import sys

import numpy as np

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


def mapVals(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def mySaveFig(plt,path,file):
    makeDirs(path)
    plt.savefig(path+file)


def discretizePolar(angle, intensity, Nsamples, radians = False):
    # Note: assumes angle = np.mod(angle, 2*np.pi);
    if not radians:
        angleInc = 360./float(Nsamples)
    else:
        angleInc = (2.*np.pi)/float(Nsamples)
    result = np.zeros(Nsamples)
    for i in range(Nsamples):
        angleIni = i*angleInc - angleInc/2.
        angleEnd = angleIni + angleInc
        (indices,) = np.where((angle >= angleIni) & (angle <= angleEnd))
        if len(indices) > 0:
            result[i] = np.mean(intensity[indices])
        else:
            result[i] = -1
    # Interpolate empty samples
    (emptySamples,) = np.where(np.array(result) == -1)
    for i in emptySamples:
        prev_i = i
        while result[prev_i] == -1:
            prev_i -= 1
        next_i = i
        while result[next_i % Nsamples] == -1:
            next_i += 1
        dist = next_i - prev_i
        t = float(i - prev_i)/float(dist)
        result[i] = result[prev_i]*t + result[next_i % Nsamples]*(1.-t)
    return result

def plotClosedLine(ax, angs, vals, lab=None):
    angs = np.append(angs,angs[0])
    vals = np.append(vals,vals[0])
    ax.plot(angs, vals, label=lab)

# Linear interpolation of intermediate values
def eval_intensity(vals, angle):
    angle = np.mod(angle, 360.);
    Nsamples = vals.size
    inc = 360./float(Nsamples)
    prev = np.floor(angle/inc)
    next = np.ceil(angle/inc)
    dist = next-prev
    t = (angle - float(prev)*inc)/float(dist)
    return vals[prev]*t + vals[next % Nsamples]*(1.-t)

def lightModelCurve(Nsamples, angle=0, distance=1, intensity=1, ambient=0):
    vals = np.zeros(Nsamples)
    angs = np.arange(0., 360., 360./float(Nsamples))
    angs -= angle
    angs = np.mod(angs, 360.);
    
    ambient = float(ambient)
    
    for i in range(Nsamples):
        perceivedIntensity = float(intensity)/(float(distance)**2.)
        if angs[i] >= 0. and angs[i] < 90.:
            t = np.cos(np.radians(angs[i]))
            vals[i] = perceivedIntensity*t + ambient
        elif angs[i] > 270. and angs[i] < 360.:
            t = np.cos(np.radians(angs[i]))
            vals[i] = perceivedIntensity*t + ambient
        else:
            vals[i] = ambient
    angs += angle
    angs = np.mod(angs, 360.);
    return (angs,vals)

# Be careful with this function
def isSameAngle(ang1, ang2, margin=10.):
    return ang1+margin > ang2 and ang1-margin < ang2

