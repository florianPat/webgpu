#pragma once

#include "Utils.h"
#include "Vector.h"
#include <string>

#define CALL_STRING_ARRAY(exp) if(shortRep) return stringUnion.stringArrayShort.exp; else return stringUnion.stringArrayLong.exp
//NOTE: For now, there is no possibiliy for a wide-character string. I could potentialy shoot myself in the foot here for localization purporses

class String
{
public:
	static constexpr int32_t SHORT_STRING_SIZE = 16;
	static constexpr uint32_t npos = (uint32_t)-1;
//NOTE: Is protected for the copy/move-constructor of Short- / LongString!
protected:
	const bool shortRep;
private:
	union StringUnion
	{
		Array<char, SHORT_STRING_SIZE> stringArrayShort;
		Vector<char> stringArrayLong;

		StringUnion() : stringArrayShort() {}
		~StringUnion();
	} stringUnion;
protected:
	String(uint32_t count, bool shortRepIn);
	String(uint32_t count, char c, bool shortRepIn);
	String(const String& other, bool shortRepIn);
	//String(const String& other, uint32_t pos, uint32_t count);
	//String(const char* s, uint32_t count);
private:
	uint32_t find(const char* str, uint32_t pos, uint32_t strSize) const;
public:
	String(uint32_t count);
	String(uint32_t count, char c);
	template <uint32_t N>
	String(const char(&s)[N], bool shortRepIn = (N <= SHORT_STRING_SIZE));
	static String createIneffectivlyFrom(const char* s);
	String(const String& other);
	String(String&& other) noexcept;
	String& operator=(const String& rhs);
	String& operator=(String&& rhs) noexcept;
	template <uint32_t N>
	String& operator=(const char(&s)[N]);
	~String();

	char& at(uint32_t pos);
	char at(uint32_t pos) const;

	char& operator[](uint32_t pos);
	char operator[](uint32_t pos) const;
	
	char& front();
	char front() const;
	char& back();
	char back() const;
	const char* data() const;
	char* data();
	const char* c_str() const;

	Iterator<char> begin();
	Iterator<char> end();

	ConstIterator<char> begin() const;
	ConstIterator<char> end() const;

	bool empty() const;
	
	uint32_t size() const;
	uint32_t length() const;

	void reserve(uint32_t count);
	uint32_t capacity() const;

	void clear();
	
	//NOTE: Implement insert if you need them

	String& erase(uint32_t pos, uint32_t count);
	Iterator<char> erase(const Iterator<char>& pos);
	Iterator<char> erase(const Iterator<char>& first, const Iterator<char>& last);

	Iterator<char> push_back(char c);
	Iterator<char> pop_back();

	//NOTE: Will fail if string is "full" if it is a short string
	String& append(uint32_t count, char c);
	String& append(const String& str);
	String& append(const String& str, uint32_t pos, uint32_t count);
	String& append(const char* str);
	String& append(const char* str, uint32_t count);
	String& operator+=(const String& str);
	String& operator+=(char c);
	String& operator+=(const char* str);

	String substr(uint32_t pos = 0, uint32_t count = npos) const;

	uint32_t find(char c, uint32_t pos = 0) const;
	template <uint32_t N>
	uint32_t find(const char(&str)[N], uint32_t pos = 0) const;
	uint32_t find(const String& str, uint32_t pos = 0) const;
	//NOTE: isn't it unnecessary?
	uint32_t find_first_of(char c, uint32_t pos = 0) const;
	uint32_t find_last_of(char c, uint32_t pos = npos) const;

	//NOTE: Implement resize if you need it!

	void swap(String& other);

	friend String operator+(const String& lhs, const String& rhs);
	friend String operator+(const String& lhs, const char* rhs);
	friend String operator+(const String& lhs, char rhs);
	template<uint32_t N>
	friend String operator+(const char(&lhs)[N], const String& rhs);
	friend String operator+(char lhs, const String& rhs);
	friend String operator+(String&& lhs, const String& rhs);
	friend String operator+(const String& lhs, String&& rhs);
	friend String operator+(String&& lhs, String&& rhs);
	friend String operator+(String&& lhs, const char* rhs);
	friend String operator+(String&& lhs, char rhs);
	template<uint32_t N>
	friend String operator+(const char (&lhs)[N], String&& rhs);
	friend String operator+(char lhs, String&& rhs);

	bool operator==(const String& rhs) const;
	bool operator==(const char* rhs) const;
	friend bool operator==(const char* lhs, const String& rhs);
	bool operator!=(const String& rhs) const;
	bool operator!=(const char* rhs) const;
	friend bool operator!=(const char* lhs, const String& rhs);
};

template<uint32_t N>
inline String::String(const char(&s)[N], bool shortRepIn) : shortRep(shortRepIn), stringUnion{}
{
	if (shortRep)
	{
		if constexpr (N > SHORT_STRING_SIZE)
			InvalidCodePath;
		for (uint32_t i = 0; i < N; ++i)
			stringUnion.stringArrayShort.push_back(s[i]);
	}
	else
	{
		new (&stringUnion.stringArrayLong) Vector<char>(N);

		for (uint32_t i = 0; i < N; ++i)
			stringUnion.stringArrayLong.push_back(s[i]);
	}
}

template<uint32_t N>
inline String & String::operator=(const char(&s)[N])
{
	this->~String();

	new (this) String(s);

	return *this;
}

template<uint32_t N>
inline uint32_t String::find(const char (&str)[N], uint32_t pos) const
{
	//NOTE: N - 1 because size() of the String class returns the size minus the 0 terminator!
	return find(str, pos, N - 1);
}

template<uint32_t N>
inline String operator+(const char (&lhsIn)[N], const String& rhs)
{
	String lhs(lhsIn);
	lhs += rhs;
	return lhs;
}

template<uint32_t N>
inline String operator+(const char (&lhsIn)[N], String&& rhs)
{
	String lhs(lhsIn);
	lhs += rhs;
	return lhs;
}

struct ShortString : public String
{
	ShortString();
	ShortString(uint32_t count);
	ShortString(uint32_t count, char c);
	template <uint32_t N>
	ShortString(const char(&s)[N]);
	ShortString(const String& other);
	ShortString(String&& other);
};

struct LongString : public String
{
	LongString();
	LongString(uint32_t count);
	LongString(uint32_t count, char c);
	template <uint32_t N>
	LongString(const char(&s)[N]);
	LongString(const String& other);
	LongString(String&& other);
};

#include <string>

//TODO: Import a HashMap implementation and replace unordered_map
namespace std {
	template<>
	struct hash<String> {
	public:
		uint32_t operator()(const String &s) const
		{
			return (uint32_t)std::hash<std::string>()(s.c_str());
		}
	};

	template<>
	struct hash<ShortString> {
	public:
		uint32_t operator()(const ShortString &s) const
		{
			return (uint32_t)std::hash<std::string>()(s.c_str());
		}
	};

	template<>
	struct hash<LongString> {
	public:
		uint32_t operator()(const LongString &s) const
		{
			return (uint32_t)std::hash<std::string>()(s.c_str());
		}
	};
}

namespace utils
{
	bool isWordInLine(const String & word, const String & lineContent);
	String getWordBetweenChars(const String& lineContent, char first, char last);
	LongString getDirPathToFile(const String& str);
	template <typename T>
	T getFromString(const String& string)
	{
		return (T)(((uint32_t)string.front()) | ((uint32_t)string.back() << 8));
	}
}

namespace StringUnitTest
{
	void runStringUnitTests();
	void runStdStringUnitTests();
}

template<uint32_t N>
inline ShortString::ShortString(const char(&s)[N]) : String(s, true)
{
}

template<uint32_t N>
inline LongString::LongString(const char(&s)[N]) : String(s, false)
{
}
