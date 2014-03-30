/* C++ code produced by gperf version 3.0.3 */
/* Command-line: gperf -e ' \\015' -Z ColorHash -L C++ -7 -C -G -E -l --ignore-case -k '*,1,$' -m 100 selection  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set"
#endif



/* maximum key range = 412, duplicates = 0 */

#ifndef GPERF_DOWNCASE
#define GPERF_DOWNCASE 1
static unsigned char gperf_downcase[256] =
  {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
  };
#endif

#ifndef GPERF_CASE_MEMCMP
#define GPERF_CASE_MEMCMP 1
static int
gperf_case_memcmp (register const char *s1, register const char *s2, register unsigned int n)
{
  for (; n > 0;)
    {
      unsigned char c1 = gperf_downcase[(unsigned char)*s1++];
      unsigned char c2 = gperf_downcase[(unsigned char)*s2++];
      if (c1 == c2)
        {
          n--;
          continue;
        }
      return (int)c1 - (int)c2;
    }
  return 0;
}
#endif

class ColorHash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  enum
  {
    TOTAL_KEYWORDS = 148,
    MIN_WORD_LENGTH = 3,
    MAX_WORD_LENGTH = 20,
    MIN_HASH_VALUE = 5,
    MAX_HASH_VALUE = 416
  };
  static const char *in_word_set (const char *str, unsigned int len);
  static const unsigned char lengthtable[];
	static const char * const ColorHash::wordlist[];
};

inline unsigned int
ColorHash::hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417, 417, 417, 417, 417, 417,
      417, 417, 417, 417, 417,   0,  32,  36,   0,   2,
       30,   9,  16,  21,   1,   0,   5,  25,   0,   3,
       62,  40,   0,   0,   5,  62, 133, 100,   6,   0,
        0, 417, 417, 417, 417, 417, 417,   0,  32,  36,
        0,   2,  30,   9,  16,  21,   1,   0,   5,  25,
        0,   3,  62,  40,   0,   0,   5,  62, 133, 100,
        6,   0,   0, 417, 417, 417, 417, 417, 417
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[19]];
      /*FALLTHROUGH*/
      case 19:
        hval += asso_values[(unsigned char)str[18]];
      /*FALLTHROUGH*/
      case 18:
        hval += asso_values[(unsigned char)str[17]];
      /*FALLTHROUGH*/
      case 17:
        hval += asso_values[(unsigned char)str[16]];
      /*FALLTHROUGH*/
      case 16:
        hval += asso_values[(unsigned char)str[15]];
      /*FALLTHROUGH*/
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      /*FALLTHROUGH*/
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      /*FALLTHROUGH*/
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      /*FALLTHROUGH*/
      case 11:
        hval += asso_values[(unsigned char)str[10]+1];
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

const unsigned char ColorHash::lengthtable[] =
  {
     0,  0,  0,  0,  0,  3,  0,  0,  3,  7,  0,  0,  0,  4,
     0,  4,  4,  8,  5,  8,  6,  4,  9,  8, 10,  9,  0,  0,
     0,  6,  9,  9,  9,  5, 13, 13, 13,  6,  8,  6,  4,  0,
     5, 10,  8, 11,  9,  6,  7,  5,  0,  0,  0,  9,  0, 12,
     0,  4,  7,  9,  6,  7,  7,  0,  7,  9,  0, 11,  0,  5,
     0,  5,  0,  8,  9,  9,  9,  9,  5, 10,  0,  0,  6,  0,
    11,  0, 10,  4, 11,  0,  0,  9,  7,  0,  0,  0,  0,  0,
    14,  0, 14,  9,  0, 11, 13,  4,  4,  4,  7,  8, 10,  0,
    13, 14,  7,  9,  0,  0,  9, 11,  0,  6,  9,  9,  9, 10,
     0,  9,  5,  8,  4,  8,  0, 10,  0,  0,  0,  4,  0,  0,
     5,  0, 11,  0,  0, 10, 13,  9,  9,  5,  8,  9,  0,  9,
     0,  0, 11,  8,  4,  0, 10,  0,  5,  6, 14,  0,  9,  6,
     0,  5,  0,  0,  7,  0,  9,  6,  0,  0,  0, 10,  0,  0,
     0,  0, 10,  0,  0, 10, 13, 14,  0, 14,  0,  0,  0,  0,
     0,  0,  0,  6,  0,  0,  0, 12,  9,  9, 11,  0,  0,  0,
     0,  0,  0,  0,  9,  0, 20,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 12,  0, 12,  0,  0,  0, 10,  0,  0,  0,  0,  0,
     0, 13,  0,  0,  0, 13,  0,  0, 17,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 16,  0, 10,  0,
    10, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15,  0,  0,
     0,  0,  0,  0, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 11,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0,
     0,  0,  0,  0,  0, 13,  0, 12,  0,  0,  0,  0,  0, 15,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15
  };

const char * const ColorHash::wordlist[] =
  {
    "", "", "", "", "",
    "RED",
    "", "",
    "TAN",
    "DARKRED",
    "", "", "",
    "GRAY",
    "",
    "GREY",
    "TEAL",
    "DARKGRAY",
    "GREEN",
    "DARKGREY",
    "ORANGE",
    "GOLD",
    "DARKGREEN",
    "SEAGREEN",
    "DARKORANGE",
    "ORANGERED",
    "", "", "",
    "SIENNA",
    "SLATEGRAY",
    "GOLDENROD",
    "SLATEGREY",
    "LINEN",
    "DARKSLATEGRAY",
    "DARKGOLDENROD",
    "DARKSLATEGREY",
    "MAROON",
    "SEASHELL",
    "SALMON",
    "CYAN",
    "",
    "KHAKI",
    "DARKSALMON",
    "DARKCYAN",
    "GREENYELLOW",
    "DARKKHAKI",
    "TOMATO",
    "MAGENTA",
    "CORAL",
    "", "", "",
    "INDIANRED",
    "",
    "DARKSEAGREEN",
    "",
    "LIME",
    "OLDLACE",
    "LEAFGREEN",
    "INDIGO",
    "THISTLE",
    "DIMGRAY",
    "",
    "DIMGREY",
    "MISTYROSE",
    "",
    "FORESTGREEN",
    "",
    "AZURE",
    "",
    "BEIGE",
    "",
    "CORNSILK",
    "LIGHTGRAY",
    "LIMEGREEN",
    "LIGHTGREY",
    "GAINSBORO",
    "BLACK",
    "LIGHTGREEN",
    "", "",
    "ORCHID",
    "",
    "DARKMAGENTA",
    "",
    "DARKORCHID",
    "PINK",
    "LIGHTYELLOW",
    "", "",
    "PALEGREEN",
    "CRIMSON",
    "", "", "", "", "",
    "LIGHTSLATEGRAY",
    "",
    "LIGHTSLATEGREY",
    "LIGHTCYAN",
    "",
    "LIGHTSALMON",
    "PALEGOLDENROD",
    "BLUE",
    "AQUA",
    "SNOW",
    "SKYBLUE",
    "DARKBLUE",
    "LIGHTCORAL",
    "",
    "LIGHTSEAGREEN",
    "BLANCHEDALMOND",
    "HOTPINK",
    "CHOCOLATE",
    "", "",
    "ROYALBLUE",
    "SPRINGGREEN",
    "",
    "YELLOW",
    "SLATEBLUE",
    "MINTCREAM",
    "STEELBLUE",
    "DODGERBLUE",
    "",
    "LAWNGREEN",
    "WHEAT",
    "MOCCASIN",
    "PERU",
    "HONEYDEW",
    "",
    "CHARTREUSE",
    "", "", "",
    "NAVY",
    "", "",
    "BROWN",
    "",
    "YELLOWGREEN",
    "", "",
    "SANDYBROWN",
    "DARKSLATEBLUE",
    "ROSYBROWN",
    "LIGHTPINK",
    "WHITE",
    "LAVENDER",
    "FIREBRICK",
    "",
    "CADETBLUE",
    "", "",
    "SADDLEBROWN",
    "DEEPPINK",
    "PLUM",
    "",
    "AQUAMARINE",
    "",
    "IVORY",
    "BISQUE",
    "MEDIUMSEAGREEN",
    "",
    "LIGHTBLUE",
    "SILVER",
    "",
    "OLIVE",
    "", "",
    "FUCHSIA",
    "",
    "ALICEBLUE",
    "VIOLET",
    "", "", "",
    "DARKVIOLET",
    "", "", "", "",
    "WHITESMOKE",
    "", "",
    "GHOSTWHITE",
    "DARKTURQUOISE",
    "LIGHTSTEELBLUE",
    "",
    "DARKOLIVEGREEN",
    "", "", "", "", "", "", "",
    "PURPLE",
    "", "", "",
    "MEDIUMORCHID",
    "TURQUOISE",
    "OLIVEDRAB",
    "DEEPSKYBLUE",
    "", "", "", "", "", "", "",
    "BURLYWOOD",
    "",
    "LIGHTGOLDENRODYELLOW",
    "", "", "", "", "", "", "", "", "",
    "FLORALWHITE",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "",
    "LIGHTSKYBLUE",
    "",
    "LEMONCHIFFON",
    "", "", "",
    "MEDIUMBLUE",
    "", "", "", "", "", "",
    "PALEVIOLETRED",
    "", "", "",
    "PALETURQUOISE",
    "", "",
    "MEDIUMSPRINGGREEN",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "",
    "MEDIUMAQUAMARINE",
    "",
    "POWDERBLUE",
    "",
    "BLUEVIOLET",
    "MIDNIGHTBLUE",
    "", "", "", "", "", "", "", "", "",
    "MEDIUMSLATEBLUE",
    "", "", "", "", "", "",
    "CORNFLOWERBLUE",
    "", "", "", "", "", "", "", "", "",
    "",
    "PEACHPUFF",
    "", "", "", "", "", "", "", "", "",
    "",
    "NAVAJOWHITE",
    "", "", "", "", "", "", "", "", "",
    "", "", "",
    "PAPAYAWHIP",
    "", "", "", "", "", "", "",
    "LAVENDERBLUSH",
    "",
    "ANTIQUEWHITE",
    "", "", "", "", "",
    "MEDIUMVIOLETRED",
    "", "", "", "", "", "", "", "", "",
    "",
    "MEDIUMPURPLE",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "",
    "MEDIUMTURQUOISE"
  };

const char *
ColorHash::in_word_set (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key];

            if ((((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !gperf_case_memcmp (str, s, len))
              return s;
          }
    }
  return 0;
}
