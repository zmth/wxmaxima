//
//  Copyright (C) 2004-2014 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2015 Gunter Königsmann <wxMaxima@physikbuch.de>
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

#ifndef LIMITCELL_H
#define LIMITCELL_H

#include "MathCell.h"

class LimitCell : public MathCell
{
public:
  LimitCell();
  ~LimitCell();
  void Destroy();
  MathCell* Copy();
  void RecalculateSize(CellParser& parser, int fontsize);
  void RecalculateWidths(CellParser& parser, int fontsize);
  void Draw(CellParser& parser, wxPoint point, int fontsize);
  void SetBase(MathCell* base);
  void SetUnder(MathCell* under);
  void SetName(MathCell* name);
  wxString ToString();
  wxString ToTeX();
  wxString ToXML();
  void SelectInner(wxRect& rect, MathCell** first, MathCell** last);
  void SetParent(MathCell *parent);
protected:
  MathCell *m_base;
  MathCell *m_under;
  MathCell *m_name;
};

#endif // LIMITCELL_H
