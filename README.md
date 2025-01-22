## Summary

This is a C++ implementation of a FIX protocol codec.

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

The project builds by default using `make` and CLang. There is a `Makefile.gcc` for building using GCC.make

## Design

The library is extremely low-level, designed for performance first. There is no automated FIX metadata. The
consumer is responsible for defining the group layout of the messages it uses.

A custom arena based memory allocator is used, and any dynamic memory is reused when parsing the next message.

Nested groups are fully supported.

## Performance

Using a 4 GHz Quad-Core Intel Core i7:

The library can parse more than **2.7M messages a second** from a single istream. An istream is almost always required
because the FIX connection is over TCP and the messages are not delineated.

Parsing messages with groups slows the parsing by 10%.

After parsing, the accessing of fields is performed at more than 55M a second.

The library can encode more than 4.5M typical "new order single" messages per second. With cached time values, this
is **more than 7.5M messages per second**. Time conversion is typically slow so caching below the required accuracy is essential.

See the [benchmark parsing](https://github.com/robaho/cpp_fix_codec/blob/main/benchmark_parse_test.cpp)<br>
See the [benchmark building](https://github.com/robaho/cpp_fix_codec/blob/main/benchmark_build_test.cpp)

<details>
    <summary>performance details</summary>

<pre>
New Order Single: parsed 1000000 messages, usec per order 0.368897, orders per sec 2710783
New Order Single w/Groups: parsed 1000000 messages, usec per order 0.496595, orders per sec 2013713
built 10000000 messages, usec per msg 0.223333, msgs per sec 4477614
with cached time, built 10000000 messages, usec per msg 0.133145, msgs per sec 7510597
</pre>

</details>

