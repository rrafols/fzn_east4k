;sample generator
;in:
;	eax		-> index
;	writebuf 	-> dsoundbuffer Locked

	finit					;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SSUUXXXXXXXXXXXXXXXXXXXXXXX
	pushad	
	mov	ebx,eax				;ebx = index	
	mov	edi,[ABS(writebuf)]
	
	movzx	ecx,word [ebx*2+ABS(m_nBytes)]	; 0		1	2	3	4	5	6	7

	fldz					; sp
	fldz					; ps	sp
	fldz					; phase	ps	sp
		
	fld	dword [ebx*4+ABS(m_Amp)]	; amp	phase	ps	sp
	fld	dword [ebx*4+ABS(m_Freq)]	; freq	amp	phase	ps	sp
bucler:
	fadd	st2,st0				; phase += freq
	fadd	dword [ebx*4+ABS(m_dFreq)]	; freq  += dfreq
	
	fld	st2				; val	freq	amp	phase	ps	sp
	mov	al,[ebx+ABS(m_tLfo)]
	test	al,1
	je	Saw
	fsin					;sin(val) freq	amp	phase	ps	sp
	jmp	NoSaw
Saw:
	fistp	word [ABS(wTemp)]
	mov	dl, byte [ABS(wTemp)]
	movsx	dx,dl
	mov	[ABS(wTemp)],dx
	fild	word [ABS(wTemp)]
NoSaw:
	test	al,2
	je	NoSquare
	fistp	word [ABS(wTemp)]
	and	word [ABS(wTemp)],0x80
	fild	word [ABS(wTemp)]
NoSquare:
	test	al, 4
	je	NoFabs
	fabs					; abs	freq	amp	phase	ps	sp
NoFabs:
	test	al,8
	je	NoPos
	fld	st0
	fabs
	faddp 	st1,st0				; val	freq	amp	phase	ps	sp
NoPos:

	fmul	st0,st2				; val	freq	amp	phase	ps	sp

	fxch	st2
	fadd	dword [ebx*4+ABS(m_dAmp)]
	fxch 	st2				; val	freq	amp	phase	ps	sp

	fsub	st0,st4				; v-ps	freq	amp phase	ps	sp
	fmul	dword [ebx*4+ABS(m_c)]
	faddp	st5,st0	
	fld	st4				; sp	freq	amp phase	ps	sp

	fadd	st4,st0				; ps	freq	amp	phase	ps	sp
	fxch	st4
	fist	word [ABS(wTemp)]
	fxch	st4
	mov	ax,word [ABS(wTemp)]
	
	fmul	dword [ebx*4+ABS(m_r)]
	fstp	st5

	add	al,127
	stosb
	dec	ecx
	jnz	near bucler
	popad