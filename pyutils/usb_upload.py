#!/usr/bin/python3

"""
Uploads files/firmware to Jolt over YMODEM

If the file has name "jolt_os.bin" or "jolt_os.bin.gz", it will start an OS update.

If the file ends in ".patch" it will be applied as a patch (triggering an OS update).
"""

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
    parser.add_argument('--monitor', '-m', action='store_true',
            help="Monitor esp32 after transfer.")
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
    ser.timeout = 0.5

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

    firmware_names = ['jolt_os.bin', 'jolt_os.bin.gz', 'JoltOS.bin', 'JoltOS.bin.gz']

    basename = os.path.basename(args.input)
    log.info("Initiating Upload")
    cmd = []
    if basename in firmware_names:
        cmd.append("upload_firmware")
    elif basename.endswith(".patch"):
        cmd.append("upload_firmware")
        cmd.append("--patch")
    else:
        cmd.append("upload")
        cmd.append(basename)

    cmd = ' '.join(cmd) + '\n'
    log.debug('Sending "%s" command' % cmd.strip())
    ser.write(cmd.encode('ascii'));
    consume(ser)

    log.info("Sending File")
    ymodem.send([args.input,], retry=60)

    if args.monitor:
        while(True):
            try:
                n = ser.inWaiting()
                if n:
                    line = ser.read(n)
                    print(line.strip().decode("utf-8", errors='replace') )
            except IOError:
                raise IOError()
            except:
                pass

if __name__ == '__main__':
    main(sys.argv)

