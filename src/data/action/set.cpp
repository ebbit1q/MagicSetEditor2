//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/action/set.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/pack.hpp>
#include <data/stylesheet.hpp>
#include <util/error.hpp>
#include <util/uid.hpp>

// ----------------------------------------------------------------------------- : Add card

AddCardAction::AddCardAction(Set& set)
  : CardListAction(set)
  , action(ADD, make_intrusive<Card>(*set.game), set.cards)
{}

AddCardAction::AddCardAction(AddingOrRemoving ar, Set& set, const vector<CardP>& cards)
  : CardListAction(set)
  , action(ar, cards, set.cards)
{
  // If we are adding cards, resolve any uid conflicts
  // We always assume uid conflicts occur because a card was copy-pasted into the same set,
  // and never because two different cards randomly got assigned the same uid
  if (!action.adding) return;
  // Tally existing unique ids
  unordered_map<String, CardP> all_existing_cards;
  unordered_set<String> all_existing_uids;
  FOR_EACH(card, set.cards) {
    all_existing_cards.insert({ card->uid, card });
    all_existing_uids.insert(card->uid);
  }
  // Tally added unique ids
  unordered_map<String, CardP> all_added_uids;
  for (size_t pos = 0; pos < action.steps.size(); ++pos) {
    CardP card = action.steps[pos].item;
    all_added_uids.insert({ card->uid, card });
  }
  // Cycle on the added cards
  unordered_map<String, CardP> all_added_uids_copy(all_added_uids);
  FOR_EACH(added_pair, all_added_uids_copy) {
    String old_uid = added_pair.first;
    CardP added_card = added_pair.second;
    // Assign new unique id if necessary
    if (all_existing_cards.find(old_uid) == all_existing_cards.end()) continue;
    String new_uid = generate_uid();
    added_card->uid = new_uid;
    all_added_uids.insert({ new_uid, added_card });
    // Update links on linked cards
    LINK_PAIRS(linked_pairs, added_card);
    FOR_EACH(linked_pair, linked_pairs) {
      String& linked_uid = linked_pair.first.get();
      String& linked_relation = linked_pair.second.get();
      if (linked_uid.empty()) continue;
      // If it's an added card, replace the link
      if (all_added_uids.find(linked_uid) != all_added_uids.end()) {
        all_added_uids.at(linked_uid)->updateLinkedUID(old_uid, new_uid);
      }
      // Otherwise, if it's an existing card, create an action to copy the link
      else if (all_existing_cards.find(linked_uid) != all_existing_cards.end()) {
        CardP linked_card = all_existing_cards.at(linked_uid);
        int linked_index = linked_card->findFreeLink(new_uid, all_existing_uids);
        if (linked_index < 0) {
          queue_message(MESSAGE_WARNING, _ERROR_1_("not enough free links", linked_card->identification()));
        }
        else {
          String relation(linked_card->getLinkedRelation(linked_index));
          card_link_actions.push_back(make_intrusive<OneWayLinkCardsAction>(set, linked_card, new_uid, relation, linked_index));
        }
      }
    }
  }
}

String AddCardAction::getName(bool to_undo) const {
  return action.getName();
}

void AddCardAction::perform(bool to_undo) {
  // Add or remove cards
  action.perform(set.cards, to_undo);
  // Update card links
  for (int i = 0; i < card_link_actions.size(); ++i) {
    card_link_actions[i]->perform(to_undo);
  }
}

// ----------------------------------------------------------------------------- : Reorder cards

ReorderCardsAction::ReorderCardsAction(Set& set, size_t card_id1, size_t card_id2)
  : CardListAction(set), card_id1(card_id1), card_id2(card_id2)
{}

String ReorderCardsAction::getName(bool to_undo) const {
  return _("Reorder cards");
}

void ReorderCardsAction::perform(bool to_undo) {
  #ifdef _DEBUG
    assert(card_id1 < set.cards.size());
    assert(card_id2 < set.cards.size());
  #endif
  if (card_id1 >= set.cards.size() || card_id2 >= set.cards.size()) {
    // TODO : Too lazy to fix this right now.
    return;
  }
  swap(set.cards[card_id1], set.cards[card_id2]);
}

// ----------------------------------------------------------------------------- : Link cards

OneWayLinkCardsAction::OneWayLinkCardsAction(Set& set, CardP& card, const String& uid, const String& relation, int index)
  : CardListAction(set), card(card), uid(uid), relation(relation)
{
  switch (index) {
    case 1: {
      linked_uid      = &card->linked_card_1;
      linked_relation = &card->linked_relation_1;
      return;
    }
    case 2: {
      linked_uid      = &card->linked_card_2;
      linked_relation = &card->linked_relation_2;
      return;
    }
    case 3: {
      linked_uid      = &card->linked_card_3;
      linked_relation = &card->linked_relation_3;
      return;
    }
    default: {
      linked_uid      = &card->linked_card_4;
      linked_relation = &card->linked_relation_4;
      return;
    }
  }
}

String OneWayLinkCardsAction::getName(bool to_undo) const {
  return _("Change link");
}

void OneWayLinkCardsAction::perform(bool to_undo) {
  swap(*linked_uid,      uid);
  swap(*linked_relation, relation);
}

// ----------------------------------------------------------------------------- : Change stylesheet

String DisplayChangeAction::getName(bool to_undo) const {
  assert(false);
  return _("");
}
void DisplayChangeAction::perform(bool to_undo) {
  assert(false);
}


ChangeCardStyleAction::ChangeCardStyleAction(const CardP& card, const StyleSheetP& stylesheet)
  : card(card), stylesheet(stylesheet), has_styling(false) // styling_data(empty)
{}
String ChangeCardStyleAction::getName(bool to_undo) const {
  return _("Change style");
}
void ChangeCardStyleAction::perform(bool to_undo) {
  swap(card->stylesheet,   stylesheet);
  swap(card->has_styling,  has_styling);
  swap(card->styling_data, styling_data);
}


ChangeSetStyleAction::ChangeSetStyleAction(Set& set, const CardP& card)
  : set(set), card(card)
{}
String ChangeSetStyleAction::getName(bool to_undo) const {
  return _("Change style (all cards)");
}
void ChangeSetStyleAction::perform(bool to_undo) {
  if (!to_undo) {
    // backup has_styling
    has_styling.clear();
    FOR_EACH(card, set.cards) {
      has_styling.push_back(card->has_styling);
      if (!card->stylesheet) {
        card->has_styling = false; // this card has custom style options for the default stylesheet
      }
    }
    stylesheet       = set.stylesheet;
    set.stylesheet   = card->stylesheet;
    card->stylesheet = StyleSheetP();
  } else {
    card->stylesheet = set.stylesheet;
    set.stylesheet   = stylesheet;
    // restore has_styling
    FOR_EACH_2(card, set.cards, has, has_styling) {
      card->has_styling = has;
    }
  }
}

ChangeCardHasStylingAction::ChangeCardHasStylingAction(Set& set, const CardP& card)
  : set(set), card(card)
{
  if (!card->has_styling) {
    // copy of the set's styling data
    styling_data.cloneFrom( set.stylingDataFor(set.stylesheetFor(card)) );
  } else {
    // the new styling data is empty
  }
}
String ChangeCardHasStylingAction::getName(bool to_undo) const {
  return _("Use custom style");
}
void ChangeCardHasStylingAction::perform(bool to_undo) {
  card->has_styling = !card->has_styling;
  swap(card->styling_data, styling_data);
}

// ----------------------------------------------------------------------------- : Change notes

ChangeCardNotesAction::ChangeCardNotesAction(const CardP& card, const String& notes)
  : card(card), notes(notes)
{}
String ChangeCardNotesAction::getName(bool to_undo) const {
  return _("Change notes");
}
void ChangeCardNotesAction::perform(bool to_undo) {
  swap(card->notes, notes);
}

// ----------------------------------------------------------------------------- : Change uid

ChangeCardUIDAction::ChangeCardUIDAction(Set& set, const CardP& card, const String& uid)
  : CardListAction(set), card(card), uid(uid)
{}
String ChangeCardUIDAction::getName(bool to_undo) const {
  return _("Change ID");
}
void ChangeCardUIDAction::perform(bool to_undo) {
  FOR_EACH(c, set.cards) {
    c->updateLinkedUID(card->uid, uid);
  }
  swap(card->uid, uid);
}

// ----------------------------------------------------------------------------- : Pack types

AddPackAction::AddPackAction(AddingOrRemoving ar, Set& set, const PackTypeP& pack)
  : PackTypesAction(set)
  , action(ar, pack, set.pack_types)
{}

String AddPackAction::getName(bool to_undo) const {
  return action.getName();
}

void AddPackAction::perform(bool to_undo) {
  action.perform(set.pack_types, to_undo);
}


ChangePackAction::ChangePackAction(Set& set, size_t pos, const PackTypeP& pack)
  : PackTypesAction(set)
  , pack(pack), pos(pos)
{}

String ChangePackAction::getName(bool to_undo) const {
  return _ACTION_1_("change",type_name(pack));
}

void ChangePackAction::perform(bool to_undo) {
  swap(set.pack_types.at(pos), pack);
}
