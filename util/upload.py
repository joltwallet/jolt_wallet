import serial
import zlib
import binascii
import logging
import os
import sys
import argparse
import time
import threading

try:
    if "serialization" in serial.__doc__ and "deserialization" in serial.__doc__:
        raise ImportError("""
esptool.py depends on pyserial, but there is a conflict with a currently installed package named 'serial'.

You may be able to work around this by 'pip uninstall serial; pip install pyserial' \
but this may break other installed Python software that depends on 'serial'.

There is no good fix for this right now, apart from configuring virtualenvs. \
See https://github.com/espressif/esptool/issues/269#issuecomment-385298196 for discussion of the underlying issue(s).""")
except TypeError:
    pass  # __doc__ returns None for pyserial

logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
log = logging.getLogger('jolt_upload')

import ipdb as pdb

def parse_args():
    ''' Parse CLI arguments into an object and a dictionary '''
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', type=str,
            help='Jolt Serial Port')
    parser.add_argument('--file', '-f', type=str,
            help ='File to upload to Jolt.')
    parser.add_argument('--compress', '-c', action='store_true',
            help = 'Compress File with Zlib prior to uploading.')
    parser.add_argument('--level', '-l', type=int, default=zlib.Z_BEST_COMPRESSION,
            help='Compression Level to Apply (0-10)')
    parser.add_argument('--baud', '-b', type=int, default=1500000,
            help='Baudrate to communicate with Jolt at.')
    parser.add_argument('--wbits', '-w', type=int, default=12,
            help='log2 of Window (dictionary) size.')
    args = parser.parse_args()
    dargs = vars(args)
    return (args, dargs)

def read_from_port(ser):
    while True:
        try:
            reading = ser.readline().decode()
            print(reading)
        except:
            return

def main():
    args, dargs = parse_args()

    with open(args.file, 'rb') as f:
        orig_data = f.read()
    log.info("Read in %s. Size: %d" % (args.file, len(orig_data)))
    data = orig_data

    if args.compress:
        w_bits = 12
        log.info("Compressing at level %d with window (dict) size %d", args.level, 2**args.wbits)
        compressor = zlib.compressobj(level=args.level, method=zlib.DEFLATED,
                wbits=args.wbits, memLevel=zlib.DEF_MEM_LEVEL, strategy=zlib.Z_DEFAULT_STRATEGY)
        compressed_data = compressor.compress(orig_data)
        compressed_data += compressor.flush()
        compress_percentage = 100*(1-(len(compressed_data)/len(orig_data)))
        log.info("Compressed data to %d bytes (%.2f%% smaller)" % \
                (len(compressed_data), compress_percentage) )
        data = compressed_data


    c_fn = args.file + '.gz'
    with open(c_fn, 'wb') as f:
        f.write(data)
    quit()

    ser = serial.Serial(baudrate=args.baud)

    ser.port = args.port
    ser.dtr = False
    ser.rts = False

    try:
        ser.open()
    except serial.serialutil.SerialException:
        print("Serial Port Timeout")
        pdb.set_trace()
        quit()

    time.sleep(1)

    thread = threading.Thread(target=read_from_port, args=(ser,))
    thread.start()

    time.sleep(1)

    #ser.reset_input_buffer()
    #ser.reset_output_buffer()

    '''
    ser.write(b'free\n')
    time.sleep(1)
    #ser.readline() # spews the same command back
    #print(ser.readline()) # Response
    ser.write(b'free\n')
    time.sleep(1)
    #ser.readline() # spews the same command back
    #print(ser.readline()) # Response
    '''

    ser.write(b'upload_firmware %d %d\n' % (len(orig_data), len(data)))
    #ser.readline() # spews the same command back
    time.sleep(10)

    step = 512
    for i in range(0, len(data), step):
        print( " Uploading %d (%.2f%%)" % (i, 100*i/len(data)), end="\n" )
        ser.write(data[i:i+step])
        time.sleep(0.01)
        #ser.readline() # wait for ack
    print("\n")
    time.sleep(5)

    ser.close()


if __name__=='__main__':
    main()
