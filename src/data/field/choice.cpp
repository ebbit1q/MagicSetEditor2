//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/field/choice.hpp>

DECLARE_TYPEOF_COLLECTION(ChoiceField::ChoiceP);
typedef map<String,ScriptableImage> map_String_ScriptableImage;
DECLARE_TYPEOF(map_String_ScriptableImage);

// ----------------------------------------------------------------------------- : ChoiceField

ChoiceField::ChoiceField()
	: choices((Choice*)new Choice)
	, default_name(_("Default"))
{}

IMPLEMENT_FIELD_TYPE(Choice)

String ChoiceField::typeName() const {
	return _("choice");
}

void ChoiceField::initDependencies(Context& ctx, const Dependency& dep) const {
	Field        ::initDependencies(ctx, dep);
	script        .initDependencies(ctx, dep);
	default_script.initDependencies(ctx, dep);
}

IMPLEMENT_REFLECTION(ChoiceField) {
	REFLECT_BASE(Field);
	REFLECT_N("choices", choices->choices);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(initial);
	REFLECT(default_name);
}

// ----------------------------------------------------------------------------- : ChoiceField::Choice

ChoiceField::Choice::Choice()
	: first_id(0)
{}
ChoiceField::Choice::Choice(const String& name)
	: first_id(0), name(name)
{}


bool ChoiceField::Choice::isGroup() const {
	return !choices.empty();
}
bool ChoiceField::Choice::hasDefault() const {
	return !isGroup() || !default_name.empty();
}


int ChoiceField::Choice::initIds() {
	int id = first_id + (hasDefault() ? 1 : 0);
	FOR_EACH(c, choices) {
		c->first_id = id;
		id = c->initIds();
	}
	return id;
}
int ChoiceField::Choice::choiceCount() const {
	return lastId() - first_id;
}
int ChoiceField::Choice::lastId() const {
	if (isGroup()) {
		// last id of last choice
		return choices.back()->lastId();
	} else {
		return first_id + 1;
	}
}


int ChoiceField::Choice::choiceId(const String& search_name) const {
	if (hasDefault() && search_name == name) {
		return first_id;
	} else if (name.empty()) { // no name for this group, forward to all children
		FOR_EACH_CONST(c, choices) {
			int sub_id = c->choiceId(search_name);
			if (sub_id != -1) return sub_id;
		}
	} else if (isGroup() && starts_with(search_name, name + _(" "))) {
		String sub_name = search_name.substr(name.size() + 1);
		FOR_EACH_CONST(c, choices) {
			int sub_id = c->choiceId(sub_name);
			if (sub_id != -1) return sub_id;
		}
	}
	return -1;
}

String ChoiceField::Choice::choiceName(int id) const {
	if (hasDefault() && id == first_id) {
		return name;
	} else {
		FOR_EACH_CONST_REVERSE(c, choices) { // take the last one that still contains id
			if (id >= c->first_id) {
				if (name.empty()) {
					return c->choiceName(id);
				} else {
					return name + _(" ") + c->choiceName(id);
				}
			}
		}
	}
	return _("");
}

String ChoiceField::Choice::choiceNameNice(int id) const {
	if (!isGroup() && id == first_id) {
		return name;
	} else if (hasDefault() && id == first_id) {
		return default_name;
	} else {
		FOR_EACH_CONST_REVERSE(c, choices) {
			if (id == c->first_id) {
				return c->name; // we don't want "<group> default"
			} else if (id > c->first_id) {
				return c->choiceNameNice(id);
			}
		}
	}
	return _("");
}


IMPLEMENT_REFLECTION_NO_GET_MEMBER(ChoiceField::Choice) {
	if (isGroup() || (tag.reading() && tag.isComplex())) {
		// complex values are groups
		REFLECT(name);
		REFLECT_N("group_choice", default_name);
		REFLECT(choices);
	} else {
		REFLECT_NAMELESS(name);
	}
}

template <> void GetDefaultMember::handle(const ChoiceField::Choice& c) {
	if (!c.isGroup()) handle(c.name);
}

template <> void GetMember::handle(const ChoiceField::Choice& c) {
	handle(_("name"),         c.name);
	handle(_("group choice"), c.default_name);
	handle(_("choices"),      c.choices);
}

// ----------------------------------------------------------------------------- : ChoiceStyle

ChoiceStyle::ChoiceStyle(const ChoiceFieldP& field)
	: Style(field)
	, popup_style(POPUP_DROPDOWN)
	, render_style(RENDER_TEXT)
	, combine(COMBINE_NORMAL)
	, alignment(ALIGN_STRETCH)
	, colors_card_list(false)
{}

// TODO
/*
void ChoiceStyle::invalidate() {
	// rebuild choice images
}
}
*/
bool ChoiceStyle::update(Context& ctx) {
	// Don't update the choice images, leave that to invalidate()
	return Style::update(ctx);
}
void ChoiceStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style::initDependencies(ctx, dep);
	FOR_EACH_CONST(ci, choice_images) {
		ci.second.initDependencies(ctx, dep);
	}
}

IMPLEMENT_REFLECTION_ENUM(ChoicePopupStyle) {
	VALUE_N("dropdown",	POPUP_DROPDOWN);
	VALUE_N("menu",		POPUP_MENU);
	VALUE_N("in place",	POPUP_DROPDOWN_IN_PLACE);
}

IMPLEMENT_REFLECTION_ENUM(ChoiceRenderStyle) {
	VALUE_N("text",			RENDER_TEXT);
	VALUE_N("image",		RENDER_IMAGE);
	VALUE_N("both",			RENDER_BOTH);
	VALUE_N("hidden",		RENDER_HIDDEN);
	VALUE_N("image hidden",	RENDER_HIDDEN_IMAGE);
}

IMPLEMENT_REFLECTION(ChoiceStyle) {
	tag.addAlias(300, _("card list colors"), _("colors card list"));
	REFLECT_BASE(Style);
	REFLECT(popup_style);
	REFLECT(render_style);
	REFLECT_N("maks",mask_filename);
	REFLECT(combine);
	REFLECT(alignment);
	REFLECT(colors_card_list);
//	REFLECT(font);
	REFLECT(choice_images);
//	if (tag.reading() && choice_colors.empty())
	REFLECT(choice_colors);
}

// ----------------------------------------------------------------------------- : ChoiceValue

String ChoiceValue::toString() const {
	return value();
}
bool ChoiceValue::update(Context& ctx) {
	Value::update(ctx);
	return field().default_script.invokeOnDefault(ctx, value)
	     | field().        script.invokeOn(ctx, value);
}

IMPLEMENT_REFLECTION_NAMELESS(ChoiceValue) {
	REFLECT_NAMELESS(value);
}
