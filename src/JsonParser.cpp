#include "JsonParser.h"

JsonParser::JsonParser(const String& json)
{
	parse(json);
}

void JsonParser::parse(const String& json)
{
	auto start = json.begin();
	jsonObjects = parse(json, start);
}

Vector<JsonParser::JsonObject> JsonParser::parse(const String& json, ConstIterator<char>& it)
{
	Vector<JsonObject> result;

	skipSpaces(it, json);
	assert((*it) == '{');
	++it;
	skipSpaces(it, json);

	while ((*it) != '}')
	{
		result.push_back(parseObject(it, json));

		while ((*it) == ',')
		{
			++it;
			result.push_back(parseObject(it, json));
		}
	}
	++it;

	return result;
}

void JsonParser::skipSpaces(ConstIterator<char>& it, const String& json)
{
	while ((*it) < ' ')
	{
		assert(it != json.end());
		++it;
	}
}

JsonParser::Object JsonParser::parseNumber(ConstIterator<char>& it)
{
	Object result;

	result.type = ObjectType::INTEGER;
	while (((*it) >= '0' && (*it) <= '9') || (*it) == '.' || (*it) == '-' || (*it) == 'e' || (*it) == 'E')
	{
		if ((*it) == '.')
		{
			result.type = ObjectType::DECIMAL;
		}

		number += *it++;
	}

	switch (result.type)
	{
		case ObjectType::INTEGER:
		{
			result.jsonUnion.emplace<int32_t>(atoi(number.c_str()));
			break;
		}
		case ObjectType::DECIMAL:
		{
			result.jsonUnion.emplace<float>((float)atof(number.c_str()));
			break;
		}
		default:
		{
			InvalidCodePath;
			break;
		}
	}

	number.clear();

	return result;
}

String JsonParser::parseString(ConstIterator<char>& it, const String& json)
{
	auto start = json.begin();
	assert((*it) == '"');

	++it;
	uint32_t keyStart = it - start;
	uint32_t keyEnd = json.find('"', keyStart);
	uint32_t keySize = keyEnd - keyStart;
	it += keySize + 1;
	skipSpaces(it, json);

	return json.substr(keyStart, keySize);
}

JsonParser::Object JsonParser::parseProperty(ConstIterator<char>& it, const String& json)
{
	Object result;

	skipSpaces(it, json);

	switch (*it)
	{
		case '"':
		{
			result.type = ObjectType::STRING;
			result.jsonUnion.emplace<String>(parseString(it, json));
			break;
		}
		case 't':
		{
			result.type = ObjectType::BOOLEAN;
			result.jsonUnion.emplace<bool>(true);
			it += 4;
			break;
		}
		case 'f':
		{
			result.type = ObjectType::BOOLEAN;
			result.jsonUnion.emplace<bool>(false);
			it += 5;
			break;
		}
		case '{':
		{
			result = create(parse(json, it));
			break;
		}
		case '[':
		{
			result = create(parseArray(json, it));
			break;
		}
		default:
		{
			assert((*it) >= '0' && (*it) <= '9' || (*it) == '-');

			result = parseNumber(it);
			break;
		}
	}

	skipSpaces(it, json);

	return result;
}

JsonParser::Object JsonParser::create(Vector<JsonObject>&& objectIn)
{
	Object result;
	result.type = ObjectType::OBJECT;
	result.jsonUnion.emplace<Vector<uint8_t*>>(convertObjectToPtr(std::move(objectIn)));
	return result;
}

JsonParser::JsonObject JsonParser::parseObject(ConstIterator<char>& it, const String& json)
{
	JsonObject result(parseString(it, json));

	assert((*it) == ':');
	++it;
	result.property = parseProperty(it, json);

	return result;
}

Vector<JsonParser::Object> JsonParser::parseArray(const String& json, ConstIterator<char>& it)
{
	Vector<Object> result;

	skipSpaces(it, json);
	assert((*it) == '[');
	++it;
	skipSpaces(it, json);

	while ((*it) != ']')
	{
		result.push_back(parseProperty(it, json));

		while ((*it) == ',')
		{
			++it;
			result.push_back(parseProperty(it, json));
		}
	}
	++it;

	return result;
}

float JsonParser::getFloatFromNumericObject(Object* element)
{
	switch (element->type)
	{
		case JsonParser::ObjectType::DECIMAL:
		{
			return std::get<float>(element->jsonUnion);
		}
		case JsonParser::ObjectType::INTEGER:
		{
			return (float) std::get<int32_t>(element->jsonUnion);
		}
		default:
		{
			InvalidCodePath;
			return 0.0f;
		}
	}
}

JsonParser::Object JsonParser::create(Vector<Object>&& objectArrayIn)
{
	Object result;
	result.type = ObjectType::ARRAY;
	result.jsonUnion.emplace<Vector<uint8_t*>>(convertArrayToPointer(std::move(objectArrayIn)));
	return result;
}

JsonParser::Object::Object(Object&& other) : type(std::exchange(other.type, ObjectType::INTEGER)), jsonUnion(std::exchange(other.jsonUnion, {}))
{
}

JsonParser::Object& JsonParser::Object::operator=(Object&& other)
{
	this->~Object();

	new (this) Object(std::move(other));

	return *this;
}

JsonParser::Object::~Object()
{
	switch (type)
	{
		case ObjectType::OBJECT:
		{
			Vector<uint8_t*>& objectPtr = std::get<Vector<uint8_t*>>(jsonUnion);
			for (auto it = objectPtr.begin(); it != objectPtr.end(); ++it)
			{
				JsonObject* jsonObject = (JsonObject*) (*it);
				jsonObject->~JsonObject();
				free(*it);
				*it = nullptr;
			}
			break;
		}
		case ObjectType::ARRAY:
		{
			Vector<uint8_t*>& objectArray = std::get<Vector<uint8_t*>>(jsonUnion);
			for (auto it = objectArray.begin(); it != objectArray.end(); ++it)
			{
				Object* object = (Object*) (*it);
				object->~Object();
				free(*it);
				*it = nullptr;
			}
			break;
		}
	}

	jsonUnion.~variant();
}

Vector<uint8_t*> JsonParser::convertObjectToPtr(Vector<JsonObject>&& object)
{
	Vector<uint8_t*> result;

	for (auto it = object.begin(); it != object.end(); ++it)
	{
		JsonObject* objectPtr = (JsonObject*) malloc(sizeof(JsonObject));
		new (objectPtr) JsonObject(std::move(*it));
		result.push_back((uint8_t*) objectPtr);
	}

	return result;
}

Vector<uint8_t*> JsonParser::convertArrayToPointer(Vector<struct Object>&& array)
{
	Vector<uint8_t*> result;

	for (auto it = array.begin(); it != array.end(); ++it)
	{
		Object* objectPtr = (Object*)malloc(sizeof(Object));
		new (objectPtr) Object(std::move(*it));
		result.push_back((uint8_t*) objectPtr);
	}

	return result;
}

JsonParser::JsonObject::JsonObject(const String& name) : name(name), property()
{
}
