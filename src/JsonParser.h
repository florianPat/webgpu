#pragma once

#include "String.h"
#include "Vector.h"
#include <variant>

struct JsonParser
{
	enum class ObjectType
	{
		INTEGER,
		OBJECT,
		STRING,
		DECIMAL,
		BOOLEAN,
		ARRAY,
	};
	struct Object
	{
		ObjectType type = ObjectType::INTEGER;
		std::variant<int32_t, Vector<uint8_t*>, String, float, bool> jsonUnion;

		Object() = default;
		Object(const Object& other) = delete;
		Object& operator=(const Object& rhs) = delete;
		Object(Object&& other);
		Object& operator=(Object&& other);
		~Object();
	};
	struct JsonObject
	{
		String name;
		Object property;

		JsonObject(const String& name);
	};
public:
	Vector<JsonObject> jsonObjects;
	LongString number;
public:
	JsonParser() = default;
	JsonParser(const String& json);
	void parse(const String& json);
	static float getFloatFromNumericObject(Object* element);
private:
	Vector<JsonObject> parse(const String& json, ConstIterator<char>& it);
	void skipSpaces(ConstIterator<char>& it, const String& json);
	Object parseNumber(ConstIterator<char>& it);
	String parseString(ConstIterator<char>& it, const String& json);
	Object parseProperty(ConstIterator<char>& it, const String& json);
	static Vector<uint8_t*> convertObjectToPtr(Vector<struct JsonObject>&& object);
	static Vector<uint8_t*> convertArrayToPointer(Vector<struct Object>&& object);
	static Object create(Vector<struct JsonObject>&& object);
	static Object create(Vector<struct Object>&& objectArray);
	JsonObject parseObject(ConstIterator<char>& it, const String& json);
	Vector<Object> parseArray(const String& json, ConstIterator<char>& it);
};