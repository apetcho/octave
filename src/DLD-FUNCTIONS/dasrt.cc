/*

Copyright (C) 2002 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>

#include "DASRT.h"
#include "lo-mappers.h"

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "ov-fcn.h"
#include "pager.h"
#include "parse.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

#include "DASRT-opts.cc"

// Global pointers for user defined function required by dasrt.
static octave_function *dasrt_f;
static octave_function *dasrt_j;
static octave_function *dasrt_cf;

// Is this a recursive call?
static int call_depth = 0;

static ColumnVector
dasrt_user_f (const ColumnVector& x, const ColumnVector& xprime,
	      double t, int& ires)
{
  ColumnVector retval;

  octave_value_list args;

  int n = x.length ();

  args(2) = t;

  if (n > 1)
    {
      args(1) = xprime;
      args(0) = x;
    }
  else if (n == 1)
    {
      args(1) = xprime(0);
      args(0) = x(0);
    }
  else
    {
      args(1) = Matrix ();
      args(0) = Matrix ();
    }

  if (dasrt_f)
    {
      octave_value_list tmp = dasrt_f->do_multi_index_op (1, args);

      if (error_state)
	{
	  gripe_user_supplied_eval ("dasrt");
	  return retval;
	}

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  retval = ColumnVector (tmp(0).vector_value ());

	  if (error_state || retval.length () == 0)
	    gripe_user_supplied_eval ("dasrt");
	}
      else
	gripe_user_supplied_eval ("dasrt");
    }

  return retval;
}

static ColumnVector
dasrt_user_cf (const ColumnVector& x, double t)
{
  ColumnVector retval;

  octave_value_list args;

  int n = x.length ();

  if (n > 1)
    args(0) = x;
  else if (n == 1)
    args(0) = x(0);
  else
    args(0) = Matrix ();

  args(1) = t;

  if (dasrt_cf)
    {
      octave_value_list tmp = dasrt_cf->do_multi_index_op (1, args);

      if (error_state)
	{
	  gripe_user_supplied_eval ("dasrt");
	  return retval;
	}

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  retval = ColumnVector (tmp(0).vector_value ());

	  if (error_state || retval.length () == 0)
	    gripe_user_supplied_eval ("dasrt");
	}
      else
	gripe_user_supplied_eval ("dasrt");
    }

  return retval;
}

static Matrix
dasrt_user_j (const ColumnVector& x, const ColumnVector& xdot,
	      double t, double cj)
{
  Matrix retval;

  int nstates = x.capacity ();

  assert (nstates == xdot.capacity ());

  octave_value_list args;

  args(3) = cj;
  args(2) = t;

  if (nstates > 1)
    {
      Matrix m1 (nstates, 1);
      Matrix m2 (nstates, 1);
      for (int i = 0; i < nstates; i++)
	{
	  m1 (i, 0) = x (i);
	  m2 (i, 0) = xdot (i);
	}
      octave_value state (m1);
      octave_value deriv (m2);
      args(1) = deriv;
      args(0) = state;
    }
  else
    {
      double d1 = x (0);
      double d2 = xdot (0);
      octave_value state (d1);
      octave_value deriv (d2);
      args(1) = deriv;
      args(0) = state;
    }

  if (dasrt_j)
    {
      octave_value_list tmp = dasrt_j->do_multi_index_op (1, args);

      if (error_state)
	{
	  gripe_user_supplied_eval ("dasrt");
	  return retval;
	}

      int tlen = tmp.length ();
      if (tlen > 0 && tmp(0).is_defined ())
	{
	  retval = tmp(0).matrix_value ();

	  if (error_state || retval.length () == 0)
	    gripe_user_supplied_eval ("dasrt");
	}
      else
	gripe_user_supplied_eval ("dasrt");
    }

  return retval;
}

#define DASRT_ABORT \
  do \
    { \
      unwind_protect::run_frame ("Fdasrt"); \
      return retval; \
    } \
  while (0)

#define DASRT_ABORT1(msg) \
  do \
    { \
      ::error ("dasrt: " msg); \
      DASRT_ABORT; \
    } \
  while (0)

#define DASRT_ABORT2(fmt, arg) \
  do \
    { \
      ::error ("dasrt: " fmt, arg); \
      DASRT_ABORT; \
    } \
  while (0)

DEFUN_DLD (dasrt, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{x}, @var{xdot}, @var{t_out}, @var{istat}, @var{msg}] =} dasrt (@var{fcn} [, @var{g}], @var{x_0}, @var{xdot_0}, @var{t} [, @var{t_crit}])\n\
Solve the set of differential-algebraic equations\n\
@tex\n\
$$ 0 = f (\\dot{x}, x, t) $$\n\
with\n\
$$ x(t_0) = x_0, \\dot{x}(t_0) = \\dot{x}_0 $$\n\
@end tex\n\
@ifinfo\n\
\n\
@example\n\
0 = f (xdot, x, t)\n\
@end example\n\
\n\
with\n\
\n\
@example\n\
x(t_0) = x_0, xdot(t_0) = xdot_0\n\
@end example\n\
\n\
@end ifinfo\n\
with functional stopping criteria (root solving).\n\
\n\
The solution is returned in the matrices @var{x} and @var{xdot},\n\
with each row in the result matrices corresponding to one of the\n\
elements in the vector @var{t_out}.  The first element of @var{t}\n\
should be @math{t_0} and correspond to the initial state of the\n\
system @var{x_0} and its derivative @var{xdot_0}, so that the first\n\
row of the output @var{x} is @var{x_0} and the first row\n\
of the output @var{xdot} is @var{xdot_0}.\n\
\n\
The vector @var{t} provides an upper limit on the length of the\n\
integration.  If the stopping condition is met, the vector\n\
@var{t_out} will be shorter than @var{t}, and the final element of\n\
@var{t_out} will be the point at which the stopping condition was met,\n\
and may not correspond to any element of the vector @var{t}.\n\
\n\
The first argument, @var{fcn}, is a string that names the function to\n\
call to compute the vector of residuals for the set of equations.\n\
It must have the form\n\
\n\
@example\n\
@var{res} = f (@var{x}, @var{xdot}, @var{t})\n\
@end example\n\
\n\
@noindent\n\
in which @var{x}, @var{xdot}, and @var{res} are vectors, and @var{t} is a\n\
scalar.\n\
\n\
If @var{fcn} is a two-element string array, the first element names\n\
the function @math{f} described above, and the second element names\n\
a function to compute the modified Jacobian\n\
\n\
@tex\n\
$$\n\
J = {\\partial f \\over \\partial x}\n\
  + c {\\partial f \\over \\partial \\dot{x}}\n\
$$\n\
@end tex\n\
@ifinfo\n\
\n\
@example\n\
      df       df\n\
jac = -- + c ------\n\
      dx     d xdot\n\
@end example\n\
\n\
@end ifinfo\n\
\n\
The modified Jacobian function must have the form\n\
\n\
@example\n\
\n\
@var{jac} = j (@var{x}, @var{xdot}, @var{t}, @var{c})\n\
\n\
@end example\n\
\n\
The optional second argument names a function that defines the\n\
constraint functions whose roots are desired during the integration.\n\
This function must have the form\n\
\n\
@example\n\
@var{g_out} = g (@var{x}, @var{t})\n\
@end example\n\
\n\
and return a vector of the constraint function values.\n\
If the value of any of the constraint functions changes sign, @sc{Dasrt}\n\
will attempt to stop the integration at the point of the sign change.\n\
\n\
If the name of the constraint function is omitted, @code{dasrt} solves\n\
the saem problem as @code{daspk} or @code{dassl}.\n\
\n\
Note that because of numerical errors in the constraint functions\n\
due to roundoff and integration error, @sc{Dasrt} may return false\n\
roots, or return the same root at two or more nearly equal values of\n\
@var{T}.  If such false roots are suspected, the user should consider\n\
smaller error tolerances or higher precision in the evaluation of the\n\
constraint functions.\n\
\n\
If a root of some constraint function defines the end of the problem,\n\
the input to @sc{Dasrt} should nevertheless allow integration to a\n\
point slightly past that root, so that @sc{Dasrt} can locate the root\n\
by interpolation.\n\
\n\
The third and fourth arguments to @code{dasrt} specify the initial\n\
condition of the states and their derivatives, and the fourth argument\n\
specifies a vector of output times at which the solution is desired,\n\
including the time corresponding to the initial condition.\n\
\n\
The set of initial states and derivatives are not strictly required to\n\
be consistent.  In practice, however, @sc{Dassl} is not very good at\n\
determining a consistent set for you, so it is best if you ensure that\n\
the initial values result in the function evaluating to zero.\n\
\n\
The sixth argument is optional, and may be used to specify a set of\n\
times that the DAE solver should not integrate past.  It is useful for\n\
avoiding difficulties with singularities and points where there is a\n\
discontinuity in the derivative.\n\
\n\
After a successful computation, the value of @var{istate} will be\n\
greater than zero (consistent with the Fortran version of @sc{Dassl}).\n\
\n\
If the computation is not successful, the value of @var{istate} will be\n\
less than zero and @var{msg} will contain additional information.\n\
\n\
You can use the function @code{dasrt_options} to set optional\n\
parameters for @code{dasrt}.\n\
@end deftypefn\n\
@seealso{daspk, dasrt, lsode, odessa}")
{
  octave_value_list retval;

  unwind_protect::begin_frame ("Fdasrt");

  unwind_protect_int (call_depth);
  call_depth++;

  if (call_depth > 1)
    DASRT_ABORT1 ("invalid recursive call");

  int argp = 0;

  int nargin = args.length ();

  if (nargin < 4 || nargin > 6)
    {
      print_usage ("dasrt");
      unwind_protect::run_frame ("Fdasrt");
      return retval;
    }

  dasrt_f = 0;
  dasrt_j = 0;
  dasrt_cf = 0;

  // Check all the arguments.  Are they the right animals?

  // Here's where I take care of f and j in one shot:

  octave_value f_arg = args(0);

  switch (f_arg.rows ())
    {
    case 1:
      dasrt_f = extract_function
	(args(0), "dasrt", "__dasrt_fcn__",
	 "function res = __dasrt_fcn__ (x, xdot, t) res = ",
	 "; endfunction");
      break;
      
    case 2:
      {
	string_vector tmp = args(0).all_strings ();
	
	if (! error_state)
	  {
	    dasrt_f = extract_function
	      (tmp(0), "dasrt", "__dasrt_fcn__",
	       "function res = __dasrt_fcn__ (x, xdot, t) res = ",
	       "; endfunction");
	    
	    if (dasrt_f)
	      {
		dasrt_j = extract_function
		  (tmp(1), "dasrt", "__dasrt_jac__",
		   "function jac = __dasrt_jac__ (x, xdot, t, cj) jac = ",
		   "; endfunction");
		
		if (! dasrt_j)
		  dasrt_f = 0;
	      }
	  }
      }
      break;
      
    default:
      DASRT_ABORT1
	("first arg should be a string or 2-element string array");
    }
  
  if (error_state || (! dasrt_f))
    DASRT_ABORT;
  
  DAERTFunc func (dasrt_user_f);
  
  argp++;
  
  if (args(1).is_string ())
    {
      dasrt_cf = is_valid_function (args(1), "dasrt", true);

      if (! dasrt_cf)
	DASRT_ABORT1 ("expecting function name as argument 2");

      argp++;

      func.set_constraint_function (dasrt_user_cf);
    }

  ColumnVector state (args(argp++).vector_value ());

  if (error_state)
    DASRT_ABORT2 ("expecting state vector as argument %d", argp);

  ColumnVector stateprime (args(argp++).vector_value ());

  if (error_state)
    DASRT_ABORT2 
       ("expecting time derivative of state vector as argument %d", argp);

  ColumnVector out_times (args(argp++).vector_value ());

  if (error_state)
    DASRT_ABORT2
	("expecting output time vector as %s argument %d", argp);

  double tzero = out_times (0);

  ColumnVector crit_times;

  bool crit_times_set = false;

  if (argp < nargin)
    {
      crit_times = ColumnVector (args(argp++).vector_value ());

      if (error_state)
	DASRT_ABORT2
	  ("expecting critical time vector as argument %d", argp);

      crit_times_set = true;
    }

  if (dasrt_j)
    func.set_jacobian_function (dasrt_user_j);

  DASRT_result output;

  DASRT dae = DASRT (state, stateprime, tzero, func);

  dae.set_options (dasrt_opts);

  if (crit_times_set)
    output = dae.integrate (out_times, crit_times);
  else
    output = dae.integrate (out_times);

  if (! error_state)
    {
      std::string msg = dae.error_message ();

      retval(4) = msg;
      retval(3) = static_cast<double> (dae.integration_state ());

      if (dae.integration_ok ())
	{
	  retval(2) = output.times ();
	  retval(1) = output.deriv ();
	  retval(0) = output.state ();
	}
      else
	{
	  retval(2) = Matrix ();
	  retval(1) = Matrix ();
	  retval(0) = Matrix ();

	  if (nargout < 4)
	    error ("dasrt: %s", msg.c_str ());
	}
    }

  unwind_protect::run_frame ("Fdasrt");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
