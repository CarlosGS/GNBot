#!/usr/bin/env python
 
"""pretty.py will color text for Xterm/VT100 type terminals using ANSI Escape Sequences
 
    A library that provides a Python print, stdout, and string wrapper that makes colored terminal
    text easier to use(e.g. without having to mess around with ANSI escape sequences).
 
    Referance Materials:
        Colored text in Python using ANSI Escape Sequences
 
http://nezzen.net/2008/06/23/colored-text-in-python-using-ansi-escape-sequences/
 
    Orginally Created By:
        Copyright (C) 2008 Brian Nez <thedude at bri1 dot com>
 
    Modified By:
        Jeff Irland (jeff.irland@gmail.com) in January 2013
"""
 
# imported modules
import sys
 
# authorship information
__author__ = "Jeff Irland"
__copyright__ = "Copyright 2013"
__credits__ = "Brian Nez"
__license__ = "GNU General Public License"
__version__ = "1.0"
__maintainer__ = "Jeff Irland"
__email__ = "jeff.irland@gmail.com"
__status__ = "Production"
 
# Dictionary of ANSI escape sequences for coloring text
colorCodes = {
    'black':    '0;30',     'bright gray':  '0;37',
    'blue':     '0;34',     'white':        '1;37',
    'green':    '0;32',     'bright blue':  '1;34',
    'cyan':     '0;36',     'bright green': '1;32',
    'red':      '0;31',     'bright cyan':  '1;36',
    'purple':   '0;35',     'bright red':   '1;31',
    'yellow':   '0;33',     'bright purple':'1;35',
    'dark gray':'1;30',     'bright yellow':'1;33',
    'normal':   '0'
}
 
def printc(text, color):
    """Print in color"""
    print "\033["+colorCodes[color]+"m"+text+"\033[0m"
 
def writec(text, color):
    """Write to stdout in color"""
    sys.stdout.write("\033["+colorCodes[color]+"m"+text+"\033[0m")
 
def switchColor(color):
    """Switch terminal color"""
    sys.stdout.write("\033["+colorCodes[color]+"m")
 
def stringc(text, color):
    """Return a string with ANSI escape sequences to color text"""
    return "\033["+colorCodes[color]+"m"+text+"\033[0m"
 
# Simple test routine to validate thing are working correctly
if __name__ == '__main__':
    printc("Welcome to the pretty.py test routine!", 'white')
 
    printc("I will now try to print a line of text in each color using \"writec()\"", 'white')
    for color in colorCodes.keys():
        writec("Hello, world!", color)
        print "\t", color
 
    printc("\n\nI will now try to print a line of text in each color using \"switchColor()\"", 'white')
    for color in colorCodes.keys():
        switchColor(color)
        print 'Hello World #2!'
 
    printc("\n\nI will now try to print a line of text in each color using \"printc()\"", 'white')
    for color in colorCodes.keys():
        printc('Hello World #3!', color)
 
    printc("\n\nI will now try to print a line of text in each color using \"stringc()\"", 'white')
    for color in colorCodes.keys():
        print stringc('Hello World #4!', color)

