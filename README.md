# LDPC-Decoder

This is an implementation of LDPC decoder, which uses Hard (Counting based) and Soft (Probabilistic) Decision decoding to recover noisy message recieved from the transmission channel. The code is optimized to use minimum run time and memory accesses while decoding. The code is almost entirely parameterized, allowing it to produce decoders for different large H matrices by simply adjusting the input size variables. A brief description of the Tanner Graph Implemention is shown below.

![TANNER GRAPH - IMPLEMENTATION](https://github.com/user-attachments/assets/88b8c290-d20d-4a5c-a4e3-6534fca34a4a)
