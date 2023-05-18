# Compilation and installation of mboxgrep

`mboxgrep` should compile on a modern Unix-like operating system, such as GNU/Linux or FreeBSD.

Autoconf and Automake are used, and the most basic compilation procedure consists of:

```
autoreconf --install
./configure
make
make install # root rights probably needed here, prefix with sudo in such case
```

(Invocation of `autoreconf` is only required if the source tree has been cloned from the
git repository.)

To see the list of flags accepted by the configure script, run:

```
./configure --help
```

Optionally, `mboxgrep` can be linked with the following libraries:

- PCRE2, to enable support for regular expressions compatible with Perl 5;
- zlib and bzlib, to enable support for compressed mbox folders.
