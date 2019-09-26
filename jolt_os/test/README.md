# Jolt Unit and Integration Tets

Tests can be ran via `make clean-jolt && make tests` in the root project directory.

# RNG

The following instructions were tested on Ubuntu 18.04.

## Dieharder Installation

Dieharder is a random number test suite.

```
sudo apt install dieharder
```

Check and compare your installed version to the [version on the official website](https://webhome.phy.duke.edu/~rgb/General/dieharder.php).

```
dieharder -h
```

## RNG File Generation
In the project root directory, flash the consumer firmware
```
# TODO chance to consumer make command
make clean-jolt && make flash -j15
```

Then, run the script to generate 20 million bytes of RNG data to file `rng.raw`:
```
python3 pyutils/rng_dump.py --port $YOUR_JOLT_PORT
```

## Run Benchmark
TODO COMPLETE
```
# -a Runs all tests with standardf parameters and options
# -o Output filename
# -g Generator Test ID. 201 is for "file input raw"
dieharder -a -o dieharder_results.txt -g 201 
```
