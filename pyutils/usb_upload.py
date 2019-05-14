#!/usr/bin/python3

'''
Uploads files/firmware to Jolt over YMODEM

if the file has name "jolt_os.bin" or "jolt_os.bin.gz", it will start an OS update.
'''

import argparse
import os, sys
import serial
import time
import logging
from xmodem import YMODEM

# Configure logging
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
log = logging.getLogger('elf2jelf')

def parse_args():
    this_path = os.path.abspath(__file__)
    default_elf_fn = os.path.join(this_path, '..', 'build', 'jolt_os.bin')

    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=str,
            help='File to send over.')
    parser.add_argument('--baudrate', type=int, default=230400,
            help="Baudrate")
    parser.add_argument('--port', type=str, default='/dev/ttyUSB0',
            help="Serial Port")
    parser.add_argument('--verbose', '-v', type=str, default='INFO',
            help='''
            Valid options:
            SILENT
            INFO
            DEBUG
            ''')
    args = parser.parse_args()
    dargs = vars(args)

    global log
    logging_level = args.verbose.upper()
    if logging_level == 'INFO':
        log.setLevel(logging.INFO)
    elif logging_level == 'DEBUG':
        log.setLevel(logging.DEBUG)
    else:
        raise("Invalid Logging Verbosity")

    return (args, dargs)

def consume(ser):
    ser.read(size=100000, )

def main(args):
    args, dargs = parse_args()

    # need to clear DTR, then clear RTS to properly reset device
    ser = serial.Serial(dsrdtr=True)
    ser.baudrate = args.baudrate
    ser.port     = args.port
    ser.dtr = 1
    ser.rts = 1
    ser.timeout = 3

    ser.open()

    ser.dtr = 0
    ser.rts = 0

    log.info("Waiting for device to boot")
    time.sleep(5) # Wait for device to boot

    def getc(size, timeout=5):
        return ser.read(size=size)

    def putc(data, timeout=5):
        return ser.write(data)

    ymodem = YMODEM(getc, putc)


    basename = os.path.basename(args.input)
    log.info("Initiating Upload")
    if basename in ['jolt_os.bin', 'jolt_os.bin.gz']:
        log.debug('Sending "upload_firmware" command')
        ser.write(b"upload_firmware\n")
    else:
        log.debug('Sending "upload" command')
        ser.write(b"upload\n")
    time.sleep(1)
    consume(ser)

    log.info("Sending File")
    ymodem.send([args.input,])

    time.sleep(2)
    log.info("Upload Complete. If you got a 'send error: expected ACK; got b\'\\x15\'' the transfer was probably fine.")

if __name__ == '__main__':
    main(sys.argv)

