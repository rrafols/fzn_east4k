%define	NCHANNELS	8


threadMain:	

	xor	ebx,ebx
	dec	ebx			;row=-1
	xor	eax,eax			;order
mainPlayer:	
	inc	ebx
	cmp	ebx, byte 16
	jne	.okRow
	xor	ebx,ebx
	inc	eax
.okRow:
	cmp	eax,63
	jge	exitIntro
	
.okOrder


	push	eax
	
	
	mov	[ABS(row)],ebx
	mov	[ABS(order)],eax

	mov	al,[eax+ABS(orderList)]		;eax=order
	movzx	eax,al
	shl	eax,3				;3 si 8 canals
						;2 si 4 canals
	mov	ebp,eax


	xor	eax,eax	
	mov	ecx,8	;4	;NCHANNELS
.loopCh:
	pushad

	add	eax,ebp				;eax=order+i
		
	mov	al,[eax+ABS(patternList)]
	movsx	eax,al
							
	or	eax,eax
	js	near .noChannel
		
	mov	esi,ABS(channelList)
	mov	edx,eax
	shl	eax,4
	add	eax,edx			;eax=ch*17
	
	movzx	edi,byte [esi+eax]
	mov	edi,[edi*4+ABS(dsamples)]	;edi = dsoundbuffer

	add	eax,ebx		;[ABS(row)]
	inc	eax
	
	mov	bl,[esi+eax]			;bl=note
	or	bl,bl
	jz	.noChannel
	mov	bh,bl
	and	bl,0x0f	
	movzx	edx,bl
	dec	edx
		
	movzx	eax,bh
	shr	eax,4
	and	al,0x03

	shr	bh,6
	and	bh,0x03
	movzx	ebx,bh
	mov	ebx,[ebx*4+ABS(vols)]	
	push	ebx				;push volume!	
		
	add	eax,byte 4
	imul	eax,byte 12		
	add	eax,edx
	
	mov	ebx,dword [edi]
	xor	esi,esi
	
	cmp	dx,0xffff
	je	.NoteOn
	
	cmp	dx,0x000e
	je	.NoteCut
	
	mov	eax,[eax*4+ABS(freqtable)]
	
	push	eax
	push	edi
	
	push	edi
	call	dword [ebx+72]		;.Stop
	
	push	esi
	push	edi
	call	dword [ebx+52]		;.SetCurrentPosition	
	
	
					; frequency (pushed)
					; soundbuffer (pushed)	
	call	dword [ebx+68]		;.SetFrequency	

					;volume is pushed :)
.NoteOn:
	push	edi
	call	dword [ebx+60]		;.SetVolume

	push	esi
	push	esi
	push	esi	
	push	edi
	call	dword [ebx+48]		;.Play	
.noChannel:			
	popad
	inc	eax
	dec	ecx
	jnz	near .loopCh	

	INVOKER ABS(WaitForSingleObject),dword [ABS(thH)],114	;edx			;6*16=96;93		;93.75	

	pop	eax
	jmp	near mainPlayer

.NoteCut:
	pop	eax
	mov	eax,-10000
	push	eax
	push	edi
	call	dword [ebx+60]		;.SetVolume
	
	push	edi
	call	dword [ebx+72]		;.Stop	
		
	push	esi
	push	edi
	call	dword [ebx+52]		;.SetCurrentPosition	
	jmp	short .noChannel