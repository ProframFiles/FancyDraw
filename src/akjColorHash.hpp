#pragma once

namespace akj{

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
	static const char * const wordlist[];
};

}
