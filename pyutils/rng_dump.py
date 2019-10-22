#!/usr/bin/python3

"""
Saves a bunch of RNG values.
"""

import argparse
import os, sys
import serial
from time import sleep, time
import logging

# Configure logging
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
log = logging.getLogger('elf2jelf')

def parse_args():
    this_path = os.path.dirname(__file__)
    default_output_fn = os.path.join(this_path, '..', 'rng.raw')

    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=str, default=default_output_fn,
            help='File to write raw values to.')
    parser.add_argument('-n', type=int, default=5000000,
            help="Number of 32-bit ints to generate. Defaults to 5 million.")
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

    log.info("Waiting for device to boot/initialize")
    sleep(5) # Wait for device to boot
    consume(ser)

    cmd = b"rng %d\n" % (args.n * 4)
    log.debug('Sending "%s" command' % cmd.decode('utf-8'))
    ser.write( cmd )

    t_start = time()
    with open(args.output, 'wb') as f:
        echo = ser.read(size=len(cmd)+1)
        assert( echo[:-2] == cmd[:-1] )
        assert( echo[-2] == 13 )  # carriage return
        assert( echo[-1] == 10 )  # newline
        for i in range(args.n):
            data = ser.read(size=4)
            f.write(data)
    t_end = time()
    print("Dumped %d 32-bit RNG values in %.2f seconds" % (args.n, t_end - t_start))

if __name__ == '__main__':
    main(sys.argv)

