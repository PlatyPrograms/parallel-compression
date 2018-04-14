# Parallelized Compression/Decompression Algorithms

---

A parallelized RLE compression/decompression algorithm using MPI.

## Implementation Details

The compression program generates two files: a `data` file and a `meta` file.

### The `data` file

Contains a 6-bit pane size tag, followed by binary data. The data is organized into 'panes,' or consistent length chunks of data. The pane size is chosen from 1 to 64 bits based on what pane length most efficiently compresses the file. The pane size tag expresses the number of bits per pane. Each pane represents the key sequence of a run; that is, the sequence which is repeated *n* number of times to form the run.

### The `meta` file

Contains a 6-bit frame size tag, followed by binary data. The data is organized into 'frames' the same way the `data` file is organizerd into 'panes.' Each frame contains the length of the corresponding key sequence in the `data` file. An escape code, 0, followed by a number *n* is used to denote a run of unique key sequences; this indicates to the decompression program that it should read the next *n* key sequences as runs of length 1.