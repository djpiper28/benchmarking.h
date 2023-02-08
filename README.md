# bencharmking.h - A Benchmarking Framework

## Design
A C benchmarking library that aims to allow for CPU time, Memory usage and, output values
to be benchmarked over input vectors.

This project should produce JSON output and, be easy to use.

## Compiling
### Requirements
GCC, Jansson, Cmake, CTest.

```sh
cmake .. && cmake --build . -j # Compiles the library on all cores
ctest -V -j # Executes all of the unit tests
```

### Using Within CMake as a Dependancy
```cmake
add_submodule(02-benchmarking-framework) # Change to be the correct directory

# ...
target_link_libraries(my-target benchmarking_h)
```

## Usage
### Ranges
Ranges are iterables from start to, end (inclusively) increasing by step each time. 
Ranges follow this rule:

`for (double start = 0; islesser(end); start += step);`

> To declare a range you should set the start and, end
```c
        range_t range;
        range.start = 0;
        range.end = 10;
        range.step = 1;
```

> Then you must start the range (this will reset the internal state so that it can be iterated on)
```c
        range_start(&range);
```

> You can now iterate over the range using `range_next`
```c
        double output;
        for (int j = 0; j <= range.end; j++) {
            // The range will return RANGE_GENERATING if it created a value or, RANGE_ERROR on failure
            ASSERT(range_next(&range, &output) == RANGE_GENERATING); 

            // The current value of the range is stored in range_t->current
            lprintf(LOG_INFO, "The range's current value is %lf!\n", range.current);
        }

        // At the end of the range the generator will report RANGE_STOPPED
        ASSERT(range_next(&range, &output) == RANGE_STOPPED);
```

### Multidimensional Ranges
These are ranges that allow you to iterate over vectors. They are defined as a list of ranges, each
representing a range for a dimension in a range. 

A mdrange can iterate over arbitary ranges i.e:
```py
[0-> 10; 1, 1-> 7; 1, 2-> 3; 1]
```

> To declare a range you should first create some ranges
```c
    // Init ranges
    range_t *ranges = malloc(sizeof(*ranges) * DIMENSIONS);
    ASSERT(ranges != NULL);
    for (size_t d = 0; d < DIMENSIONS; d++) {
        ranges[d].start = 0;
        ranges[d].end = 10;
        ranges[d].step = 1;
    }
```

> These allocated ranges can then be used in initialisation
```c
    // test init and, next
    multi_dimensional_range_t range;

    // Init the multi_dimensional_range_t using the allocated ranges from earlier
    ASSERT(init_multi_dimensional_range_arr(&range, d, ranges));
```

> To iterate over the range use the `multi_dimensional_range_next` 
```c
    // Iterating over the multi_dimensional_range_t will produce a vector at each iteration step
    vector_t out;
    for (size_t i = 0; i < ITEMS_PER_DIMENSION(d); i++) {
        // Returns RANGE_ERROR on output malloc errors
        ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_GENERATING);
        // Free the vector as it has heap allocated values.
        free_vector(&out);
    }

    // The range has ended here
    ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_STOPPED);

    // Free the array, it uses a copied array of ranges so we free ranges as well (it can be done after init tbh)
    free_multi_dimensional_range(&range);
    
    // Remember we allocated these earlier
    free(ranges);
```

> Alternatively you can use the `init_multi_dimensional_range_arr` function with some hard coded values
```c
    multi_dimensional_range_t range;
    memset(&range, 0, sizeof(range));

    range_t range_1, range_2, range_3;
    range_1.start = range_2.start = range_3.start = 1;
    range_1.end = range_2.end = range_3.end = 3;
    range_1.step = range_2.step = range_3.step = 1;

    init_multi_dimensional_range(&range, range_1, range_2, range_3);
    ASSERT(range.dimensions == 3);
    ASSERT(range.ranges != NULL);
    ASSERT(memcmp(&range.ranges[0], &range_1, sizeof(range_1)) == 0);
    ASSERT(memcmp(&range.ranges[1], &range_2, sizeof(range_2)) == 0);
    ASSERT(memcmp(&range.ranges[2], &range_3, sizeof(range_3)) == 0);

    multi_dimensional_range_start(&range);

    vector_t out;
    for (size_t i = 0; i < 3 * 3 * 3; i++) {
        ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_GENERATING);
        ASSERT(out.dimensions== range.dimensions);
        ASSERT(out.values != NULL);
        free_vector(&out);
    }

    ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_STOPPED);

    free_multi_dimensional_range(&range);
```

### Benchmark Configuration
Lorem ipsum dolor sit amet, qui minim labore adipisicing minim sint cillum sint consectetur cupidatat.

### Benchmark Output Configuration
Lorem ipsum dolor sit amet, qui minim labore adipisicing minim sint cillum sint consectetur cupidatat.

### Benchmark Running
Lorem ipsum dolor sit amet, qui minim labore adipisicing minim sint cillum sint consectetur cupidatat.

