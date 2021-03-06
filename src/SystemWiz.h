//
//  Copyright (C) 2004-2014 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SYSTEMWIZ_H
#define SYSTEMWIZ_H

#include <wx/wx.h>
#include <wx/statline.h>

#include "BTextCtrl.h"

#include <vector>
using namespace std;

class SysWiz: public wxDialog
{
public:
  SysWiz(wxWindow* parent, int id, const wxString& title,
         int eqn, const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
  wxString GetValue();
private:
  void set_properties();
  void do_layout();
  int m_size;
  vector<BTextCtrl*> m_inputs;
  BTextCtrl* variables;
  wxStaticLine* static_line_1;
  wxButton* button_1;
  wxButton* button_2;
};

#endif // SYSTEMWIZ_H
