;input number and putchar
.model small
.stack 1024
.data
.code

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
main proc
	cin
	sub al,'0'
	mov datc,al
	mov al,'a'
	mov cl,0
re:
	cmp cl,datc
	jae tohalt
	char al
	inc al
	inc cl          
	jmp re
tohalt:
	HALT
datc db 0
main endp
end main  
