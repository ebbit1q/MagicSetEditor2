//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/boolean.hpp>

// ----------------------------------------------------------------------------- : BooleanField

BooleanField::BooleanField() {
  choices->choices.push_back(make_intrusive<Choice>(_("yes")));
  choices->choices.push_back(make_intrusive<Choice>(_("no")));
  choices->initIds();
}

IMPLEMENT_FIELD_TYPE(Boolean, "boolean");

IMPLEMENT_REFLECTION(BooleanField) {
  REFLECT_BASE(Field); // NOTE: don't reflect as a ChoiceField
  REFLECT(script);
  REFLECT_N("default", default_script);
  REFLECT(initial);
}

// ----------------------------------------------------------------------------- : BooleanStyle

BooleanStyle::BooleanStyle(const ChoiceFieldP& field)
  : ChoiceStyle(field)
{
  render_style = RENDER_BOTH;
  choice_images[_("yes")] = ScriptableImage(make_intrusive<BuiltInImage>(_("bool_yes")));
  choice_images[_("no")]  = ScriptableImage(make_intrusive<BuiltInImage>(_("bool_no")));
}

IMPLEMENT_REFLECTION(BooleanStyle) {
  REFLECT_BASE(ChoiceStyle);
}

// ----------------------------------------------------------------------------- : BooleanValue

IMPLEMENT_REFLECTION_NAMELESS(BooleanValue) {
  REFLECT_BASE(ChoiceValue);
}
