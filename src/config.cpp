#include <stdlib.h>
#include <mug.h>
#include <cJSON.h>
#include <string>

using namespace std;

#define DEFAULT_CONFIG_FILE "mug_config.json"

static cJSON *json = NULL;

int get_default_int(const char *key)
{
  string input(key);

#define DEF_INT(e, key, t, val, desc) \
  if(input == key)  \
    return val;
#define DEF_STR(...)
#define DEF_DB(...)

#include <def.h>

#undef DEF_INT
#undef DEF_STR
#undef DEF_DB

  MUG_ASSERT(false, "No default setting");
}

double get_default_double(const char *key)
{
  string input(key);

#define DEF_INT(...)
#define DEF_DB(e, key, t, val, desc) \
  if(input == key)  \
    return val;
#define DEF_STR(...)

#include <def.h>

#undef DEF_INT
#undef DEF_STR
#undef DEF_DB

  MUG_ASSERT(false, "No default setting");
}

const char* get_default_string(const char *key)
{
  string input(key);

#define DEF_STR(e, key, t, val, desc) \
  if(input == key)  \
    return val;
#define DEF_INT(...)
#define DEF_DB(...)

#include <def.h>

#undef DEF_INT
#undef DEF_STR
#undef DEF_DB

  MUG_ASSERT(false, "No default setting for %s\n", key);
}

void read_config_json()
{
  if(json != NULL) 
    return;

  char *dir = getenv(MUG_ENV);
  MUG_ASSERT(dir != NULL, "didn't set %s\n", MUG_ENV);

  std::string path(dir);
  path += "/";
  path += DEFAULT_CONFIG_FILE;

  FILE* fp = fopen(path.c_str(), "r");

  // If there is not config file, just return and keep json NULL
  if(fp == NULL) {
    json = NULL;
    return;
  }

  // check file length
  fseek(fp,0,SEEK_END);
  long len=ftell(fp);
  fseek(fp,0,SEEK_SET);

  char *data=(char*)malloc(len+1);
  fread(data,1,len,fp);
  fclose(fp);

  json = cJSON_Parse(data);
  MUG_ASSERT(json != NULL && json->type == cJSON_Object, "%s is NOT a valid mug config file", path.c_str());

  free(data);
}

cJSON* mug_query_config(const char *key)
{
  read_config_json();

  if(json == NULL)
    return NULL;

  cJSON *result = cJSON_GetObjectItem(json, key);
  return result;
}

int mug_query_config_int(const char *key)
{
  cJSON *result = mug_query_config(key);

  if(result == NULL) {
    return get_default_int(key);
  }

  MUG_ASSERT(result->type == cJSON_Number, "%s is not set integer\n", key);

  return result->valueint;
}

double mug_query_config_double(const char *key)
{
  cJSON *result = mug_query_config(key);

  if(result == NULL) {
    return get_default_double(key);
  }

  MUG_ASSERT(result->type == cJSON_Number, "%s is not set double\n", key);

  return result->valuedouble;
}

const char* mug_query_config_string(const char *key)
{
  cJSON *result = mug_query_config(key);

  if(result == NULL) {
    return get_default_string(key);
  }

  MUG_ASSERT(result->type == cJSON_String, "%s is not set string\n", key);

  return result->valuestring;
}

