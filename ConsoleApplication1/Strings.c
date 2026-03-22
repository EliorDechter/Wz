#include "Strings.h"

str1024 str1024_create(const char* format, ...) {
	va_list arg;
	str1024 result = { 0 };

	if (!format)
		return result;

	va_start(arg, format);
	vsnprintf(result.val, 1024, format, arg);
	va_end(arg);

	result.len = (int)strlen(result.val);

	return result;
}

void str1024_concat(str1024* dest, str1024 src)
{
	strcat(dest->val, src.val);
	dest->len = strlen(dest->val);
}

str128 str128_create(const char* format, ...) {
	va_list arg;
	str128 result = { 0 };

	va_start(arg, format);
	vsnprintf(result.val, 128, format, arg);
	va_end(arg);

	result.len = (int)strlen(result.val);

	return result;
}

bool str128_equal(str128 a, str128 b) {
	bool result = false;
	if (a.len == b.len)
	{
		if (strcmp(a.val, b.val) == 0)
		{
			result = true;
		}
	}

	return result;
}

void str128_concat(str128* dest, str128 src) 
{
	int i = (int)dest->len;
	for (; i < 128; ++i)
	{
		if (src.val[i] == '0')
			break;
		dest->val[i] = src.val[i];
	}

	dest->len = i;
}