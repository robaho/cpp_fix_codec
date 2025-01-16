## Summary

This is a C++ implementation of a FIX protocol codec.

It uses [cpp_fixed](https://github.com/robaho/cpp_fixed) to perform fixed decimal point integer math.

## Building

Remove `fixed.h` , and run `make all` to obtain the latest version of the fixed point decimal library.

The project builds by default using `make` and CLang. There is a `Makefile.gcc` for building using GCC.

## Design

The library is extremely low-level, designed for performance first. There is no automated FIX metadata. The
consumer is responsible for defining the group layout of the messages it uses.

A custom arena based memory allocator is used, and any dynamic memory is reused when parsing the next message.

Nested groups are fully supported.

## Performance

Using a 4 GHz Quad-Core Intel Core i7:

The library can parse nearly 2.2M messages a second from a single istream. An istream is almost always required
because the FIX connection is over TCP and the messages are not delineated.

Parsing messages with groups slows the parsing by 10%.

After parsing, the accessing of fields is performed at more than 55M a second.

See the [benchmarks](https://github.com/robaho/cpp_fix_codec/blob/main/benchmark_parse_test.cpp)

## ToDo

Fix message creation is in the works.
