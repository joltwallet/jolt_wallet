import os, shutil
from glob import glob
from subprocess import call
import argparse

import ipdb as pdb

def main():
    parser = argparse.ArgumentParser(description="Convert gif to xbm")
    parser.add_argument('input', type=str)
    parser.add_argument('output', type=str)
    parser.add_argument('--frameskip', type=int, default = 1)
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
    #numbers = [int(x[len(basename)+5:-4]) for x in fns]
    numbers = list(range(0, len(fns), args.frameskip))
    reg = os.path.join("tmp", basename + "-%d.xbm")

    with open(args.output, 'w') as dst:
        fn = reg % 0
        with open(fn, 'r') as f:
            content = f.read()

        first_newline = content.find("\n")
        width = int(content[content.find("width")+len("width")+1:first_newline])
        height = int(content[content.find("height")+len("height")+1:content.find("\n", first_newline+1)])
        data = content[content.find('{'):content.find('}')+1]
        n_char = data.count('x')

        dst.write("#define GRAPHIC_%s_H %d\n" % (basename.upper(), height))
        dst.write("#define GRAPHIC_%s_W %d\n" % (basename.upper(), width))
        dst.write("#define GRAPHIC_%s_F %d\n" % (basename.upper(), len(numbers)))
        dst.write("const unsigned char graphic_%s[%d][%d] = {\n" %
                (basename, len(numbers), n_char))

        for i in numbers:
            fn = reg % i
            with open(fn, 'r') as f:
                content = f.read()
            data = content[content.find('{'):content.find('}')+1]
            dst.write("%s,\n" % data)
    shutil.rmtree('tmp')


if __name__ == "__main__":
    main()
