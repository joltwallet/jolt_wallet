#!/usr/bin/python3
import os
import sys
import serial
import time
from xmodem import YMODEM

import ipdb as pdb


def consume(ser):
    ser.read(size=100000, )

def main(args):
    # need to clear DTR, then clear RTS
    port = '/dev/ttyUSB0'
    baudrate = 230400
    filename = args[1]

    ser = serial.Serial(dsrdtr=True)
    ser.baudrate = baudrate
    ser.port     = port
    ser.dtr = 1
    ser.rts = 1
    ser.timeout = 1

    ser.open()

    ser.dtr = 0
    ser.rts = 0

    print("Waiting for device to boot")
    time.sleep(4) # Wait for device to boot

    def getc(size, timeout=5):
        return ser.read(size=size)

    def putc(data, timeout=5):
        return ser.write(data)

    ymodem = YMODEM(getc, putc)

    print("Initiating Upload")
    ser.write(b"upload\n")
    consume(ser)

    print("Sending File")
    ymodem.send([filename,])

    print("Upload Complete. If you got a 'send error: expected ACK; got b\'\\x15\'' the transfer was probably fine.")

if __name__ == '__main__':
    main(sys.argv)

