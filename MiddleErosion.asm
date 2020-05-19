section	.text
global  MiddleErosion

MiddleErosion:
	push ebp
	mov	ebp, esp

    ; bajt pierwszy
    ; obliczanie adresu bajtu
    mov	eax, DWORD [ebp+8]
    mov edx, DWORD [ebp+16]

    add eax, edx
    dec eax      ; wsk na bajt poprzedzajacy

    ; wczytanie zawartosci czterech bajtow
    mov dl, byte [eax]
    shl edx, 8
    mov dl, byte [eax+1]
    shl edx, 8
    mov dl, byte [eax+2]
    shl edx, 8
    push edx

    ; bajt drugi
    ; obliczanie adresu bajtu
    mov	eax, DWORD [ebp+8]
    mov edx, DWORD [ebp+20]

    add eax, edx
    dec eax      ; wsk na bajt poprzedzajacy

    ; wczytanie zawartosci czterech bajtow
    mov dl, byte [eax]
    shl edx, 8
    mov dl, byte [eax+1]
    shl edx, 8
    mov dl, byte [eax+2]
    shl edx, 8


    pop eax ; pierwszy DWORD
    ; edx - drugi DWORD

    ; erozja bajtow
	or eax, edx

	shl edx, 1
	or eax, edx

	shr edx, 2
	or eax, edx

    shr eax, 16

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