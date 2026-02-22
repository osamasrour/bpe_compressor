# Implementation of Byte-Pair Encoding in pure C
___

bpe_compressor is a simple compressor for .txt files using [BPE](https://en.wikipedia.org/wiki/Byte-pair_encoding) algorithm.


## Warning
It's not meant to be used in production.
___

## Build
```console
.\build.bat
```

##  Quick Start
```console
$ .\main.exe zip input.txt output.bpe
```
or
```console
$ .\main.exe unzip input.bpe output.txt
```

## Format of BPE container

```console
_______________________________________
|BPE(version)(LE|BE)                   |
|                                      |
|(compressed data length as uint32_t)  |
|____________________________          |
|                            |         |
|                            |         |
|     (compressed data       |         |
|      as linked list)       |         |
|                            |         |
|____________________________|         |
|   (highest element value in the      |
|      compressed data as uint32_t)    |
|                                      |
|   (pairs array length as uint32_t)   |
|                                      |
|____________________________          |
|                            |         |
|                            |         |
|     (pairs array           |         |
|      as uint32_t[])        |         |
|                            |         |
|____________________________|         |
|                                      |
|                EOF                   |
|--------------------------------------|
```

## References
[Byte-Pair Encoding (Medium)](https://medium.com/data-science/byte-pair-encoding-subword-based-tokenization-algorithm-77828a70bee0)

[Byte-pair encoding (Wikipedia)](https://en.wikipedia.org/wiki/Byte-pair_encoding)

[Serialize and Deserialize Binary in C](https://ssojet.com/serialize-and-deserialize/serialize-and-deserialize-binary-in-c#using-existing-libraries-for-serialization)

[Implementing Serialization and Deserialization in C](https://projectai.in/projects/ba998ed6-a7f0-4f16-b7b7-27958a398cbc/tasks/3b82744c-2875-47fe-9fef-1ecaf274c033?tab=task)