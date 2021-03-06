//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <gui/value/choice.hpp>
#include <render/value/multiple_choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceValueEditor

/// An editor 'control' for editing MultipleChoiceValues
class MultipleChoiceValueEditor : public MultipleChoiceValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(MultipleChoice);
  ~MultipleChoiceValueEditor();
  
  void onValueChange() override;
  
  void determineSize(bool force_fit) override;
  
  bool onLeftDown(const RealPoint& pos, wxMouseEvent& ev) override;
  bool onChar(wxKeyEvent& ev) override;
  void onLoseFocus() override;
  
private:
  DropDownListP drop_down;
  vector<int> active;      ///< Which choices are active? (note: vector<bool> is evil)
  friend class DropDownMultipleChoiceList;
  /// Initialize the drop down list
  DropDownList& initDropDown();
  /// Toggle a choice or on or off
  void toggle(int id);
  /// Toggle defaultness or on or off
  void toggleDefault();
};

