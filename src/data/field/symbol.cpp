//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/symbol.hpp>

// ----------------------------------------------------------------------------- : SymbolField

StyleP SymbolField::newStyle(const FieldP& thisP) const {
	return new_shared<SymbolStyle>();
}

ValueP SymbolField::newValue(const FieldP& thisP) const {
	return new_shared<SymbolValue>();
}

String SymbolField::typeName() const {
	return _("symbol");
}

IMPLEMENT_REFLECTION(SymbolField) {
	REFLECT_BASE(Field);
}


// ----------------------------------------------------------------------------- : SymbolStyle

IMPLEMENT_REFLECTION(SymbolStyle) {
	REFLECT_BASE(Style);
	REFLECT(variations);
}

SymbolStyle::Variation::Variation()
	: border_radius(0.05)
{}

IMPLEMENT_REFLECTION(SymbolStyle::Variation) {
	REFLECT(name);
	REFLECT(border_radius);
	//REFLECT_NAMELESS(filter);
}

// ----------------------------------------------------------------------------- : SymbolValue

String SymbolValue::toString() const {
	return filename.empty() ? wxEmptyString : _("<symbol>");
}

IMPLEMENT_REFLECTION_NAMELESS(SymbolValue) {
	REFLECT_NAMELESS(filename);
}
