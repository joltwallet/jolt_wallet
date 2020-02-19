"""
Merges 2 sdkconfig files, where the first sdkconfig file gets definition priortiy.

Comments are ignored.
"""

import argparse
from hashlib import sha256
from binascii import hexlify


DUMMY_DIGEST = '61c29711f796f7ac0b15b3e978c636da7680e106dbf64495c51494bbcf0332a7'


def parse_args():
    """ Parse CLI arguments into an object and a dictionary """
    parser = argparse.ArgumentParser()
    parser.add_argument('new_sdkconfigdefault', type=str,
            help='Path to the new sdkconfig default file')
    parser.add_argument('jolt_sdkconfigdefault', type=str,
            help='Path to the new sdkconfig default file')
    parser.add_argument('--output', type=str, default='sdkconfig.defaults',
            help='Path to the new sdkconfig default file')
    parser.add_argument('--force', '-f', action='store_true',
            help='Generate even if output file exists and isnt default')

    args = parser.parse_args()
    dargs = vars(args)
    return (args, dargs)

def append_or_replace(new_entry, config):
    var, val = new_entry.split('=')
    var += '='

    for i, line in enumerate(config):
        if line.startswith(var):
            config[i] = new_entry
            break
    else:
        config.append(new_entry)
    return config


def main():
    args, dargs = parse_args()

    # See if output file already exists
    if not args.force:
        try:
            with open(args.output, 'r') as f:
                data = f.read()
            digest = hexlify(sha256(data.encode('utf-8')).digest()).decode('utf-8')
            if digest != DUMMY_DIGEST: 
                # Default dummy sdkconfig.default
                return

        except FileNotFoundError:
            # Output file doesn't exist, continue
            pass

    with open(args.new_sdkconfigdefault, 'r') as f:
        new_data = f.readlines()
        new_data = [x.strip() for x in new_data]
    with open(args.jolt_sdkconfigdefault, 'r') as f:
        jolt_data = f.readlines()
        jolt_data = [x.strip() for x in jolt_data]

    for line in new_data:
        if line[0] == '#':
            # skip comments
            continue

        jolt_data = append_or_replace(line, jolt_data)

    #################
    # Special Rules #
    #################

    # Override partition table
    partition_file = "jolt_wallet/partitions.csv" 
    jolt_data = append_or_replace('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="%s"' % partition_file, jolt_data)
    jolt_data = append_or_replace('CONFIG_PARTITION_TABLE_FILENAME="%s"' % partition_file, jolt_data)

    ########################
    # Write Output To Disk #
    ########################
    jolt_data = [x+"\n" for x in jolt_data]
    with open(args.output, 'w') as f:
        f.writelines(jolt_data)

if __name__=='__main__':
    main()
