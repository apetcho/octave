/*

Copyright (C) 2019 JunWang

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

#if ! defined (octave_set_path_model_h)
#define octave_set_path_model_h 1

#include <QAbstractListModel>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QIcon>

namespace octave
{
  class set_path_model : public QAbstractListModel
  {
    Q_OBJECT

  public:

    set_path_model (QObject *p = nullptr);

    ~set_path_model (void) = default;

    void clear (void);

    void add_dir (const QString& p);

    void rm_dir (const QModelIndexList& indices);

    void move_dir_up (const QModelIndexList& indices);

    void move_dir_down (const QModelIndexList& indices);

    void move_dir_top (const QModelIndexList& indices);

    void move_dir_bottom (const QModelIndexList& indices);

    std::string to_string (void);

    // Overloaded Qt methods

    void model_to_path (void);

    int rowCount (const QModelIndex& p = QModelIndex ()) const;

    QVariant data (const QModelIndex& idx, int role) const;

  public slots:

    void path_to_model (void);

    void save (void);

    void revert (void);

    void revert_last (void);

  signals:

    void update_data_signal (const QStringList& dirs);

  private slots:

    void update_data (const QStringList& dirs);

  private:

    QStringList m_dirs;

    QStringList m_orig_dirs;

    QStringList m_last_dirs;

    bool m_revertible;
  };
}

#endif
