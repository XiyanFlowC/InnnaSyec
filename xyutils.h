#ifndef XY_UTILS_H__
#define XY_UTILS_H__

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
// 获取第一个非指定的ch的字符位置
// 同时过滤空白字符（' ', '\\n', '\\t'）
char *str_first_not(const char *src, const char ch);

char *str_trim_end(char *src);

// 获取第一个指定的ch的字符位置
// 同时过滤空白字符（' ', '\\n', '\\t'）
char * str_first(const char *src, const char ch);

// 获取最后一个指定的ch的字符位置
// 同时过滤空白字符（' ', '\\n', '\\t'）
char *str_last(const char *src, const char ch);

// 获取一个项，遇到第一个空白字符或行末时终止
// 获取到的项存入dst中，假设src已经位于项的起始处
// 返回值：成功取得的字符个数
int get_term2(char *dst, const char *src);

// 获取一个以 end_ch 结尾的项到 dst, endch 自身不包含在结果中
// 返回值：成功获取的字符个数
int get_term(char *dst, const char *src, const char end_ch);

// 计算一个以end_ch结尾的项的长度
// 返回值：成功获取的字符个数
int count_term(const char *src, const char end_ch);

int dechex(const char ch);

// 解析整型
// 允许0x、0前缀表示的16进制及8进制解析。允许负号前缀。
// 返回成功处理的字符个数，数字格式错误返回-1
int parse_int(long long *result, const char *src);

#ifdef __cplusplus
}
#endif

#endif