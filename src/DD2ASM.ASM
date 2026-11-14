; /* target: DD2ASM */
; /* opts: /mx */
;
; DD2ASM.ASM: the game's screen routines in assembly (_TEXT 5576-6233, _DATA DS:1294-12CF).
;
; Tiles are 16x16 in four planes, 32 bytes a plane, each at the start of its own segment
; (the table of tile segments is IDLIBC.C's tilesegs). The map is mapplane[0], mapbwide
; bytes a row. All of these are called from the screen refresh in the main file.
;
; name: the file name is descriptive (no evidence of the original); so are the procedure
; names. The code is written as Hovertank 3-D's assembly is (IDEAL mode, MODEL SMALL,C,
; DS restored from SS at the end).
;

IDEAL
MODEL	SMALL,C

SC_INDEX	=	3C4h
SC_MAPMASK	=	2
GC_INDEX	=	3CEh
GC_DATAROTATE	=	3
GC_MODE		=	5

SCREENWIDTH	=	64		;bytes a line

ROWTILES	=	21		;a row of tiles across the screen
COLUMNTILES	=	14		;a column of tiles down it

DATASEG

tilebuf		dw	30 dup (0)	;tile segments of the row or column being drawn

EXTRN	screenseg:WORD,linewidth:WORD
EXTRN	mapbwide:WORD,mapplane:DWORD
EXTRN	unk_9eea6a:WORD			;segment of each tile (IDLIBC.C; IDLIB.H calls it tilesegs)
tilesegs	EQU	unk_9eea6a

CODESEG

;============================
;
; DrawTileAt
;
; DrawTileAt (unsigned source, unsigned dest): one tile, whose segment is source, at
; screen offset dest, through DrawTile. Called from C (DD2ACTS.C's status box).
; The game has it here, not in DD2ACTS.C: it is the first thing in this module, and the
; EVEN after it (a 90 nop) puts DrawTile on a word, which C code would not do.
;
;============================

PROC	DrawTileAt	source:WORD, dest:WORD
USES	si,di
PUBLIC	DrawTileAt
	mov	bx,[linewidth]
	dec	bx
	dec	bx
	mov	dx,SC_INDEX
	mov	es,[screenseg]
	mov	di,[dest]
	mov	ds,[source]
	call	DrawTile
	mov	ax,ss
	mov	ds,ax
	ret
ENDP

EVEN

;============================
;
; DrawTile
;
; ds:0 = the tile, es:di = where on the screen, bx = linewidth-2, dx = SC_INDEX
; Leaves di 15 lines down and two bytes right of where it started
;
;============================

PROC	DrawTile
PUBLIC	DrawTile

	mov	cx,di
	xor	si,si

	mov	ax,SC_MAPMASK+1*256
	out	dx,ax
REPT	15
	movsw
	add	di,bx
ENDM
	movsw

	mov	ax,SC_MAPMASK+2*256
	out	dx,ax
	mov	di,cx
REPT	15
	movsw
	add	di,bx
ENDM
	movsw

	mov	ax,SC_MAPMASK+4*256
	out	dx,ax
	mov	di,cx
REPT	15
	movsw
	add	di,bx
ENDM
	movsw

	mov	ax,SC_MAPMASK+8*256
	out	dx,ax
	mov	di,cx
REPT	15
	movsw
	add	di,bx
ENDM
	movsw

	ret

ENDP


;============================
;
; DrawMapRow (unsigned mapofs, unsigned dest)
;
; A row of ROWTILES tiles from the map at mapofs, left to right from dest
;
;============================

PROC	DrawMapRow mapofs:WORD, dest:WORD
USES	SI,DI
PUBLIC	DrawMapRow

	mov	bx,[mapofs]
	mov	es,[WORD mapplane+2]
	mov	cx,[mapbwide]

count	=	0
REPT	ROWTILES
	mov	si,[es:bx]
	inc	bx
	inc	bx
	shl	si,1
	mov	si,[tilesegs+si]
	mov	[tilebuf+count],si
count	=	count+2
ENDM

	mov	bx,[linewidth]
	dec	bx
	dec	bx
	mov	dx,SC_INDEX
	mov	es,[screenseg]
	mov	di,[dest]
	mov	bp,[linewidth]		;back up 15 lines to the next tile's top
	shl	bp,1
	shl	bp,1
	shl	bp,1
	shl	bp,1
	sub	bp,[linewidth]

count	=	0
REPT	ROWTILES
	mov	ds,[ss:tilebuf+count]
	call	DrawTile
	sub	di,bp
count	=	count+2
ENDM

	mov	ax,ss
	mov	ds,ax
	ret

ENDP


;============================
;
; DrawMapColumn (unsigned mapofs, unsigned dest)
;
; A column of COLUMNTILES tiles from the map at mapofs, top to bottom from dest
;
;============================

PROC	DrawMapColumn mapofs:WORD, dest:WORD
USES	SI,DI
PUBLIC	DrawMapColumn

	mov	bx,[mapofs]
	mov	es,[WORD mapplane+2]
	mov	cx,[mapbwide]

count	=	0
REPT	COLUMNTILES
	mov	si,[es:bx]
	add	bx,cx
	shl	si,1
	mov	si,[tilesegs+si]
	mov	[tilebuf+count],si
count	=	count+2
ENDM

	mov	bx,[linewidth]
	dec	bx
	dec	bx
	mov	dx,SC_INDEX
	mov	es,[screenseg]
	mov	di,[dest]

count	=	0
REPT	COLUMNTILES
	mov	ds,[ss:tilebuf+count]
	call	DrawTile
	add	di,bx			;down to the next tile's top
count	=	count+2
ENDM

	mov	ax,ss
	mov	ds,ax
	ret

ENDP


;============================
;
; CopyBand (unsigned source, unsigned dest)
;
; Copies 8 lines of 42 bytes within screen memory
;
;============================

PROC	CopyBand source:WORD, dest:WORD
USES	SI,DI
PUBLIC	CopyBand

	mov	bx,42
	mov	dx,SCREENWIDTH-42
	mov	si,[source]
	mov	di,[dest]
	mov	ax,[screenseg]
	mov	ds,ax
	mov	es,ax

REPT	7
	mov	cx,bx
	rep	movsb
	add	si,dx
	add	di,dx
ENDM
	mov	cx,bx
	rep	movsb

	mov	ax,ss
	mov	ds,ax
	ret

ENDP


;============================
;
; CopyColumn (unsigned source, unsigned dest)
;
; Copies a column one byte wide and 224 lines high within screen memory
;
;============================

PROC	CopyColumn source:WORD, dest:WORD
USES	SI,DI
PUBLIC	CopyColumn

	mov	si,[source]
	mov	di,[dest]
	mov	ds,[screenseg]

line	=	0
REPT	224
	mov	al,[si+line]
	mov	[di+line],al
line	=	line+SCREENWIDTH
ENDM

	mov	ax,ss
	mov	ds,ax
	ret

ENDP


;============================
;
; XorPic (int wide, int height, unsigned source, unsigned num, unsigned dest)
;
; Xors the inverse of a one-plane picture (source:num*4) onto all planes
;
;============================

PROC	XorPic wide:WORD, height:WORD, source:WORD, num:WORD, dest:WORD
USES	SI,DI
PUBLIC	XorPic

	cli
	mov	dx,GC_INDEX
	mov	ax,GC_MODE
	out	dx,ax
	mov	ax,GC_DATAROTATE+10h*256	;xor
	out	dx,ax
	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax
	sti

	mov	si,[wide]
	mov	bx,SCREENWIDTH
	sub	bx,si
	mov	dx,[height]
	mov	si,[num]
	shl	si,1
	shl	si,1
	mov	di,[dest]
	mov	es,[screenseg]
	mov	ds,[source]

@@lineloop:
	mov	cx,[wide]
@@byteloop:
	mov	al,[es:di]		;load the latches
	lodsb
	not	al
	stosb
	loop	@@byteloop
	add	di,bx
	dec	dx
	jnz	@@lineloop

	mov	dx,GC_INDEX
	mov	ax,GC_DATAROTATE
	out	dx,ax
	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax

	mov	ax,ss
	mov	ds,ax
	ret

ENDP


;============================
;
; CopyScreen (int wide, int height, unsigned source, unsigned dest)
;
; Copies a block within the screen at A000 (write mode 1 does the planes)
;
;============================

PROC	CopyScreen wide:WORD, height:WORD, source:WORD, dest:WORD
USES	SI,DI
PUBLIC	CopyScreen

	mov	ax,0A000h
	mov	ds,ax
	mov	es,ax
	mov	bl,[BYTE wide]
	mov	bh,[BYTE height]
	mov	di,[dest]
	mov	si,[source]
	mov	dx,SCREENWIDTH
	sub	dl,bl
	xor	ch,ch
@@lineloop:
	mov	cl,bl
	rep	movsb
	add	si,dx
	add	di,dx
	dec	bh
	jnz	@@lineloop

	mov	ax,ss
	mov	ds,ax
	ret

ENDP

END
