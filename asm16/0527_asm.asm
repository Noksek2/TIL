;1====================================================================================
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


;2====================================================================================


;;input number and repeat as number
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
    mov num,0
inp:
	cin
	sub al,'0'
	cmp al,10
	jae init ;0<10 
	
	mov datc,al
	mov ax,0
	mov al,num
	mov bx,10
	mul bx ;ax*10
	add al,datc;
	cmp ax,100h
	jae init;jmp if ax>=100h
	mov num,al
	jmp inp;re input
init:	
	mov al,'*'
	mov cl,0
re:
	cmp cl,num
	jae tohalt
	char al
	inc cl          
	jmp re
tohalt:
	HALT
main ends
end 
