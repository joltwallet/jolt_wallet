These files offer a unified hashing API for Jolt applications to leverage 
built-in hashing functions with access to just 4 functions.

# Adding a new hash function

1. Make files `algorithms/<my_hash_func>.c` and `algorithms/<my_hash_func>.h`.

2. In these file, define the 4 functions:
    a. `jolt_hash_<my_hash_func>` - 
    b. `jolt_hash_<my_hash_func>_init` - 
    c. `jolt_hash_<my_hash_func>_update` - 
    d. `jolt_hash_<my_hash_func>_final` - 

3. Add an entry to the end of enumerated type `jolt_hash_type_t` in `hash.h`.
   Do not put it after `JOLT_HASH_MAX_IDX`.

4. Add your hashing header and add an entry registering your functions to your 
   enumerated value in `hash_registry.c`.

5. Add unit tests to `tests/hash/algorithms/test_<my_hash_func>.c`
