from enum import Enum


class conio:
    def __init__(self):
        pass

    def gotoRowCol(self, row, col):
        print(("\033[" + str(row) + ";" + str(col) + "H"), end="")

    def fgColor(self, color):
        return self.getColorSequence(color, Foreground)

    def bgColor(self, color):
        return self.getColorSequence(color, Background)

    def setTextStyle(self, txtStyle):
        print(("\033[" + str(txtStyle) + "m"), end="")

    def resetAll(self):
        print("\033[0m", end="")

    def clrscr(self):
        print(("\033[2J" + "\033[H"), end="")

    def getColorSequence(self, color, fgOrBg):
        BGOFFSET = 10
        offset = 0
        tmp = False
        if fgOrBg == Background:
            offset += BGOFFSET

        for c in Color:
            if c.value == color:
                tmp = True
                break

        if tmp:
            print(("\033[" + str(color + offset) + "m"), end="")
        else:
            return "conio: invalid color: " + str(color)


Foreground = 1
Background = 2


class Color(Enum):
    BLACK = 30
    RED = 31
    GREEN = 32
    YELLOW = 33
    BLUE = 34
    MAGENTA = 35
    CYAN = 36
    LIGHT_GRAY = 37
    RESET = 39
    GRAY = 90
    LIGHT_RED = 91
    LIGHT_GREEN = 92
    LIGHT_YELLOW = 93
    LIGHT_BLUE = 94
    LIGHT_MAGENTA = 95
    LIGHT_CYAN = 96
    WHITE = 97


class TextStyle(Enum):
    BOLD = 1
    FAINT = 2
    ITALIC = 3
    UNDERLINE = 4
    SLOW_BLINK = 5
    RAPID_BLINK = 6
    NEGATIVE_IMAGE = 7
    CONCEAL = 8
    DOUBLE_UNDERLINE = 9
    NORMAL_INTENSITY = 10
    REVEAL = 11


def main():
    con = conio()
    con.clrscr()
    con.gotoRowCol(20, 20)
    con.setTextStyle(TextStyle.UNDERLINE.value)
    con.fgColor(Color.BLACK.value)
    con.bgColor(Color.GREEN.value)
    print("Hi")
    con.resetAll()


main()
