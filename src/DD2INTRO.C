/* target: INTRO */
/* opts: -ms -O -G -Z */
/*
 * DD2INTRO.C: the title sequence of Dangerous Dave in the Haunted Mansion.
 *
 * Shows STARPIC.DD2 with the copyright line on both screen pages, flies the INTRO.DD2
 * picture in along a curve, scaled through ScaleLine (IDASME.ASM) with the PC speaker
 * sweeping, then waits a few seconds; 'I' shows the Gamer's Edge panel.
 *
 * name: the file name is descriptive (no evidence of the original). Its code is
 * _TEXT 6234-6B9F, its strings DS:272A-2827 and its _BSS DS:90AC-934B; link order puts it
 * after the map header object and before IDLIBC.C. Function and variable names are
 * provisional; the _BSS names are chosen so Borland's _BSS order puts each where the game
 * has it.
 */

#include <math.h>
#include <dos.h>
#include "IDLIB.H"
#include "DD2DEF.H"

void SetLineWidth (int width);
void CopyEGA (int wide, int height, unsigned source, unsigned dest);
void SetScreen (unsigned crtc, unsigned pel);
void ScaleLine (int pixels, byte far *scaleptr, byte far *picptr, byte far *screen);
void Ack (void);


/*
 * A compiled shape: for each column of pixels (each byte column at each of 8 shifts), the
 * offset of its run: a first-row byte, a count byte, then the pixels' colours.
 */
typedef struct
{
	int		width,height;
	unsigned	columns[256];	// 32 byte columns x 8 shifts
} shapetype;

//
// _BSS: the scaler's state. Borland C++ 2.0 lays out _BSS in ascending order of a hash of
// each name, starting from hash 960 (see notes/fingerprint.md); these names keep the
// game's order.
//
unsigned	scrleft,scrtop;			// DS:90AC
int		updx,updy;		// DS:90B0: the last scaled draw, in bytes and lines
unsigned	scrright,scrbottom;			// DS:90B4
memptr		shapemem;			// DS:90B8: the compiled intro picture
byte far	*shrinktab;			// DS:90BA: scaled size to source, per scale
unsigned	updateofs,updheight,updwidth;	// DS:90BE
memptr		tabmem;			// DS:90C4: both tables, 64K
memptr		rawpic;			// DS:90C6: INTRO.DD2 while it is compiled
byte		pixbyte[320];			// DS:90C8: byte column of each pixel
byte far	*growtab;			// DS:9208: source size to scaled, per scale
byte		pixelmask[320];			// DS:920C: bit mask of each pixel


/*
=================
=
= CompileShape
=
= Builds a shapetype from a picture of four planes, width bytes by height lines
=
=================
*/

void CompileShape (byte far *source, int width, int height, shapetype far *shape)
{
	byte far *plane0,far *plane1,far *plane2,far *plane3;
	byte c0,c1,c2,c3;
	int found,first,count,x,shift,pixel,rot;
	byte far *out,far *header;
	register int srcofs,y;

	shape->width = width*8-1;
	shape->height = height;
	out = (byte far *)(shape->columns + width*8);

	plane0 = source;
	plane1 = plane0 + width*height;
	plane2 = plane1 + width*height;
	plane3 = plane2 + width*height;

	for (x=0;x<width;x++)
		for (shift=0;shift<8;shift++)
		{
			rot = 8-shift;
			srcofs = x;
			*(shape->columns + x*8 + shift) = (unsigned)out;
			header = out;
			out += 2;
			first = count = found = 0;
			for (y=0;y<height;y++)
			{
				c0 = plane0[srcofs];
				c1 = plane1[srcofs];
				c2 = plane2[srcofs];
				c3 = plane3[srcofs];
				srcofs += width;
				pixel = 0;
asm				mov	cl,BYTE PTR rot
asm				mov	al,c3
asm				rcr	al,cl
asm				rcl	BYTE PTR pixel,1
asm				mov	cl,BYTE PTR rot
asm				mov	al,c2
asm				rcr	al,cl
asm				rcl	BYTE PTR pixel,1
asm				mov	cl,BYTE PTR rot
asm				mov	al,c1
asm				rcr	al,cl
asm				rcl	BYTE PTR pixel,1
asm				mov	cl,BYTE PTR rot
asm				mov	al,c0
asm				rcr	al,cl
asm				rcl	BYTE PTR pixel,1
				if (pixel != 1)
				{
					if (!found)
					{
						found = 1;
						first = y;
					}
					count++;
					*out++ = pixel;
				}
				else if (found)
					break;
			}
			*header = first;
			*(header+1) = count;
		}
}


/*
=================
=
= InitScaler
=
=================
*/

void InitScaler (void)
{
	unsigned mask;
	byte far *grow,far *shrink;
	register unsigned i,size;

	mask = 0x80;
	for (i=0;i<320;i++)
	{
		pixbyte[i] = i/8;
		pixelmask[i] = mask;
		if (!(mask >>= 1))
			mask = 0x80;
	}

	MMGetPtr (&tabmem,0x1000);
	growtab = grow = (byte far *)tabmem;
	shrinktab = shrink = growtab + 0x8000;

	for (size=2;size<=256;size+=2)
		for (i=0;i<256;i++)
		{
			*grow++ = (size*i)>>8;
			*shrink++ = (i<<8)/size;
		}

	scrleft = 0;
	scrtop = 0;
	scrright = 319;
	scrbottom = 179;
}


/*
=================
=
= DrawScaled
=
= Draws a compiled shape centred on x,y at a scale (1-512), clipped to the clip window;
= updx/y/width/height/ofs say what it touched
=
=================
*/

void DrawScaled (unsigned x, unsigned y, unsigned scale, unsigned far *shape)
{
	unsigned width,height,swidth,sheight;
	unsigned left,right,top,bottom;
	int px,cleft,cright,ctop,cbottom;
	unsigned dest;
	int first,col,skip;
	byte far *grow,far *shrink,far *src,far *screen;
	register int count,ystart;

	scale /= 2;
	if (scale > 0)
		scale--;
	grow = growtab + (scale<<8);
	shrink = shrinktab + (scale<<8);

	updwidth = 0;
	width = *shape++;
	height = *shape++;
	swidth = grow[width];
	sheight = grow[height];

	left = x - swidth/2;
	right = left+swidth;
	top = y - sheight/2;
	bottom = top+sheight;

	if (left > scrright || right < scrleft || bottom < scrtop || top > scrbottom)
		return;

asm	mov	ax,GC_MODE+256*2		// write mode 2
asm	mov	dx,GC_INDEX
asm	out	dx,ax
asm	mov	ax,SC_MAPMASK+15*256
asm	mov	dx,SC_INDEX
asm	out	dx,ax

	cleft = left < scrleft ? scrleft : left;
	cright = right > scrright ? scrright : right;
	ctop = top < scrtop ? scrtop : top;
	cbottom = bottom > scrbottom ? scrbottom : bottom;

	updx = cleft/8;
	updy = ctop;
	updateofs = bufferofs + updx + ylookup[updy];
	updwidth = cright/8 - updx + 1;
	updheight = cbottom - ctop + 1;

	for (px=cleft;px<cright;px++)
	{
asm		mov	bx,px
asm		mov	ah,[BYTE PTR pixelmask+bx]
asm		mov	al,GC_BITMASK
asm		mov	dx,GC_INDEX
asm		out	dx,ax
asm		mov	al,[BYTE PTR pixbyte+bx]
asm		xor	ah,ah
asm		mov	dest,ax
		col = shrink[px-left];
		src = MK_FP(FP_SEG(shape),*(shape+col+2));
		first = grow[*src++];
		count = grow[*src++];
		ystart = top+first;
		if (ystart < scrtop)
		{
			skip = scrtop-ystart;
			src += skip;
			count -= skip;
			ystart = scrtop;
		}
		if (ystart+count > scrbottom)
			count = scrbottom-ystart;
		if (count > 0)
		{
			dest += ylookup[top+first]+bufferofs;
			screen = MK_FP(0xa000,dest);
			ScaleLine (count,shrink,src,screen);
		}
	}

asm	mov	ax,GC_MODE				// write mode 0
asm	mov	dx,GC_INDEX
asm	out	dx,ax
asm	mov	ax,GC_BITMASK+255*256
asm	mov	dx,GC_INDEX
asm	out	dx,ax
}


/*
=================
=
= Intro
=
=================
*/

#define PI	3.141592657			// sic: the digits after 3.1415926 are off; the folded
					// ANGLE matches only with these
#define ANGLE	(PI*0.6)			// the arc the picture flies along
#define STEPS	300.0

void Intro (void)
{
	byte far *source;
	shapetype far *dest;
	int i,x,y;
	unsigned upofs[2],upwidth[2],upheight[2];
	float wx,wy,z,angle,t,sina,cosa,focal,scale,ystep,xradius,startz,yoffset,xoffset;
	register int page,frame;

	screenofs = 0;
	Bar (0,0,40,200,0);
	screenofs = 0x4000;
	Bar (0,0,40,200,0);
	SetLineWidth (80);
	LoadPage ("STARPIC.DD2",0);
	EGAWRITEMODE(1);
	EGAMAPMASK(15);
	CopyEGA (40,200,0,0x4000);
	CopyEGA (40,200,0,0x8000);
	InitScaler ();
	StopDrive ();

	for (bufferofs=0;bufferofs<=0x4000;bufferofs+=0x4000)
	{
		pxl = 0;
		pxh = 320;
		py = 180;
		screenofs = bufferofs;
		CPPrint ("Copyright (c) 1991 Softdisk inc.\n");
		CPPrint ("'I' for information");
	}

	BloadinRLEMM ("INTRO.DD2",&rawpic);
	MMGetPtr (&shapemem,0xf00);
	source = (byte far *)rawpic;
	dest = (shapetype far *)shapemem;
	CompileShape (source,32,64,dest);
	MMFreePtr (&rawpic);

	x = 160;
	y = 180;
	startz = cos(ANGLE)*1000;
	startz = startz+1000;
	focal = 256*startz;
	ystep = 80 - focal/1000*32/256;
	z = focal/256000;
	ystep = ystep/z;
	yoffset = ystep-1000;
	xradius = sin(ANGLE)*1000/2;
	xoffset = -xradius;

	frame = page = 1;
	inttime = bufferofs = upwidth[0] = upwidth[1] = 0;

	do
	{
		t = frame/STEPS;
		angle = t*ANGLE;
		sina = sin(angle);
		cosa = cos(angle);
		wx = xoffset + sina*1000.0/2;
		wy = yoffset + sina*1000.0;
		z = cosa*1000+1000;
		scale = focal/z;
		x = (int)(wx*scale/256)+160;
		y = 100-(int)(wy*scale/256);
		inttime = 0;
		sound (sina*1500);

		if (upwidth[page])
		{
			EGAWRITEMODE(1);
			EGAMAPMASK(15);
			CopyEGA (upwidth[page],upheight[page],
				upofs[page]+(page ? 0x4000 : 0x8000),upofs[page]);
		}
		DrawScaled (x,y,scale,(unsigned far *)shapemem);
		upwidth[page] = updwidth;
		upheight[page] = updheight;
		upofs[page] = updateofs;

		SetScreen (bufferofs,0);
		page ^= 1;
		bufferofs = screenofs = page<<14;

		frame++;
		if (frame < STEPS)
		{
			frame = frame+inttime;
			if (frame > STEPS)
				frame = 300;
		}
		else
			frame++;
	} while (NBKscan <= 127 && frame <= STEPS);

	nosound ();

	for (i=0;i<200;i++)
	{
		WaitVBL (1);
		if (NBKscan > 127)
		{
			if (NBKscan == 0x97)
			{
				screenofs = bufferofs^0x4000;
				CenterWindow (24,8);
				py += 2;
				CPPrint ("Gamer's Edge delivers a\n");
				CPPrint ("high quality EGA game to\n");
				CPPrint ("your door every month!\n");
				CPPrint ("Call 1-800-831-2694 for\n");
				CPPrint ("subscription rates and\n");
				CPPrint ("back issues.\n");
				ClearKeys ();
				Ack ();
			}
			ClearKeys ();
			break;
		}
	}

	MMFreePtr (&shapemem);
	MMFreePtr (&tabmem);
}
