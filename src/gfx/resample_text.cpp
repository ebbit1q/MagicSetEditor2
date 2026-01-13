//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>
#include <gui/util.hpp> // clearDC_black

void blur_image(const Image& img_in, Image& img_out);

// ----------------------------------------------------------------------------- : Resampled text

// scaling factor to use when drawing resampled text
const int text_scaling = 4;

// Draw text by first drawing it using a larger font and then downsampling it
// optionally rotated by an angle
void draw_resampled_text(DC& dc, const String& text, const RealPoint& pos, const RealRect& rect, Radians angle, Color color, int blur_radius, Color stroke_color, int stroke_radius, double stretch) {
  // transparent text can be ignored
  if (color.Alpha() == 0) return;
  // enlarge slightly; some fonts are larger then the GetTextExtent tells us (especially italic fonts)
  int w = static_cast<int>(rect.width) + 4, h = static_cast<int>(rect.height) + 2;
  // determine sub-pixel position
  int xi = static_cast<int>(rect.x),
      yi = static_cast<int>(rect.y);
  int xsub = static_cast<int>(text_scaling * (pos.x - xi)),
      ysub = static_cast<int>(text_scaling * (pos.y - yi));
  // draw text as mask (white text on black background)
  Bitmap buffer(w * text_scaling, h * text_scaling, 24);
  wxMemoryDC mdc;
  mdc.SelectObject(buffer);
  clearDC_black(mdc);
  mdc.SetFont(dc.GetFont());
  mdc.SetTextForeground(*wxWHITE);
  mdc.DrawRotatedText(text, xsub, ysub, rad_to_deg(angle));
  mdc.SelectObject(wxNullBitmap);
  // downsample
  double ca = fabs(cos(angle)), sa = fabs(sin(angle));
  w += int(w * (stretch - 1) * ca); // GCC makes annoying conversion warnings if *= is used here.
  h += int(h * (stretch - 1) * sa);
  Image img(w, h, false);
  downsample_to_alpha(buffer, img);
  // if there is no stroke effect, just add blur and draw
  if (stroke_radius == 0) {
    if (color.Alpha() != 255) {
      set_alpha(img, color.Alpha() / 255.);
    }
    if (blur_radius > 0) {
      Image s_img(w + 2*blur_radius, h + 2*blur_radius, false);
      set_alpha(s_img, 0);
      s_img.Paste(img, blur_radius, blur_radius, wxIMAGE_ALPHA_BLEND_COMPOSE);
      for (int i = 0 ; i < blur_radius ; ++i) {
        blur_image_alpha(s_img, 3);
      }
      fill_image(s_img, color);
      dc.DrawBitmap(s_img, xi-blur_radius, yi-blur_radius);
    }
    else {
      fill_image(img, color);
      dc.DrawBitmap(img, xi, yi);
    }
  }
  // otherwise add stroke effect
  else {
    int radius = blur_radius + stroke_radius;
    Image s_img(w + 2*radius, h + 2*radius, false);
    set_alpha(s_img, 0);
    s_img.Paste(img, radius, radius, wxIMAGE_ALPHA_BLEND_COMPOSE);
    for (int i = 0 ; i < stroke_radius ; ++i) {
      thicken_image_alpha(s_img, 1);
    }
    for (int i = 0 ; i < blur_radius ; ++i) {
      blur_image_alpha(s_img, 3);
    }
    if (stroke_color.Alpha() != 255) {
      set_alpha(s_img, stroke_color.Alpha() / 255.);
    }
    fill_image(s_img, stroke_color);
    if (stroke_color != color) {
      fill_image(img, color);
      if (color.Alpha() != 255) {
        set_alpha(img, color.Alpha() / 255.);
      }
      s_img.Paste(img, radius, radius, wxIMAGE_ALPHA_BLEND_COMPOSE);
    }
    dc.DrawBitmap(s_img, xi-radius, yi-radius);
  }
}

