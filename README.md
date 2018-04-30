# Parallelized Compression/Decompression Algorithms

---

An parallelized RLE compression/decompression algorithm using MPI.

## Implementation Details

The compression program generates two files: a `data` file and a `meta` file.

### The `data` file

Contains a 6-bit key size tag, followed by binary data. The data is organized into 'keys,' or consistent length chunks of data. The key size is chosen from 1 to 64 bits based on what key length most effectively compresses the file. The key size tag expresses the number of bits per key. Each key represents the key sequence of a run; that is, the sequence which is repeated *n* number of times to form the run.

`| key size (6b) |         data         |`

### The `meta` file

Contains a 6-bit run size tag, followed by binary data, terminated by a double escape. The data is organized into 'runs' the same way the `data` file is organizerd into 'keys.' Each run contains the length of the corresponding key sequence in the `data` file. An escape code, 0, followed by a number *n* is used to denote a run of unique keys; this indicates to the decompression program that it should read the next *n* key sequences as runs of length 1. When the decompressor reaches \0 (literally 00, or \\), it halts decompression.

`| expected num procs (1b)  | run size (6b) | bits per run (1b) |         data         | \0 |`



# Compiling and Running

## Compiling

Invoking `make` will make both serial and parallel compression and decompression in the root directory.

Invoking `make clean` removes all .meta, .data, .o, and executable files.

To generate nicely compressable files, invoke `python genText.py [numBytes] [filename]`. This will generate a file name `filename` with the specified number of bytes.

## Running

To compress a file in serial, invoke

`./serialcompress [fileName] [outputFileName]`.

To decompress a file in serial, invoke

`./serialdecompress [fileName] [outputFileName]`

To compress a file in parallel, invoke

`mpirun -n [threads] ./compress [fileName] [sliceLen]`

To decompress a file in parallel, invoke

`mpirun -n [threads] ./decompress [fileName] [outputFileName]`

# Division of Labor

Gregory Bolet wrote the compression program, while Eric Andrews wrote decompression. The writeup and makefile were joint efforts.
