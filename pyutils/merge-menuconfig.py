"""
Merges 2 sdkconfig files, where the first sdkconfig file gets definition priortiy.

Comments are ignored.
"""

import argparse

def parse_args():
    """ Parse CLI arguments into an object and a dictionary """
    parser = argparse.ArgumentParser()
    parser.add_argument('new_sdkconfigdefault', type=str,
            help='Path to the new sdkconfig default file')
    parser.add_argument('jolt_sdkconfigdefault', type=str,
            help='Path to the new sdkconfig default file')
    parser.add_argument('--output', type=str, default='sdkconfig.defaults',
            help='Path to the new sdkconfig default file')
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
