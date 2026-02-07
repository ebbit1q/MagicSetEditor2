//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>
#include <script/scriptable.hpp>
#include <gfx/color.hpp>

DECLARE_POINTER_TYPE(FontRef);

// ----------------------------------------------------------------------------- : Font

enum FontRefFlags
{  FONT_NORMAL      = 0
,  FONT_BOLD        = 0x01
,  FONT_ITALIC      = 0x02
,  FONT_SOFT        = 0x04
,  FONT_CODE        = 0x08
,  FONT_CODE_KW     = 0x10 // syntax highlighting
,  FONT_CODE_STRING = 0x20 // syntax highlighting
,  FONT_CODE_NUMBER = 0x40 // syntax highlighting
,  FONT_CODE_OPER   = 0x80 // syntax highlighting
,  FONT_FROM_TAG    = 0x100 // this font is defined by a markup tag
};

/// A reference to a font for rendering text
/** Contains additional information about scaling, color and shadow */
class FontRef : public IntrusivePtrBase<FontRef> {
public:
  Scriptable<String> name;                 ///< Name of the referenced font
  Scriptable<String> italic_name;          ///< Font name for italic text (optional)
  Scriptable<double> size;                 ///< Size of the font
  Scriptable<String> weight, style;        ///< Weight and style of the font (bold/italic)
  Scriptable<bool>   underline;            ///< Underlined?
  Scriptable<bool>   strikethrough;        ///< Struck through?
  double             scale_down_to;        ///< Smallest size to scale down to
  double             max_stretch;          ///< How much should the font be stretched before scaling down?
  Scriptable<Color>  color;                ///< Color to use
  Scriptable<Color>  shadow_color;         ///< Color for the shadow
  Scriptable<double> shadow_displacement_x;///< Offset of the shadow in pixels, for a font size of 15
  Scriptable<double> shadow_displacement_y;///< Offset of the shadow in pixels, for a font size of 15
  Scriptable<double> shadow_blur;          ///< Blur radius of the shadow in pixels, for a font size of 15
  Scriptable<Color>  stroke_color;         ///< Color for the stroke
  Scriptable<double> stroke_radius;        ///< Thickness of the stroke in pixels, for a font size of 15
  Scriptable<double> stroke_blur;          ///< Blur radius of the stroke in pixels, for a font size of 15
  Color              separator_color;      ///< Color for <sep> text
  int                flags;                ///< FontFlags for this font

  FontRef();
 
  /// Load fonts (.ttf or .otf) from all directories in the app directory that contain "fonts" in their names,
  /// and optionaly their subdirectories, returns true if there were errors
  static bool PreloadResourceFonts(bool recursive);
  /// Adds font file paths from the given directory into fontFilePaths
  static void TallyResourceFonts(String fontsDirectoryPath, vector<String>& fontFilePaths, bool recursive);

  /// Update the scritables, returns true if there is a change
  bool update(Context& ctx);
  /// Add the given dependency to the dependent_scripts list for the variables this font depends on
  void initDependencies(Context&, const Dependency&) const;

  /// Add a shadow under the text?
  inline bool hasShadow() const {
    return (!almost_equal(shadow_blur(), 0.0) || !almost_equal(shadow_displacement_x(), 0.0) || !almost_equal(shadow_displacement_y(), 0.0)) && shadow_color().Alpha() != 0;
  }
  /// Add a stroke effect around the text?
  inline bool hasStroke() const {
    return (!almost_equal(stroke_blur(), 0.0) || !almost_equal(stroke_radius(), 0.0)) && stroke_color().Alpha() != 0;
  }

  /// Add style, and optionally change the font family, color and size
  FontRefP make(int add_flags, bool add_underline, bool add_strikethrough, String const* other_family, Color const* other_color, double const* other_size) const;
  
  /// Convert this font reference to a wxFont
  wxFont toWxFont(double scale) const;

private:
  DECLARE_REFLECTION();
};


