//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol/editor.hpp>
#include <gui/symbol/window.hpp>

// ----------------------------------------------------------------------------- : SymbolEditorBase

void SymbolEditorBase::SetStatusText(const String& text) {
  control.parent->SetStatusText(text);
}

void SymbolEditorBase::addAction(unique_ptr<Action> action, bool allow_merge) {
  getSymbol()->actions.addAction(move(action), allow_merge);
}
