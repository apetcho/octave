Summary of important user-visible changes for version 9 (yyyy-mm-dd):
---------------------------------------------------------------------

### General improvements

### Graphical User Interface

### Graphics backend

### Matlab compatibility

### Alphabetical list of new functions added in Octave 9

### Deprecated functions, properties, and operators

The following functions and properties have been deprecated in Octave 9
and will be removed from Octave 11 (or whatever version is the second
major release after 9):

- Functions

        Function               | Replacement
        -----------------------|------------------

- Properties

  The following property names are discouraged, but there is no fixed
  date for their removal.

        Object           | Property    | Replacement
        -----------------|-------------|------------

The following features were deprecated in Octave 7 and have been removed
from Octave 9.

- Functions

        Function                   | Replacement
        ---------------------------|------------------
        disable_diagonal_matrix    | optimize_diagonal_matrix
        disable_permutation_matrix | optimize_permutation_matrix
        disable_range              | optimize_range

- For plot functions, the use of numbers to select line colors in
  shorthand formats was an undocumented feature was removed from Octave 9.

- The environment variable used by `mkoctfile` for linker flags is now
  `LDFLAGS` rather than `LFLAGS`.  `LFLAGS` was deprecated in Octave 6
  and has been removed.

### Old release news

- [Octave 8.x](etc/NEWS.8)
- [Octave 7.x](etc/NEWS.7)
- [Octave 6.x](etc/NEWS.6)
- [Octave 5.x](etc/NEWS.5)
- [Octave 4.x](etc/NEWS.4)
- [Octave 3.x](etc/NEWS.3)
- [Octave 2.x](etc/NEWS.2)
- [Octave 1.x](etc/NEWS.1)