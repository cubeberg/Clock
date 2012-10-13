;
;    Copyright (C) 2011  Kevin Timmerman
;
;	This program is free software: you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation, either version 3 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

		.cdecls C, LIST, "msp430fr5739.h"

		.text
		.def	utoa					; void utoa(unsigned, char *)
		.def	itoa					; void itoa(int, char *)
		.def	btoa					; void btoa(unsigned, char *)


utoa									; --- Unsigned to ASCII ---
										; - Range 0 to 65535
										; - Leading zeros supressed
		push	R10						;
		clr		R14						; Clear packed BCD
		mov		#-1, R10				; Flag as positive
		rla		R12						; Get bit 15 of binary										
		jmp		unsigned				;
itoa									; --- Integer to ASCII ---
										; - Range -32768 to +32767
										; - Leading zeros supressed
		push	R10						;
		clr		R14						; Clear packed BCD
		rla		R12						; Get bit 15 of binary
		subc	R10, R10				; Save sign bit
		jne		notneg					; Positive...
		inv		R12						; Negate negative numbers
		inc		R12						; This will overflow only for -32768
unsigned								;		
		dadd	R14, R14				; Multiply BCD by 2 and add binary bit
notneg									;
		.loop 12						; Do 12 bits
		rla		R12						; Get bit 14 to 3 of binary
		dadd	R14, R14				; Multiply BCD by 2 and add binary bit
		.endloop						;
		clr		R15						; Clear digit 1 of packed BCD
		.loop 3							; Do 3 bits
		rla		R12						; Get bit 2 to 0 of binary
		dadd	R14, R14				; Multiply BCD by 2 and add binary bit
		dadd	R15, R15				;
		.endloop						;
		swpb	R14						; Swap digit order
		mov		R14, R12				; Copy packed BCD digits
		and		#0x0F0F, R12			; Mask digits 5 & 3
		rra		R14						; Shift digits 4 & 2 to lower nibble
		rra		R14						;
		rra		R14						;
		rra		R14						;
		and		#0x0F0F, R14			; Mask digits 4 & 2
		tst		R10						; Negative?
		jne		nosign					; No, skip sign...
		mov.b	#'-', 0(R13)			; Negative sign to string
		inc		R13						;
nosign									;
		mov		#('0' << 8) | '0', R10	; Make ASCII
		add		R10, R12				;
		add		R10, R14				;
		add		R10, R15				;
		cmp.b	R10, R15				; Is first digit a 0?
		jne		dig5					; No...
		cmp.b	R10, R14				; Is second digit a 0?
		jne		dig4					; No, only the first...
		cmp.b	R10, R12				; Is third digit a 0?
		jne		dig3					; No, only the first two...
		cmp		R10, R14				; Is fourth digit a 0? (second is zero)
		jne		dig2					; No, only the first three...
dig1									; First four digits are all 0
		swpb	R12						; Fifth digit to string
		mov.b	R12, 0(R13)				;
		inc		R13						;
		clr.b	0(R13)					; NULL terminate string
		pop		R10						;
		ret								; Return
										;
dig5									;
		mov.b	R15, 0(R13)				; First digit to string
		inc		R13						;
dig4									;
		mov.b	R14, 0(R13)				; Second digit to string
		inc		R13						;
dig3									;
		mov.b	R12, 0(R13)				; Third digit to string
		inc		R13						;
dig2									;
		swpb	R14						; Fourth digit to string
		mov.b	R14, 0(R13)				;
		inc		R13						;
		jmp		dig1					;
										;
btoa									; --- Byte to ASCII ---
										; - Range 0 to 255
										; - Leading zeros converted to spaces
		clr		R14						; Clear packed BCD
		swpb	R12						; Move LSB to MSB
		.loop 8							; Do 8 bits
		rla		R12						; Get a bit of binary
		dadd	R14, R14				; Multiply BCD by 2 and add binary bit
		.endloop						;
										;
		mov		R14, R12				; Copy packed BCD digits
		and		#0x0F0F, R12			; Mask digits 1 & 3
		mov		#('0' << 8) | '0', R15	; Make ASCII
		add		R15, R12				;
		mov.b	R12, 2(R13)				; Move digit 3 to bcd[2]
		swpb	R12						; Swap digits 1 & 3
		mov.b	R12, 0(R13)				; Move digit 1 to bcd[0]
		rra		R14						; Shift digit 2 to lower nibble
		rra		R14						;
		rra		R14						;
		rra		R14						;
		and		#0x0F0F, R14			; Mask digit 2
		add		R15, R14				; Make ASCII
		mov.b	R14, 1(R13)				; Move digit 2 to bcd[1]
		clr.b	3(R13)					; NULL terminate string
		cmp.b	R15, 0(R13)				; Is first digit a 0?
		jne		b2aex					; No...
		cmp.b	R15, 1(R13)				; Is second digit a 0?
		jne		b2a2d					; No, only the first..
		mov		#(' ' << 8) | ' ', 0(R13); Make first two digits spaces
		ret								; Return
b2a2d									;
		mov.b	#' ', 0(R13)			; Make first digit a space
b2aex									;
		ret								; Return
																