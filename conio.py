from enum import Enum
#IMPORT AREA HERE

class conio:
    def __init__(self):
        pass

    def gotoRowCol(self, row, col):
        return CSI+str(row)+";"+str(col)+"H"

    def fgColor(self, color): 
        return getColorSequence(color, Foreground)

    def bgColor(self, color):
        return getColorSequence(color, Background)

    def setTextStyle(self, txtStyle):
        return CSI+txtStyle+"m"

    def resetAll(self):
        return CSI+"0m"

    def clrscr(self):
        return CSI+"H"+CSI+"2J" 

    def getColorSequence(self, color, fgOrBg):
        BGOFFSET = 10
        offset = 0
        tmp = False
        if ( fgOrBg == Background ):
            offset += BGOFFSET
        
        for c in Color:
            if c == color:
                tmp = True
                break

        if tmp:
            return CSI+Color(color+offset)+"m"
        else:
            return "conio: invalid color: "+int(color)

CSI="\x00[\x00"

Foreground=1
Background=2

class Color(Enum):
    BLACK           =30
    RED             =31
    GREEN           =32
    YELLOW          =33
    BLUE            =34
    MAGENTA         =35
    CYAN            =36
    LIGHT_GRAY      =37
    RESET           =39
    GRAY            =90
    LIGHT_RED       =91
    LIGHT_GREEN     =92
    LIGHT_YELLOW    =93
    LIGHT_BLUE      =94
    LIGHT_MAGENTA   =95
    LIGHT_CYAN      =96
    WHITE           =97

class TextStyle(Enum):
    BOLD                =1
    FAINT               =2
    ITALIC              =3
    UNDERLINE           =4
    SLOW_BLINK          =5
    RAPID_BLINK         =6
    NEGATIVE_IMAGE      =7
    CONCEAL             =8
    DOUBLE_UNDERLINE    =9
    NORMAL_INTENSITY    =10
    REVEAL              =11


def main():
    con = conio()
    con.clrscr()
    #con.gotoRowCol(20, 20)
    print('Hi')


main()






