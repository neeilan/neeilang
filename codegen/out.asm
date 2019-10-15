section .data
newline db 10

section .bss
  dump_reg resb 8

default rel
global start
section .text

start:
	mov r12, 4
	push r12
	call label_1_print_digit
	mov rax, 0x2000001
	mov edi, 15 ; 		edi holds the exit status
	syscall

label_0_math:
	mov r12, 3
	mov r13, 6
	add r12, r13
	
	mov r14, 48
	add r14, r12
	mov [dump_reg], r14
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, dump_reg
	mov rdx, 8
	syscall
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, newline
	mov rdx, 1
	syscall
	add r12, r12
	
	mov r14, 48
	add r14, r12
	mov [dump_reg], r14
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, dump_reg
	mov rdx, 8
	syscall
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, newline
	mov rdx, 1
	syscall
	ret 

label_1_print_digit:
	pop r12
	pop r13
	
	mov r14, 48
	add r14, r13
	mov [dump_reg], r14
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, dump_reg
	mov rdx, 8
	syscall
	mov rax, 0x2000004
	mov rdi, 1
	mov rsi, newline
	mov rdx, 1
	syscall
	push r12
	ret 

