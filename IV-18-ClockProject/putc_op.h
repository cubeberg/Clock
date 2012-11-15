/*
 * putc_op.c
 *
 *  Created on: Oct 4, 2012
 *      Author: um - not sure.  I didn't write this.  I'm pretty sure I got this from oPossum on 43oh.com
 */

#ifndef PUTC_OP_C_
#define PUTC_OP_C_

unsigned char _CIOBUF_[12 + 32];                // This *must* be global and named _CIOBUF_
                                                //  12 bytes needed for header and null terminator
                                                //
void putc(char c)                               // --- Send char to debugger using CIO
{                                               //
    static char * p = (char *)&_CIOBUF_[11];    // Pointer to string buffer
                                                //
    if(c) *p++ = c;                             // Append any non-null char to buffer
                                                // Write to host when buffer is full or char is null
    if((p >= (char *)&_CIOBUF_[sizeof(_CIOBUF_) - 1]) || (c == 0)) {
        *p = 0;                                 // Null terminate string
        const unsigned l = p - (char *)&_CIOBUF_[10];// String lengh including null
        _CIOBUF_[0] = _CIOBUF_[5] = l & 0xFF;   // Data and string length LSB
        _CIOBUF_[1] = _CIOBUF_[6] = l >> 8;     // Data and string length MSB
        _CIOBUF_[2] = 0xF3;                     // Write command
        _CIOBUF_[3] = 1; _CIOBUF_[4] = 0;       // stdout stream
        __asm(" .global C$$IO$$");              // CIO breakpoint
        __asm("C$$IO$$:nop");                   //
        p = (char *)&_CIOBUF_[11];              // Reset string buffer pointer
    }                                           //
}

void puts(char *s) { while (*s) putc(*s++); }   // Send string to debugger

inline void cio_flush(void) { cio_putc(0); }    // Flush the CIO write buffer


#endif /* PUTC_OP_C_ */
