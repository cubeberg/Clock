
		.cdecls C, LIST, "msp430.h"

		.text

		.def	owex					; int owex(int data, unsigned bit_count)
		.def	owdport
		.def	owiport
		.def	owbit
		.def	owtiming
		
		.bss	owdport, 2				; Direction port
		.bss	owiport, 2				; Input port
		.bss	owbit, 2				; Port bitmask
		.bss	owtiming, 0				; Timing array
		.bss	owda, 2					; 28  Zero
		.bss	owdb, 2					; 33
		.bss	owdc, 2					; 39
		.bss	owdd, 2					; 7   One
		.bss	owde, 2					; 8
		.bss	owdf, 2					; 85
		.bss	owdg, 2					; 500 Reset
		.bss	owdh, 2					; 50
		.bss	owdi, 2					; 450
		
owex									;
		tst		R13						; Reset?
		jeq		owrst					; Yes...
		push	R11						; Save R11, R13
		push	R13						;
		clr		R13						; Clear bit count
										;
owloop									; --- Tx/Rx bit loop
		rra		R12						; Get tx bit
		jc		owone					; If one...
										;
										; - Send and verify zero
		mov		&owda, R15				;
		nop								;
		mov		&owdport, R14			; Bus low
		bis.b	&owbit, 0(R14)			;
										;
		call	#owdelay				; Delay 28 us
										;
		mov		&owiport, R14			; Sample bus
		mov.b	@R14, R14				;
		bit		&owbit, R14				;
		rrc		R11						;
										;
		mov		&owdb, R15				; Delay 33 us
		call	#owdelay				;
										;		
		mov		&owdc, R15				;
		mov		&owdport, R14			; Bus open 
		bic.b	&owbit, 0(R14)			;
										;
		jmp		ownext					; Delay 39 us
										;		
owone									; - Send one and read bit
		mov		&owdd, R15				;
		tst		R15						;
		mov		&owdport, R14			; Bus low
		bis.b	&owbit, 0(R14)			;
										;
		jn		owoneo					; Delay 7 us										
owdlyd									;										
		sub		#8, R15					;
		nop								;
		jc		owdlyd					;
		subc	R15, PC					;
		nop								;
		nop								;
		nop								;
owoneo									;		
		bic.b	&owbit, 0(R14)			; Bus open
										;
		jc		owones					; Delay 8 us										
		mov		&owde, R15				;
owdlye									;		
		sub		#8, R15					;
		nop								;
		jc		owdlye					;
		subc	R15, PC					;
		nop								;
		nop								;
		nop								;
owones									;		
		mov		&owiport, R14			; Sample bus
		mov.b	@R14, R14				;
		bit		&owbit, R14				;
		rrc		R11						;
										;
		mov		&owdf, R15				; Delay 85 us
ownext									;
		call	#owdelay				;
										; - Next bit
		inc		R13						; Increment bit count
		cmp		R13, 0(SP)				; Compare bit count
		jne		owloop					; Loop if not done...
owrxa									; - Align rx data
		cmp		#16, R13				; Rx data aligned?
		jeq		owrex					; Yes..
		rra		R11						; Shift in a zero bit
		inc		R13						; Inc bit count
		jmp		owrxa					; Next bit...		
owrex									;		
		mov		R11, R12				; Get rx data to R12
		pop		R13						; Restore R11, R13
		pop		R11						;
		ret								; Return										
										;
										;
owrst									; - Reset and presence detect
		mov		&owdport, R14			; Bus low
		bis.b	&owbit, 0(R14)			;
										;
		mov		&owdg, R15				; Delay 500 us
		call	#owdelay				;
										;		
		bic.b	&owbit, 0(R14)			; Bus open
										;
		mov		&owdh, R15				; Delay 50 us
		call	#owdelay				;
										;		
		mov		&owiport, R14			; Sample bus
		mov.b	@R14, R14				;
		bit		&owbit, R14				;
		subc	R12, R12				;
										;
		mov		&owdi, R15				; Delay 450 us
		;jmp	owdelay					;  and return
										;
owdelay									;
		sub		#8, R15					;
		nop								;
		jc		owdelay					;
		subc	R15, PC					;
		nop								;
		nop								;
		nop								;
		ret								;
										;
		.end							;
												
