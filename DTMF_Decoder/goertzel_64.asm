;///////////////////////////////////////////////////////////////////////////////
;//          University of Hawaii, College of Engineering
;//          DTMF_Decoder - EE 469 - Fall 2022
;//
;/// A Windows Desktop C program that decodes DTMF tones
;///
;/// A hand-coded, optimized Goertzel DFT that's intended to do all of its
;/// calculations in the CPU/registers.
;/// 
;/// @file goertzel.asm
;/// @version 1.0
;///
;/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
;/// @see https://en.wikipedia.org/wiki/Goertzel_algorithm
;/// 
;/// @author Mark Nelson <marknels@hawaii.edu>
;/// @date   25_Oct_2022
;///////////////////////////////////////////////////////////////////////////////


externdef queueHead:qword
externdef queueSize:qword
externdef pcmQueue:qword

.code

option casemap:none    ; Make symbols case sensitive

; Note:  One optimization (for the sale of simplicity) is that we are
;        analyzing the queue from position 0 to queueSize -- not from
;        the head of the queue.  This gets us down to 1 counter and 
;        makes the program easier to read without affecting its accuracy
;
; CL = The UINT8 index (param 1)
; RDX = The 64-bit pointer to the dtmfTone struct (param 2)
; XXXR8 = queueSize (copied from external size_t)
; R8 = pcmQueue (copied from external size_t)
; R9 = pcmQueue + queueSize 
; XXXR9 = i (local, the iterator in the for(;;) loop)
; XXXR10 = pcmQueue (copied from external BYTE*)
; R11 = Unused
; XMM0 = q0
; XMM1 = q1
; XMM2 = q2
; XMM3 = coeff
; 
public goertzel_magnitude_64
goertzel_magnitude_64 PROC

	MOV R8, pcmQueue              ; Read from this point in the Queue
	MOV R9, R8
	ADD R9, queueSize             ; Read up to this position

;	MOV R8, queueSize             ; The size of the queue
;	XOR R9, R9                    ; for ( size_t i = 0; ...
;	MOV R10, pcmQueue             ; Pointer to the head of the PCM queue
	VPXOR XMM1, XMM1, XMM1        ; float q1 = 0;
	VPXOR XMM2, XMM1, XMM1        ; float q2 = 0;
	MOVSS XMM3, dword ptr [RDX + 56]  ; Copy toneStruct->coeff into XMM3

forLoop:
	CMP R8, R9                   ; pcmQueue < (pcmQueue + queueSize);
	JNB exitForLoop
	; Do the work of the for loop

	; MOV AL, byte ptr [pcmQueue + R9]
	VMULSS XMM0, XMM3, XMM1       ; q0 = toneStruct->coeff * q1
	VSUBSS XMM0, XMM0, XMM2       ; q0 -= q2

	; Done inside the for loop
	INC R8                        ; i++
	JMP forLoop


exitForLoop:

	RET

goertzel_magnitude_64 ENDP

END
