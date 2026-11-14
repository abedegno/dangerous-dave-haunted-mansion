/*
 * DD2DEF.H: the game's own types and the main file's globals, for Dangerous Dave in the
 * Haunted Mansion.
 *
 * The object and state records are an early form of the Keen Dreams engine's (kd_def.h):
 * the layout here comes from the game's code (offsets in the comments), the names are
 * provisional except where Hovertank 3-D or Keen Dreams use the same field the same way.
 */

#ifndef __DD2DEF__
#define __DD2DEF__

//
// object classes (objtype.obclass), as Keen Dreams' classtype; ScanInfoPlane and the
// spawners give each class its creature (notes/names.md)
//
typedef enum
{
	nothing,		// 0: a free slot
	daveobj,		// 1: objlist[0]
	zombieobj,
	throwerobj,
	slimeobj,
	spiderobj,
	ghostobj,
	werewolfobj,
	skullobj,
	bonusobj,		// 9: treasures and extra Daves
	frankobj,		// 10: Frankenstein, a boss
	masterobj,		// 11: the Master, a boss
	delbertobj,		// 12: Dave's brother in his sphere
	lastmonsterobj,		// 13: never spawned; CheckOffScreen treats classes below it as creatures
	handobj,		// 14: the Master's hands
	mastershotobj,		// 15
	knifeobj,		// 16: the thrower's
	fireballobj,		// 17: Frankenstein's
	inertobj		// 18: flashes, sparks, points, gore: they hurt nothing
} classtype;

//
// playstate: 0 while playing; how the level ended; below 0, what killed Dave (the death
// animation GameLoop shows depends on it), like Keen Dreams' exittype
//
#define ex_playing	0
#define ex_completed	1
#define ex_bossdead	100		// a boss is dead: play on until its shots and hands are gone
#define ex_knifed	-1
#define ex_slimed	-2
#define ex_grabbed	-3		// by a zombie
#define ex_spider	-4
#define ex_ghost	-5
#define ex_werewolf	-6
#define ex_hit		-7		// skull, Frankenstein, fireball, the Master's hands or shots
#define ex_deadlytile	-8		// an info tile of type 13

typedef struct statestruct
{
	int	leftshapenum,rightshapenum;	// 0
	int	thinkalways;			// 4: think every frame, not only when the state ends
	int	tictime;			// 6: 0 = think every frame and move by tics
	int	xmove,ymove;			// 8: per state change, times the direction
	void	(*think) (void);		// C
	void	(*contact) ();			// E: (struct objstruct *ob, struct objstruct *hit)
	void	(*react) (void);		// 10: after all have moved: draws the object
	struct statestruct *nextstate;		// 12: 0 removes the object
} statetype;

typedef struct objstruct
{
	int		obclass;		// 0
	int		active;			// 2
	int		shapenum;		// 4
	unsigned	x,y;			// 6: in 1/256 tile (1/16 pixel)
	unsigned	left,top,right,bottom;	// A: hit rectangle
	int		tileleft,tiletop,tileright,tilebottom;	// 12
	unsigned	oldleft,oldtop,oldright,oldbottom;	// 1A: before this move
	int		oldtileleft,oldtiletop,oldtileright,oldtilebottom;	// 22
	int		xmove,ymove;		// 2A: this frame's movement
	int		xspeed,yspeed;		// 2E
	int		hittop,hitright,hitbottom,hitleft;	// 32: sides that met a wall
	int		xdir,ydir;		// 3A
	int		hitpoints;		// 3E: shots it takes (temp3 set)
	int		ticcount;		// 40
	int		thinkdelay;		// 42: frames to skip thinking
	statetype	*state;			// 44
	int		temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8;	// 46
	void		(*contact) ();		// 56
	void		(*react) (void);	// 58
} objtype;

#define MAXOBJECTS	164


void	Quit (char *error);
objtype	*GetNewObj (void);
void	SpawnNewObj (int obclass, unsigned x, unsigned y, int xdir, statetype *state);
int	CheckContact (objtype *ob1, objtype *ob2);
void	CalcBounds (void);
void	DoGravity (void);
void	ClipToWalls (void);
void	ClipYMove (void);
void	ClipXMove (void);
void	CheckGround (void);
void	ChangeState (statetype *state);
void	DoActor (void);


/*
=============================================================================

				THE MAIN FILE'S GLOBALS

Their original names are unknown, and Borland C++ 2.0 lays out a file's _BSS by a hash of
each name (ascending from 960, ties in definition order: notes/fingerprint.md), which the
readable names below do not follow. So the symbols are neutral, g_<address>_v, which all
hash alike, and DD2MAIN.C defines them in address order; the readable names are aliases.

=============================================================================
*/

typedef struct
{
	unsigned	wide;		// 0 ends the list; high bit: xor a picture instead of a sprite
	unsigned	height,source,dest,planesize;
} updatetype;

typedef struct grheadstruct		// the graphics header (egahead, linked data)
{
	long		planesize;	// 0
	int		unk4,unk6;
	unsigned	pictableofs;	// 8: bytes from the header
	int		unkA;
	unsigned	spritetableofs;	// C
	int		unkE;
	int		tile8count;	// 10
	int		unk12[8];
	int		piccount;	// 22
	int		unk24[2];
	int		picmcount;	// 28
	long		picbase;	// 2A
	int		spritecount;	// 2E
} grheadtype;

#define kbdstarted       g_4AB0_v
#define originxmax       g_4AB2_v
#define originymax       g_4AB4_v
#define soundstarted     g_4AB6_v
#define originxmin       g_4AB8_v
#define originymin       g_4ABA_v
#define godmode          g_4ABC_v
#define grplanesize      g_4ABE_v
#define northwall        g_4AC2_v
#define southwall        g_4AC4_v
#define joystickok       g_4AC6_v
#define loadedlevel      g_4AC8_v
#define playerxmax       g_4ACA_v
#define playerymax       g_4ACC_v
#define playerxmin       g_4ACE_v
#define playerymin       g_4AD0_v
#define numpicm          g_4AD2_v
#define chunkneeded      g_4AD4_v
#define titletics        g_4ADA_v
#define numpics          g_4ADC_v
#define spriteneeded     g_4ADE_v
#define noendscreen      g_4DFE_v
#define tinf             g_4E00_v
#define unused4E02       g_4E02_v
#define introdest        g_4E06_v
#define eastwall         g_4E08_v
#define grhead           g_4E0A_v
#define westwall         g_4E0C_v
#define titletime        g_4E0E_v
#define introheight      g_4E10_v
#define introshape       g_4E12_v
#define introplanesize   g_4E14_v
#define numtile8         g_4E16_v
#define introwidth       g_4E18_v
#define numsprites       g_4E1A_v
#define animtile         g_4E1C_v
#define tileneeded       g_4E1E_v
#define needsprites      g_521E_v
#define tilenames        g_5232_v
#define intile           g_5234_v
#define debugok          g_5236_v
#define handle           g_5238_v
#define objlist          g_523A_v
#define c                g_8BE2_v
#define lastdir          g_8BE8_v
#define jumpheld         g_8BEA_v
#define unused8BEC       g_8BEC_v
#define new              g_8BEE_v
#define check            g_8BF0_v
#define savetime         g_8BF2_v
#define fireready        g_8BF6_v
#define lives            g_8BF8_v
#define lastobjindex     g_8BFA_v
#define lastobj          g_8BFC_v
#define ob               g_8BFE_v
#define unused8C00       g_8C00_v
#define obj              g_8C04_v
#define fadein           g_8C5E_v
#define unused8C60       g_8C60_v
#define resetgame        g_8C64_v
#define singlestep       g_8C66_v
#define playstate        g_8C68_v
#define nextextra        g_8C6A_v
#define lasttimecount    g_8C6E_v
#define unused8C72       g_8C72_v
#define originxtile      g_8C74_v
#define unused8C76       g_8C76_v
#define upxl             g_8C78_v
#define upxh             g_8C7E_v
#define upyl             g_8C84_v
#define upyh             g_8C8A_v
#define statuscount      g_8C90_v
#define oldxtile         g_8C92_v
#define oldytile         g_8C94_v
#define page             g_8C96_v
#define originytile      g_8C98_v
#define tics             g_8C9A_v
#define bufferofs        g_8C9C_v
#define statusofs        g_8C9E_v
#define updatelist       g_8CA0_v
#define unused9088       g_9088_v

extern int	kbdstarted;	// DS:4AB0: the keyboard handler is installed
extern unsigned	originxmax;	// DS:4AB2: scroll limits, 1/256 tile
extern unsigned	originymax;	// DS:4AB4
extern int	soundstarted;	// DS:4AB6: the sound handler is installed
extern unsigned	originxmin;	// DS:4AB8
extern unsigned	originymin;	// DS:4ABA
extern int	godmode;	// DS:4ABC: debug key G
extern long	grplanesize;	// DS:4ABE: bytes in each plane of the pictures
extern byte	*northwall;	// DS:4AC2: tile info: solid from above (floors)
extern byte	*southwall;	// DS:4AC4: solid from below
extern int	joystickok;	// DS:4AC6
extern int	loadedlevel;	// DS:4AC8: the level whose graphics are in memory
extern unsigned	playerxmax;	// DS:4ACA: where the player may go
extern unsigned	playerymax;	// DS:4ACC
extern unsigned	playerxmin;	// DS:4ACE
extern unsigned	playerymin;	// DS:4AD0
extern int	numpicm;	// DS:4AD2
extern char	chunkneeded[5];	// DS:4AD4: sprite files to load
extern unsigned	titletics;	// DS:4ADA: int8hook counter on the level intro
extern int	numpics;	// DS:4ADC
extern char	spriteneeded[800];	// DS:4ADE: by shape (sprite*4+shift)
extern int	noendscreen;	// DS:4DFE: playing a level from TED (/LEVEL): no end screen
extern unsigned	*tinf;	// DS:4E00: the map header
extern long	unused4E02;	// DS:4E02: nothing reads it
extern unsigned	introdest;	// DS:4E06: the level intro's marker sprite
extern byte	*eastwall;	// DS:4E08
extern grheadtype _seg	*grhead;	// DS:4E0A: the graphics header
extern byte	*westwall;	// DS:4E0C
extern unsigned	titletime;	// DS:4E0E: seconds on the level intro
extern unsigned	introheight;	// DS:4E10
extern int	introshape;	// DS:4E12
extern unsigned	introplanesize;	// DS:4E14
extern int	numtile8;	// DS:4E16
extern unsigned	introwidth;	// DS:4E18
extern int	numsprites;	// DS:4E1A
extern byte	*animtile;	// DS:4E1C
extern char	tileneeded[1024];	// DS:4E1E
extern int	needsprites[10];	// DS:521E: sprite sets the level uses
extern char	*tilenames;	// DS:5232: the tile info planes' names
extern byte	*intile;	// DS:5234: what touching a tile does
extern int	debugok;	// DS:5236: the debug keys work (only while playing)
extern int	handle;	// DS:5238: EGATILES.DD2
extern objtype	objlist[MAXOBJECTS];	// DS:523A: [0] is the player
extern ControlStruct	c;	// DS:8BE2: this frame's controls
extern int	lastdir;	// DS:8BE8
extern int	jumpheld;	// DS:8BEA
extern int	unused8BEC;	// DS:8BEC: nothing reads it
extern objtype	*new;	// DS:8BEE
extern objtype	*check;	// DS:8BF0
extern long	savetime;	// DS:8BF2: timecount while single stepping
extern int	fireready;	// DS:8BF6
extern int	lives;	// DS:8BF8
extern int	lastobjindex;	// DS:8BFA
extern objtype	*lastobj;	// DS:8BFC
extern objtype	*ob;	// DS:8BFE
extern long	unused8C00;	// DS:8C00: nothing reads it
extern objtype	obj;	// DS:8C04: the object being thought about
extern int	fadein;	// DS:8C5E
extern long	unused8C60;	// DS:8C60: nothing reads it
extern int	resetgame;	// DS:8C64
extern int	singlestep;	// DS:8C66: debug key S
extern int	playstate;	// DS:8C68: 0 playing, 1 level done, 100 boss level, below 0 killed (how)
extern long	nextextra;	// DS:8C6A: score for the next extra man, less 10000
extern long	lasttimecount;	// DS:8C6E
extern int	unused8C72;	// DS:8C72: nothing reads it
extern int	originxtile;	// DS:8C74
extern int	unused8C76;	// DS:8C76: nothing reads it
extern int	upxl[3];	// DS:8C78: what is left to update, per page and the master
extern int	upxh[3];	// DS:8C7E
extern int	upyl[3];	// DS:8C84
extern int	upyh[3];	// DS:8C8A
extern int	statuscount;	// DS:8C90: pages left to copy the status box to
extern int	oldxtile;	// DS:8C92
extern int	oldytile;	// DS:8C94
extern int	page;	// DS:8C96: the page being drawn
extern int	originytile;	// DS:8C98
extern unsigned	tics;	// DS:8C9A
extern unsigned	bufferofs;	// DS:8C9C
extern unsigned	statusofs;	// DS:8C9E
extern updatetype	updatelist[2][50];	// DS:8CA0: sprites drawn on each page
extern long	unused9088;	// DS:9088: nothing reads it

#endif
