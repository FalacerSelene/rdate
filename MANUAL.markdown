RDATE
=====

NAME
----

rdate - print the date in the roman calendar

SYNOPSIS
--------

rdate [OPTION]... [+FORMAT]

VERSION
-------

- Version: 0.1.0
- Release Date: Wednesday, 8 Kalends March, 2770
- Url: `https://www.github.com/FalacerSelene/rdate`

DESCRIPTION
-----------

Display the current time in the given FORMAT.

-h
    Print help text.

-d
    Print all numbers in dozenal, using 'D' and 'E' for the extra digits.

-D
    Print all numbers in decimal. The default.

-V
    Print version information.

FORMAT control the output. Interpreted sequences are:

- %%    A literal %
- %a    Short weekday name e.g. Sun
- %A    Full weekday name e.g. Sunday
- %b    Short month name e.g. Jan
- %B    Full month name e.g. January
- %d    Short day of month, always with 2 digits e.g. 04N
- %D    Full day of month, always with 2 digits e.g. 04 Nones
- %e    Short day of month e.g. 4N or I
- %E    Full day of month e.g. 4 Nones or Ides
- %h    Synonym for %b
- %m    Month number, always with 2 digits e.g. 01 for January
- %n    A line break
- %t    A tab character
- %u    Day of week number, where Sunday is 7
- %w    Day of week number, where Sunday is 0
- %y    Final 2 digits of year
- %Y    Full 4 digit year

LICENCE
-------

This software is, as far as possible, placed into the public domain under the
terms of the `UNLICENCE`. See the licence file distributed with the source
code or at the url listed in the `VERSION` section for more information.
