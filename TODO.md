# The TODO list for mboxgrep

## Behavior

- [x] use cryptographic hashes for detecting duplicate messages
- [x] add checking for conflicting command-line options
- [ ] support for deletion of messages after being matched and displayed
- [x] ignore .overview when grepping Gnus folders
- [x] inverted matching
- [x] recursive search through directories
- [x] writing selected messages to a new folder
- [x] deleting selected messages
- [ ] basic time and date matching
- [ ] more advanced time and date matching, with strings such as "yesterday"
- [x] reading messages from standard input
- [x] run-time selection of file locking method
- [x] add a debug function

## File formats, encodings and standards

- [x] migrate to pcre2, as pcre is obsolete
- [ ] use a more modern hash function than MD5
- [ ] MIME support
- [ ] support for GnuPG
- [x] support for compressed mbox folders
- [x] support for bzip2 compression
- [ ] support for XZ-format compression
- [ ] support for mail folder conversion
- [ ] improve error detection when a directory is not a Maildir or MH folder
- [ ] document criteria for folder format detection

## Miscellaneous

- [x] write Texinfo documentation
- [ ] configuration files
- [ ] make use of lockfile library
- [ ] make use of Solaris' maillock library
- [x] provide possibility to use flock() instead of fcntl()
- [ ] provide national language support with gettext()
