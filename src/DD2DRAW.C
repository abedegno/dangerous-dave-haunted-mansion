/* target: DRAW */
/* opts: -ms -O -G -Z -k- */
/*
 * DD2DRAW.C: the playfield's refresh for Dangerous Dave in the Haunted Mansion.
 *
 * Three screens: two pages shown in turn (bufferofs, +0x5000) and a master copy of the
 * map at +0xA000. Scrolling moves the origin and leaves columns and rows of tiles to draw
 * into the master (upxl..upyh[2]) and to copy from it into each page (upxl..upyh[page]).
 * Sprites go into the page's update list as they are drawn, and are erased from the
 * master the next time the page comes round.
 *
 * name: the file name is descriptive (Hovertank 3-D has HOVDRAW.C); so are the function
 * and variable names. Code _TEXT 2573-2DF4, no _DATA, _BSS DS:908C-9097. Link order puts
 * it between the main file and the actors (from the _BSS: 908C-9097 is used only here and
 * by the main file).
 */

#include "IDLIB.H"
#include "DD2DEF.H"

void DrawMapRow (unsigned mapofs, unsigned dest);
void DrawMapColumn (unsigned mapofs, unsigned dest);
void CopyBand (unsigned source, unsigned dest);
void CopyColumn (unsigned source, unsigned dest);
void XorPic (int wide, int height, unsigned source, unsigned num, unsigned dest);
void CopyScreen (int wide, int height, unsigned source, unsigned dest);
void SetScreen (unsigned crtc, unsigned pel);

#define PAGESIZE	0x5000		// two pages, then the master
#define MASTEROFS	0xa000
#define UPDATEWIDE	42		// bytes of tiles across a page
#define UPDATEHIGH	28		// 8-line rows of tiles down it

extern spritetype image;
extern spritetype _seg *spritetable;
extern unsigned spritesegs[];

// the main file's

//
// _BSS: Borland C++ 2.0's _BSS order (ascending hash of the name from 960) puts these
// where the game has them
//
updatetype	*endupdate;		// DS:908C: the end of the page's update list
unsigned	framecount;		// DS:908E
unsigned	originxglobal,originyglobal;	// DS:9090: the scroll origin, 1/256 tile
int		panx,pany;		// DS:9094: pel and line panning


/*
=================
=
= CalcTics
=
=================
*/

void CalcTics (void)
{
	long newtime;

	endupdate = updatelist[page];
	endupdate->wide = 0;

	newtime = timecount & 0xfffffffeL;
	tics = (newtime-lasttimecount)/2;
	lasttimecount = newtime;
}


/*
=================
=
= RefreshScreen
=
=================
*/

void RefreshScreen (void)
{
	unsigned mapofs,base,crtc,pel,wide;
	register unsigned master,ofs;

	endupdate->wide = 0;

	EGAWRITEMODE(0);
	screenofs = page ? bufferofs+PAGESIZE : bufferofs;
	master = page ? screenofs+PAGESIZE : screenofs+MASTEROFS;
	mapofs = (unsigned)(mapplane[0]+(originytile*mapwwide+originxtile));

//
// draw new tiles into the master
//
	while (upxl[2] > 0)
	{
		upxl[2] -= 2;
		DrawMapColumn (mapofs+upxl[2],master+upxl[2]);
	}
	while (upxh[2] < UPDATEWIDE)
	{
		DrawMapColumn (mapofs+upxh[2],master+upxh[2]);
		upxh[2] += 2;
	}
	while (upyl[2] > 0)
	{
		upyl[2] -= 2;
		DrawMapRow (mapofs+upyl[2]/2*mapbwide,master+upyl[2]*SCREENWIDTH*8);
	}
	while (upyh[2] < UPDATEHIGH)
	{
		DrawMapRow (mapofs+upyh[2]/2*mapbwide,master+upyh[2]*SCREENWIDTH*8);
		upyh[2] += 2;
	}

//
// copy them into this page
//
	EGAWRITEMODE(1);
	EGAMAPMASK(15);
	while (upxl[page] > 0)
	{
		upxl[page]--;
		CopyColumn (master+upxl[page],screenofs+upxl[page]);
	}
	while (upxh[page] < UPDATEWIDE)
	{
		CopyColumn (master+upxh[page],screenofs+upxh[page]);
		upxh[page]++;
	}
	while (upyl[page] > 0)
	{
		upyl[page]--;
		ofs = upyl[page]*8*SCREENWIDTH;
		CopyBand (master+ofs,screenofs+ofs);
	}
	while (upyh[page] < UPDATEHIGH)
	{
		ofs = upyh[page]*8*SCREENWIDTH;
		CopyBand (master+ofs,screenofs+ofs);
		upyh[page]++;
	}

//
// draw the sprites
//
	EGAWRITEMODE(0);
	endupdate = updatelist[page];
	while (wide = endupdate->wide, wide)	// match: the test reads wide back (cmp [wide],0)
	{
		if (!(wide & 0x8000))
			DrawSpriteT (wide,endupdate->height,endupdate->source,endupdate->dest,
				endupdate->planesize);
		else
			XorPic (wide & 0x7fff,endupdate->height,endupdate->source,
				endupdate->planesize,endupdate->dest);
		endupdate++;
	}

//
// show it
//
	crtc = screenofs + ((originxglobal&0x80)!=0) + ((originyglobal>>4)&15)*SCREENWIDTH;
	screenofs = crtc;
	pel = (originxglobal>>4)&4;
	SetScreen (crtc,pel);
	page ^= 1;
	framecount++;

//
// erase the other page's sprites
//
	EGAWRITEMODE(1);
	EGAMAPMASK(15);
	base = page ? PAGESIZE : MASTEROFS;
	endupdate = updatelist[page];
	if (statuscount)
	{
		statuscount--;
		CopyScreen (4,48,bufferofs+statusofs+MASTEROFS,bufferofs+statusofs+page*PAGESIZE);
	}
	while (endupdate->wide)
	{
		CopyScreen (endupdate->wide & 0x7fff,endupdate->height,endupdate->dest+base,
			endupdate->dest);
		endupdate++;
	}
}


/*
=================
=
= InitRefresh
=
= Everything is drawn again
=
=================
*/

void InitRefresh (void)
{
	register int i;

	originxtile = oldxtile = originxglobal>>8;
	originytile = oldytile = originyglobal>>8;

	for (i=0;i<3;i++)
	{
		upxl[i] = UPDATEWIDE;
		upxh[i] = UPDATEWIDE;
		upyl[i] = 0;
		upyh[i] = UPDATEHIGH;
	}

	updatelist[1][0].wide = updatelist[0][0].wide = 0;
	statuscount = 0;
}


/*
=================
=
= ScrollOrigin
=
= The origin has moved: move the screens with it, and what is left to draw
=
=================
*/

void ScrollOrigin (void)
{
	int ydelta,move;
	register int i,xdelta;

	originxtile = originxglobal>>8;
	originytile = originyglobal>>8;
	panx = (originxglobal&0x80)>>7;
	pany = (originyglobal&0xf0)>>4;

	xdelta = originxtile-oldxtile;
	ydelta = originytile-oldytile;
	oldxtile = originxtile;
	oldytile = originytile;

	move = xdelta*2 + ydelta*1024;
	bufferofs += move;

	xdelta *= 2;
	ydelta *= 2;

	for (i=0;i<3;i++)
	{
		upxl[i] -= xdelta;
		upxh[i] -= xdelta;
		upyl[i] -= ydelta;
		upyh[i] -= ydelta;

		if (upxl[i] > UPDATEWIDE)
			upxl[i] = UPDATEWIDE;
		else if (upxl[i] < 0)
			upxl[i] = 0;

		if (upxh[i] < 0)
			upxh[i] = 0;
		else if (upxh[i] > UPDATEWIDE)
			upxh[i] = UPDATEWIDE;

		if (upyl[i] > UPDATEHIGH)
			upyl[i] = UPDATEHIGH;
		else if (upyl[i] < 0)
			upyl[i] = 0;

		if (upyh[i] < 0)
			upyh[i] = 0;
		if (upyh[i] > UPDATEHIGH)
			upyh[i] = UPDATEHIGH;
	}
}


/*
=================
=
= QueueSprite
=
= Puts a sprite into the update list if any of it is on the screen. x and y are in 1/16
= pixels; the sprite's shift comes from x
=
=================
*/

int QueueSprite (register int x, int y, int num)
{
	int ybyte,shapenum,dest;
	register int xbyte;

	x += spritetable[num*4].xofs;
	y += spritetable[num*4].yofs;

	xbyte = ((x&~15) - (int)(originxglobal&0xff00))/16;
	ybyte = (y - (int)(originyglobal&0xff00) + 72*16)/16 - 72;
	shapenum = num*4 + (xbyte&7)/4;
	image = spritetable[shapenum];
	xbyte = (xbyte+64)/8 - 8;

	if (xbyte+image.width <= panx || ybyte+image.height <= pany
	|| panx+40 < xbyte || pany+199 < ybyte)
		return 0;

	dest = bufferofs+page*PAGESIZE+ybyte*SCREENWIDTH+xbyte;
	endupdate->wide = image.width;
	endupdate->height = image.height;
	endupdate->source = spritesegs[shapenum];
	endupdate->dest = dest;
	endupdate->planesize = image.planesize;
	endupdate++;
	return 1;
}


/*
=================
=
= QueueXorSprite
=
= The same, xored onto the screen
=
=================
*/

int QueueXorSprite (register int x, int y, int num)
{
	int ybyte,shapenum;
	register int xbyte;

	x += spritetable[num*4].xofs;
	y += spritetable[num*4].yofs;

	xbyte = (x - (int)(originxglobal&0xff00))/16;
	ybyte = (y - (int)(originyglobal&0xff00) + 72*16)/16 - 72;
	shapenum = num*4 + (xbyte&7)/4;
	image = spritetable[shapenum];
	xbyte = (xbyte+64)/8 - 8;

	if (xbyte+image.width <= panx || ybyte+image.height <= pany
	|| panx+40 < xbyte || pany+199 < ybyte)
		return 0;

	endupdate->wide = image.width | 0x8000;
	endupdate->height = image.height;
	endupdate->source = spritesegs[shapenum];
	endupdate->dest = bufferofs+page*PAGESIZE+ybyte*SCREENWIDTH+xbyte;
	endupdate->planesize = image.planesize;
	endupdate++;
	return 1;
}


/*
=================
=
= QueueSpriteAt
=
= Puts a sprite into the update list unclipped, with no position offsets
=
=================
*/

void QueueSpriteAt (int x, int y, int num)
{
	int ybyte,dest;
	register int xbyte,shapenum;

	xbyte = (x - (int)(originxglobal&0xff00))/16;
	ybyte = (y - (int)(originyglobal&0xff00))/16;
	shapenum = num*4 + (xbyte&7)/4;
	image = spritetable[shapenum];
	xbyte = (xbyte+64)/8 - 8;

	dest = bufferofs+page*PAGESIZE+ybyte*SCREENWIDTH+xbyte;
	endupdate->wide = image.width;
	endupdate->height = image.height;
	endupdate->source = spritesegs[shapenum];
	endupdate->dest = dest;
	endupdate->planesize = image.planesize;
	endupdate++;
}
