# Jolt Unit and Integration Tets

Tests can be ran via `make clean-jolt && make tests` in the root project directory.

# RNG

The following instructions were tested on Ubuntu 18.04.

## Dieharder

Dieharder is a random number test suite.

### Dieharder Installation


```
sudo apt install dieharder
```

Check and compare your installed version to the [version on the official website](https://webhome.phy.duke.edu/~rgb/General/dieharder.php).

```
dieharder -h
```

### RNG File Generation
In the project root directory, flash the consumer firmware

```
# TODO chance to consumer make command
make clean-jolt && make flash -j15
```

In order to run all the tests, Dieharder needs about *4GB* of random data to run 
all the tests.

Then, run the script to generate 20 million bytes of RNG data to file `rng.raw`:

```
python3 pyutils/rng_dump.py -n 4294967295 --port $YOUR_JOLT_PORT
```

It will take about *9 days* to generate 20MB of data from the internal RNG 
source.

### Run Benchmark

TODO COMPLETE

```
# -a Runs all tests with standardf parameters and options
# -g Generator Test ID. 201 is for "file input raw"
# -f input file
dieharder -a -g 201 -f rng.raw
```

### Remarks
The ESP32's internal RNG generator.

The ESP32's internal RNG is XOR'd with the ATAES132a's secure RNG source. However,
prior to locking the ATAES132a, it only outputs `0xA5` as a debugging measure.
