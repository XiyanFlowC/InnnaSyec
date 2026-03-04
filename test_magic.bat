@echo off
echo 编译预处理器...
g++ -o preprocessor preprocessor.cpp
if %errorlevel% neq 0 (
    echo 编译失败!
    pause
    exit /b 1
)

echo.
echo 测试魔法宏功能:
echo 输入文件 simple_test.asm:
type simple_test.asm
echo.
echo 预处理器输出:
preprocessor simple_test.asm
echo.
pause