
.offset 0x1000
; Codes 
.vma 0x2000
TestFunction:
addiu $sp, $sp, -0x10
sw $ra, 0x0($sp)
addiu $v0, $zero, 42
li  $t1, 0x100
.L1:
addu  $v1, $a0, $a1
bne  $t1, $zero, .L1
addiu $t1, $t1, -1
lw $ra, 0x0($sp)
jr $ra
addiu $sp, $sp, 0x10

main:
li $a0, 10
li $a1, 20
jal TestFunction
nop
la $a0, someData
lw $t0, 0($a0)
lw $t1, (someData2)($a0)
jal TestFunction
nop

someData:
.word 0xABCDEF01
someData2:
.word -98765432
