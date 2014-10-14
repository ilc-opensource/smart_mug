#ifndef CONFIG_H
#define CONFIG_H

#define DEF_INT(e, key, t, val, desc) static const char * e = key;
#define DEF_STR(e, key, t, val, desc) static const char * e = key;
#define DEF_DB(e, key, t, val, desc)  static const char * e = key;
#include <def.h>
#undef DEF_INT
#undef DEF_STR
#undef DEF_DB

#endif
