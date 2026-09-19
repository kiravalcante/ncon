# ncon

CLI utility to convert and print decimal numbers to octal, hexadecimal, and binary format.

## Installation

Clone the repository, then compile the `main.c` file using `cc main.c -O2 -o ncon`.

Optionally, move the resulting `ncon` program to a directory in your `PATH` variable,
so you can use it from anywhere.

## Usage

```
ncon <NUMBER_1> <NUMBER_2> (...)
```

### Example

Input:

```
ncon 69 420 67
```

Output:

```
+-----------+-----------+-----------+--------------------+
| DEC       | OCT       | HEX       | BIN                |
+-----------+-----------+-----------+--------------------+
|        69 |       105 |        45 |            1000101 |
+-----------+-----------+-----------+--------------------+
|       420 |       644 |       1a4 |          110100100 |
+-----------+-----------+-----------+--------------------+
|        67 |       103 |        43 |            1000011 |
+-----------+-----------+-----------+--------------------+
```