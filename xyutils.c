#include "xyutils.h"
#include <ctype.h>

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

int parse_int(long long *result, const char *src)
{
	const char *p = skip_ws(src);
	int len = parse_int_literal(result, p);
	if (len < 0)
		return -1;
	return (int)(p - src) + len;
}

int eval_int_expr(long long *result, const char *src)
{
	const char *p = src;
	if (parse_or(&p, result) != 0)
		return -1;
	p = skip_ws(p);
	return (int)(p - src);
}

