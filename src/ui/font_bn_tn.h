// -----------------------------------------------------------------------------------------
// This file defines the font data for 'u8g2_font_bn_tn'.
// It is designed for use with the U8g2 graphics library.
// The 'bn' in the name might suggest a "block" or "bold numeric" style,
// and 'tn' could indicate "tiny numbers" or a similar characteristic.
// The font data itself is a byte array representing the glyphs and their metrics.
//
// Font Details (from original header):
//   Fontname: -FontForge-Blocktopia-Medium-R-Normal--16-150-75-75-P-77-ISO10646-1
//   Copyright: memesbruh03
//   Glyphs: 14/196 (Indicates a small subset of glyphs, likely numbers and a few symbols)
//   BBX Build Mode: 0
// -----------------------------------------------------------------------------------------

/**
 * @brief Font data for u8g2_font_bn_tn (Blocktopia, likely for numbers).
 * This array contains the binary representation of the font glyphs and metadata
 * required by the U8g2 library to render text. The `U8G2_FONT_SECTION` macro
 * ensures that the font data is placed in a specific memory section, which can be
 * important on memory-constrained devices (e.g., placing it in PROGMEM on AVR).
 */
const uint8_t u8g2_font_bn_tn[177] U8G2_FONT_SECTION("u8g2_font_bn_tn") = 
  "\16\0\3\4\3\4\1\4\5\7\12\0\0\12\374\12\0\0\0\0\0\0\230+\13\307\211\65\70\311ip"
  "\22\0-\6\227\214\341\0.\6\223Ha\0\60\13\327\210S\242b/*%\0\61\11\327\210QT\70"
  "\237\34\62\14\327\210ar\70\205ep\320\1\63\15\327\210ar\70\305\342\340C\10\0\64\12\327\210\61"
  "b\313\203\340\34\65\16\327\210\361 \70hr\70\370\20\2\0\66\16\327\210\363\20\70h\242b\242R\2"
  "\0\67\7\327\210\341\70\70\17\327\210S\242b\62R\242b\242R\2\0\71\16\327\210S\242b\242b"
  "\70\370\20\2\0:\10\273Ja\16d\0\0\0\0";
