#include "xyutils.h"

char *str_first_not(const char *src, const char ch)
{
    while (*src != '\0' && (*src == ' ' || *src == '\n' || *src == '\t' || *src == ch))
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

char *str_trim_end(char *src)
{
	if(!*src) return NULL;
	while(*src != '\0') ++src;
	--src;
	while(*src == '\t' || *src == ' ') --src;
	*(++src) = '\0';
}

char * str_first(const char *src, const char ch)
{
    while (*src != '\0' && *src != ch)
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

char *str_last(const char *src, const char ch)
{
    const char *end = src;
    src += strlen(src);
    while (src != end && *src != ch)
        --src;
    if (*src != ch)
        return NULL;
    return (char *)src;
}

int get_term2(char *dst, const char *src)
{
    int i = 0;
    while(*src != '\0' && *src != '\t' && *src != ' ')
    {
        *dst++ = *src++;
        ++i;
    }
    *dst = '\0';
    return i;
}

int get_term(char *dst, const char *src, const char end_ch)
{
	int i = 0;
	while(src[i] != '\0' && src[i] != end_ch)
	{
		dst[i] = src[i];
		++i;
	}
	dst[i] = '\0';
	return i;
}

int count_term(const char *src, const char end_ch)
{
	int i = 0;
	while(src[i] != '\0' && src[i] != end_ch)
	{
		++i;
	}
	return i;
}

int dechex(const char ch)
{
	if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if(ch >= '0' && ch <= '9') return ch - '0';
	return -1;
}

int parse_int(long long *result, const char *src)
{
	src = str_first_not(src, '\r');
	int i = 0, m = 1;
	if(*src == '-')
	{
		++src;
		m = -1;
		++i;
	}
	*result = 0;
	if(*src == '0')
	{
		if(src[1] == 'x') // hex
		{
			src += 2, i += 2;
			int tmp;
			while(-1 != (tmp = dechex(*src++)))
			{
				++i;
				*result = (*result << 4) + tmp;
			}
			*result *= m;
			return i;
		}
		if(src[1] >= '0' && src[1] <= '7') // oct
		{
			src ++;
			i++;
			while(*src <= '7' && *src >= '0')
			{
				*result = (*result << 3) + *src - '0';
				++i;
			}
			*result *= m;
			return i;
		}
		return 1; // 十进制的单个‘0’
	}
	else if (*src >= '0' && *src <= '9')
	{
		while(*src >= '0' && *src <= '9')
		{
			*result = *result * 10 + *src++ - '0';
			++i;
		}
		*result *= m;
		return i;
	}
	return -1;
}

