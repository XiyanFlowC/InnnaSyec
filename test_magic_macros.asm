; 测试魔法宏功能
%define TEST_MACRO __LINE__

main:
    ; 这是第 __LINE__ 行
    mov r1, __LINE__
    mov r2, __FILE__
    
    ; 测试日期和时间宏
    .ascii __DATE__
    .ascii __TIME__
    
    ; 测试在宏定义中使用
    mov r3, TEST_MACRO
    
    ; 测试在表达式中使用
    add r4, __LINE__, 10