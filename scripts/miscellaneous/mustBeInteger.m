########################################################################
##
## Copyright (C) 2019-2020 The Octave Project Developers
##
## See the file COPYRIGHT.md in the top-level directory of this
## distribution or <https://octave.org/copyright/>.
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.
##
########################################################################

## -*- texinfo -*-
## @deftypefn {} {@var{x} =} mustBeInteger (@var{x})
##
## Requires that input @var{x} is integer-valued (but not necessarily
## integer-typed).
##
## Raises an error if any element of the input @var{x} is not a finite,
## real, integer-valued numeric value, as determined by various checks.
##
## @end deftypefn

function x = mustBeInteger (x)
  if isinteger (x) || islogical (x)
    return
  endif
  but = [];
  if ! isnumeric (x)
    but = sprintf ("it was non-numeric (got a %s)", class (x));
  elseif any (! isfinite (x))
    but = "there were Inf values";
  elseif ! isreal (x)
    but = "it was complex";
  elseif ! all (floor (x) == x)
    but = "it had fractional values in some elements";
  end
  if ! isempty (but)
    label = inputname (1);
    if isempty (label)
      label = "input";
    endif
    error ("%s must be integer-valued; but %s", label, but);
  endif
endfunction