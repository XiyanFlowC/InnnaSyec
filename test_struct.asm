; 测试结构体功能
; 定义一个结构体
.struct MyStruct word foo, byte bar, byte padding, half baz

.loc 0x1000

; 使用结构体实例化数据
someData:
  struct MyStruct <0x12345678, 0x42, 0xFF, 0xABCD>

anotherData:
  struct MyStruct <baseAddr+4, 1, 0, 233>

.loc 0x2000

; 在代码中使用结构体字段偏移
baseAddr:
  .word 0x80000000

someCode:
  ; 这会自动替换为 lw $t7, 0($t1)
  lw  $t7, (MyStruct.foo)($t1)
  
  ; 这会自动替换为 lb $t0, 4($t2)  
  lb  $t0, (MyStruct.bar)($t2)
  
  ; 这会自动替换为 lh $t3, 6($t4)
  lh  $t3, (MyStruct.baz)($t4)
  
  nop
