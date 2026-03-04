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
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif
// ��ȡ��һ����ָ����ch���ַ�λ��
// ͬʱ���˿հ��ַ���' ', '\\n', '\\t'��
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

// ��ȡ��һ��ָ����ch���ַ�λ��
static char * str_first(const char *src, const char ch)
{
    while (*src != '\0' && *src != ch)
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

// ��ȡ���һ��ָ����ch���ַ�λ��
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


// ��ȡһ���������һ���հ��ַ�����ĩʱ��ֹ
// ��ȡ���������dst�У�����src�Ѿ�λ�������ʼ��
// ����ֵ���ɹ�ȡ�õ��ַ�����
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

// ��ȡһ���� end_ch ��β��� dst, endch �����������ڽ����
// ����ֵ���ɹ���ȡ���ַ�����
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

// ����һ����end_ch��β����ĳ���
// ����ֵ���ɹ���ȡ���ַ�����
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

// ��������
// ����0x��0ǰ׺��ʾ��16���Ƽ�8���ƽ�������������ǰ׺��
// ���سɹ��������ַ����������ָ�ʽ���󷵻�-1
static const char *skip_ws(const char *p)
{
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		++p;
	return p;
}

static int parse_int_literal(long long *result, const char *src)
{
	int i = 0;
	*result = 0;
	if (*src == '0')
	{
		if (src[1] == 'x')
		{
			src += 2, i += 2;
			int tmp;
			while (-1 != (tmp = dechex(*src++)))
			{
				++i;
				*result = (*result << 4) + tmp;
			}
			return i;
		}
		if (src[1] >= '0' && src[1] <= '7')
		{
			src++;
			i++;
			while (*src <= '7' && *src >= '0')
			{
				*result = (*result << 3) + *src - '0';
				++i;
			}
			return i;
		}
		return 1;
	}
	else if (*src >= '0' && *src <= '9')
	{
		while (*src >= '0' && *src <= '9')
		{
			*result = *result * 10 + *src++ - '0';
			++i;
		}
		return i;
	}
	return -1;
}

static int parse_primary(const char **pp, long long *out);
static int parse_or(const char **pp, long long *out);

static int parse_primary(const char **pp, long long *out)
{
	const char *p = skip_ws(*pp);
	if (*p == '(')
	{
		++p;
		if (parse_or(&p, out) != 0)
			return -1;
		p = skip_ws(p);
		if (*p != ')')
			return -1;
		*pp = p + 1;
		return 0;
	}

	long long val = 0;
	int len = parse_int_literal(&val, p);
	if (len <= 0)
		return -1;
	*out = val;
	*pp = p + len;
	return 0;
}

static int parse_unary(const char **pp, long long *out)
{
	const char *p = skip_ws(*pp);
	if (*p == '+' || *p == '-' || *p == '~')
	{
		char op = *p++;
		if (parse_unary(&p, out) != 0)
			return -1;
		if (op == '-')
			*out = -*out;
		else if (op == '~')
			*out = ~(*out);
		*pp = p;
		return 0;
	}
	return parse_primary(pp, out);
}

static int parse_mul(const char **pp, long long *out)
{
	if (parse_unary(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		char op = *p;
		if (op != '*' && op != '/' && op != '%')
			break;
		++p;
		long long rhs = 0;
		if (parse_unary(&p, &rhs) != 0)
			return -1;
		if (op == '*')
			*out = (*out) * rhs;
		else if (op == '/')
			*out = rhs == 0 ? 0 : (*out) / rhs;
		else
			*out = rhs == 0 ? 0 : (*out) % rhs;
		*pp = p;
	}
	return 0;
}

static int parse_add(const char **pp, long long *out)
{
	if (parse_mul(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		char op = *p;
		if (op != '+' && op != '-')
			break;
		++p;
		long long rhs = 0;
		if (parse_mul(&p, &rhs) != 0)
			return -1;
		if (op == '+')
			*out = (*out) + rhs;
		else
			*out = (*out) - rhs;
		*pp = p;
	}
	return 0;
}

static int parse_shift(const char **pp, long long *out)
{
	if (parse_add(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		if (p[0] == '<' && p[1] == '<')
		{
			p += 2;
			long long rhs = 0;
			if (parse_add(&p, &rhs) != 0)
				return -1;
			*out = (*out) << rhs;
			*pp = p;
		}
		else if (p[0] == '>' && p[1] == '>')
		{
			p += 2;
			long long rhs = 0;
			if (parse_add(&p, &rhs) != 0)
				return -1;
			*out = (*out) >> rhs;
			*pp = p;
		}
		else
			break;
	}
	return 0;
}

static int parse_and(const char **pp, long long *out)
{
	if (parse_shift(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		if (*p != '&' || p[1] == '&')
			break;
		++p;
		long long rhs = 0;
		if (parse_shift(&p, &rhs) != 0)
			return -1;
		*out = (*out) & rhs;
		*pp = p;
	}
	return 0;
}

static int parse_xor(const char **pp, long long *out)
{
	if (parse_and(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		if (*p != '^')
			break;
		++p;
		long long rhs = 0;
		if (parse_and(&p, &rhs) != 0)
			return -1;
		*out = (*out) ^ rhs;
		*pp = p;
	}
	return 0;
}

static int parse_or(const char **pp, long long *out)
{
	if (parse_xor(pp, out) != 0)
		return -1;
	while (1)
	{
		const char *p = skip_ws(*pp);
		if (*p != '|')
			break;
		++p;
		long long rhs = 0;
		if (parse_xor(&p, &rhs) != 0)
			return -1;
		*out = (*out) | rhs;
		*pp = p;
	}
	return 0;
}

// 解析并计算整型表达式（高开销）
// 支持整数字面量、一元/二元运算符（+-*/%&|^~<<>>）和括号。
// 返回成功处理的字符个数，表达式格式错误返回-1
static int eval_int_expr(long long *result, const char *src)
{
	const char *p = src;
	if (parse_or(&p, result) != 0)
		return -1;
	p = skip_ws(p);
	return (int)(p - src);
}

// 解析整数字面量（轻量级）
// 仅支持 0x/0 前缀和十进制，不支持运算符和括号。
// 返回成功处理的字符个数，格式错误返回-1
static int parse_int(long long *result, const char *src)
{
	const char *p = skip_ws(src);
	int len = parse_int_literal(result, p);
	if (len < 0)
		return -1;
	return (int)(p - src) + len;
}
#ifndef _WIN32
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
#endif

#ifdef __cplusplus
}
#endif

#endif