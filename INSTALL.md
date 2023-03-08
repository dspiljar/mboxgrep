# How to compile and install mboxgrep

mboxgrep should compile on a modern Unix-like operating system, such as GNU/Linux or FreeBSD.

It uses autoconf, so the most basic compilation procedure consists of:

```
./configure
make
make install # root rights probably needed here, prefix with sudo in such case
```

To see the list of flags accepted by the configure script, run:

```
./configure --help
```

Optionally, `mboxgrep` can be linked with the following libraries:

- PCRE, to enable support for regular expressions compatible with Perl 5;
- zlib and bzlib, to enable support for compressed mbox folders.
