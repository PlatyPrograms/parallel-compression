# Parallelized Compression/Decompression Algorithms

---

A parallelized RLE compression/decompression algorithm using MPI.

## Implementation Details

The compression program generates two files: a `data` file and a `meta` file.

### The `data` file

Contains a 6-bit key size tag, followed by binary data. The data is organized into 'keys,' or consistent length chunks of data. The key size is chosen from 1 to 64 bits based on what key length most effectively compresses the file. The key size tag expresses the number of bits per key. Each key represents the key sequence of a run; that is, the sequence which is repeated *n* number of times to form the run.

`| key size (6b) |         data         |`

### The `meta` file

Contains a 6-bit run size tag, followed by binary data, terminated by a double escape. The data is organized into 'runs' the same way the `data` file is organizerd into 'keys.' Each run contains the length of the corresponding key sequence in the `data` file. An escape code, 0, followed by a number *n* is used to denote a run of unique keys; this indicates to the decompression program that it should read the next *n* key sequences as runs of length 1. When the decompressor reaches \0 (literally 00, or \\), it halts decompression.

`| expected num procs (1b)  | run size (6b) | bits per run (1b) |         data         | \0 |`



# Example runs

## Serial Code

### Serial Compression

Compress the nameofFile.txt file using a key size of 23. Note that the name of the given file should have an extension denoted by a '.'

' ./compress nameOfFile.txt 23 '

    
### Serial Decompression

Decompress the nameOfFile.data and nameOfFile.meta files. Note that the 'nameOfFile' should be without any extension.

' ./decompress nameOfFile nameOfOutputFile.txt'

## Parallel Code

### Parallel Compression

Compress the nameOfFile.txt file using a key size of 23 and '4' processes.

'mpirun -n 4 ./compress nameOfFile.txt 23 '


### Parallel Decompression

Decompress the nameOfFile#.data and nameOfFile#.meta files. Note that the 'nameOfFile' should be without any extension. Also note that the number of processes used for decompression must match the number of processes used for compresssion.

' mpirun -n 4 ./decompress nameOfFile nameOfOutputFile.txt '





