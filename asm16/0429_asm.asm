#1
;loop until number
main segment
    assume cs:main
    mov cl,0
     
    re:
    mov dl,cl
    add dl,'0'
    mov ah, 2
    int 21h
    inc cl
    cmp cl,10
    jne re
    
    
    mov ah,4ch
    int 21h
    
    main ends
end


#2
main segment
    assume cs:main
    L1: mov ah,1 ;input key
    int 21h
    cmp al,1ah
    je fin
    
    cmp al,'a'
    jb L1
    cmp al,'z'
    ja L1
    
    sub al, 'a'-'A'
    
    L2: 
    mov dl,al
    mov ah,2
    int 21h
    jmp L1
    
    fin:mov ah,4ch
    int 21h
                      
SUM DW ?
sum2 dw ?    
    
    main ends
end
