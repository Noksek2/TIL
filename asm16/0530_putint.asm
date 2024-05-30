;input number and putchar
;.model small
;.stack 1024
;.data
;.code
datc db 0
num db 240
char macro c
	mov dl, c
	mov ah,2
	int 21h
	endm
halt macro
    MOV AH, 4CH
    INT 21H
endm
cin macro 
    MOV AH, 1
    INT 21H
endm
	
main segment
    assume cs:main 
re:
    mov ax,0
    mov bx,0
    mov cx,0    
    mov al,num
    mov bl,100
inp:
	;cin
	                      
	cmp al,bl  ;al 40 bl 10 datc 40
	mov datc,al
	div bl
	add al,'0'
	char al
	sub al,'0'
	mul bl
	sub datc,al
	mov al,datc

    xchg al,bl;10, 01
    mov cl,10
	div cl
	xchg bl,al
	cmp bl,0 
	ja inp 
    
    char ' '
    
    inc num
    cmp num,255
    jbe re
    
	HALT
	
	
main ends
end 
