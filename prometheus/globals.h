 #pragma once
#include "intrin.h"
#include "string"
#include <Windows.h>
#include <algorithm>

#define owassert(expr) if (!(expr)) { printf("%s:%d\n", "FAILED ASSERTION:\nAssertion: " #expr "\nAt: " __FILE__, __LINE__); }

#define emoji(str) (char*)u8##str

#define EMOJI_SHARE emoji("") //share
#define EMOJI_REFRESH emoji("") //rotate-right
#define EMOJI_CROSS emoji("") //cross
#define EMOJI_CHECK emoji("") //check
#define EMOJI_LOCATE emoji("") //location-dot
#define EMOJI_EDIT emoji("") //pen-to-square
#define EMOJI_TRASH emoji("") //trash
#define EMOJI_DENY emoji("") //ban
#define EMOJI_COMMENT emoji("") //comment
#define EMOJI_COMMENT_2 emoji("") //comments
#define EMOJI_COPY emoji("") //copy
#define EMOJI_BACK emoji("") //arrow-left
#define EMOJI_FORWARD emoji("") //arrow-right
#define EMOJI_PLAY emoji("") //play
#define EMOJI_STOP emoji("") //stop

#define MH_VERIFY(returnVal) { auto mh_ret = returnVal; if (mh_ret != 0) { printf(#returnVal " failed! (status code: %d)\n", mh_ret); return; } }
#define MH_VERIFY_RET(returnVal, retError) { auto mh_ret = returnVal; if (mh_ret != 0) { printf(#returnVal " failed! (status code: %d)\n", mh_ret); return retError; } }

__int64 ow_memalloc(int size); //dllmain
void ow_dealloc(__int64 address);

struct Vector3 {
	float X;
	float Y;
	float Z;

	Vector3() :
		X(0),
		Y(0),
		Z(0)
	{
	}

	Vector3(float values) :
		X(values),
		Y(values),
		Z(values)
	{
	}

	Vector3(float x, float y, float z) :
		X(x),
		Y(y),
		Z(z)
	{
	}

	Vector3 operator+(const Vector3& right) {
		return Vector3(X + right.X, Y + right.Y, Z + right.Z);
	}

	Vector3 operator-(const Vector3& right) {
		return Vector3(X - right.X, Y - right.Y, Z - right.Z);
	}

	Vector3 operator*(const Vector3& right) {
		return Vector3(X * right.X, Y * right.Y, Z * right.Z);
	}

	Vector3 operator*(const float right) {
		return Vector3(X * right, Y * right, Z * right);
	}

	Vector3 operator/(const Vector3& right) {
		return Vector3(right.X == 0 ? 0 : X / right.X, right.Y == 0 ? 0 : Y / right.Y, right.Z == 0 ? 0 : Z / right.Z);
	}

	float dot_product(const Vector3& right) {
		return X * right.X + Y * right.Y + Z * right.Z;
	}

	Vector3 cross_product(const Vector3& right) {
		return Vector3(Y * right.Z - Z * right.Y, Z * right.X - X * right.Z, X * right.Y - Y * right.X);
	}

	float length() {
		return sqrtf(X * X + Y * Y + Z * Z);
	}

	Vector3 normalize() {
		float len = length();
		return Vector3(X / len, Y / len, Z / len);
	}
};

struct Vector4 {
	float X;
	float Y;
	float Z;
	float W;

	Vector4() :
		X(0),
		Y(0),
		Z(0),
		W(0)
	{
	}

	Vector4(float values) :
		X(values),
		Y(values),
		Z(values),
		W(values)
	{
	}

	Vector4(float x, float y, float z, float w) :
		X(x),
		Y(y),
		Z(z),
		W(w)
	{
	}
};

struct Matrix4x4 {
	Vector4 row_1;
	Vector4 row_2;
	Vector4 row_3;
	Vector4 row_4;
};

template <typename T>
struct teList {
	T* ptr;
	int num;
	int max;

	T* begin() {
		return &ptr[0];
	}

	T* end() {
		return &ptr[num];
	}

	void emplace_item(T item) {
		if (num == max || ptr == nullptr) {
			max++;
			
			auto new_ptr = ow_memalloc(sizeof(T) * max);
			if (ptr) {
				memcpy((void*)new_ptr, (void*)ptr, num * sizeof(T));
				ow_dealloc((__int64)ptr);
			}
			printf("emplace_item allocate at %p\n", new_ptr); //TODO dont remove or msvc will kill itself
			ptr = (T*)new_ptr;
			if (!ptr)
				printf("ow_memalloc failed!\n");
		}
		ptr[num] = item;
		num++;
	}

	void remove_item(int index) {
		owassert(index < num);
		memcpy((void*)(ptr + sizeof(T) * index), (void*)(ptr + sizeof(T) * (index + 1)), sizeof(T) * (num - index - 1));
		num--;
	}

	void clear() {
		ZeroMemory(ptr, sizeof(T) * num);
		num = 0;
	}

	void dealloc() {
		clear();
		ow_dealloc((__int64)ptr);
		ptr = nullptr;
	}
};

namespace globals {
	extern DWORD_PTR gameBase;
	extern DWORD_PTR gameWindow;
	extern DWORD_PTR gameSize;
	extern DWORD_PTR modBase;
	extern DWORD_PTR modSize;
	extern bool exit_normal;
	extern bool pauseLogHook;
	extern bool isDemo;
	void ensure_console_allocated();
	extern bool switchGameEA;
}

namespace std {
	inline std::string to_string_hex(int input) {
		char buf[16];
		sprintf_s(buf, "%x", input);
		return buf;
	}
}

#define STRUCT_PLACE(type, item, offset) \
struct { \
char __pad_##item[offset]; \
type item; \
};

#define STRUCT_PLACE_CUSTOM(name, offset, definition) \
struct{ \
char __pad_##name[offset]; \
definition; \
}; 

#define STRUCT_PLACE_CUSTOM_2(offset, definition) \
struct { \
char __pad__##offset [offset];\
definition; \
};

#define STRUCT_PLACE_ARRAY(type, item, array_size, offset) \
struct { \
char __pad_##item[offset]; \
type item[array_size]; \
};

#define STRUCT_MIN_SIZE(size) \
char __pad_maxsize[size]

inline bool ichar_equals(char a, char b)
{
	return std::tolower(static_cast<unsigned char>(a)) ==
		std::tolower(static_cast<unsigned char>(b));
}

inline bool iequals(const std::string& a, const std::string& b)
{
	return a.size() == b.size() &&
		std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}

inline bool icontains(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(), strNeedle.end(),
		[](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != strHaystack.end());
}

inline std::string stacktrace_str() {
	char buf[4096]{};

	void* stack[48];
	USHORT count = CaptureStackBackTrace(0, 48, stack, NULL);
	for (USHORT c = 0; c < count; c++) {
		__int64 addr = (__int64)stack[c];
		//TODO: Properly enumerate the modules... someday
		if (addr > globals::gameBase && (addr - globals::gameBase) < globals::gameSize)
			sprintf(buf + strlen(buf), "addr %02d: %p (RVA %p)\n", c, addr, addr - globals::gameBase);
		else if (addr > globals::modBase && (addr - globals::modBase) < globals::modSize)
			sprintf(buf + strlen(buf), "addr %02d: %p (inject.dll+%p)\n", c, addr, addr - globals::modBase);
		else
			sprintf(buf + strlen(buf), "addr %02d: %p\n", c, addr);
	}

	return std::string(buf);
}

inline void stacktrace() {
	printf("%s\n", stacktrace_str().c_str());
}