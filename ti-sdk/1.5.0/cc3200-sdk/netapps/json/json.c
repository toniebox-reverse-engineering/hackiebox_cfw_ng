#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"


static int getDepth(jsmntok_t *t);
static int dump(const char *js, jsmntok_t *t, size_t count, int indent, jsonPrintCallbackFxn printCallback);
static void * get_object_by_type(jsonParser *jParser, jsonObj jObj, char *tagName, jsonObjType objType);
static void* get_array_object_by_type(jsonParser *jParser, jsonArray jArray, int jElement, jsonObjType objType);


jsonObj json_parser_init(jsonParser *jParser, char *jsonStream)
{
  jsmn_parser   *parser;

  if(!jParser || !jsonStream)
  {
    return JSON_INVALID_OBJECT;
  }

  jParser->jsonStream = jsonStream;
  jParser->noOfToken = -1;
  jParser->tokenList = NULL;

  parser = &jParser->parser;

  /* Initialize JSON PArser */
  jsmn_init(parser);

  /* Get number of JSON token in stream as we we dont know how many tokens need to pass */
  jParser->noOfToken = jsmn_parse(parser, (const char *)jParser->jsonStream, strlen((const char *)jParser->jsonStream), NULL, 10);

  /* Allocate memory to store token */
  jParser->tokenList = (jsmntok_t *) malloc(jParser->noOfToken*sizeof(jsmntok_t));
  if(jParser->tokenList == NULL)
  {
    return JSON_INVALID_OBJECT;
  }

  /* Initialize JSON Parser again */
  jsmn_init(parser);
  jParser->noOfToken = jsmn_parse(parser, (const char *)jParser->jsonStream, strlen((const char *)jParser->jsonStream), jParser->tokenList, jParser->noOfToken);

  return (jsonObj) 0;
}

int json_parser_deinit(jsonParser *jParser)
{
    jParser->jsonStream = NULL;
    jParser->noOfToken = -1;
    free(jParser->tokenList);
    jParser->tokenList = NULL;
    return 0;
}

void	json_parser_dump(jsonParser *jParser, jsonPrintCallbackFxn printCallback)
{
	dump(jParser->jsonStream, jParser->tokenList, jParser->noOfToken, 4, printCallback);
}

jsonObj json_get_object(jsonParser *jParser, jsonObj jObj, char *tagName)
{
  jsonObj		jObj1;

  jObj1 = (jsonObj)get_object_by_type(jParser, jObj, tagName, JSON_OBJECT);
  if( (jsonObjType)jObj == JSON_INVALID_OBJECT)
  {
	  return JSON_INVALID_OBJECT;
  }
  else
  {
	  return jObj1;
  }

}


char * json_object_get_string(jsonParser *jParser, jsonObj jObj, char *tagName, char *str)
{
	  char 		*cmpStr;
	  int   	cmpStrLen = 0;
	  jsonString	jString;

	  jString = (jsonString)get_object_by_type(jParser, jObj, tagName, JSON_STRING_OBJECT);
	  if(jString == JSON_INVALID_OBJECT)
	  {
		  return NULL;
	  }
	  else
	  {
		  cmpStr = jParser->jsonStream + jParser->tokenList[jString].start;
		  cmpStrLen = jParser->tokenList[jString].end - jParser->tokenList[jString].start;
		  strncpy(str, cmpStr, cmpStrLen);
		  str[cmpStrLen] = '\0';
		  return str;
	  }
}



int json_object_get_int(jsonParser *jParser, jsonObj jObj, char *tagName)
{
	  char		tmp[16]={0,};

	  if(!json_object_get_string(jParser, jObj, tagName, tmp))
	  {
		  return -1;
	  }

	  return atoi(tmp);
}


long long 	json_object_get_long_long(jsonParser *jParser, jsonObj jObj, char *tagName)
{
	  char		tmp[32]={0,};

	  if(!json_object_get_string(jParser, jObj, tagName, tmp))
	  {
		  return -1;
	  }

	  return atoll(tmp);
}

double 		json_object_get_double(jsonParser *jParser, jsonObj jObj, char *tagName)
{
	  char		tmp[32]={0,};

	  if(!json_object_get_string(jParser, jObj, tagName, tmp))
	  {
		  return -1.0;
	  }

	  return atof(tmp);
}

tBoolean json_object_get_boolean(jsonParser *jParser, jsonObj jObj, char *tagName)
{
	  char		tmp[8]={0,};

	  if(!json_object_get_string(jParser, jObj, tagName, tmp))
	  {
		  return false;
	  }
	  if(!strcmp(tmp, "true"))
	  {
		  return true;
	  }

	  if(!strcmp(tmp, "false"))
	  {
		  return false;
	  }
	  return false;
}



jsonArray	json_object_get_array(jsonParser *jParser, jsonObj jObj, char *tagName)
{
	  jsonArray	jArray;

	  jArray = (jsonArray)get_object_by_type(jParser, jObj, tagName, JSON_ARRAY_OBJECT);
	  if( (jsonObjType)jObj == JSON_INVALID_OBJECT)
	  {
		  return JSON_INVALID_ARRAY;
	  }
	  else
	  {
		  return jArray;
	  }
}

char * 		json_array_get_string(jsonParser *jParser, jsonArray jArray, int jElement, char *str)
{
	char	*cmpStr;
	char	cmpStrLen;
	jsonString	jString;

	jString = (jsonString)get_array_object_by_type(jParser, jArray, jElement, JSON_STRING_OBJECT);
	if(jString == JSON_INVALID_OBJECT)
	{
		return NULL;
	}
	else
	{
		if( (jParser->tokenList[jString].type == JSMN_STRING) ||
				(jParser->tokenList[jString].type == JSMN_PRIMITIVE) )
		{
			cmpStr = jParser->jsonStream + jParser->tokenList[jString].start;
			cmpStrLen = jParser->tokenList[jString].end - jParser->tokenList[jString].start;
			strncpy(str, cmpStr, cmpStrLen);
			str[cmpStrLen] = '\0';
			return str;
		}
	}

	return NULL;
}

int 		json_array_get_int(jsonParser *jParser, jsonArray jArray, int jElement)
{
	char		tmp[16]={0,};

	if(!json_array_get_string(jParser, jArray, jElement, tmp))
	{
		return -1;
	}
	return atoi(tmp);
}

long long 	json_array_get_long_long(jsonParser *jParser, jsonArray jArray, int jElement)
{
	  char		tmp[32]={0,};

	  if(!json_array_get_string(jParser, jArray, jElement, tmp))
	  {
		  return -1;
	  }

	  return atoll(tmp);
}

double 		json_array_get_double(jsonParser *jParser, jsonArray jArray, int jElement)
{
	  char		tmp[32]={0,};

	  if(!json_array_get_string(jParser, jArray, jElement, tmp))
	  {
		  return -1.0;
	  }

	  return atof(tmp);
}

tBoolean	json_array_get_boolean(jsonParser *jParser, jsonArray jArray, int jElement)
{
	char		tmp[8]={0,};

	if(!json_array_get_string(jParser, jArray, jElement, tmp))
	{
		return false;
	}
	if(!strcmp(tmp, "true"))
	{
		return true;
	}

	if(!strcmp(tmp, "false"))
	{
		return false;
	}
	return false;
}
jsonArray	json_array_get_array(jsonParser *jParser, jsonArray jArray, int jElement)
{
	jsonArray	jArray1;

	jArray1 = (jsonObj)get_array_object_by_type(jParser, jArray, jElement, JSON_OBJECT);
	if(jArray1 == JSON_INVALID_OBJECT)
	{
		return JSON_INVALID_ARRAY;
	}
	else
	{
		return (jsonArray) jArray1;
	}
}

jsonObj	json_array_get_object(jsonParser *jParser, jsonArray jArray, int jElement)
{
	jsonObj	jObj;

	jObj = (jsonObj)get_array_object_by_type(jParser, jArray, jElement, JSON_OBJECT);
	return (jsonObj) jObj;
}


static void* get_array_object_by_type(jsonParser *jParser, jsonArray jArray, int jElement, jsonObjType objType)
{
	int		i = 0;
	int   	j = 0;

	if(!jParser)
	{
		return (void*)JSON_INVALID_OBJECT;
	}

	if(jArray == JSON_INVALID_ARRAY)
	{
		return (void*)JSON_INVALID_OBJECT;
	}

	if(jParser->noOfToken <= 0 || jElement < 0)
	{
		return (void*)JSON_INVALID_OBJECT;
	}

	if(jParser->tokenList == NULL)
	{
		return (void*)JSON_INVALID_OBJECT;
	}

	for(i=0, j=jArray+1; (i < jElement) && (i < jParser->tokenList[jArray].size); i++)
	{
		j += getDepth(&jParser->tokenList[j]);
	}

	if(i < jParser->tokenList[jArray].size)
	{
		return (void *) j;
	}

	return (void*)JSON_INVALID_OBJECT;
}


static void* get_object_by_type(jsonParser *jParser, jsonObj jObj, char *tagName, jsonObjType objType)
{

	  int		i = 0;
	  int   	j = 0;
	  char 		*cmpStr;
	  int   	cmpStrLen = 0;
	  int 		depth = 0;

	  if(!jParser || !tagName)
	  {
	    return (void*)JSON_INVALID_OBJECT;
	  }

	  if(jObj == JSON_INVALID_OBJECT || jObj < 0)
	  {
		return (void*)JSON_INVALID_OBJECT;
	  }

	  if(jParser->noOfToken <= 0)
	  {
	    return (void*)JSON_INVALID_OBJECT;
	  }

	  if(jParser->tokenList == NULL)
	  {
	    return (void*)JSON_INVALID_OBJECT;
	  }

	  if(jParser->tokenList[jObj].type != JSMN_OBJECT)
	  {
		  return (void*)JSON_INVALID_OBJECT;
	  }

	  for(i=0, j=jObj+1; i < jParser->tokenList[jObj].size; i++)
	  {
		  if( (objType == JSON_ARRAY_OBJECT && jParser->tokenList[j+1].type == JSMN_ARRAY) ||
			  (objType == JSON_OBJECT && jParser->tokenList[j+1].type == JSMN_OBJECT) ||
		  	  (objType == JSON_STRING_OBJECT &&  jParser->tokenList[j].type == JSMN_STRING &&
					    (jParser->tokenList[j+1].type == JSMN_PRIMITIVE || jParser->tokenList[j+1].type == JSMN_STRING ))
			)

		  {
			  cmpStr = jParser->jsonStream + jParser->tokenList[j].start;
			  cmpStrLen = jParser->tokenList[j].end - jParser->tokenList[j].start;

			  if(cmpStrLen == strlen(tagName) && !strncmp((const char *) cmpStr, tagName, cmpStrLen))
			  {
				  return (void*) (j+1);
			  }
	  	  }

		  depth = getDepth(&jParser->tokenList[j+1]);
	  	  j += (depth + 1);

	  }
	  return (void*)JSON_INVALID_OBJECT;
}

static int getDepth(jsmntok_t *t)
{
	int    i;
	int    j;

    if (t->type == JSMN_PRIMITIVE)
    {
        return 1;
    }
    else if (t->type == JSMN_STRING)
    {
        return 1;
    }
    else if (t->type == JSMN_OBJECT)
    {
        j = 0;
        for (i = 0; i < t->size; i++)
        {
            j += getDepth(t+1+j);
            j += getDepth(t+1+j);
        }
        return j+1;
    }
    else if (t->type == JSMN_ARRAY)
    {
        j = 0;
        for (i = 0; i < t->size; i++)
        {
            j += getDepth(t+1+j);
        }
        return j+1;
    }
    return 0;
}

static int dump(const char *js, jsmntok_t *t, size_t count, int indent, jsonPrintCallbackFxn printCallback)
{
	int    i;
	int    j;
	int    k;

    if (count == 0)
    {
        return 0;
    }
    if (t->type == JSMN_PRIMITIVE)
    {
    	printCallback("%.*s", t->end - t->start, js+t->start, printCallback);
        return 1;
    }
    else if (t->type == JSMN_STRING)
    {
    	printCallback("'%.*s'", t->end - t->start, js+t->start, printCallback);
        return 1;
    }
    else if (t->type == JSMN_OBJECT)
    {
    	printCallback("\n\r");
        j = 0;
        for (i = 0; i < t->size; i++)
        {
            for (k = 0; k < indent; k++)
            {
            	printCallback("  ");
            }
            j += dump(js, t+1+j, count-j, indent+1, printCallback);
            printCallback(": ");
            j += dump(js, t+1+j, count-j, indent+1, printCallback);
            printCallback("\n\r");
        }
        return j+1;
    }
    else if (t->type == JSMN_ARRAY)
    {
        j = 0;
        printCallback("\n\r");
        for (i = 0; i < t->size; i++)
        {
            for (k = 0; k < indent-1; k++)
            {
            	printCallback("  ");
            }
            printCallback("   - ");
            j += dump(js, t+1+j, count-j, indent+1, printCallback);
            printCallback("\n\r");
        }
        return j+1;
    }
    return 0;
}

