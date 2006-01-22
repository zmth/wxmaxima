/*
 *  Copyright (C) 2004-2006 Andrej Vodopivec <andrejv@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "FracCell.h"
#include "TextCell.h"

FracCell::FracCell() : MathCell()
{
  m_num = NULL;
  m_denom = NULL;
  m_fracStyle = FC_NORMAL;
  m_exponent = false;

  m_open1 = NULL;
  m_close1 = NULL;
  m_open2 = NULL;
  m_close2 = NULL;
  m_divide = NULL;
}

MathCell* FracCell::Copy(bool all)
{
  FracCell* tmp = new FracCell;
  tmp->SetNum(m_num->Copy(true));
  tmp->SetDenom(m_denom->Copy(true));
  tmp->m_fracStyle = m_fracStyle;
  tmp->m_exponent = m_exponent;
  tmp->m_type = m_type;
  tmp->SetupBreakUps();
  if (all && m_next!=NULL)
    tmp->AppendCell(m_next->Copy(all));
  return tmp;
}

FracCell::~FracCell()
{
  if (m_num != NULL)
    delete m_num;
  if (m_denom != NULL)
    delete m_denom;
  if (m_next != NULL)
    delete m_next;
}

void FracCell::Destroy()
{
  if (m_num != NULL)
    delete m_num;
  if (m_denom != NULL)
    delete m_denom;
  m_num = NULL;
  m_denom = NULL;
  m_next = NULL;

  delete m_open1;
  delete m_open2;
  delete m_close1;
  delete m_close2;
  delete m_divide;
}

void FracCell::SetNum(MathCell *num)
{
  if (num == NULL)
    return;
  if (m_num != NULL)
    delete m_num;
  m_num = num;
}

void FracCell::SetDenom(MathCell *denom)
{
  if (denom == NULL)
    return;
  if (m_denom != NULL)
    delete m_denom;
  m_denom = denom;
}

void FracCell::RecalculateWidths(CellParser& parser, int fontsize, bool all)
{
  double scale = parser.GetScale();
  if (m_isBroken) {
    m_num->RecalculateWidths(parser, fontsize, true);
    m_denom->RecalculateWidths(parser, fontsize, true);
  }
  else {
    m_num->RecalculateWidths(parser, MAX(8, fontsize-2), true);
    m_denom->RecalculateWidths(parser, MAX(8, fontsize-2), true);
  }
  if (m_exponent && !m_isBroken) {
    wxDC& dc = parser.GetDC();
    int width, height;
    dc.GetTextExtent(wxT("/"), &width, &height);
    m_width = m_num->GetFullWidth(scale) + m_denom->GetFullWidth(scale) + width;
  }
  else {
    m_width = MAX(m_num->GetFullWidth(scale), m_denom->GetFullWidth(scale));
  }
  m_open1->RecalculateWidths(parser, fontsize, false);
  m_close1->RecalculateWidths(parser, fontsize, false);
  m_open2->RecalculateWidths(parser, fontsize, false);
  m_close2->RecalculateWidths(parser, fontsize, false);
  m_divide->RecalculateWidths(parser, fontsize, false);
  MathCell::RecalculateWidths(parser, fontsize, all);
}

void FracCell::RecalculateSize(CellParser& parser, int fontsize, bool all)
{
  double scale = parser.GetScale();
  if (m_isBroken) {
    m_num->RecalculateSize(parser, fontsize, true);
    m_denom->RecalculateSize(parser, fontsize, true);
  }
  else {
    m_num->RecalculateSize(parser, MAX(8, fontsize-2), true);
    m_denom->RecalculateSize(parser, MAX(8, fontsize-2), true);
  }
  if (!m_exponent) {
    m_height = m_num->GetMaxHeight() + m_denom->GetMaxHeight() +
               SCALE_PX(4, scale);
    m_center = m_num->GetMaxHeight() + SCALE_PX(2, scale);
  }
  else {
    m_height = m_num->GetMaxHeight();
    m_center = m_height/2;
  }

  m_open1->RecalculateSize(parser, fontsize, false);
  m_close1->RecalculateSize(parser, fontsize, false);
  m_open2->RecalculateSize(parser, fontsize, false);
  m_close2->RecalculateSize(parser, fontsize, false);
  m_divide->RecalculateSize(parser, fontsize, false);
  MathCell::RecalculateSize(parser, fontsize, all);
}

void FracCell::Draw(CellParser& parser, wxPoint point, int fontsize, bool all)
{
  if (DrawThisCell(parser, point)) {
    wxDC& dc = parser.GetDC();
    double scale = parser.GetScale();
    wxPoint num, denom;

    if (m_exponent && !m_isBroken) {
      int width, height;
      double scale = parser.GetScale();
      dc.GetTextExtent(wxT("/"), &width, &height);
      num.x = point.x;
      num.y = point.y;
      denom.x = point.x + m_num->GetFullWidth(scale) + width;
      denom.y = num.y;

      dc.GetTextExtent(wxT("/"), &width, &height);
      m_num->Draw(parser, num, MAX(8, fontsize-2), true);
      m_denom->Draw(parser, denom, MAX(8, fontsize-2), true);
      dc.DrawText(wxT("/"),
                  point.x + m_num->GetFullWidth(scale),
                  point.y - m_num->GetMaxCenter() + SCALE_PX(2, scale));
    }
    else {
      num.x = point.x + (m_width - m_num->GetFullWidth(scale))/2;
      num.y = point.y - m_num->GetMaxHeight() + m_num->GetMaxCenter() -
              SCALE_PX(2, scale);
      m_num->Draw(parser, num, MAX(8, fontsize-2), true);

      denom.x = point.x + (m_width - m_denom->GetFullWidth(scale))/2;
      denom.y = point.y + m_denom->GetMaxCenter() + SCALE_PX(2, scale);
      m_denom->Draw(parser, denom, MAX(8, fontsize-2), true);
      SetPen(parser);
      if (m_fracStyle != FC_CHOOSE)
        dc.DrawLine(point.x, point.y, point.x + m_width, point.y);
      UnsetPen(parser);
    }
  }

  MathCell::Draw(parser, point, fontsize, all);
}

wxString FracCell::ToString(bool all)
{
  wxString s;
  if (!m_isBroken) {
    if (m_fracStyle == FC_NORMAL) {
      if (m_num->IsCompound())
        s += wxT("(") + m_num->ToString(true) + wxT(")/");
      else
        s += m_num->ToString(true) + wxT("/");
      if (m_denom->IsCompound())
        s += wxT("(") + m_denom->ToString(true) + wxT(")");
      else
        s += m_denom->ToString(true);
    }
    else if (m_fracStyle == FC_CHOOSE) {
      s = wxT("binom(") + m_num->ToString(true) + wxT(",") +
          m_denom->ToString(true) + wxT(")");
    }
    else {
      MathCell* tmp = m_denom;
      while (tmp != NULL) {
        tmp = tmp->m_next;   // Skip the d
        if (tmp == NULL)
          break;
        tmp = tmp->m_next;   // Skip the *
        if (tmp == NULL)
          break;
        s += tmp->GetDiffPart();
        tmp = tmp->m_next;   // Skip the *
        if (tmp == NULL)
          break;
        tmp = tmp->m_next;
      }
    }
  }
  if (m_fracStyle == FC_NORMAL)
    s += MathCell::ToString(all);
  return s;
}

void FracCell::SelectInner(wxRect& rect, MathCell **first, MathCell **last)
{
  *first = NULL;
  *last = NULL;
  if (m_num->ContainsRect(rect))
    m_num->SelectRect(rect, first, last);
  else if (m_denom->ContainsRect(rect))
    m_denom->SelectRect(rect, first, last);
  if (*first == NULL || *last == NULL) {
    *first = this;
    *last = this;
  }
}

void FracCell::SetExponentFlag()
{
  if (m_num->IsShortNum() && m_denom->IsShortNum())
    m_exponent = true;
}

void FracCell::SetupBreakUps()
{
  if (m_fracStyle == FC_NORMAL) {
    m_open1 = new TextCell(wxT("("));
    m_close1 = new TextCell(wxT(")"));
    m_open2 = new TextCell(wxT("("));
    m_close2 = new TextCell(wxT(")"));
    if (!m_num->IsCompound()) {
      m_open1->m_isHidden = true;
      m_close1->m_isHidden = true;
    }
    if (!m_denom->IsCompound()) {
      m_open2->m_isHidden = true;
      m_close2->m_isHidden = true;
    }
    m_divide = new TextCell(wxT("/"));
  }
  else {
    m_open1 = new TextCell(wxT("binom("));
    m_close1 = new TextCell(wxT("x"));
    m_open2 = new TextCell(wxT("x"));
    m_close2 = new TextCell(wxT(")"));
    m_divide = new TextCell(wxT(","));
    m_close1->m_isHidden = true;
    m_open2->m_isHidden = true;
  }

  m_last1 = m_num;
  while (m_last1->m_next != NULL)
    m_last1 = m_last1->m_next;

  m_last2 = m_denom;
  while (m_last2->m_next != NULL)
    m_last2 = m_last2->m_next;
}

bool FracCell::BreakUp()
{
  if (m_fracStyle == FC_DIFF)
    return false;

  if (!m_isBroken) {
    m_isBroken = true;
    m_open1->m_previousToDraw = this;
    m_open1->m_nextToDraw = m_num;
    m_num->m_previousToDraw = m_open1;
    m_last1->m_nextToDraw = m_close1;
    m_close1->m_previousToDraw = m_last1;
    m_close1->m_nextToDraw = m_divide;
    m_divide->m_previousToDraw = m_close1;
    m_divide->m_nextToDraw = m_open2;
    m_open2->m_previousToDraw = m_divide;
    m_open2->m_nextToDraw = m_denom;
    m_denom->m_previousToDraw = m_open2;
    m_last2->m_nextToDraw = m_close2;
    m_close2->m_previousToDraw = m_last2;
    m_close2->m_nextToDraw = m_nextToDraw;
    if (m_nextToDraw != NULL)
      m_nextToDraw->m_previousToDraw = m_close2;
    m_nextToDraw = m_open1;
    return true;
  }
  return false;
}

void FracCell::Unbreak(bool all)
{
  if (m_isBroken) {
    m_num->Unbreak(true);
    m_denom->Unbreak(true);
  }
  MathCell::Unbreak(all);
}
