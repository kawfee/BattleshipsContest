#!/usr/bin/env python3
# Problem I encountered make sure you have LF end of line sequence for the file otherwise you may encounter an issue.
# Also doesn't need to be a .py file but doing that for github file tpying.

# Grabbing some constants similar to defines.h
# Shot related
SHOT = '@'
MISS = 'O'
DUPLICATE_SHOT  = '!'
HIT  = 'X'
KILL = 'K'
OPPONENT_SHOT = '*'
SHIP = 'S'
WATER = '~'
INVALID_SHOT = '\0'
# Meta information -- win/lose/quit
WIN  = 'W'
LOSE = 'L'
TIE  = 'T'
QUIT = 'Q'
PEEK = 'P'
PLACE_SHIP = 'Z'
# enum Direction
NONE=0
HORIZONTAL=1
VERTICAL=2


import socket
import json


def wipeBoards(shipBoard, shotBoard, boardSize):
    array = [WATER, WATER, WATER, WATER, WATER, WATER, WATER, WATER, WATER, WATER]
    shipBoard = []
    shotBoard = []
    for _ in range(boardSize):
        shipBoard.append(array)
        shotBoard.append(array)
    return shipBoard, shotBoard


def sendGameVars(msg):
    msg["str"] = "Python AI"


def placeShip():
    pass


def messageHandler(msg):
    if(msg["messageType"] == "setupGame"):
        sendGameVars(msg)
    elif(msg["messageType"] == "matchOver"):
        pass
    elif(msg["messageType"] == "placeShip"):
        pass



def main():
    # Initialize socket connection
    sock = socket.socket()
    host = socket.gethostname()
    port = 54321
    sock.connect((host, port))
    
    # Setup Game Vars
    round = 0
    boardSize = 10
    shipBoard = []
    shotBoard = []
    # Populating boards with blank data
    shipBoard, shotBoard = wipeBoards(shipBoard, shotBoard, boardSize)

    while True:
        round += 1

        msg = json.loads(sock.recv(1499))
        messageHandler(msg)

        #print("Received:", msg["messageType"])

        toSend = (json.dumps(msg)).encode()
        sock.send(toSend)


        sock.close()


if __name__ == "__main__":
    main()
