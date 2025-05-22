default rel

section .data
    t0: dq 0
    name: dq 0
    sum: dq 0
    t2: dq 0
    i: dq 0
    L0: dq 0
    t3: dq 0
    t4: dq 0
    t5: dq 0
    L1: dq 0
    L2: dq 0
    t6: dq 0
    t7: dq 0
    t8: dq 0
    t9: dq 0
    t10: dq 0
    t11: dq 0
    t1: db "jatin", 0

section .text
global _start
extern show_num
extern show_str
extern process_exit

_start:
    lea rax, [rel t1]
    mov [t0], rax
    mov rax, [t0]
    mov [name], rax
    mov rax, 10
    mov [t1], rax
    mov rax, [t1]
    mov [sum], rax
    mov rax, 0
    mov [t2], rax
    mov rax, [t2]
    mov [i], rax
L0:
    mov rax, [i]
    mov [t3], rax
    mov rax, [sum]
    mov [t4], rax
    mov rax, [t5]
    cmp rax, 0
    jne L1
    jmp L2
L1:
    mov rax, [name]
    mov [t6], rax
    mov rcx, [t6]
    call show_num
    mov rax, [i]
    mov [t7], rax
    mov rax, [i]
    mov [t8], rax
    mov rax, 1
    mov [t9], rax
    mov rax, [t8]
    add rax, [t9]
    mov [t10], rax
    mov rax, [t10]
    mov [i], rax
    jmp L0
L2:
    mov rax, [sum]
    mov [t11], rax
    mov rcx, [t11]
    call show_num

    mov rcx, 0
    call process_exit
