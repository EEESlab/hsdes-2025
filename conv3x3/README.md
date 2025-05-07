# A 2D convolution filter
The example is a very basic 2D convolution filter run over a 64x64 image.
Variants of this kernel are a very common basic block of traditional image processing and compression algorithms -- but also of state-of-the-art Artificial Intelligence, in particular deep neural networks.
The code is organized in four files: 
- `convolution.c` includes the `main` function of the tests and various checks to ensure it works correctly
- `conv_kernel.c` is the actual convolution kernel;
- `conv_kernel.h` is the header of `conv_kernel.c`, used to include its function.
- `data.h` includes the input data and filter.
In this Google Colab notebook you can find a full description of the operation that is performed by the filter: https://colab.research.google.com/drive/1eebiD-10vLbwi1qPG0DpL9KnzY_GStPb

## Running the example
```
cd conv_naive
make clean all run
```
The expected output is
```
errors=0
cycles=269468
```
