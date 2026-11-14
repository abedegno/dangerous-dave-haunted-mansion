/* target: MAIN */
/* opts: -ms -a -O -G -Z -k- -K -y */
/*
 * DD2MAIN.C: the main file of Dangerous Dave in the Haunted Mansion: the help and status
 * windows, the function keys, loading, the object list and its movement code, the game
 * loop, game over and the high scores.
 *
 * name: the file name is descriptive (Hovertank 3-D has HOVMAIN.C); so are most function
 * and variable names. Code _TEXT 0239-2572, _DATA DS:00AA-0743, _BSS DS:4AB0-908B.
 */

#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include "IDLIB.H"
#include "DD2DEF.H"

//
// _BSS (DS:4AB0-908B), in address order: see DD2DEF.H
//
int	kbdstarted;
unsigned	originxmax;
unsigned	originymax;
int	soundstarted;
unsigned	originxmin;
unsigned	originymin;
int	godmode;
long	grplanesize;
byte	*northwall;
byte	*southwall;
int	joystickok;
int	loadedlevel;
unsigned	playerxmax;
unsigned	playerymax;
unsigned	playerxmin;
unsigned	playerymin;
int	numpicm;
char	chunkneeded[5];
unsigned	titletics;
int	numpics;
char	spriteneeded[800];
int	noendscreen;
unsigned	*tinf;
long	unused4E02;
unsigned	introdest;
byte	*eastwall;
grheadtype _seg	*grhead;
byte	*westwall;
unsigned	titletime;
unsigned	introheight;
int	introshape;
unsigned	introplanesize;
int	numtile8;
unsigned	introwidth;
int	numsprites;
byte	*animtile;
char	tileneeded[1024];
int	needsprites[10];
char	*tilenames;
byte	*intile;
int	debugok;
int	handle;
objtype	objlist[MAXOBJECTS];
ControlStruct	c;
int	lastdir;
int	jumpheld;
int	unused8BEC;
objtype	*new;
objtype	*check;
long	savetime;
int	fireready;
int	lives;
int	lastobjindex;
objtype	*lastobj;
objtype	*ob;
long	unused8C00;
objtype	obj;
int	fadein;
long	unused8C60;
int	resetgame;
int	singlestep;
int	playstate;
long	nextextra;
long	lasttimecount;
int	unused8C72;
int	originxtile;
int	unused8C76;
int	upxl[3];
int	upxh[3];
int	upyl[3];
int	upyh[3];
int	statuscount;
int	oldxtile;
int	oldytile;
int	page;
int	originytile;
unsigned	tics;
unsigned	bufferofs;
unsigned	statusofs;
updatetype	updatelist[2][50];
long	unused9088;

void	GivePoints (unsigned x, unsigned y, int type);
void	InitRefresh (void);
void	SetScreen (unsigned crtc, unsigned pel);
unsigned MMUnusedMemory (void);
unsigned MMTotalFree (void);
void	PPrintUnsigned (unsigned value);
void	Ack (void);
void	SetNormalPalette (void);

extern char		keydown[128];
extern unsigned		totalmem;
extern char		str[80];
extern unsigned		maphead[];		// DS:12D0: the map header object
extern char far		endscreen[4000];	// segment 112F: the text screen shown on quitting
extern memptr		levelseg;		// the level's memory (IDLIBC.C)

void	SpriteTest (void);
void	ScanInfoPlane (void);



/*
=================
=
= HelpScreen
=
=================
*/

void HelpScreen (void)
{
	CenterWindow (30,22);
	CPPrint ("Dangerous Dave Commands\n");
	CPPrint ("-----------------------\n\n");
	PPrint ("F2  : Sound on / off\n");
	PPrint ("F3  : Keyboard mode / custom\n");
	PPrint ("F4  : Joystick mode\n");
	PPrint ("F5  : Reset game\n");
	PPrint ("ESC : Quit\n");
	PPrint ("TAB : Status window\n\n");
	PPrint ("CTRL: Button 1 (jump)\n");
	PPrint ("  DOWN + CTRL : Drop down\n");
	PPrint ("ALT : Button 2 (fire)\n");
	PPrint ("UP / DOWN : Aim gun\n");
	PPrint ("LEFT / RIGHT : move\n");
	CPPrint ("");
	PGet ();
}


/*
=================
=
= MemoryScreen
=
=================
*/

void MemoryScreen (void)
{
	CenterWindow (16,7);
	CPPrint ("Memory Usage\n");
	CPPrint ("------------");
	PPrint ("\nTotal     :");
	PPrintUnsigned (totalmem>>6);
	PPrint ("k\nFree      :");
	PPrintUnsigned (MMUnusedMemory()>>6);
	PPrint ("k\nWith purge:");
	PPrintUnsigned (MMTotalFree()>>6);
	PPrint ("k\n");
	CPPrint ("");
	PGet ();
}


/*
=================
=
= DebugKeys
=
=================
*/


void DebugKeys (void)
{
	if (keydown[0x22])		// G
	{
		ExpWin (12,1);
		if (godmode)
			CPPrint ("God mode off");
		else
			CPPrint ("God mode on");
		Ack ();
		godmode ^= 1;
	}
	else if (keydown[0x32])		// M
		MemoryScreen ();
	else if (keydown[0x19])		// P
		Ack ();
	else if (keydown[0x1f])		// S
		singlestep = 1;
	else if (keydown[0x14])		// T
	{
		SpriteTest ();
		InitRefresh ();
	}
	else if (keydown[0x11] && debugok)	// W
	{
		ExpWin (24,1);
		PPrint ("Warp to which level(1-8):");
		ch = PGet ();
		if (ch >= '1' && ch <= '8')
		{
			level = ch-'1';
			playstate = ex_completed;
		}
		InitRefresh ();
	}
	else if (keydown[0x21] && debugok)	// F
		GivePoints (objlist[0].x,objlist[0].y,5);
}


/*
=================
=
= CheckKeys
=
= The function keys; true if a window was shown
=
=================
*/


int CheckKeys (void)
{
	int n;
	register int x,i;

	if (!NBKscan)
		return 0;

	switch (NBKscan&0x7f)
	{
	case 0x3b:			// F1
		ClearKeys ();
		HelpScreen ();
		break;

	case 0x3c:			// F2
		ClearKeys ();
		ExpWin (13,1);
		PPrint ("Sound (Y/N)?");
		ch = toupper(PGet());
		if (ch == 'N')
			soundmode = 0;
		else if (ch == 'Y')
			soundmode = 1;
		break;

	case 0x3d:			// F3
		ClearKeys ();
		calibratekeys ();
		break;

	case 0x3e:			// F4
		ClearKeys ();
		CalibrateJoy (1);
		break;

	case 0x3f:			// F5
		ClearKeys ();
		ExpWin (18,1);
		PPrint ("RESET GAME (Y/N)?");
		ch = toupper(PGet());
		if (ch == 'Y')
			resetgame = 1;
		break;

	case 0x58:			// F12
		DebugKeys ();
		break;

	case 1:				// ESC
		ClearKeys ();
		ExpWin (12,1);
		PPrint ("QUIT (Y/N)?");
		ch = toupper(PGet());
		if (ch == 'Y')
			Quit ("");
		break;

	case 0xf:			// TAB
		if (!debugok)
			return 0;
		ClearKeys ();
		ExpWin (14,14);
		py += 4;
		CPPrint ("Score\n");
		ltoa (score,str,10);
		CPPrint (str);
		PPrint ("\n\n");
		CPPrint ("High Score\n");
		ltoa (highscore,str,10);
		CPPrint (str);
		PPrint ("\n\n");
		CPPrint ("Daves\n");
		n = lives;
		if (n > 8)
			n = 8;
		x = 0xa4-n*8;
		for (i=1;i<n;i++,x+=16)
			DrawSprite (x,py,1);
		Ack ();
		break;

	default:
		return 0;
	}

	ClearKeys ();
	InitRefresh ();
	return 1;
}


/*
=================
=
= Quit
=
=================
*/


void Quit (char *error)
{
	if (handle)
		close (handle);
	if (!*error)
		SaveCtrls ();
	else
		puts (error);
	if (kbdstarted)
		ShutdownKbd ();
	if (soundstarted)
		ShutdownSound ();
	_AX = 3;
	geninterrupt (0x10);
	if (!*error && !noendscreen)
	{
		movedata (FP_SEG(endscreen),FP_OFF(endscreen),0xb800,0,4000);
		bioskey (0);
		clrscr ();
	}
	exit (0);
}


/*
=================
=
= DiskError
=
= The harderr handler
=
=================
*/

int DiskError (int errval, int ax, int bp, int si)
{
	register int key;

	key = ax+bp+si+errval;		// uses the arguments, against warnings
	screenofs = bufferofs = 0;
	SetScreen (0,0);
	CenterWindow (32,3);
	py++;
	CPPrint ("Disk I/O error! Press ENTER to\n");
	CPPrint ("resume, or ESC to abort:");
	SetNormalPalette ();
	ClearKeys ();
	do
	{
		key = PGet()&0xff;
	} while (key != 27 && key != 13);
	if (key != 27)
		hardresume (1);
	_AX = 3;
	geninterrupt (0x10);
	if (kbdstarted)
		ShutdownKbd ();
	if (soundstarted)
		ShutdownSound ();
	return 2;
}


/*
=================
=
= SetupTileInfo
=
=================
*/


void SetupTileInfo (void)
{
	tinf = maphead;
	northwall = (byte *)tinf + tinf[2];
	southwall = (byte *)tinf + tinf[3];
	eastwall = (byte *)tinf + tinf[4];
	westwall = (byte *)tinf + tinf[5];
	animtile = (byte *)tinf + tinf[6];
	intile = (byte *)tinf + tinf[7];
	tilenames = (char *)tinf + tinf[8];
}


/*
=================
=
= PatchPointers
=
=================
*/

void PatchPointers (void)
{
	register int i;

	levelheader = (LevelDef far *)levelseg;
	for (i=0;i<levelheader->planes;i++)
		mapplane[i] = (unsigned far *)((byte far *)levelheader+levelheader->planesize*i+32);
}


/*
=============================================================================

				GRAPHICS LOADING

=============================================================================
*/

extern grheadtype far	egahead;	// segment 1229: the graphics header
extern char far		egagraph[];	// segment 188E: the four planes of pictures
extern long		picoffset;	// IDASME.ASM's (a word there)
extern unsigned		spritesegs[];
extern spritetype _seg	*spritetable;
extern spritetype	image;
extern unsigned		originxglobal;
extern unsigned		picsegs[4];
extern unsigned		tilesegs[];	// IDLIBC.C's: the tile segments


int	chunkfirst[10] = {0,85,103,121,130,148,156,162,174,187};	// DS:00AA: sprite numbers of each set
int	chunklast[10] = {84,102,120,129,147,155,161,173,186,198};	// DS:00BE

void LoadSpriteChunk (int chunk);


void InitGraphics (void)
{
	int n;
	register int i,shape;

	grhead = (grheadtype _seg *)&egahead;
	numtile8 = grhead->tile8count;
	numpics = grhead->piccount;
	numpicm = grhead->picmcount;
	numsprites = grhead->spritecount;
	picoffset = grhead->picbase;
	grplanesize = grhead->planesize;
	pictable = (pictype _seg *)((unsigned)grhead+(grhead->pictableofs>>4));
	spritetable = (spritetype _seg *)((unsigned)grhead+(grhead->spritetableofs>>4));

	for (shape=0;shape<numsprites*4;shape++)
		spritetable[shape].planesize = spritetable[shape].width*spritetable[shape].height;

	n = grplanesize/16;
	picsegs[0] = FP_SEG(egagraph);
	picsegs[1] = picsegs[0]+n;
	picsegs[2] = picsegs[1]+n;
	picsegs[3] = picsegs[2]+n;

	memset (spriteneeded,0,sizeof(spriteneeded));
	for (i=chunkfirst[0]*4;i<=chunklast[0]*4;i+=3)
	{
		spriteneeded[i++] = 1;
		spriteneeded[i] = 1;
	}
	LoadSpriteChunk (0);

	if ((handle = open ("EGATILES.DD2",O_RDONLY|O_BINARY)) == -1)
		Quit ("Unable to open tile file!");
}


/*
=================
=
= CacheTiles
=
= Reads the needed tiles not yet in memory from EGATILES.DD2 (128 bytes each)
=
=================
*/

void CacheTiles (void)
{
	long pos,filepos;
	char buffer[128];
	register int i;

	lseek (handle,0L,SEEK_SET);
	filepos = 0;
	for (i=0;i<1024;i++)
	{
		if (tileneeded[i] && !tilesegs[i])
		{
			MMGetPtr ((memptr *)&tilesegs[i],8);
			pos = (long)i*32*4;
			if (filepos != pos)
				lseek (handle,pos-filepos,SEEK_CUR);
			read (handle,buffer,128);
			filepos = pos+128;
			movedata (_SS,(unsigned)buffer,tilesegs[i],0,128);
		}
	}
}


/*
=================
=
= LoadSpriteChunk
=
= Loads one file of sprites and makes the shifted copies the level needs: each sprite has
= four shapes, the odd ones drawn from the one before, a pixel over
=
=================
*/

int	chunkfirst2[10] = {0,85,121,174,187,0,0,0,0,0};	// DS:00D2: sprite numbers in each file
int	chunklast2[10] = {84,120,173,186,198,0,0,0,0,0};	// DS:00E6

void LoadSpriteChunk (int chunk)
{
	int s,plane,shape,bufseg,srcseg,destseg;
	unsigned srcofs,destofs,planesize;
	int mask;
	int planeofs[5];
	int lines,width,height;
	char *name;
	memptr buf;

	switch (chunk)
	{
	case 0:
		name = "S_DAVE.DD2";
		break;
	case 1:
		name = "S_CHUNK1.DD2";
		break;
	case 2:
		name = "S_CHUNK2.DD2";
		break;
	case 3:
		name = "S_FRANK.DD2";
		break;
	case 4:
		name = "S_MASTER.DD2";
		break;
	}

	BloadinHUFFMM (name,&buf);
	bufseg = (unsigned)buf;
	for (s=0;s<5;s++)
		planeofs[s] = *(unsigned _seg *)buf*s+2;

	srcofs = 0;
	for (s=chunkfirst2[chunk];s<=chunklast2[chunk];s++)
	{
		shape = s*4;
		if (spriteneeded[shape])
		{
			planesize = spritetable[shape].planesize;
			MMGetPtr ((memptr *)&spritesegs[shape],(planesize*5+15)>>4);
			destofs = 0;
			for (plane=0;plane<5;plane++)
			{
				movedata (bufseg,planeofs[plane]+srcofs,spritesegs[shape],destofs,planesize);
				destofs += planesize;
			}
		}
		srcofs += spritetable[s*4].planesize;

		shape++;
		if (spriteneeded[shape])
		{
			planesize = spritetable[shape].planesize;
			MMGetPtr ((memptr *)&spritesegs[shape],(planesize*5+15)>>4);
			srcseg = spritesegs[shape-1];
			destseg = spritesegs[shape];
			lines = spritetable[shape].height;
			width = (spritetable+shape-1)->width;
			height = lines;
			lines *= 5;

asm		push	si
asm		push	di
asm		mov	es,destseg
asm		xor	si,si
asm		xor	di,di
asm		mov	mask,di
asm		mov	dx,lines
lineloop:
asm		cmp	dx,height
asm		jne	notmask
asm		dec	mask			// the mask plane shifts in ones
notmask:
asm		mov	cx,width
asm		push	ds
asm		mov	ds,srcseg
asm		rep	movsb
asm		pop	ds
asm		mov	al,BYTE PTR mask
asm		stosb
asm		mov	bx,4
shiftloop:
asm		sub	di,width
asm		dec	di
asm		mov	cx,width
asm		inc	cx
asm		mov	al,BYTE PTR mask
asm		rcr	al,1
byteloop:
asm		rcr	BYTE PTR es:[di],1
asm		inc	di
asm		loop	byteloop
asm		dec	bx
asm		jnz	shiftloop
asm		dec	dx
asm		jnz	lineloop
asm		pop	di
asm		pop	si
		}
	}

	MMFreePtr (&buf);
}


/*
=================
=
= SpriteTest
=
= Debug key T: shows each loaded sprite shape, arrows to step
=
=================
*/

void SpriteTest (void)
{
	int y,step,w,h,seg,dest;
	register int i,x;

	SetDefaultColors ();
	CenterWindow (30,20);
	CPPrint ("Sprite Test\n\n");
	y = py;
	x = (px+64)&0x1f8;
	PPrint ("Sprite:\nWidth:\nHeight:\nSource:");
	i = -1;
	step = 1;
	do
	{
		do
		{
			i += step;
			if (i >= numsprites*4)
				i = 0;
			else if (i < 0)
				i = numsprites*4-1;
		} while (!spritesegs[i]);

		w = spritetable[i].width;
		h = spritetable[i].height;
		seg = spritesegs[i];
		dest = screenofs+y*linewidth+x/8+6;

		px = x;
		py = y;
		PPrintUnsigned (i);
		PPrint ("\n");
		px = x;
		PPrintUnsigned (w);
		PPrint ("\n");
		px = x;
		PPrintUnsigned (h);
		PPrint ("\n");
		px = x;
		PPrintUnsigned (seg);
		DrawSpriteT (w,h,seg,dest,spritetable[i].planesize);
		PGet ();
		Bar (x/8,y,8,40,0xff);
		Bar (x/8+6,y,w,h,0xff);

		switch (NBKscan)
		{
		case 0x48:
		case 0x4b:
			step = -1;
			break;
		case 0x4d:
		case 0x50:
			step = 1;
			break;
		}
	} while (NBKscan != 1);
}


void LoadNeededChunks (void)
{
	register int i;

	for (i=0;i<5;i++)
		if (chunkneeded[i])
			LoadSpriteChunk (i);
}


/*
=============================================================================

				LEVELS

=============================================================================
*/

void	EGAplane (int plane);
void	RLEWExpand (unsigned far *source, unsigned far *dest);


int	chunkof[10] = {0,1,1,2,2,2,2,2,3,4};	// DS:00FA: which file each sprite set is in


/*
=================
=
= LoadLevel
=
=================
*/

void LoadLevel (void)
{
	unsigned far *p,far *end;
	int i;
	unsigned size;
	char num[4];
	unsigned seg;
	char name[30];
	register int set,shape;

	EGAplane (0);
	if (level < 10)
	{
		itoa (level,num,10);
		strcpy (name,"LEVEL0");
	}
	else
	{
		itoa (level,num,10);
		strcpy (name,"LEVEL");
	}
	strcat (name,num);
	strcat (name,".DD2");

	seg = ((linewidth*200)>>4)+0xa000;
	LoadFile (name,MK_FP(seg,0));
	size = *(unsigned far *)MK_FP(seg,0);
	if (levelseg)
		MMFreePtr (&levelseg);
	MMGetPtr (&levelseg,(size+15)>>4);
	RLEWExpand (MK_FP(seg,0),(unsigned far *)levelseg);

	levelheader = (LevelDef far *)levelseg;
	for (i=0;i<levelheader->planes;i++)
		mapplane[i] = (unsigned far *)((byte far *)levelheader+levelheader->planesize*i+32);
	mapwwide = levelheader->width;
	mapheight = levelheader->height;
	mapbwide = mapwwide*2;

	originxmin = originymin = playerxmin = playerymin = 0x200;
	originxmax = (mapwwide-22)<<8;
	originymax = (mapheight-15)<<8;
	playerxmax = (mapwwide-2)<<8;
	playerymax = ((mapheight-2)<<8)-0x80;

	if (level == loadedlevel)
		ScanInfoPlane ();
	else
	{
		memset (tileneeded,0,sizeof(tileneeded));
		p = mapplane[0];
		end = mapplane[0]+mapwwide*mapheight;
		while (p != end)
			tileneeded[*p++] = 1;
		for (set=0;set<1024;set++)
			if (tilesegs[set])
				MMSetPurge ((memptr *)&tilesegs[set],!tileneeded[set] ? 1 : 0);

		ScanInfoPlane ();
		memset (spriteneeded,0,sizeof(spriteneeded));
		memset (chunkneeded,0,sizeof(chunkneeded));
		for (set=1;set<10;set++)
			if (needsprites[set])
			{
				for (shape=chunkfirst[set]*4;shape<=chunklast[set]*4;shape+=3)
				{
					if (spritesegs[shape])
						MMSetPurge ((memptr *)&spritesegs[shape],0);
					else
					{
						spriteneeded[shape] = 1;
						chunkneeded[chunkof[set]] = 1;
					}
					shape++;
					if (spritesegs[shape])
						MMSetPurge ((memptr *)&spritesegs[shape],0);
					else
					{
						spriteneeded[shape] = 1;
						chunkneeded[chunkof[set]] = 1;
					}
				}
			}
			else
				for (shape=chunkfirst[set]*4;shape<=chunklast[set]*4;shape++)
					if (spritesegs[shape])
						MMSetPurge ((memptr *)&spritesegs[shape],1);
		MMSortMem ();
		CacheTiles ();
		LoadNeededChunks ();
	}
	useegamem = 0;
}


/*
=================
=
= WinGame
=
=================
*/

void WinGame (void)
{
	DrawPic (7,8,40);
	WaitVBL (180);
	ExpWin (28,5);
	CPPrint ("Congratulations!\n");
	CPPrint ("You have freed Delbert from\n");
	CPPrint ("the wicked grasp of\n");
	CPPrint ("Silas' Master!\n");
	ClearKeys ();
	Ack ();
	ExpWin (28,5);
	CPPrint ("Delbert mentions that\n");
	CPPrint ("he  was thinking of going\n");
	CPPrint ("with his friends to \n");
	CPPrint ("Booger Hollow, where\n");
	ClearKeys ();
	Ack ();
	ExpWin (28,5);
	py += 9;
	CPPrint ("old Man Clinton's ghost is\n");
	CPPrint ("rumored to haunt his shack...");
	ClearKeys ();
	Ack ();
	ExpWin (28,5);
	py += 4;
	CPPrint ("You thwack Delbert solidly\n");
	CPPrint ("on the head and hope he\n");
	CPPrint ("comes to his senses.");
	ClearKeys ();
	Ack ();
	ExpWin (28,5);
	py += 14;
	CPPrint ("You win!");
	ClearKeys ();
	Ack ();
}


/*
=================
=
= the title screen's timer: int8hook counts twentieths of seconds
=
=================
*/


void TitleTimer (void)
{
	if (++titletics >= 20)
	{
		titletics = 0;
		titletime++;
		EGAplane (0);
	}
}


void WaitTitle (void)
{
	while (titletime < 20)
	{
		if (NBKscan & 0x80)
		{
			ClearKeys ();
			break;
		}
		if (playermode[1] == 2 && JoyButton())
			break;
	}
	int8hook = 0;
}


/*
=================
=
= LevelIntro
=
= The map of the mansion (PROGPIC.DD2) with Dave standing at the level's place
=
=================
*/

void	FadeIn (void);
void	PPrintLong (long value);
extern memptr	progpic;		// IDLIBC.C's: PROGPIC.DD2 while it is cached

int	levelx[9] = {0,0x14,0x96,0xb6,0xe2,0xf4,0x122,0x46,0x3e};	// DS:010E: where Dave stands
int	levely[9] = {0,0x50,0x16,0x65,0x50,0x50,0x60,0x6a,0x92};	// DS:0120


void LevelIntro (void)
{
	unsigned dest,x;
	int shape,sprite,width,height,lx,n;
	register int plane,y;

	if (!progpic)
	{
		BloadinHUFFMM ("PROGPIC.DD2",&progpic);
		StopDrive ();
	}
	CRTCstart (0);

	dest = 8;
	width = ((unsigned _seg *)progpic)[2];
	height = ((unsigned _seg *)progpic)[3];
	for (plane=0;plane<4;plane++)
	{
		EGAplane (plane);
		for (y=0;y<height;y++)
			movedata ((unsigned)progpic,dest+width*y,0xa000,linewidth*y,width);
		dest += width*height;
	}
	MMSetPurge (&progpic,1);

	SetScreen (0,0);
	screenofs = 0;
	pxl = 0;
	pxh = 320;
	py = 5;
	DrawWindow (0,0,10,3);
	py += 3;
	switch (level)
	{
	case 1:
		CPPrint ("LEVEL 1\n");
		break;
	case 2:
		CPPrint ("LEVEL 2\n");
		break;
	case 3:
		CPPrint ("LEVEL 3\n");
		break;
	case 4:
		CPPrint ("LEVEL 4\n");
		break;
	case 5:
		CPPrint ("LEVEL 5\n");
		break;
	case 6:
		CPPrint ("LEVEL 6\n");
		break;
	case 7:
		CPPrint ("LEVEL 7\n");
		break;
	case 8:
		CPPrint ("LEVEL 8\n");
		break;
	}

	py = 0xa5;
	lx = 0x2c;
	n = lives;
	if (n > 8)
		n = 8;
	for (plane=1;plane<n;plane++,lx+=16)
		DrawSprite (lx,py,1);
	px = 0xf4;
	py = 0xa6;
	PPrintLong (score);
	px = 0xf4;
	py = 0xb9;
	PPrintLong (highscore);
	FadeIn ();
	ClearKeys ();

	sprite = 0x43;
	x = levelx[level];
	introshape = shape = sprite*4+((x&7)>>2);
	image = spritetable[shape];
	x = ((x+0x40)>>3)-8;
	introwidth = image.width;
	introheight = image.height;
	introdest = levely[level]*linewidth+x+screenofs;
	introplanesize = image.planesize;
	titletics = titletime = 0;
	DrawSpriteT (introwidth,introheight,spritesegs[introshape+(titletime&1)*4],introdest,
		introplanesize);
	int8hook = (unsigned)TitleTimer;
	if (level != loadedlevel)
	{
		DrawWindow (0,5,10,8);
		py += 3;
		CPPrint ("LOADING\n");
	}
}


/*
=================
=
= title
=
=================
*/

void	SetLineWidth (int width);
void	CopyEGA (int wide, int height, unsigned source, unsigned dest);
void	FadeOut (void);
void	GameLoop (void);

void DrawTitle (void)
{
	LoadPage ("TITLE1.DD2",0);
	LoadPage ("TITLE2.DD2",40);
	EGAWRITEMODE(0);
	EGAWRITEMODE(1);
	EGAMAPMASK(15);
	CopyEGA (80,200,0,0x4000);
}


void TitleLoop (void)
{
	ControlStruct c;
	register int i;

	noendscreen = 0;
	FadeOut ();
	SetLineWidth (80);
	SetScreen (0,0);
	screenofs = 0;
	StopDrive ();
	DrawTitle ();
	FadeIn ();

	i = originxglobal = 0;
	do
	{
		if (i > 200 && i <= 280)
			originxglobal += 4;
		if (i > 400 && i <= 480)
			originxglobal -= 4;
		if (i > 480)
			i = 0;
		SetScreen (originxglobal>>3,originxglobal&7);
		i++;
		screenofs = originxglobal>>3;
		if (CheckKeys ())
		{
			EGAWRITEMODE(1);
			EGAMAPMASK(15);
			CopyEGA (80,200,0x4000,0);
		}
		c = ControlPlayer (1);
	} while (!(c.button1 || c.button2 || keydown[0x39]));

	ClearKeys ();
	PlaySound (LEVELSTARTSND);
	GameLoop ();
}


/*
=================
=
= main
=
=================
*/

extern char far		sounds[];	// segment 257A: the PC speaker sounds
extern char far		font[];		// segment 106B: the proportional font
extern fontstruct _seg	*fontseg;


void main (void)
{
	int key,x,y;

	puts ("Dangerous Dave is executing...");
	ReadJoystick (1,&x,&y);
	if (x < 500)
	{
		puts ("Joystick detected");
		joystickok = 1;
	}
	else
	{
		puts ("Joystick not detected");
		joystickok = 0;
	}

	videocard = VideoID ();
	if (videocard == EGAcard)
		puts ("EGA card detected");
	else if (videocard == VGAcard)
		puts ("VGA card detected");
	else
	{
		puts ("Hey, I don't see an EGA or VGA card here!  Do you want to run the program ");
		puts ("anyway (Y = go ahead, N = quit to dos) ?");
		ClearKeys ();
		key = toupper(bioskey(0)&0xff);
		if (key != 'Y')
			exit (1);
	}

	grmode = EGAgr;
	SetupTileInfo ();
	MMStartup ();
	soundseg = (memptr)sounds;
	fontseg = (fontstruct _seg *)font;
	SetScreenMode (grmode);
	SetLineWidth (64);
	InitGraphics ();
	harderr (DiskError);
	timerspeed = 0x2147;
	StartupSound ();
	soundstarted = 1;
	StartupKbd ();
	kbdstarted = 1;
	InitRndT (1);
	InitRnd (1);
	LoadCtrls ();
	playermode[1] = keyboard;
	screencenterx = 19;
	screencentery = 11;
	timecount = lasttimecount = 0;

	if (_argc > 1 && !strcmp(strupr(_argv[1]),"/LEVEL"))
	{
		FadeOut ();
		LoadFile ("TEDLEVEL.DD2",(char huge *)&level);
		noendscreen = 1;
		level--;
		GameLoop ();
		Quit ("Exiting to ted...");
	}

	if (!keydown[1])		// ESC held skips the intro
		Intro ();
	debugok = 0;
	for (;;)
		TitleLoop ();
	Quit ("");			// never reached; its "" is still in _DATA
}


void Dummy (void)
{
}


void BadReact (void)
{
	Quit ("BadReact called!");
}


/*
=============================================================================

				OBJECTS

=============================================================================
*/


extern unsigned	originyglobal;
void	ScrollOrigin (void);
void	MoveObj (void);


objtype *GetNewObj (void)
{
	register int i;

	for (i=1,new=&objlist[1];new->obclass && i<lastobjindex;i++,new++)
	;
	if (i >= lastobjindex)
	{
		lastobjindex++;
		if (lastobjindex >= MAXOBJECTS)
			Quit ("Object list overflow!");
	}
	memset (new,0,sizeof(objtype));
	new->react = BadReact;
	return new;
}


void SpawnNewObj (int obclass, unsigned x, unsigned y, int xdir, register statetype *state)
{
	int shape;

	GetNewObj ();
	new->obclass = obclass;
	new->x = x;
	new->y = y;
	new->xdir = xdir;
	new->state = state;
	if (new->xdir > 0)
		new->shapenum = state->rightshapenum;
	else
		new->shapenum = state->leftshapenum;
	new->react = state->react;
	new->ticcount = 0;

	shape = new->shapenum*4+((new->x>>5)&3);
	image = spritetable[shape];
	new->left = new->x+image.xl+image.xofs;
	new->right = new->x+image.xh+image.xofs;
	new->top = new->y+image.yl+image.yofs;
	new->bottom = new->y+image.yh+image.yofs;
	new->tileleft = new->oldtileleft = new->left>>8;
	new->tileright = new->oldtileright = new->right>>8;
	new->tiletop = new->oldtiletop = new->top>>8;
	new->tilebottom = new->oldtilebottom = new->bottom>>8;
}


int CheckContact (register objtype *ob1, register objtype *ob2)
{
	if (!ob1->left || !ob2->left)
		return 0;
	if (ob1->right < ob2->left)
		return 0;
	if (ob1->bottom < ob2->top)
		return 0;
	if (ob1->left > ob2->right)
		return 0;
	if (ob1->top > ob2->bottom)
		return 0;
	return 1;
}


void CalcBounds (void)
{
	int shape;

	shape = obj.shapenum*4+((obj.x>>6)&1);
	image = spritetable[shape];
	obj.left = obj.x+image.xl+image.xofs;
	obj.right = obj.x+image.xh+image.xofs;
	obj.top = obj.y+image.yl+image.yofs;
	obj.bottom = obj.y+image.yh+image.yofs;
	obj.tileleft = obj.left>>8;
	obj.tileright = obj.right>>8;
	obj.tiletop = obj.top>>8;
	obj.tilebottom = obj.bottom>>8;
}


void DoGravity (void)
{
	register unsigned i;

	for (i=0;i<tics;i++)
	{
		obj.yspeed += 3;
		if (obj.yspeed > 80)
			obj.yspeed = 80;
		obj.ymove += obj.yspeed;
	}
}


void ClipToWalls (void)
{
	ClipXMove ();
	ClipYMove ();
}


void ClipYMove (void)
{
	int y;
	register int x,move;

	for (y=obj.oldtilebottom+1;y<=obj.tilebottom;y++)
		for (x=obj.tileleft;x<=obj.tileright;x++)
			if (northwall[mapplane[0][y*mapwwide+x]])
			{
				move = obj.bottom+1-(y<<8);
				obj.y -= move;
				obj.top -= move;
				obj.bottom -= move;
				obj.tilebottom--;
				obj.hitbottom = 1;
				goto ceiling;
			}

ceiling:
	for (y=obj.oldtiletop-1;y>=obj.tiletop;y--)
		for (x=obj.tileleft;x<=obj.tileright;x++)
			if (southwall[mapplane[0][y*mapwwide+x]])
			{
				move = ((y+1)<<8)-obj.top;
				obj.y += move;
				obj.top += move;
				obj.bottom += move;
				obj.tiletop++;
				obj.hittop = 1;
				return;
			}
}


void ClipXMove (void)
{
	int move;
	register int y,x;

	for (x=obj.oldtileright+1;x<=obj.tileright;x++)
		for (y=obj.tiletop;y<=obj.tilebottom;y++)
			if (westwall[mapplane[0][y*mapwwide+x]])
			{
				move = obj.right+1-(x<<8);
				obj.x -= move;
				obj.left -= move;
				obj.right -= move;
				obj.tileright = x-1;
				obj.hitright = 1;
				goto left;
			}

left:
	for (x=obj.oldtileleft-1;x>=obj.tileleft;x--)
		for (y=obj.tiletop;y<=obj.tilebottom;y++)
			if (eastwall[mapplane[0][y*mapwwide+x]])
			{
				move = ((x+1)<<8)-obj.left;
				obj.x += move;
				obj.left += move;
				obj.right += move;
				obj.tileleft = x+1;
				obj.hitleft = 1;
				return;
			}
}


void CheckGround (void)
{
	register int x;

	if (obj.bottom & 1)
		for (x=obj.tileleft;x<=obj.tileright;x++)
			if (northwall[mapplane[0][(obj.tilebottom+1)*mapwwide+x]])
			{
				obj.hitbottom = 1;
				return;
			}
}


/*
=================
=
= ScrollScreen
=
= Keeps the player inside a box in the middle of the screen
=
=================
*/

void ScrollScreen (void)
{
	unsigned y;
	register unsigned move,x;

	x = objlist[0].x;
	y = objlist[0].y;

	if (originxglobal+0x900 > x)
	{
		move = originxglobal+0x900-x;
		if (originxglobal-originxmin < move)
			originxglobal = originxmin;
		else
			originxglobal -= move;
	}
	else if (originxglobal+0xb00 < x)
	{
		move = x-(originxglobal+0xb00);
		if (move+originxglobal > originxmax)
			originxglobal = originxmax;
		else
			originxglobal += move;
	}

	if (originyglobal+0x400 > y)
	{
		move = originyglobal+0x400-y;
		if (originyglobal-originymin < move)
			originyglobal = originymin;
		else
			originyglobal -= move;
	}
	else if (originyglobal+0x800 < y)
	{
		move = y-(originyglobal+0x800);
		if (move+originyglobal > originymax)
			originyglobal = originymax;
		else
			originyglobal += move;
	}

	ScrollOrigin ();
}


void CenterScreen (void)
{
	originxglobal = objlist[0].x-0xa00;
	originyglobal = objlist[0].y-0x700;
	if (originxglobal < originxmin || objlist[0].x < 0xa00)
		originxglobal = originxmin;
	if (originyglobal < originymin || objlist[0].y < 0x700)
		originyglobal = originymin;
	if (originxglobal > originxmax)
		originxglobal = originxmax;
	if (originyglobal > originymax)
		originyglobal = originymax;
	originxtile = (originxglobal>>8)&0xfe;
	originytile = (originyglobal>>8)&0xfe;
	InitRefresh ();
}


/*
=================
=
= ChangeState
=
=================
*/

void ChangeState (statetype *state)
{
	obj.state = state;
	obj.shapenum = obj.xdir > 0 ? obj.state->rightshapenum : obj.state->leftshapenum;
	if (obj.state->tictime < obj.ticcount)
		obj.ticcount = obj.state->tictime-1;
	obj.react = obj.state->react;
	obj.contact = obj.state->contact;
}


/*
=================
=
= DoActor
=
= Runs obj's state machine for this frame and moves it, then copies it back to its slot
=
=================
*/

void DoActor (void)
{
	statetype *oldstate;
	register int changed;

	changed = 0;
	obj.hittop = obj.hitright = obj.hitbottom = obj.hitleft = 0;
	obj.ticcount = obj.ticcount+tics;

	if (obj.state->tictime <= obj.ticcount)
	{
		obj.ticcount -= obj.state->tictime;
		CalcBounds ();
		memcpy (&obj.oldleft,&obj.left,16);
		obj.xmove = obj.state->xmove*obj.xdir;
		obj.ymove = obj.state->ymove*obj.ydir;
		if (!obj.state->tictime)
		{
			obj.ticcount = 0;
			obj.xmove = obj.xmove*tics;
			obj.ymove = obj.ymove*tics;
		}
		if (obj.state->think)
		{
			if (obj.thinkdelay)
			{
				obj.thinkdelay--;
				changed = 1;
			}
			else
			{
				oldstate = obj.state;
				obj.state->think ();
				if (oldstate == obj.state)
					changed = 1;
			}
		}
		else
			changed = 1;
		if (changed)
		{
			if (obj.state->nextstate)
				ChangeState (obj.state->nextstate);
			else
				obj.active = obj.obclass = 0;
		}
		MoveObj ();
	}
	else if (obj.state->thinkalways)
	{
		CalcBounds ();
		memcpy (&obj.oldleft,&obj.left,16);
		obj.xmove = 0;
		obj.ymove = 0;
		if (obj.state->think)
			obj.state->think ();
		MoveObj ();
	}
	*ob = obj;
}


/*
=============================================================================

				THE GAME

=============================================================================
*/

void	CalcTics (void);
void	RefreshScreen (void);
void	QueueSpriteAt (int x, int y, int num);
void	CheckTiles (void);
void	PPrintInt (int value);
void	DrawPic (int x, int y, int picnum);
void	WaitEndSound (void);

extern statetype	s_delbertfall;
extern int		shells;



/*
=================
=
= PlayLoop
=
=================
*/

void PlayLoop (void)
{
	register int i,j;

	lastdir = nodir;
	fireready = 0;
	do
	{
		CalcTics ();
		c = ControlPlayer (1);
		if (!c.button2)
			fireready = 1;

//
// think
//
		lastobj = &objlist[lastobjindex];
		for (ob=objlist;ob<lastobj;ob++)
			if (ob->obclass)
			{
				obj = *ob;
				if (!obj.active && originxtile-2 < obj.tileright
				&& originytile-2 < obj.tilebottom && originxtile+23 > obj.tileleft
				&& originytile+12 > obj.tiletop)
					obj.active = 1;
				if (obj.active)
					DoActor ();
			}

//
// contacts
//
		ob = objlist;
		for (ob=objlist;ob<lastobj;ob++)
			if (ob->active)
			{
				check = &objlist[j]+1;		// sic: j is never set, and the next line replaces it
				for (check=ob+1;check<lastobj;check++)
					if (check->active && CheckContact (ob,check))
					{
						if (ob->contact)
							ob->contact (ob,check);
						if (check->contact)
							check->contact (check,ob);
					}
			}

//
// reacts
//
		ob = &objlist[lastobjindex-1];
		for (ob=lastobj-1;ob>=objlist;ob--)
			if (ob->active && ob->react)
			{
				obj = *ob;
				obj.react ();
				*ob = obj;
			}

		ScrollScreen ();
		CheckTiles ();
		QueueSpriteAt (originxglobal+0x80,originyglobal+0x80,shells+19);
		RefreshScreen ();

		if (fadein)
		{
			fadein = 0;
			FadeIn ();
		}

		if (singlestep)
		{
			savetime = timecount;
			DrawWindow (15,0,24,2);
			PPrintInt (tics);
			PPrint (" tics");
			PGet ();
			if ((NBKscan&0x7f) == 1)
			{
				singlestep = 0;
				ClearKeys ();
			}
			EGAWRITEMODE(1);
			CopyEGA (12,24,page ? screenofs+0xa00f : screenofs+0x500f,screenofs+15);
			timecount = savetime;
		}

		CheckKeys ();

		if (playstate == ex_bossdead)
		{
			playstate = ex_completed;
			for (i=1;i<lastobjindex;i++)
				if (objlist[i].active && objlist[i].obclass != delbertobj)
				{
					playstate = ex_bossdead;
					break;
				}
			if (playstate == ex_completed && level == 8)
			{
				playstate = ex_playing;
				objlist[1].state = &s_delbertfall;
			}
		}
	} while ((playstate == ex_playing || playstate == ex_bossdead) && !resetgame);
}


/*
=================
=
= GameLoop
=
=================
*/

void GameLoop (void)
{
	int frames,s,x,y;
	char num[20];
	register int pic,i;

	if (!noendscreen)
		level = 0;

restart:
	resetgame = 0;
	score = 0;
	nextextra = 0;
	lives = 4;
	level++;
	loadedlevel = -1;

	do
	{
		FadeOut ();
		SetLineWidth (64);
		if (!keydown[1])
			LevelIntro ();
		ClearKeys ();
		lastobjindex = 1;
		timecount = lasttimecount = tics = 0;
		playstate = ex_playing;
		memset (objlist,0,sizeof(objlist));
		LoadLevel ();
		loadedlevel = level;
		StopDrive ();
		if (int8hook)
			WaitTitle ();
		FadeOut ();
		CenterScreen ();
		fadein = 1;
		shells = 8;
		debugok = 1;
		PlayLoop ();
		debugok = 0;

		if (noendscreen)
			break;
		if (resetgame)
		{
			// match: pic = pic does nothing, but the block is not empty while BC
			// optimises jumps, so this test and its break stay apart (jumps counted
			// 5+3 bytes, not 5), and that is what pads the switch table at the end
			// of this function to an even offset (ALIGN_SEGMENT_EVEN runs before the
			// jumps shrink). The move itself is removed later.
			pic = pic;
			break;
		}

		if (playstate < 0)
		{
			PlaySound (DEATHSND);
			switch (playstate)
			{
			case ex_knifed:
				pic = 0;
				frames = 4;
				break;
			case ex_slimed:
				pic = 5;
				frames = 5;
				break;
			case ex_grabbed:
				pic = 10;
				frames = 5;
				break;
			case ex_spider:
				pic = 15;
				frames = 5;
				break;
			case ex_ghost:
				pic = 25;
				frames = 5;
				break;
			case ex_werewolf:
				pic = 20;
				frames = 5;
				break;
			case ex_hit:
				pic = 30;
				frames = 5;
				break;
			case ex_deadlytile:
				pic = 35;
				frames = 5;
				break;
			}
			x = ((objlist[0].x-(originxglobal&0xff00))>>4)/8-2;
			y = ((objlist[0].y-(originyglobal&0xff00))>>4)-8;
			for (i=1;i<=frames;i++)
			{
				DrawPic (x,y,pic);
				pic++;
				if (keydown[1])
					break;
				WaitVBL (20);
			}
			lives--;
			if (lives == 0)
			{
				ExpWin (17,3);
				py += 8;
				CPPrint ("G A M E   O V E R");
				PlaySound (GAMEOVERSND);
				WaitEndSound ();
				WaitVBL (120);
			}
		}
		else
		{
			level++;
			PlaySound (LEVELDONESND);
			if (level == 9)
				WinGame ();
		}
	} while (lives && level < 9);

	if (score > highscore)
	{
		ExpWin (18,11);
		py += 3;
		CPPrint ("New High Score!\n");
		py += 5;
		CPPrint ("Score\n");
		ltoa (score,str,10);
		CPPrint (str);
		PPrint ("\n\n");
		CPPrint ("Old\n");
		ltoa (highscore,str,10);
		CPPrint (str);
		PPrint ("\n");
		py += 5;
		CPPrint ("Congratulations!\n");
		CPPrint ("");
		Ack ();
		highscore = score;
	}

	if (levelseg)
		MMFreePtr (&levelseg);
	for (i=0;i<1024;i++)
		if (tilesegs[i])
			MMSetPurge ((memptr *)&tilesegs[i],1);
	for (s=chunkfirst[1]*4;s<800;s++)
		if (spritesegs[s])
			MMSetPurge ((memptr *)&spritesegs[s],1);

	if (!resetgame && level > 2 && level < 9)
	{
		DrawWindow (7,20,32,23);
		py += 3;
		strcpy (str,"Continue from level ");
		itoa (level-1,num,10);
		strcat (str,num);
		strcat (str," ?");
		CPPrint (str);
		ClearKeys ();
		if (toupper (ch = PGet()) == 'Y')
		{
			level -= 2;
			goto restart;
		}
	}
}
