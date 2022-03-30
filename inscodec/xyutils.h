/**
 * @file xyutils.h
 * @author xiyan
 * @brief Some utils i used
 * @version 0.5
 * @date 2022-03-29
 * 
 * @copyright Licensed with MIT License
 * 
 */
#ifndef XY_UTILS_H__
#define XY_UTILS_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
// 获取第一个非指定的ch的字符位置
// 同时过滤空白字符（' ', '\\n', '\\t'）
static char *str_first_not(const char *src, const char ch)
{
    while (*src != '\0' && (*src == ' ' || *src == '\n' || *src == '\t' || *src == ch))
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

static char *str_trim_end(char *src)
{
	char *ret = src;
	if(!*src) return NULL;
	while(*src != '\0') ++src;
	--src;
	while(*src == '\t' || *src == ' ') --src;
	*(++src) = '\0';
	return ret;
}

// 获取第一个指定的ch的字符位置
static char * str_first(const char *src, const char ch)
{
    while (*src != '\0' && *src != ch)
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

// 获取最后一个指定的ch的字符位置
static char *str_last(const char *src, const char ch)
{
    const char *end = src;
    src += strlen(src);
    while (src != end && *src != ch)
        --src;
    if (*src != ch)
        return NULL;
    return (char *)src;
}


// 获取一个项，遇到第一个空白字符或行末时终止
// 获取到的项存入dst中，假设src已经位于项的起始处
// 返回值：成功取得的字符个数
static int get_term2(char *dst, const char *src)
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

// 获取一个以 end_ch 结尾的项到 dst, endch 自身不包含在结果中
// 返回值：成功获取的字符个数
static int get_term(char *dst, const char *src, const char end_ch)
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

// 计算一个以end_ch结尾的项的长度
// 返回值：成功获取的字符个数
static int count_term(const char *src, const char end_ch)
{
	int i = 0;
	while(src[i] != '\0' && src[i] != end_ch)
	{
		++i;
	}
	return i;
}

static int index_of(const char *str_grp[], int limit, const char *target) {
	int i;
	for(i = 0; i < limit; ++i) {
		if(strcmp(str_grp[i], target) == 0) return i;
	}
	return -1;
}


static int dechex(const char ch)
{
	if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if(ch >= '0' && ch <= '9') return ch - '0';
	return -1;
}

static int write_uint(char *buffer, unsigned long long value) {
	int ret = 0;
	if(value < 10) {
		buffer[0] = value + '0';
		return 1;
	}

	*buffer++ = '0';
	*buffer++ = 'x';
	ret += 2;

	char buf[32], *p = buf;
	while(value) {
		*p++ = "0123456789ABCDEF"[value & 0xF];
		value >>= 4;
	}
	--p;
	while(p != buf) {
		++ret;
		*buffer++ = *p--;
	}
	*buffer++ = *p;
	return ret + 1;
}

static int output_int(char *buffer, long long value, int is_unsigned) {
	if(is_unsigned) return write_uint(buffer, *(unsigned long long *)&value);
	if(value < 0) {
		buffer[0] = '-';
		return write_uint(buffer + 1, -value) + 1;
	}
	return write_uint(buffer, value);
}

// 解析整型
// 允许0x、0前缀表示的16进制及8进制解析。允许负号前缀。
// 返回成功处理的字符个数，数字格式错误返回-1
static int parse_int(long long *result, const char *src)
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

static char *strupr(char *str)
{
	char *ret = str;
	while(*str != '\0')
	{
		if (*str >= 'a' && *str <= 'z')
		{
			*str = *str - 'a' + 'A';
		}

		str++;
	}
	return ret;
}

static char *strlwr(char *str)
{
	char *ret = str;
	while(*str != '\0')
	{
		if (*str >= 'A' && *str <= 'Z')
		{
			*str = *str - 'A' + 'a';
		}

		str++;
	}
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif