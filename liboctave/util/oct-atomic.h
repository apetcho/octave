/*

Copyright (C) 2019 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_oct_atomic_h)
#define octave_oct_atomic_h 1

#include "octave-config.h"

#if defined __cplusplus
extern "C" {
#endif

extern octave_idx_type octave_atomic_increment (octave_idx_type *x);

  extern octave_idx_type octave_atomic_decrement (octave_idx_type *x);

#if defined __cplusplus
}
#endif

#endif