;
;   E.COM : Execute from Archive File
;   Version 0.3
;
;   Nov.12,1989 by autumn
;

;----------------------------------------------------------
; Environment :
;   General MS-DOS 3.x or more, not depends any hardwares.
;----------------------------------------------------------
; This program requires follow products :
;    in Assemble
;     Turbo Assembler 1.0 (or MASM compatible assembler)
;     AMACROS / Macro sets for MASM produced by ASCII
;     EXE2BIN
;
;    in Runing
;     LARC 3.33 ( or same specific versions )
;----------------------------------------------------------

; include AMACROS system
include	amscls.inc
include	amsdos.inc
include amssio.inc

; AMCROS initialize for "macro lines in list file" and "COM model"
$_init	GEN,COM

; This program is .COM model. comseg is only segment in this program.
comseg		segment
; assumes all segment register allows same segment.
		assume	cs:comseg, ds:comseg, es:comseg

; command line area
		org	0080h
nArgLength	db	?		;length of arg length by bytes
sArgString	db	7fh dup(?)	;arg string body

;
		org	0100h	;program entry point
start:		jmp	main	;vector to the main routine entry

StackSize	equ	512	;stack size by bytes

TAB_CODE	equ	9

; data definition area
szUsageMessage	db	"usage : e CommandName ArgsForCommands",0dh,0ah
		db	"Execute from Archive File",0dh,0ah
		db	0

; command name for execute
szCommandName	db	128 dup(?)







AnalizeCommandLine	proc	near
		; check argment whether null strings
		mov	cl,nArgLength
		mov	ch,0
		mov	si,offset nArgString
		; if no arg strings then put Usage and Error exit
		$_if	<or cx,cx>,E
			jmp	UsageAndError
		$_endif

; Skip with SPACE and TABs
MainLoop1:
		lodsb
		cmp	al," "
		jz	MainSkip1
		cmp	al,TAB_CODE
		jnz	MainGetCommandName
MainSkip1:
		loop	MainLoop1
		jmp	UsageAndError

; now we are in the head of command name string
; then get it to the buffer
MainGetCommandName:
		mov	di,offset szCommandName
MainLoop2:
		lodsb
		stosb
		cmp	al," "
		jz	MainSkip2
		cmp	al,TAB_CODE
		jz	MainSkip2
		loop	MainLoop2
MainSkip2:
		xor	al,al		;string terminate ZERO
		stosb			;into end of string

; now we must check strings empty
; if so, we must fill parameter block by null.
		cmp	cx,cx
		jnz	MainGetArgsForExec


; now we must skip
MainGetArgsForExec:




		ret
AnalizeCommandLine	endp








; main, initial running procedure.
main		proc	near

; Stack Area Alocate
StackInit	equ	(offset AllEnd + 1) / 2 * 2 + StackSize
		mov	ax,StackInit
		mox	sp,ax

; calculate spent/need memory by paragraph, and free memory not use
ParaMemEnd	equ	(StackInit + 15) / 16
		$_realloc	ParaMemEnd, ES
		$_if	<>,C
			jmp	MemoryError
		$_endif


		call	AnalizeCommandLine

		call	GetEnviron

		call	ExtractTarget

		call	ExecuteTarget

		call	DeleteTarget

		mov	al,nTargetReturnCode
		mov	ah,4ch
		int	21h

main		endp

; defining program end address
AllEnd		label	byte

comseg		ends

		end	start
