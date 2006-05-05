///
///  Copyright (C) 2006 Andrej Vodopivec <andrejv@users.sourceforge.net>
///
///  This program is free software; you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation; either version 2 of the License, or
///  (at your option) any later version.
///
///  This program is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///
///  You should have received a copy of the GNU General Public License
///  along with this program; if not, write to the Free Software
///  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///

#include "EditorCell.h"
#include "wxMaxima.h"
#include "wxMaximaFrame.h"

#include <wx/clipbrd.h>

EditorCell::EditorCell() : MathCell()
{
  m_text = wxEmptyString;
  m_fontSize = -1;
  m_positionOfCaret = 0;
  m_selectionStart = -1;
  m_selectionEnd = -1;
  m_isActive = false;
  m_matchParens = true;
  m_paren1 = m_paren2 = -1;
}

EditorCell::~EditorCell()
{
  if (m_next != NULL)
    delete m_next;
}

MathCell *EditorCell::Copy(bool all)
{
  EditorCell *tmp = new EditorCell();
  tmp->SetValue(m_text);
  CopyData(this, tmp);
  if (all && m_next != NULL)
    tmp->AppendCell(m_next->Copy(all));
  return tmp;
}

void EditorCell::Destroy()
{
  m_next = NULL;
}

wxString EditorCell::ToString(bool all)
{
  wxString text = m_text;
  return text + MathCell::ToString(all);
}

void EditorCell::RecalculateWidths(CellParser& parser, int fontsize, bool all)
{
  if (m_height == -1 || m_width == -1 || fontsize != m_fontSize || parser.ForceUpdate())
  {
    m_fontSize = fontsize;
    wxDC& dc = parser.GetDC();
    double scale = parser.GetScale();
    SetFont(parser, fontsize);

    dc.GetTextExtent(wxT("X"), &m_charWidth, &m_charHeight);

    int newLinePos = 0, prevNewLinePos = 0;
    int width = 0, width1, height1;

    m_numberOfLines = 1;

    while (newLinePos < m_text.Length())
    {
      while (newLinePos < m_text.Length())
      {
        if (m_text.GetChar(newLinePos) == '\n')
          break;
        newLinePos++;
      }

      dc.GetTextExtent(m_text.SubString(prevNewLinePos, newLinePos), &width1, &height1);
      width = MAX(width, width1);

      while (newLinePos < m_text.Length() && m_text.GetChar(newLinePos) == '\n')
      {
        newLinePos++;
        m_numberOfLines++;
      }

      prevNewLinePos = newLinePos;
    }

    if (m_text == wxEmptyString)
      width = m_charWidth;

    m_width = width + 2 * SCALE_PX(2, scale);
    m_height = m_numberOfLines * m_charHeight + 2 * SCALE_PX(2, scale);

    m_center = m_charHeight / 2 + SCALE_PX(2, scale);
  }
  MathCell::RecalculateWidths(parser, fontsize, all);
}

void EditorCell::RecalculateSize(CellParser& parser, int fontsize, bool all)
{
  MathCell::RecalculateSize(parser, fontsize, all);
}

void EditorCell::Draw(CellParser& parser, wxPoint point, int fontsize, bool all)
{
  double scale = parser.GetScale();
  wxDC& dc = parser.GetDC();

  if (m_width == -1 || m_height == -1)
    RecalculateWidths(parser, fontsize, false);

  if (DrawThisCell(parser, point) && !m_isHidden)
  {
    SetForeground(parser);
    SetPen(parser);
    SetFont(parser, fontsize);

    int newLinePos = 0, prevNewLinePos = 0, numberOfLines = 0;

    //
    // Draw the text
    //
    while (newLinePos < m_text.Length())
    {
      while (newLinePos < m_text.Length())
      {
        if (m_text.GetChar(newLinePos) == '\n')
          break;
        newLinePos++;
      }

      dc.DrawText(m_text.SubString(prevNewLinePos, newLinePos),
                  point.x + SCALE_PX(2, scale),
                  point.y - m_center + SCALE_PX(2, scale) + m_charHeight * numberOfLines);

      m_numberOfLines++;
      newLinePos++;
      prevNewLinePos = newLinePos;
      numberOfLines++;
    }

    if (m_isActive)
    {
      //
      // Draw the caret
      //
      int caretInLine = 0;
      int caretInColumn = 0;
      
      PositionToXY(m_positionOfCaret, &caretInColumn, &caretInLine);
      
      wxString line = GetLineString(caretInLine, 0, caretInColumn);
      int lineWidth, lineHeight;
      dc.GetTextExtent(line, &lineWidth, &lineHeight);
      

      dc.DrawLine(point.x + SCALE_PX(2, scale) + lineWidth,
                  point.y + SCALE_PX(2, scale) - m_center + caretInLine * m_charHeight,
                  point.x + SCALE_PX(2, scale) + lineWidth,
                  point.y + SCALE_PX(2, scale) - m_center + (caretInLine + 1) * m_charHeight);

      //
      // Mark selection
      //
      if (m_selectionStart > -1)
      {
        dc.SetLogicalFunction(wxINVERT);
        wxPoint point;
        long save = m_positionOfCaret;
        long start = MIN(m_selectionStart, m_selectionEnd);
        long end = MAX(m_selectionStart, m_selectionEnd);
        for (m_positionOfCaret = start;
             m_positionOfCaret < end;
             m_positionOfCaret++)
        {
          point = PositionToPoint(dc);
          dc.DrawRectangle(point.x + SCALE_PX(2, scale),
                           point.y  + SCALE_PX(2, scale) - m_center,
                           m_charWidth,
			   m_charHeight);
        }
        m_positionOfCaret = save;
        dc.SetLogicalFunction(wxCOPY);
      }

      //
      // Matching parens
      //
      else if (m_paren1 != -1 && m_paren2 != -1)
      {
        dc.SetLogicalFunction(wxAND);
        dc.SetBrush(*wxLIGHT_GREY_BRUSH);
        wxPoint point = PositionToPoint(dc, m_paren1);
        dc.DrawRectangle(point.x + SCALE_PX(2, scale),
                         point.y  + SCALE_PX(2, scale) - m_center,
                         m_charWidth, m_charHeight);
        dc.SetLogicalFunction(wxCOPY);
      }
    }
    UnsetPen(parser);
  }
  MathCell::Draw(parser, point, fontsize, all);
}

void EditorCell::SetFont(CellParser& parser, int fontsize)
{
  wxDC& dc = parser.GetDC();
  double scale = parser.GetScale();

  int fontsize1 = (int) (((double)fontsize) * scale + 0.5);
  fontsize1 = MAX(fontsize1, 1);

  switch(m_type)
  {
  case MC_TYPE_COMMENT:
    dc.SetFont(wxFont(fontsize1, wxMODERN,
                      wxNORMAL,
                      wxNORMAL,
                      0,
                      parser.GetFontName(),
                      parser.GetFontEncoding()));
    break;
  case MC_TYPE_SECTION:
    dc.SetFont(wxFont(fontsize1 + 4, wxMODERN,
                      wxNORMAL,
                      wxBOLD,
                      1,
                      parser.GetFontName(),
                      parser.GetFontEncoding()));
    break;
  case MC_TYPE_TITLE:
    dc.SetFont(wxFont(fontsize1 + 8, wxMODERN,
                      wxSLANT,
                      wxBOLD,
                      1,
                      parser.GetFontName(),
                      parser.GetFontEncoding()));
    break;
  default:
    dc.SetFont(wxFont(fontsize1, wxMODERN,
                      parser.IsItalic(TS_INPUT),
                      parser.IsBold(TS_INPUT),
                      parser.IsUnderlined(TS_INPUT),
                      parser.GetFontName()));
  }
}

void EditorCell::SetForeground(CellParser& parser)
{
  wxDC& dc = parser.GetDC();

  switch (m_type)
  {
  case MC_TYPE_COMMENT:
  case MC_TYPE_SECTION:
  case MC_TYPE_TITLE:
    dc.SetTextForeground(wxTheColourDatabase->Find(parser.GetColor(TS_NORMAL_TEXT)));
    break;
  default:
    dc.SetTextForeground(wxTheColourDatabase->Find(parser.GetColor(TS_INPUT)));
    break;
  }
}

void EditorCell::ProcessEvent(wxKeyEvent &event)
{
  switch (event.GetKeyCode())
  {
  case WXK_ESCAPE:
    {
      wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, deactivate_cell_cancel);
      (wxGetApp().GetTopWindow())->ProcessEvent(ev);
    }
    break;
  case WXK_LEFT:
    if (event.ShiftDown())
    {
      if (m_selectionStart == -1)
        m_selectionEnd = m_selectionStart = m_positionOfCaret;
    }
    else
      m_selectionEnd = m_selectionStart = -1;
    if (m_positionOfCaret > 0)
      m_positionOfCaret--;
    if (event.ShiftDown())
      m_selectionEnd = m_positionOfCaret;
    break;
  case WXK_RIGHT:
    if (event.ShiftDown())
    {
      if (m_selectionStart == -1)
        m_selectionEnd = m_selectionStart = m_positionOfCaret;
    }
    else
      m_selectionEnd = m_selectionStart = -1;
    if (m_positionOfCaret < m_text.Length())
      m_positionOfCaret++;
    if (event.ShiftDown())
      m_selectionEnd = m_positionOfCaret;
    break;
  case WXK_PAGEDOWN:
  case WXK_DOWN:
    {
      if (event.ShiftDown())
      {
        if (m_selectionStart == -1)
          m_selectionEnd = m_selectionStart = m_positionOfCaret;
      }
      else
        m_selectionEnd = m_selectionStart = -1;
      int column, line;
      PositionToXY(m_positionOfCaret, &column, &line);

      line = line < m_numberOfLines-1 ? line + 1 : line;

      m_positionOfCaret = XYToPosition(column, line);

      if (event.ShiftDown())
        m_selectionEnd = m_positionOfCaret;
    }
    break;
  case WXK_PAGEUP:
  case WXK_UP:
    {
      if (event.ShiftDown())
      {
        if (m_selectionStart == -1)
          m_selectionEnd = m_selectionStart = m_positionOfCaret;
      }
      else
        m_selectionEnd = m_selectionStart = -1;
      int column, line;
      PositionToXY(m_positionOfCaret, &column, &line);

      line = line > 0 ? line - 1 : 0;

      m_positionOfCaret = XYToPosition(column, line);
      if (event.ShiftDown())
        m_selectionEnd = m_positionOfCaret;
    }
    break;
  case WXK_RETURN:
    m_text = m_text.SubString(0, m_positionOfCaret - 1) +
             wxT("\n") +
             m_text.SubString(m_positionOfCaret, m_text.Length());
    m_positionOfCaret++;
    break;
  case WXK_END:
    if (event.ShiftDown())
    {
      if (m_selectionStart == -1)
        m_selectionEnd = m_selectionStart = m_positionOfCaret;
    }
    else
      m_selectionEnd = m_selectionStart = -1;
    if (event.ControlDown())
      m_positionOfCaret = m_text.Length();
    else
    {
      while (m_positionOfCaret < m_text.Length() &&
             m_text.GetChar(m_positionOfCaret) != '\n')
        m_positionOfCaret++;
    }
    if (event.ShiftDown())
      m_selectionEnd = m_positionOfCaret;
    break;
  case WXK_HOME:
    {
      if (event.ShiftDown())
      {
        if (m_selectionStart == -1)
          m_selectionEnd = m_selectionStart = m_positionOfCaret;
      }
      else
        m_selectionEnd = m_selectionStart = -1;
      if (event.ControlDown())
        m_positionOfCaret = 0;
      else
      {
        int col, lin;
        PositionToXY(m_positionOfCaret, &col, &lin);
        m_positionOfCaret = XYToPosition(0, lin);
      }
      if (event.ShiftDown())
        m_selectionEnd = m_positionOfCaret;
    }
    break;
  case WXK_DELETE:
    if (m_positionOfCaret < m_text.Length())
    {
      if (m_selectionStart == -1)
        m_text = m_text.SubString(0, m_positionOfCaret - 1) +
                 m_text.SubString(m_positionOfCaret + 1, m_text.Length());
      else
      {
        long start = MIN(m_selectionEnd, m_selectionStart);
        long end = MAX(m_selectionEnd, m_selectionStart);
        m_text = m_text.SubString(0, start - 1) +
                 m_text.SubString(end, m_text.Length());
        m_positionOfCaret = start;
        m_selectionEnd = m_selectionStart = -1;
      }
    }
    break;
  case WXK_BACK:
    if (m_selectionStart > -1) {
      long start = MIN(m_selectionEnd, m_selectionStart);
      long end = MAX(m_selectionEnd, m_selectionStart);
      m_text = m_text.SubString(0, start - 1) +
               m_text.SubString(end, m_text.Length());
      m_positionOfCaret = start;
      m_selectionEnd = m_selectionStart = -1;
      break;
    }
    else if (m_positionOfCaret > 0)
    {
      m_text = m_text.SubString(0, m_positionOfCaret - 2) +
               m_text.SubString(m_positionOfCaret, m_text.Length());
      m_positionOfCaret--;
    }
    break;
  case WXK_TAB:
    {
      if (m_selectionStart > -1) {
        long start = MIN(m_selectionEnd, m_selectionStart);
        long end = MAX(m_selectionEnd, m_selectionStart);
        m_text = m_text.SubString(0, start - 1) +
                 m_text.SubString(end, m_text.Length());
        m_positionOfCaret = start;
        m_selectionEnd = m_selectionStart = -1;
        break;
      }
      int col, line;
      PositionToXY(m_positionOfCaret, &col, &line);
      wxString ins;
      do {
        col++;
        ins += wxT(" ");
      } while (col%4 != 0);

      m_text = m_text.SubString(0, m_positionOfCaret - 1) +
               ins +
               m_text.SubString(m_positionOfCaret, m_text.Length());
      m_positionOfCaret += ins.Length();
    }
    break;
  default:
    if (event.ControlDown())
      break;
    if (m_selectionStart > -1) {
      long start = MIN(m_selectionEnd, m_selectionStart);
      long end = MAX(m_selectionEnd, m_selectionStart);
      m_text = m_text.SubString(0, start - 1) +
               m_text.SubString(end, m_text.Length());
      m_positionOfCaret = start;
      m_selectionEnd = m_selectionStart = -1;
    }
    m_text = m_text.SubString(0, m_positionOfCaret - 1) +
             wxString::Format(wxT("%c"), event.GetKeyCode()) +
             m_text.SubString(m_positionOfCaret, m_text.Length());
    m_positionOfCaret++;
    if (m_matchParens)
    {
      switch (event.GetKeyCode())
      {
      case '(':
        m_text = m_text.SubString(0, m_positionOfCaret - 1) +
                 wxT(")") +
                 m_text.SubString(m_positionOfCaret, m_text.Length());
        break;
      case '[':
        m_text = m_text.SubString(0, m_positionOfCaret - 1) +
                 wxT("]") +
                 m_text.SubString(m_positionOfCaret, m_text.Length());
        break;
      case '{':
        m_text = m_text.SubString(0, m_positionOfCaret - 1) +
                 wxT("}") +
                 m_text.SubString(m_positionOfCaret, m_text.Length());
        break;
      }
    }
    break;
  }

  if (m_type == MC_TYPE_INPUT)
    FindMatchingParens();
  m_width = m_height = m_maxDrop = m_center = -1;
}

void EditorCell::FindMatchingParens()
{
  wxChar first = m_text.GetChar(m_positionOfCaret);
  wxChar second;
  int dir;
  m_paren1 = m_paren2 = -1;

  switch (first)
  {
  case '(':
    second = ')';
    dir = 1;
    break;
  case '[':
    second = ']';
    dir = 1;
    break;
  case '{':
    second = '}';
    dir = 1;
    break;
  case ')':
    second = '(';
    dir = -1;
    break;
  case ']':
    second = '[';
    dir = -1;
    break;
  case '}':
    second = '{';
    dir = -1;
    break;
  default:
    return;
  }

  m_paren2 = m_positionOfCaret;
  m_paren1 = m_paren2 + dir;
  int depth = 1;
  while (m_paren1 >= 0 && m_paren1 < m_text.Length())
  {
    if (m_text.GetChar(m_paren1) == second)
      depth--;
    else if (m_text.GetChar(m_paren1) == first)
      depth++;
    if (depth == 0)
      break;
    m_paren1 += dir;
  }
  if (m_paren1 < 0 || m_paren1 >= m_text.Length())
    m_paren1 = m_paren2 = -1;
}

bool EditorCell::ActivateCell()
{
  m_isActive = !m_isActive;

  m_selectionEnd = m_selectionStart = -1;

  return true;
}

void EditorCell::AddEnding()
{
  wxString text = m_text.Trim();
  if (text.Right(1) != wxT(";") && text.Right(1) != wxT("$"))
    m_text += wxT(";");
}

//
// lines and comuns are counter from zero
// position of caret is pos if caret is just before the character
//   at position pos in m_text.
//
void EditorCell::PositionToXY(int position, int* x, int* y)
{
  int col = 0, lin = 0;
  int pos = 0;

  while (pos < position)
  {
    if (m_text.GetChar(pos) == '\n')
    {
      col = 0,
      lin++;
    }
    else
      col++;
    pos++;
  }

  *x = col;
  *y = lin;
}

int EditorCell::XYToPosition(int x, int y)
{
  int col = 0, lin = 0;
  int pos = 0;

  while (pos < m_text.Length() && lin < y)
  {
    if (m_text.GetChar(pos) == '\n')
      lin++;
    pos++;
  }

  while (pos < m_text.Length() && col < x)
  {
    if (m_text.GetChar(pos) == '\n')
      break;
    pos++;
    col++;
  }

  return pos;
}

wxPoint EditorCell::PositionToPoint(wxDC& dc, int pos)
{
  int x = m_currentPoint.x, y = m_currentPoint.y;
  int height, width;
  int cX, cY;
  wxString line = wxEmptyString;

  if (pos == -1)
    pos = m_positionOfCaret;

  if (x == -1 || y == -1)
    return wxPoint(-1, -1);

  PositionToXY(pos, &cX, &cY);
  if (cX>0)
    line = GetLineString(cY, 0, cX);

  dc.GetTextExtent(line, &width, &height);

  x += width;
  y += m_charHeight * cY;

  return wxPoint(x, y);
}

void EditorCell::SelectPointText(CellParser& parser, wxPoint& point)
{
  double scale = parser.GetScale();
  wxDC& dc = parser.GetDC();
  wxString s;
  
  SetPen(parser);
  SetFont(parser, m_fontSize);

  m_selectionEnd = m_selectionStart = -1;
  wxPoint translate(point);

  translate.x -= m_currentPoint.x - 2;
  translate.y -= m_currentPoint.y - 2 - m_center;

  int col = translate.x / m_charWidth;
  int lin = translate.y / m_charHeight;
  int width, height;

  s = GetLineString(lin, 0, col + 1);
  dc.GetTextExtent(s, &width, &height);
  while (width < translate.x && col < m_text.Length())
  {
    col++;
    s = GetLineString(lin, 0, col + 1);
    dc.GetTextExtent(s, &width, &height);
  }
  
  s = GetLineString(lin, 0, col);
  dc.GetTextExtent(s, &width, &height);
  while (width > translate.x && col > 0)
  {
    col--;
    s = GetLineString(lin, 0, col);
    dc.GetTextExtent(s, &width, &height);
  }

  m_positionOfCaret = XYToPosition(col, lin);
  FindMatchingParens();
}

void EditorCell::SelectRectText(CellParser& parser, wxPoint& one, wxPoint& two)
{
  SelectPointText(parser, one);
  long start = m_positionOfCaret;
  SelectPointText(parser, two);
  m_selectionEnd = m_positionOfCaret;
  m_selectionStart = start;
  m_paren2 = m_paren1 = -1;
  if (m_selectionStart == m_selectionEnd)
  {
    m_selectionStart = -1;
    m_selectionEnd = -1;
  }
}

bool EditorCell::CopyToClipboard()
{
  if (m_selectionStart == -1)
    return false;
  if (wxTheClipboard->Open())
  {
    long start = MIN(m_selectionStart, m_selectionEnd);
    long end = MAX(m_selectionStart, m_selectionEnd) - 1;
    wxString s = m_text.SubString(start, end);

    wxTheClipboard->SetData(new wxTextDataObject(s));
    wxTheClipboard->Close();
  }
  return true;
}

bool EditorCell::CutToClipboard()
{
  if (m_selectionStart == -1)
    return false;

  CopyToClipboard();

  long start = MIN(m_selectionStart, m_selectionEnd);
  long end = MAX(m_selectionStart, m_selectionEnd);
  m_positionOfCaret = start;
  m_text = m_text.SubString(0, start - 1) +
           m_text.SubString(end, m_text.Length());

  m_selectionEnd = m_selectionStart = -1;

  m_width = m_height = m_maxDrop = m_center = -1;

  return true;
}

void EditorCell::PasteFromClipboard()
{
  if (wxTheClipboard->Open())
  {
    if (wxTheClipboard->IsSupported(wxDF_TEXT))
    {
      wxTextDataObject obj;
      wxTheClipboard->GetData(obj);
      if (m_selectionStart > -1)
      {
        long start = MIN(m_selectionStart, m_selectionEnd);
        long end = MAX(m_selectionStart, m_selectionEnd);
        m_positionOfCaret = start;
        m_text = m_text.SubString(0, start - 1) +
                 m_text.SubString(end, m_text.Length());
      }
      wxString data = obj.GetText();
      m_text = m_text.SubString(0, m_positionOfCaret - 1) +
               data +
               m_text.SubString(m_positionOfCaret, m_text.Length());
      m_selectionStart = m_positionOfCaret;
      m_positionOfCaret += data.Length();
      m_selectionEnd = m_positionOfCaret;
    }
  }

  m_width = m_height = m_maxDrop = m_center = -1;
}

wxString EditorCell::GetLineString(int line, int start, int end)
{
  if (start >= end)
    return wxEmptyString;

  int posStart = 0, posEnd = 0;
  
  posStart = XYToPosition(start, line);
  if (end == -1)
  {
    posEnd = XYToPosition(0, line+1);
    posEnd--;
  }
  else
    posEnd = XYToPosition(end, line);
  
  return m_text.SubString(posStart, posEnd - 1);
}