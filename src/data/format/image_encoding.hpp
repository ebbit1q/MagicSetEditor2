//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>
#include <boost/json.hpp>
#include <wx/filename.h>
#include <fstream>

// ----------------------------------------------------------------------------- : Crop Rect Encoding

/// Encode a rect in a std::string
inline static std::string encodeRectInStdString(RealRect rect, int degrees) {
  return "<mse-crop-data>" + std::to_string((int)std::ceil (rect.x)) +
         ";"               + std::to_string((int)std::ceil (rect.y)) +
         ";"               + std::to_string((int)std::floor(rect.width)) +
         ";"               + std::to_string((int)std::floor(rect.height)) +
         ";"               + std::to_string(degrees) +
         "</mse-crop-data>";
}

/// Encode a rect in a wxString
inline static String encodeRectInWxString(RealRect rect, int degrees) {
  return _("<mse-crop-data>") + wxString::Format(wxT("%i"), (int)std::ceil (rect.x)) +
         _(";")               + wxString::Format(wxT("%i"), (int)std::ceil (rect.y)) +
         _(";")               + wxString::Format(wxT("%i"), (int)std::floor(rect.width)) +
         _(";")               + wxString::Format(wxT("%i"), (int)std::floor(rect.height)) +
         _(";")               + wxString::Format(wxT("%i"), degrees) +
         _("</mse-crop-data>");
}

/// Retreive a rect encoded in a string, return true if successful
inline static bool decodeRectFromString(const String& rectString, RealRect& rect_out, int& degrees_out) {
  size_t start = rectString.find(_("<mse-crop-data>"));
  if (start == String::npos) return false;
  size_t end = rectString.find(_("</mse-crop-data>"), start + 15);
  if (end == String::npos) return false;
  String string = rectString.substr(start + 15, end - (start + 15));
  if (string.empty()) return false;

  size_t divider = string.find(_(";"));
  if (divider == String::npos) return false;
  if (divider == 0) return false;
  int x;
  if(!string.substr(0, divider).ToInt(&x)) return false;
  string = string.substr(divider + 1);

  divider = string.find(_(";"));
  if (divider == String::npos) return false;
  if (divider == 0) return false;
  int y;
  if(!string.substr(0, divider).ToInt(&y)) return false;
  string = string.substr(divider + 1);

  divider = string.find(_(";"));
  if (divider == String::npos) return false;
  if (divider == 0) return false;
  int width;
  if(!string.substr(0, divider).ToInt(&width)) return false;
  string = string.substr(divider + 1);

  divider = string.find(_(";"));
  if (divider == String::npos) return false;
  if (divider == 0) return false;
  int height;
  if(!string.substr(0, divider).ToInt(&height)) return false;
  string = string.substr(divider + 1);

  if(!string.ToInt(&degrees_out)) return false;

  rect_out = RealRect(x, y, width, height);
  return true;
}

/// Retreive a rect encoded in a string, apply a transformation, then encode it back
inline static String transformEncodedRect(const String& rectString, RectTransform transform, double param_x, double param_y, int mode) {
  RealRect rect(0,0,0,0);
  int degrees;
  if (!decodeRectFromString(rectString, rect, degrees)) return _("");
  transform(rect, degrees, param_x, param_y, mode);
  return encodeRectInWxString(rect, degrees);
}

/// Retreive all rects encoded in a string, apply a transformation, then encode them back
inline static String transformAllEncodedRects(const String& rectString, RectTransform transform, double param_x, double param_y, int mode = 0) {
  RealRect rect(0,0,0,0);
  int degrees;
  size_t start = rectString.find(_("<mse-crop-data>"));
  if (start == String::npos) return rectString;
  size_t end = 0;
  String result;
  while (start != String::npos) {
    result = result + rectString.substr(end, start - end);
    end = rectString.find(_("</mse-crop-data>"), start + 15);
    if (end == String::npos) return rectString;
    end += 16;
    result = result + transformEncodedRect(rectString.substr(start, end - start), transform, param_x, param_y, mode);
    start = rectString.find(_("<mse-crop-data>"), end);
  }
  result = result + rectString.substr(end);
  return result;
}

// ----------------------------------------------------------------------------- : File to UTF8 Encoding

/// Encode a file in a string
inline static std::string fileToUTF8(const std::string& filepath) {
  // File to char
  std::ifstream file(filepath, std::ios::binary);
  file.unsetf(std::ios::skipws);
  std::vector<unsigned char> buffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  int size = buffer.size();
  if (size < 2) {
    queue_message(MESSAGE_WARNING, _("File too small to encode"));
    return "";
  }
  // All bytes that have a highest bit of 0 are valid UTF8 characters, so:
  // Reset the highest bit of each byte, store these bits in additional bytes at the end
  const unsigned char highest_bit = 1 << 7;
  unsigned char added_byte = 0;
  for (int i = 0, b = 0 ; i < size ; ++i, ++b) {
    if (b == 7) {                         // Never set the highest bit of the added byte
      buffer.push_back(added_byte);
      b = 0;
    }
    unsigned char bit = 1 << b;
    if ((buffer[i] & highest_bit) != 0) { // The highest bit of the buffer is set
      buffer[i] &= ~highest_bit;          // Reset the highest bit of the buffer
      added_byte |= bit;                  // Set the bit of the added byte
    } else {
      added_byte &= ~bit;                 // Reset the bit of the added byte
    }
  }
  buffer.push_back(added_byte);
  // Char to string
  return std::string(buffer.begin(), buffer.end());
}

/// Retreive a file encoded in a string, return true if successful
inline static bool UTF8ToFile(const std::string& filepath, std::string& string) {
  // String to char
  std::vector<unsigned char> buffer(string.begin(), string.end());
  int size = buffer.size();
  if (size < 2) {
    queue_message(MESSAGE_WARNING, _("File too small to decode"));
    return false;
  }
  // Restore the highest bit of each byte
  size = (size * 7) / 8;
  const unsigned char highest_bit = 1 << 7;
  unsigned char added_byte = buffer[size];
  for (int i = 0, j = size, b = 0 ; i < size ; ++i, ++b) {
    if (b == 7) {
      ++j;
      added_byte = buffer[j];
      b = 0;
    }
    unsigned char bit = 1 << b;
    if ((added_byte & bit) != 0) { // The bit of the added byte is set
      buffer[i] |= highest_bit;    // Set the highest bit of the buffer
    }
  }
  buffer.resize(size);
  // Char to file
  std::ofstream file(filepath, std::ios::out|std::ios::binary);
  std::copy(buffer.cbegin(), buffer.cend(), std::ostream_iterator<unsigned char>(file));
  return true;
}

/// Encode an image in a string
inline static std::string encodeImageInString(const Image& img) {
  String temppath = wxFileName::CreateTempFileName(_("mse")) + _(".png");
  img.SaveFile(temppath);
  std::string s = "<mse-image-data>" + fileToUTF8(temppath.ToStdString()) + "</mse-image-data>";
  wxRemoveFile(temppath);
  wxRemoveFile(temppath.substr(0, temppath.size() - 4));
  return s;
}

/// Retreive an image encoded in a string
inline static Image decodeImageFromString(const String& string) {
  Image img;
  size_t first = string.find(_("<mse-image-data>"));
  if (first == String::npos) return img;
  size_t last = string.find(_("</mse-image-data>"), first + 16);
  if (last == String::npos) return img;
  std::string s = string.substr(first + 16, last - (first + 16)).ToStdString();
  if (s.empty()) return img;

  const std::string& temppath = (wxFileName::CreateTempFileName(_("mse")) + _(".png")).ToStdString();
  UTF8ToFile(temppath, s);
  img.LoadFile(temppath, wxBITMAP_TYPE_PNG);
  wxRemoveFile(temppath);
  wxRemoveFile(temppath.substr(0, temppath.size() - 4));
  return img;
}

// ----------------------------------------------------------------------------- : Metadata manipulation

inline static String metadata_merge(const Image& img1, const Image& img2, int offset_x1 = 0, int offset_y1 = 0, int offset_x2 = 0, int offset_y2 = 0)
{
  if (img1.HasOption(wxIMAGE_OPTION_PNG_DESCRIPTION)) {
    String metadata1 = img1.GetOption(wxIMAGE_OPTION_PNG_DESCRIPTION);
    if (offset_x1 != 0 || offset_y1 != 0) metadata1 = transformAllEncodedRects(metadata1, RealRect::translate, offset_x1, offset_y1);
    if (img2.HasOption(wxIMAGE_OPTION_PNG_DESCRIPTION)) {
      String metadata2 = img2.GetOption(wxIMAGE_OPTION_PNG_DESCRIPTION);
      if (offset_x2 != 0 || offset_y2 != 0) metadata2 = transformAllEncodedRects(metadata2, RealRect::translate, offset_x2, offset_y2);
      size_t end1 = metadata1.find(_("</mse-card-data>"));
      size_t start2 = metadata2.find(_("<mse-card-data>"));
      if (end1 != String::npos && start2 != String::npos && end1 > 0 && start2 + 16 < metadata2.size()) {
        metadata1 = metadata1.substr(0, end1 - 1) + "," + metadata2.substr(start2 + 16);
      }
    }
    return metadata1;
  }
  else if (img2.HasOption(wxIMAGE_OPTION_PNG_DESCRIPTION)) {
    String metadata2 = img2.GetOption(wxIMAGE_OPTION_PNG_DESCRIPTION);
    if (offset_x2 != 0 || offset_y2 != 0) metadata2 = transformAllEncodedRects(metadata2, RealRect::translate, offset_x2, offset_y2);
    return metadata2;
  }
  return _("");
}

inline static boost::json::array metadata_to_json(const String& metadata) {
  size_t start = metadata.find(_("<mse-card-data>"));
  if (start == String::npos) return boost::json::array();
  size_t end = metadata.find(_("</mse-card-data>"), start + 15);
  if (end == String::npos) return boost::json::array();
  String string = metadata.substr(start + 15, end - (start + 15));
  try {
    boost::system::error_code ec;
    boost::json::parse_options options;
    options.allow_invalid_utf8 = true;
    boost::json::value jv = boost::json::parse(string.ToStdString(), ec, {}, options);
    if(ec || !jv.is_array()) {
      queue_message(MESSAGE_ERROR, _ERROR_("json cant parse"));
      return boost::json::array();
    }
    return jv.as_array();
  }
  catch (...) {
    queue_message(MESSAGE_ERROR, _ERROR_("json cant parse"));
    return boost::json::array();
  }
}
