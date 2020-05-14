section	.text
global  RightEdgeErosion

RightEdgeErosion:
	push ebp
	mov	ebp, esp

    ; bajt pierwszy
    ; obliczanie adresu bajtu
    mov	eax, DWORD [ebp+8]
    mov edx, DWORD [ebp+16]

    add eax, edx
	dec eax

    ; wczytanie zawartosci dwoch bajtow
    movbe dx, WORD [eax]
    push dx

    ; bajt drugi
    ; obliczanie adresu bajtu
    mov	eax, DWORD [ebp+8]
    mov edx, DWORD [ebp+20]

    add eax, edx
	dec eax

    ; wczytanie zawartosci dwoch bajtow
    movbe dx, WORD [eax]


    pop ax ; pierwszy WORD
    ; dx - drugi WORD

    ; erozja bajtow
	or ax, dx

	shl dx, 1
	or ax, dx

	shr dx, 2
	or ax, dx

    ; obliczenie adresu do zapisu
    mov edx, DWORD [ebp+12]
    mov ecx, DWORD [ebp+16]

    add edx, ecx

    ; suma logiczna bajtow
    mov cl, byte [edx]
    or al, cl

    ; zapis bajtu
    mov [edx], al

	pop	ebp
	ret