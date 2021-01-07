#ifndef JSON_H_
#define JSON_H_

#include "hw_types.h"
#include "jsmn.h"

/* JSON parser */
typedef struct jsonParser_t
{
  char          *jsonStream;
  int           noOfToken;
  jsmn_parser   parser;
  jsmntok_t     *tokenList;
}jsonParser;

/* Object */
typedef int jsonObj;

/* Array */
typedef int jsonArray;

/* String */
typedef int jsonString;

/* enum for JSON objects type */
typedef enum jsonObjType_t
{
	JSON_VOID_OBJECT=0,
	JSON_INT_OBJECT=1,
	JSON_FLOAT_OBJECT,
	JSON_STRING_OBJECT,
	JSON_OBJECT,
	JSON_ARRAY_OBJECT,
	JSON_INVALID_OBJECT=-1,
	JSON_INVALID_ARRAY=-1

}jsonObjType;

/* typedef for printf */
typedef int (*jsonPrintCallbackFxn) (const char *format, ...);


//*****************************************************************************
//
//! Initialize JSON parser. It will allocate memory to store JSON objects and
//!		generate JSON tree for parsing.
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jsonStream 	- pointer to persistent JSON stream/data
//!
//! \return Return pointer to root object
//
//*****************************************************************************
jsonObj		json_parser_init(jsonParser *jParser, char *jsonStream);

//*****************************************************************************
//
//! De-initialize JSON parser. It will free allocated memory
//!
//! \param  jParser 	- pointer to JSON parser structure
//!
//! \return 0 on success
//
//*****************************************************************************
int 		json_parser_deinit(jsonParser *jParser);

//*****************************************************************************
//
//! Dump formatted JSON data on console
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  printCallback 	- Callback function for print equivalent to printf.
//!
//! \return None
//
//*****************************************************************************
void		json_parser_dump(jsonParser *jParser, jsonPrintCallbackFxn printCallback);

//*****************************************************************************
//
//! Extract object member from parent object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- Object tag name
//!
//! \return Pointer to extracted object or NULL
//
//*****************************************************************************
jsonObj 	json_get_object(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Extract string member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to object
//! \param  tagName 	- Object tag name
//! \param  tagName 	- Container to hold string
//!
//! \return Pointer to string container
//
//*****************************************************************************
char * 		json_object_get_string(jsonParser *jParser, jsonObj jObj, char *tagName, char *str);

//*****************************************************************************
//
//! Get integer member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- integer member tag name
//!
//! \return value of integer member
//
//*****************************************************************************
int 		json_object_get_int(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Get long long member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- long long member tag name
//!
//! \return value of long long member
//
//*****************************************************************************
long long 	json_object_get_long_long(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Get double member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- Object tag name
//!
//! \return value of double member
//
//*****************************************************************************
double 		json_object_get_double(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Get boolean member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- boolean member tag name
//!
//! \return value of boolean member
//
//*****************************************************************************
tBoolean	json_object_get_boolean(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Get array member from an object
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jObj 		- pointer to parent object
//! \param  tagName 	- array member tag name
//!
//! \return array object
//
//*****************************************************************************
jsonArray	json_object_get_array(jsonParser *jParser, jsonObj jObj, char *tagName);

//*****************************************************************************
//
//! Get string member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//! \param  str			- Container to hold string
//!
//! \return pointer to container of string
//
//*****************************************************************************
char * 		json_array_get_string(jsonParser *jParser, jsonArray jArray, int jElement, char *str);

//*****************************************************************************
//
//! Get integer member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
int 		json_array_get_int(jsonParser *jParser, jsonArray jArray, int jElement);

//*****************************************************************************
//
//! Get long long member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
long long 	json_array_get_long_long(jsonParser *jParser, jsonArray jArray, int jElement);

//*****************************************************************************
//
//! Get double member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
double 		json_array_get_double(jsonParser *jParser, jsonArray jArray, int jElement);

//*****************************************************************************
//
//! Get boolean member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
tBoolean	json_array_get_boolean(jsonParser *jParser, jsonArray jArray, int jElement);

//*****************************************************************************
//
//! Get array member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
jsonArray	json_array_get_array(jsonParser *jParser, jsonArray jArray, int jElement);

//*****************************************************************************
//
//! Get object member from an array
//!
//! \param  jParser 	- pointer to JSON parser structure
//! \param  jArray 		- pointer to array
//! \param  jElement	- array index
//!
//! \return value of array[jElement]
//
//*****************************************************************************
int 		json_array_get_object(jsonParser *jParser, jsonArray jArray, int jElement);


#endif /* JSON_H_ */
