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
externdef gfScaleFactor:dword

.code

option casemap:none    ; Make symbols case sensitive

; Note:  One optimization (for the sale of simplicity) is that we are
;        analyzing the queue from position 0 to queueSize -- not from
;        the head of the queue.  This gets us down to 1 counter and 
;        makes the program easier to read without affecting its accuracy
;
; CL = The UINT8 index (param 1)
; RDX = The 64-bit pointer to the dtmfTone struct (param 2)
; R8 = pcmQueue (copied from external size_t)
; R9 = pcmQueue + queueSize (marks the end of the queue)
; R10 = Unused
; R11 = Unused
; XMM0 = q0
; XMM1 = q1
; XMM2 = q2
; XMM3 = coeff
; XMM4 = Scratch (PCM byte)
; XMM5 = Scratch
; 
public goertzel_magnitude_64
goertzel_magnitude_64 PROC

	XOR RAX, RAX                  ; Zero out RAX
	MOV R8, pcmQueue              ; Read from this point in the Queue
	MOV R9, R8
	ADD R9, queueSize             ; Read up to this position

	VPXOR XMM1, XMM1, XMM1        ; float q1 = 0;
	VPXOR XMM2, XMM1, XMM1        ; float q2 = 0;
	MOVSS XMM3, dword ptr [RDX + 56]  ; Copy toneStruct->coeff into XMM3

forLoop:
	CMP R8, R9                   ; pcmQueue < (pcmQueue + queueSize);
	JNB exitForLoop
	; Do the work of the for() loop

	MOV AL, byte ptr [R8]         ; Get the PCM byte from the queue
	VMULSS XMM0, XMM3, XMM1       ; q0 = toneStruct->coeff * q1
	CVTSI2SS XMM4, EAX            ; Copy the PCM byte into XMM4
	VSUBSS XMM0, XMM0, XMM2       ; q0 -= q2
	MOVSS XMM2, XMM1              ; q2 = q1
	VADDSS XMM0, XMM0, XMM4       ; q0 += the PCM byte
	MOVSS XMM1, XMM0              ; q1 = q0

	; Done inside the for() loop
	INC R8                        ; i++
	JMP forLoop

exitForLoop:

	MOVSS XMM4, dword ptr [RDX + 52] ; Get toneStruct->cosine
	MOVSS XMM5, dword ptr [RDX + 48] ; Get toneStruct->sine
	MULSS XMM4, XMM1                 ; q1 * toneStruct->cosine
	MULSS XMM5, XMM1                 ; q1 * toneStruct->sine
	SUBSS XMM4, XMM2                 ; q1 * toneStruct->cosine - q2
	MULSS XMM4, XMM4                 ; real * real
	MULSS XMM5, XMM5                 ; imag * imag
	ADDSS XMM4, XMM5                 ; real * real + imag * imag
	MOVSS XMM5, dword ptr [gfScaleFactor]
	SQRTSS XMM4, XMM4                ; Square root
	DIVSS XMM4, XMM5                 ; / gfScaleFactor
	MOVSS dword ptr [RDX + 44], XMM4 ; Store in toneStruct->goertzelMagnitude

	RET

goertzel_magnitude_64 ENDP

END
