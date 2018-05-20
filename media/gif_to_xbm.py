import os, shutil
from glob import glob
from subprocess import call
import argparse

import ipdb as pdb

def main():
    parser = argparse.ArgumentParser(description="Convert gif to xbm")
    parser.add_argument('input', type=str)
    parser.add_argument('output', type=str)
    args = parser.parse_args()

    try:
        shutil.rmtree('tmp')
    except:
        pass
    os.makedirs('tmp')
    basename = args.output.split('.')[0]

    output_fn = os.path.join('tmp', args.output)
    call(["convert", "-coalesce", args.input, output_fn])

    fns = glob("tmp/*.xbm")
    numbers = [int(x[len(basename)+5:-4]) for x in fns]
    numbers.sort()
    reg = os.path.join("tmp", basename + "-%d.xbm")

    with open(args.output, 'w') as dst:
        fn = reg % 0
        with open(fn, 'r') as f:
            content = f.read()

        first_newline = content.find("\n")
        width = int(content[content.find("width")+len("width")+1:first_newline])
        height = int(content[content.find("height")+len("height")+1:content.find("\n", first_newline+1)])
        dst.write("#define %s_width %d\n" % (basename, width))
        dst.write("#define %s_height %d\n" % (basename, height))
        dst.write("#define %s_frames %d\n" % (basename, len(numbers)))
        dst.write("static char %s[][] = {\n" % basename)

        for i in numbers:
            fn = reg % i
            with open(fn, 'r') as f:
                content = f.read()
            data = content[content.find('{'):content.find('}')+1]
            dst.write("%s,\n" % data)
    shutil.rmtree('tmp')


if __name__ == "__main__":
    main()
