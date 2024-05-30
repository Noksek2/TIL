;input number and putchar
;.model small
;.stack 1024
;.data
;.code
datc db 0
num db 0
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
    
    
    mov al,255
    mov bl,100
inp:
	;cin
	
	cmp al,bl
	jb ten
	mov datc,al
	div bl
	add al,'0'
	char al
	sub al,'0'
	mul bl
	sub datc,al
	mov al,datc
	
	
ten:
    xchg al,bl;100, 255
    mov cl,10
	div cl
	xchg bl,al
	cmp bl,0 
	ja inp
	HALT
	
	
main ends
end 
