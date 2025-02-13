////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1994-2023 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

#if ! defined (octave_pt_cmd_h)
#define octave_pt_cmd_h 1

#include "octave-config.h"

#include <string>

#include "ov-fcn.h"
#include "pt.h"
#include "pt-bp.h"
#include "pt-walk.h"

OCTAVE_BEGIN_NAMESPACE(octave)

// A base class for commands.

class tree_command : public tree
{
public:

  tree_command (int l = -1, int c = -1)
    : tree (l, c) { }

  OCTAVE_DISABLE_COPY_MOVE (tree_command)

  virtual ~tree_command () = default;
};

// No-op.

class tree_no_op_command : public tree_command
{
public:

  tree_no_op_command (const std::string& cmd = "no_op", bool e = false,
                      int l = -1, int c = -1)
    : tree_command (l, c), m_eof (e), m_orig_cmd (cmd) { }

  OCTAVE_DISABLE_COPY_MOVE (tree_no_op_command)

  ~tree_no_op_command () = default;

  void accept (tree_walker& tw)
  {
    tw.visit_no_op_command (*this);
  }

  bool is_end_of_fcn_or_script () const
  {
    return (m_orig_cmd == "endfunction" || m_orig_cmd == "endscript");
  }

  bool is_end_of_file () const { return m_eof; }

  std::string original_command () { return m_orig_cmd; }

private:

  bool m_eof;

  std::string m_orig_cmd;
};

// Function definition.

class tree_function_def : public tree_command
{
public:

  tree_function_def (octave_function *f, int l = -1, int c = -1)
    : tree_command (l, c), m_fcn (f) { }

  OCTAVE_DISABLE_CONSTRUCT_COPY_MOVE (tree_function_def)

  ~tree_function_def () = default;

  void accept (tree_walker& tw)
  {
    tw.visit_function_def (*this);
  }

  octave_value function () { return m_fcn; }

private:

  octave_value m_fcn;

  tree_function_def (const octave_value& v, int l = -1, int c = -1)
    : tree_command (l, c), m_fcn (v) { }
};

OCTAVE_END_NAMESPACE(octave)

#endif
