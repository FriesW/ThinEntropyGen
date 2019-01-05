# Thin Entropy Generator

A collection of software for thin clients, single board computers, virtual machines,
and micro-controllers to augment sources of entropy (randomness).

Repo consists of two primary parts: `Expander` for stretching, `Source` for generating.

Was created to cheaply and quickly generate lots of entropy on a SBC with no entropy source.

## Expander

#### Utility for stretching the length of entropy of a random input

Software depends on `libssl-dev`, and is compiled by `gcc` with the `-lcrypto` flag.

#### Example
```
apt install libssl-dev
cd [repo]/Expander
gcc main.c -lcrypto -o expander
head -c 64 /dev/urandom | ./expander | hexdump | less
```

#### Operation
Reads in 64 byte blocks on STDIN,
outputs on STDOUT in 1,048,576 byte blocks of "stretched" data. Uses SHA512.

Stretching is done via a hash tree. Each node is 512 bytes big.
In the hash tree, every node's
first 256 bits is hashed to create a left child node, and the
last 256 bits is hashed to create a right child node.
The tree is built from the root node outward until the tree
has sufficient depth (15, but configurable).
All final leafs (nodes without children) are concatenated together and
used as the output.

The hashing algorithm is SHA512, which used in this way should be
cryptographically secure. However, don't trust it.


## Source

#### Generating entropy on a cheap microcontroller

Random data is base64 encoded and broken into standard lines/blocks.

Currently two MCU implementations: Teensy LC and Arduino Nano/Uno/328p.

All compiling and flashing done via Arduino and Teensy software.
Shouldn't require external libraries.

Teensy LC uses its continuous ADC mode and averages around 12,500 bytes/s.
Arduino boards are very slow.

#### Example

Seed OS entropy source.
```
minicom -D /dev/ttyACM[] > /dev/random
```

Note: this will not update `entropy_count` and unlock `/dev/random`. To do so
you'll need something like `rng-tools` and root access.

For more info, see:
* https://security.stackexchange.com/a/69433
* https://cryptotronix.com/2014/08/28/tpm-rng/

<br>

---

<br>

## Combined Example

Fill a hard drive with randomness and print completion time.
Useful for erasing drives or preparing a new drive for an encrypted partition.
```
minicom -D /dev/ttyACM[] | base64 --decode | ./expander | \
dd of=/dev/sd[] bs=4M status=progress conv=fdatasync \
; date
```
The combination of a Teensy LC `Source` and the `Expander` will
produce around 195 Mbytes/sec.



## TODO
Test `Source`, `Source -> Expander`, and `/dev/random -> Expander` with `dieharder`.
