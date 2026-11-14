/* target: ACTORS */
/* opts: -ms -a -O -G -Z -k- -y */
/*
 * DD2ACTS.C: the objects of Dangerous Dave in the Haunted Mansion: their states and think,
 * contact and react routines.
 *
 * name: the file name is descriptive (Hovertank 3-D has HOVACTS.C); so are the function
 * and variable names. Code _TEXT 2DF5-5575, _DATA DS:0744-1293 (the state records from
 * DS:0780), _BSS DS:9098-90AB.
 */

#include <mem.h>
#include <dos.h>
#include "IDLIB.H"
#include "DD2DEF.H"

int	QueueSprite (int x, int y, int num);
int	QueueXorSprite (int x, int y, int num);
void	QueueSpriteAt (int x, int y, int num);


//
// _BSS (DS:9098-90AB). Borland C++ 2.0 lays it out by a hash of each name, from 960, ties in
// order of definition (notes/fingerprint.md); these names keep the game's order.
//
unsigned	doorx,doory;		// DS:9098: the door tile the player stands at
unsigned	doorinfo;		// DS:909C: its map plane 1 value (0x8000 up: a bonus behind it)
int		doortile;		// DS:909E
int		doortype;		// DS:90A0: its intile type
unsigned	shotx,shoty;		// DS:90A2: where the last shot stopped
int		shells;			// DS:90A6: loaded; idling reloads up to 8
int		idletime;		// DS:90A8
unsigned	jumptime;		// DS:90AA: tics the jump keeps rising

void	MoveObj (void);
void	DrawReact (void);
void	DrawFlashReact (void);
void	ClipDrawReact (void);
void	ClipFlashReact (void);
void	WalkReact (void);
void	CeilingWalkReact (void);
int	CheckOffScreen (void);
void	FacePlayer (void);

/*
=============================================================================

				STATES

The records are named by their address until their objects are worked out.

=============================================================================
*/

void PointsThink (void);
void OneUpThink (void);
void ChunkThink (void);
void ChunkFallThink (void);
void ChunkReact (void);
void ChunkVanishReact (void);
void PlayerThink (void);
void DoorThink (void);
void OpenDoorThink (void);
void UpDownThink (void);
void FidgetThink (void);
void ShootThink (void);
void ResumeControlThink (void);
void AirThink (void);
void PlayerContact (objtype *ob, objtype *hit);
void PlayerWalkReact (void);
void PlayerStandReact (void);
void PlayerAirReact (void);
void ZombieThink (void);
void ZombieFallReact (void);
void ThrowerThink (void);
void ThrowThink (void);
void BoltReact (void);
void SlimeThink (void);
void SlimeFlipReact (void);
void GhostWaitThink (void);
void GhostAppearThink (void);
void GhostContact (objtype *ob, objtype *hit);
void GhostFlyThink (void);
void WerewolfThink (void);
void WerewolfWalkReact (void);
void WerewolfFallThink (void);
void WerewolfJumpReact (void);
void SkullFlyThink (void);
void SkullReact (void);
void SpiderThink (void);
void SpiderDropReact (void);
void SpiderRiseReact (void);
void FrankThink (void);
void FrankFireThink (void);
void FireballThink (void);
void FireballTimeThink (void);
void FireballReact (void);
void MasterThink (void);
void MasterSpitThink (void);
void HandThink (void);
void MasterShotThink (void);
void DrawOnlyReact (void);
void DelbertFallReact (void);
void WinThink (void);

extern statetype s_davestand;
extern statetype s_davewalk1;
extern statetype s_davewalk2;
extern statetype s_davewalk3;
extern statetype s_davewalk4;
extern statetype s_davewalk5;
extern statetype s_davewalk6;
extern statetype s_davewalk7;
extern statetype s_davewalk8;
extern statetype s_daveopendoor;
extern statetype s_davedoor1;
extern statetype s_davedoor2;
extern statetype s_davedoor3;
extern statetype s_davejump1;
extern statetype s_davejump2;
extern statetype s_davefall1;
extern statetype s_davefall2;
extern statetype s_daveshoot1;
extern statetype s_daveshoot2;
extern statetype s_daveshoot3;
extern statetype s_daveaimup;
extern statetype s_daveshootup1;
extern statetype s_daveshootup2;
extern statetype s_daveshootup3;
extern statetype s_daveaimdown;
extern statetype s_daveshootdown1;
extern statetype s_daveshootdown2;
extern statetype s_daveshootdown3;
extern statetype s_davereload1;
extern statetype s_davereload2;
extern statetype s_davereload3;
extern statetype s_flashup1;
extern statetype s_flashup2;
extern statetype s_flashup3;
extern statetype s_flash1;
extern statetype s_flash2;
extern statetype s_flash3;
extern statetype s_flashdown1;
extern statetype s_flashdown2;
extern statetype s_flashdown3;
extern statetype s_shothit;
extern statetype s_bonus1a;
extern statetype s_bonus1b;
extern statetype s_bonus2a;
extern statetype s_bonus2b;
extern statetype s_bonus3a;
extern statetype s_bonus3b;
extern statetype s_bonus4a;
extern statetype s_bonus4b;
extern statetype s_bonus5a;
extern statetype s_bonus5b;
extern statetype s_bonus6a;
extern statetype s_bonus6b;
extern statetype s_points1;
extern statetype s_points2;
extern statetype s_points3;
extern statetype s_points4;
extern statetype s_points5;
extern statetype s_oneup;
extern statetype s_chunk1a;
extern statetype s_chunk1b;
extern statetype s_chunk2a;
extern statetype s_chunk2b;
extern statetype s_chunkland;
extern statetype s_bosschunk;
extern statetype s_zombiewalk1;
extern statetype s_zombiewalk2;
extern statetype s_zombiewalk3;
extern statetype s_zombiewalk4;
extern statetype s_zombiegrab1;
extern statetype s_zombiegrab2;
extern statetype s_zombiegrab3;
extern statetype s_zombiefall1;
extern statetype s_zombiefall2;
extern statetype s_throwerwalk1;
extern statetype s_throwerwalk2;
extern statetype s_throwerwalk3;
extern statetype s_throwerwalk4;
extern statetype s_throwerthrow1;
extern statetype s_throwerthrow2;
extern statetype s_throwerthrow3;
extern statetype s_knife;
extern statetype s_slimeceiling1;
extern statetype s_slimeceiling2;
extern statetype s_slimefloor1;
extern statetype s_slimefloor2;
extern statetype s_slimerise1;
extern statetype s_slimerise2;
extern statetype s_slimedrop1;
extern statetype s_slimedrop2;
extern statetype s_slimeceilingland;
extern statetype s_slimefloorland;
extern statetype s_ghostwait;
extern statetype s_ghostappear1;
extern statetype s_ghostappear2;
extern statetype s_ghostappear3;
extern statetype s_ghostappear4;
extern statetype s_ghostappear5;
extern statetype s_ghostfly1;
extern statetype s_ghostfly2;
extern statetype s_ghostrise;
extern statetype s_werewolfwalk1;
extern statetype s_werewolfwalk2;
extern statetype s_werewolfwalk3;
extern statetype s_werewolfwalk4;
extern statetype s_werewolfjump1;
extern statetype s_werewolfjump2;
extern statetype s_skullfly1;
extern statetype s_skullfly2;
extern statetype s_spiderwalk1;
extern statetype s_spiderwalk2;
extern statetype s_spiderdrop;
extern statetype s_spiderrise;
extern statetype s_master1;
extern statetype s_master2;
extern statetype s_master3;
extern statetype s_masterspit1;
extern statetype s_masterspit2;
extern statetype s_handappear;
extern statetype s_hand1;
extern statetype s_hand2;
extern statetype s_mastershot1;
extern statetype s_mastershot2;
extern statetype s_delbert1;
extern statetype s_delbert2;
extern statetype s_delbertfall;
extern statetype s_spherebreak1;
extern statetype s_spherebreak2;
extern statetype s_frankwalk1;
extern statetype s_frankwalk2;
extern statetype s_frankwalk3;
extern statetype s_frankwalk4;
extern statetype s_frankthrow1;
extern statetype s_frankthrow2;
extern statetype s_fireballstart1;
extern statetype s_fireballstart2;
extern statetype s_fireball1;
extern statetype s_fireball2;

int		doortiles[5][6] = {148,151,149,152,150,153,148,151,149,152,150,153,78,79,91,92,104,105,78,79,91,92,104,105,833,832,724,725,737,738};

statetype s_davestand = {9,1,1,6,0,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davestand};
statetype s_davewalk1 = {10,2,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk2};
statetype s_davewalk2 = {10,2,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk3};
statetype s_davewalk3 = {11,3,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk4};
statetype s_davewalk4 = {11,3,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk5};
statetype s_davewalk5 = {12,4,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk6};
statetype s_davewalk6 = {12,4,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk7};
statetype s_davewalk7 = {13,5,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk8};
statetype s_davewalk8 = {13,5,1,3,64,0,PlayerThink,PlayerContact,PlayerWalkReact,&s_davewalk1};
statetype s_daveopendoor = {40,40,0,25,0,0,OpenDoorThink,PlayerContact,DrawReact,&s_davestand};
statetype s_davedoor1 = {41,41,0,25,0,0,0,0,DrawReact,&s_davedoor2};
statetype s_davedoor2 = {42,42,0,25,0,0,0,0,DrawReact,&s_davedoor3};
statetype s_davedoor3 = {43,43,0,25,0,0,DoorThink,0,DrawReact,&s_davestand};
statetype s_davejump1 = {14,6,1,1,0,0,AirThink,PlayerContact,PlayerAirReact,&s_davejump1};
statetype s_davejump2 = {15,7,1,1,0,0,AirThink,PlayerContact,PlayerAirReact,&s_davejump2};
statetype s_davefall1 = {16,8,1,60,0,0,AirThink,PlayerContact,PlayerAirReact,&s_davefall2};
statetype s_davefall2 = {15,7,1,20,0,0,AirThink,PlayerContact,PlayerAirReact,&s_davefall1};
statetype s_daveshoot1 = {34,28,0,12,-64,0,ShootThink,PlayerContact,PlayerStandReact,&s_daveshoot2};
statetype s_daveshoot2 = {34,28,0,10,-64,0,0,PlayerContact,PlayerStandReact,&s_daveshoot3};
statetype s_daveshoot3 = {35,29,0,10,0,0,ResumeControlThink,PlayerContact,PlayerStandReact,&s_davestand};
statetype s_daveaimup = {36,30,1,1,0,0,UpDownThink,PlayerContact,PlayerStandReact,&s_daveaimup};
statetype s_daveshootup1 = {36,30,0,1,-32,0,ShootThink,PlayerContact,PlayerStandReact,&s_daveshootup2};
statetype s_daveshootup2 = {36,30,0,15,-32,0,0,PlayerContact,PlayerStandReact,&s_daveshootup3};
statetype s_daveshootup3 = {37,31,0,15,0,0,ResumeControlThink,PlayerContact,PlayerStandReact,&s_davestand};
statetype s_daveaimdown = {38,32,1,1,0,0,UpDownThink,PlayerContact,PlayerStandReact,&s_daveaimdown};
statetype s_daveshootdown1 = {38,32,0,1,-32,0,ShootThink,PlayerContact,PlayerStandReact,&s_daveshootdown2};
statetype s_daveshootdown2 = {38,32,0,15,-32,0,0,PlayerContact,PlayerStandReact,&s_daveshootdown3};
statetype s_daveshootdown3 = {39,33,0,15,0,0,ResumeControlThink,PlayerContact,PlayerStandReact,&s_davestand};
statetype s_davereload1 = {17,17,1,20,0,0,PlayerThink,PlayerContact,ClipFlashReact,&s_davereload2};
statetype s_davereload2 = {18,18,0,1,0,0,FidgetThink,PlayerContact,ClipFlashReact,&s_davereload3};
statetype s_davereload3 = {18,18,1,20,0,0,PlayerThink,PlayerContact,ClipFlashReact,&s_davereload1};
statetype s_flashup1 = {52,46,0,10,0,64,0,0,DrawReact,&s_flashup2};
statetype s_flashup2 = {53,47,0,10,0,64,0,0,DrawReact,&s_flashup3};
statetype s_flashup3 = {53,47,0,10,0,0,0,0,DrawReact,0};
statetype s_flash1 = {50,44,0,10,0,64,0,0,DrawReact,&s_flash2};
statetype s_flash2 = {51,45,0,10,0,64,0,0,DrawReact,&s_flash3};
statetype s_flash3 = {51,45,0,10,0,0,0,0,DrawReact,0};
statetype s_flashdown1 = {54,48,0,10,0,64,0,0,DrawReact,&s_flashdown2};
statetype s_flashdown2 = {55,49,0,10,0,64,0,0,DrawReact,&s_flashdown3};
statetype s_flashdown3 = {55,49,0,10,0,0,0,0,DrawReact,0};
statetype s_shothit = {56,56,0,10,0,0,0,0,DrawReact,0};
statetype s_bonus1a = {57,57,0,10,0,0,0,0,DrawReact,&s_bonus1b};
statetype s_bonus1b = {58,58,0,10,0,0,0,0,DrawReact,&s_bonus1a};
statetype s_bonus2a = {59,59,0,10,0,0,0,0,DrawReact,&s_bonus2b};
statetype s_bonus2b = {60,60,0,10,0,0,0,0,DrawReact,&s_bonus2a};
statetype s_bonus3a = {61,61,0,10,0,0,0,0,DrawReact,&s_bonus3b};
statetype s_bonus3b = {62,62,0,10,0,0,0,0,DrawReact,&s_bonus3a};
statetype s_bonus4a = {63,63,0,10,0,0,0,0,DrawReact,&s_bonus4b};
statetype s_bonus4b = {64,64,0,10,0,0,0,0,DrawReact,&s_bonus4a};
statetype s_bonus5a = {65,65,0,10,0,0,0,0,DrawReact,&s_bonus5b};
statetype s_bonus5b = {66,66,0,10,0,0,0,0,DrawReact,&s_bonus5a};
statetype s_bonus6a = {67,67,0,10,0,0,0,0,DrawReact,&s_bonus6b};
statetype s_bonus6b = {68,68,0,10,0,0,0,0,DrawReact,&s_bonus6a};
statetype s_points1 = {69,69,1,0,0,6,PointsThink,0,DrawReact,&s_points1};
statetype s_points2 = {70,70,1,0,0,6,PointsThink,0,DrawReact,&s_points2};
statetype s_points3 = {71,71,1,0,0,6,PointsThink,0,DrawReact,&s_points3};
statetype s_points4 = {72,72,1,0,0,6,PointsThink,0,DrawReact,&s_points4};
statetype s_points5 = {73,73,1,0,0,6,PointsThink,0,DrawReact,&s_points5};
statetype s_oneup = {74,74,1,0,0,6,OneUpThink,0,DrawReact,&s_oneup};
statetype s_chunk1a = {75,77,1,4,0,0,ChunkThink,0,ChunkReact,&s_chunk1b};
statetype s_chunk1b = {76,78,1,4,0,0,ChunkThink,0,ChunkReact,&s_chunk1a};
statetype s_chunk2a = {79,81,1,4,0,0,ChunkThink,0,ChunkReact,&s_chunk2b};
statetype s_chunk2b = {80,82,1,4,0,0,ChunkThink,0,ChunkReact,&s_chunk2a};
statetype s_chunkland = {83,83,0,50,0,0,0,0,DrawReact,0};
statetype s_bosschunk = {185,185,1,999,0,0,ChunkFallThink,0,ChunkVanishReact,&s_bosschunk};
statetype s_zombiewalk1 = {93,86,0,14,128,0,ZombieThink,0,WalkReact,&s_zombiewalk2};
statetype s_zombiewalk2 = {94,87,0,14,128,0,ZombieThink,0,WalkReact,&s_zombiewalk3};
statetype s_zombiewalk3 = {95,88,0,14,128,0,ZombieThink,0,WalkReact,&s_zombiewalk4};
statetype s_zombiewalk4 = {96,89,0,14,128,0,ZombieThink,0,WalkReact,&s_zombiewalk1};
statetype s_zombiegrab1 = {97,90,0,8,0,0,FacePlayer,0,WalkReact,&s_zombiegrab2};
statetype s_zombiegrab2 = {98,91,0,8,0,0,FacePlayer,0,WalkReact,&s_zombiegrab3};
statetype s_zombiegrab3 = {99,92,0,16,0,0,FacePlayer,0,WalkReact,&s_zombiewalk1};
statetype s_zombiefall1 = {100,100,0,20,0,128,FacePlayer,0,ZombieFallReact,&s_zombiefall2};
statetype s_zombiefall2 = {101,101,0,20,0,128,FacePlayer,0,ZombieFallReact,&s_zombiefall1};
statetype s_throwerwalk1 = {108,104,0,8,128,0,ThrowerThink,0,WalkReact,&s_throwerwalk2};
statetype s_throwerwalk2 = {109,105,0,8,128,0,ThrowerThink,0,WalkReact,&s_throwerwalk3};
statetype s_throwerwalk3 = {110,106,0,8,128,0,ThrowerThink,0,WalkReact,&s_throwerwalk4};
statetype s_throwerwalk4 = {111,107,0,8,128,0,ThrowerThink,0,WalkReact,&s_throwerwalk1};
statetype s_throwerthrow1 = {115,112,0,8,0,0,FacePlayer,0,WalkReact,&s_throwerthrow2};
statetype s_throwerthrow2 = {116,113,0,8,0,0,FacePlayer,0,WalkReact,&s_throwerthrow3};
statetype s_throwerthrow3 = {117,114,0,16,0,0,ThrowThink,0,WalkReact,&s_throwerwalk1};
statetype s_knife = {118,119,0,0,32,0,0,0,BoltReact,&s_knife};
statetype s_slimeceiling1 = {131,133,0,4,64,0,SlimeThink,0,CeilingWalkReact,&s_slimeceiling2};
statetype s_slimeceiling2 = {132,134,0,4,64,0,SlimeThink,0,CeilingWalkReact,&s_slimeceiling1};
statetype s_slimefloor1 = {135,137,0,4,64,0,SlimeThink,0,WalkReact,&s_slimefloor2};
statetype s_slimefloor2 = {136,138,0,4,64,0,SlimeThink,0,WalkReact,&s_slimefloor1};
statetype s_slimerise1 = {146,145,0,20,64,64,0,0,ClipDrawReact,&s_slimerise2};
statetype s_slimerise2 = {139,140,0,0,64,64,0,0,SlimeFlipReact,&s_slimerise2};
statetype s_slimedrop1 = {143,144,0,20,64,64,0,0,ClipDrawReact,&s_slimedrop2};
statetype s_slimedrop2 = {142,141,0,0,40,40,0,0,SlimeFlipReact,&s_slimedrop2};
statetype s_slimeceilingland = {145,146,0,20,0,0,0,0,DrawReact,&s_slimeceiling1};
statetype s_slimefloorland = {144,143,0,20,0,0,0,0,DrawReact,&s_slimefloor1};
statetype s_ghostwait = {0,0,0,5,0,0,GhostWaitThink,0,0,&s_ghostwait};
statetype s_ghostappear1 = {124,124,0,20,0,0,0,0,DrawReact,&s_ghostappear2};
statetype s_ghostappear2 = {125,125,0,20,0,0,0,0,DrawReact,&s_ghostappear3};
statetype s_ghostappear3 = {126,126,0,20,0,0,0,0,DrawReact,&s_ghostappear4};
statetype s_ghostappear4 = {127,127,0,30,0,0,GhostAppearThink,0,DrawReact,&s_ghostappear5};
statetype s_ghostappear5 = {128,128,0,30,0,0,GhostAppearThink,0,DrawReact,&s_ghostrise};
statetype s_ghostfly1 = {122,123,0,1,0,0,GhostFlyThink,GhostContact,DrawReact,&s_ghostfly2};
statetype s_ghostfly2 = {0,0,0,1,0,0,GhostFlyThink,GhostContact,0,&s_ghostfly1};
statetype s_ghostrise = {122,123,1,5,0,0,GhostFlyThink,0,DrawReact,&s_ghostrise};
statetype s_werewolfwalk1 = {163,168,0,4,192,0,WerewolfThink,0,WerewolfWalkReact,&s_werewolfwalk2};
statetype s_werewolfwalk2 = {164,169,0,4,192,0,WerewolfThink,0,WerewolfWalkReact,&s_werewolfwalk3};
statetype s_werewolfwalk3 = {165,170,0,4,128,0,WerewolfThink,0,WerewolfWalkReact,&s_werewolfwalk4};
statetype s_werewolfwalk4 = {166,171,0,4,192,0,WerewolfThink,0,WerewolfWalkReact,&s_werewolfwalk1};
statetype s_werewolfjump1 = {166,171,0,15,0,0,0,0,ClipFlashReact,&s_werewolfjump2};
statetype s_werewolfjump2 = {167,172,0,0,55,0,WerewolfFallThink,0,WerewolfJumpReact,&s_werewolfjump2};
statetype s_skullfly1 = {159,157,1,4,0,0,SkullFlyThink,0,SkullReact,&s_skullfly2};
statetype s_skullfly2 = {160,158,1,4,0,0,SkullFlyThink,0,SkullReact,&s_skullfly1};
statetype s_spiderwalk1 = {151,149,0,8,64,0,SpiderThink,0,CeilingWalkReact,&s_spiderwalk2};
statetype s_spiderwalk2 = {152,150,0,8,64,0,SpiderThink,0,CeilingWalkReact,&s_spiderwalk1};
statetype s_spiderdrop = {0,0,0,0,0,32,0,0,SpiderDropReact,&s_spiderdrop};
statetype s_spiderrise = {0,0,0,0,0,32,0,0,SpiderRiseReact,&s_spiderrise};
statetype s_master1 = {188,188,1,60,0,0,MasterThink,0,DrawFlashReact,&s_master2};
statetype s_master2 = {188,189,1,60,0,0,MasterThink,0,DrawFlashReact,&s_master1};
statetype s_master3 = {188,189,0,60,0,0,MasterThink,0,DrawFlashReact,&s_master1};
statetype s_masterspit1 = {188,188,1,5,0,0,MasterSpitThink,0,DrawFlashReact,&s_masterspit2};
statetype s_masterspit2 = {189,189,1,5,0,0,MasterSpitThink,0,DrawFlashReact,&s_masterspit1};
statetype s_handappear = {192,192,0,100,0,0,0,0,DrawFlashReact,&s_hand1};
statetype s_hand1 = {192,192,1,20,0,0,HandThink,0,DrawFlashReact,&s_hand2};
statetype s_hand2 = {193,193,1,20,0,0,HandThink,0,DrawFlashReact,&s_hand1};
statetype s_mastershot1 = {190,190,1,5,0,0,MasterShotThink,0,DrawReact,&s_mastershot2};
statetype s_mastershot2 = {191,191,1,5,0,0,MasterShotThink,0,DrawReact,&s_mastershot1};
statetype s_delbert1 = {194,194,0,20,0,0,0,0,DrawOnlyReact,&s_delbert2};
statetype s_delbert2 = {194,195,0,20,0,0,0,0,DrawOnlyReact,&s_delbert1};
statetype s_delbertfall = {194,194,0,0,0,64,0,0,DelbertFallReact,&s_delbertfall};
statetype s_spherebreak1 = {196,196,0,20,320,160,0,0,DrawOnlyReact,&s_spherebreak2};
statetype s_spherebreak2 = {197,197,0,120,0,0,WinThink,0,DrawOnlyReact,&s_spherebreak2};
statetype s_frankwalk1 = {175,179,0,20,256,0,FrankThink,0,WalkReact,&s_frankwalk2};
statetype s_frankwalk2 = {176,180,0,20,374,0,FrankThink,0,WalkReact,&s_frankwalk3};
statetype s_frankwalk3 = {177,181,0,20,256,0,FrankThink,0,WalkReact,&s_frankwalk4};
statetype s_frankwalk4 = {178,182,0,20,374,0,FrankThink,0,WalkReact,&s_frankwalk1};
statetype s_frankthrow1 = {175,179,0,30,256,0,FrankFireThink,0,WalkReact,&s_frankthrow2};
statetype s_frankthrow2 = {175,179,0,120,256,0,0,0,WalkReact,&s_frankwalk2};
statetype s_fireballstart1 = {183,183,1,5,0,0,FireballTimeThink,0,FireballReact,&s_fireballstart2};
statetype s_fireballstart2 = {184,184,1,5,0,0,FireballTimeThink,0,FireballReact,&s_fireballstart1};
statetype s_fireball1 = {183,183,1,5,0,0,FireballThink,0,FireballReact,&s_fireball2};
statetype s_fireball2 = {184,184,1,5,0,0,FireballThink,0,FireballReact,&s_fireball1};

int		points[12] = {-1,-1,1,1,2,2,2,3,-1,-1,5,6};	// DS:1248, by class
statetype	*bonusstates[6] = {&s_bonus1a,&s_bonus2a,&s_bonus3a,&s_bonus4a,&s_bonus5a,&s_bonus6a};
int		shotxstep[8] = {128,128,128,128,128,128,128,128};
int		shotystep[8] = {-128,-100,-80,100,80,64,48,32};
statetype	*zombiestart[4] = {&s_zombiewalk1,&s_zombiewalk2,&s_zombiewalk3,&s_zombiewalk4};


/*
=================
=
= MoveObj
=
= Moves obj by xmove and ymove (each held to a tile a frame)
=
=================
*/

void MoveObj (void)
{
	if (obj.xmove > 0x100)
		obj.xmove = 0x100;
	else if (obj.xmove < -0x100)
		obj.xmove = -0x100;

	if (obj.ymove > 0x100)
		obj.ymove = 0x100;
	else if (obj.ymove < -0x100)
		obj.ymove = -0x100;

	obj.x += obj.xmove;
	obj.y += obj.ymove;
	CalcBounds ();
}


/*
=================
=
= reacts: draw the object
=
=================
*/

void DrawReact (void)
{
	QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


void DrawFlashReact (void)
{
	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


void ClipDrawReact (void)
{
	ClipToWalls ();
	QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


void ClipFlashReact (void)
{
	ClipToWalls ();
	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= WalkReact
=
= Walks along the floor, turning at walls and edges
=
=================
*/

void WalkReact (void)
{
	register int tile;

	ClipXMove ();
	tile = northwall[mapplane[0][obj.xdir > 0 ? (obj.tilebottom+1)*mapwwide+obj.tileright
		: (obj.tilebottom+1)*mapwwide+obj.tileleft]];

	if (!tile || (obj.hitleft && obj.xdir == -1) || (obj.hitright && obj.xdir == 1))
	{
		if (obj.thinkdelay)
			obj.thinkdelay = 0;
		else
			obj.thinkdelay = RndT()/32;
		obj.xdir = -obj.xdir;
		memcpy (&obj.oldleft,&obj.left,16);
		ChangeState (obj.state);
		CalcBounds ();
		ClipXMove ();
	}

	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= CeilingWalkReact
=
= Walks along a ceiling, turning at walls and edges
=
=================
*/

void CeilingWalkReact (void)
{
	register int tile;

	ClipToWalls ();
	tile = southwall[mapplane[0][obj.xdir > 0 ? (obj.tilebottom-1)*mapwwide+obj.tileright
		: (obj.tilebottom-1)*mapwwide+obj.tileleft]];

	if (!tile || (obj.hitleft && obj.xdir == -1) || (obj.hitright && obj.xdir == 1))
	{
		obj.xdir = -obj.xdir;
		memcpy (&obj.oldleft,&obj.left,16);
		ChangeState (obj.state);
		CalcBounds ();
		ClipXMove ();
	}

	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= CheckOffScreen
=
= Objects far enough off the screen are stopped (and, from class 13, removed)
=
=================
*/

int CheckOffScreen (void)
{
	if (originxtile-8 > obj.tileright || originytile-8 > obj.tilebottom
	|| originxtile+28 < obj.tileleft || originytile+18 < obj.tiletop)
	{
		if (obj.obclass < lastmonsterobj)
			obj.active = 0;			// stays in the list, inactive
		else
		{
			obj.obclass = nothing;		// removed
			obj.active = 0;
		}
		return 1;
	}
	return 0;
}


/*
=================
=
= FacePlayer
=
=================
*/

void FacePlayer (void)
{
	if (obj.left > objlist[0].left)
		obj.xdir = -1;
	else
		obj.xdir = 1;
}


/*
=================
=
= Explode
=
= Blows an object into chunks, scores it and removes it. The two bosses (classes 10 and
= 11) take their minions (14, 15 and 17) with them and burst into fourteen more chunks
=
=================
*/


void GivePoints (unsigned x, unsigned y, int type);

void Explode (register objtype *ob)
{
	register int i;

	SpawnNewObj (inertobj,ob->x,ob->y,-1,ob->obclass == fireballobj ? &s_bosschunk : &s_chunk1a);
	new->xspeed = -30;
	new->yspeed = -40;
	new->active = 1;
	SpawnNewObj (inertobj,ob->x+0x100,ob->y+0x40,1,ob->obclass == fireballobj ? &s_bosschunk : &s_chunk1a);
	new->xspeed = 30;
	new->yspeed = -40;
	new->active = 1;

	switch (ob->obclass)
	{
	case frankobj:
	case masterobj:
		for (i=1;i<lastobjindex;i++)
			if (objlist[i].obclass == fireballobj || objlist[i].obclass == mastershotobj || objlist[i].obclass == handobj)
				objlist[i].obclass = objlist[i].active = 0;
		playstate = ex_bossdead;

		SpawnNewObj (inertobj,ob->x,ob->y+0x140,-1,&s_chunk2a);
		new->xspeed = -30;
		new->active = 1;
		SpawnNewObj (inertobj,ob->x+0x100,ob->y+0x100,1,&s_chunk2a);
		new->xspeed = 30;
		SpawnNewObj (inertobj,ob->x+0x200,ob->y,-1,&s_chunk1a);
		new->xspeed = -30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x300,ob->y+0x40,1,&s_chunk1a);
		new->xspeed = 30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x200,ob->y+0x140,-1,&s_chunk2a);
		new->xspeed = -30;
		SpawnNewObj (inertobj,ob->x+0x300,ob->y+0x100,1,&s_chunk2a);
		new->xspeed = 30;
		SpawnNewObj (inertobj,ob->x,ob->y+0x200,-1,&s_chunk1a);
		new->xspeed = -30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x100,ob->y+0x240,1,&s_chunk1a);
		new->xspeed = 30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x200,ob->y+0x340,-1,&s_chunk2a);
		new->xspeed = -30;
		SpawnNewObj (inertobj,ob->x+0x300,ob->y+0x300,1,&s_chunk2a);
		new->xspeed = 30;
		SpawnNewObj (inertobj,ob->x*2,ob->y+0x200,-1,&s_chunk1a);	// sic: x*2, where the pattern wants x+0x200
		new->xspeed = -30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x300,ob->y+0x240,1,&s_chunk1a);
		new->xspeed = 30;
		new->yspeed = -40;
		SpawnNewObj (inertobj,ob->x+0x200,ob->y+0x340,-1,&s_chunk2a);
		new->xspeed = -30;
		SpawnNewObj (inertobj,ob->x+0x300,ob->y+0x300,1,&s_chunk2a);
		new->xspeed = 30;
		break;

	case 2:
	case 7:
	case 18:
		SpawnNewObj (inertobj,ob->x,ob->y+0x140,-1,&s_chunk2a);
		new->xspeed = -30;
		new->active = 1;
		SpawnNewObj (inertobj,ob->x+0x100,ob->y+0x100,1,&s_chunk2a);
		new->xspeed = 30;
		new->active = 1;
	}

	GivePoints (ob->x+0x40,ob->y+0x40,points[ob->obclass]);
	ob->obclass = nothing;
	ob->active = 0;
}


/*
=================
=
= PointsThink / OneUpThink: the score popups rise for a while
=
=================
*/

void PointsThink (void)
{
	obj.temp4 = obj.temp4 + tics;
	if (obj.temp4 > 60)
		obj.obclass = obj.active = 0;
}

void OneUpThink (void)
{
	obj.temp4 = obj.temp4 + tics;
	if (obj.temp4 > 180)
		obj.obclass = obj.active = 0;
}


/*
=================
=
= SpawnBonus
=
=================
*/

void SpawnBonus (int tilex, int tiley, int type)
{
	statetype *state;

	state = bonusstates[type];
	SpawnNewObj (bonusobj,tilex<<8,tiley<<8,-1,state);
	new->temp4 = type;
}


/*
=================
=
= GivePoints
=
= Scores type 1-5 (100-1600) with a popup; type 0 only shows an extra man. Every 10000
= points gives an extra man
=
=================
*/


void GivePoints (unsigned x, unsigned y, int type)
{
	register statetype *state;

	switch (type)
	{
	case 0:
		state = &s_oneup;
		break;
	case 1:
		score += 100;
		state = &s_points1;
		break;
	case 2:
		score += 200;
		state = &s_points2;
		break;
	case 3:
		score += 400;
		state = &s_points3;
		break;
	case 4:
		score += 800;
		state = &s_points4;
		break;
	case 5:
		score += 1600;
		state = &s_points5;
		break;
	default:
		return;
	}

	SpawnNewObj (inertobj,x,y,0,state);
	new->active = 1;
	new->ydir = -1;

	if (nextextra+10000 <= score)
	{
		nextextra += 10000;
		lives++;
		PlaySound (EXTRADAVESND);
		SpawnNewObj (inertobj,x,y-0x100,0,&s_oneup);
		new->ydir = -1;
	}
}


/*
=================
=
= chunks
=
=================
*/

void ChunkThink (void)
{
	obj.xmove = obj.xspeed*tics;
	DoGravity ();
	if (obj.yspeed > 0 && obj.ydir <= 0)
	{
		ChangeState (&s_chunk2a);
		obj.ydir = 1;
	}
}

void ChunkFallThink (void)
{
	obj.xmove = obj.xspeed*tics;
	DoGravity ();
}

void ChunkReact (void)
{
	ClipToWalls ();
	if (obj.hitbottom)
	{
		obj.y += 0x80;
		ChangeState (&s_chunkland);
	}
	else if (obj.hitleft || obj.hitright)
	{
		obj.xdir = -obj.xdir;
		obj.xspeed = -obj.xspeed/2;
		ChangeState (obj.state);
	}
	if (obj.hittop)
		obj.yspeed = 0;
	QueueSprite (obj.x,obj.y,obj.shapenum);
}

void ChunkVanishReact (void)
{
	ClipToWalls ();
	if (obj.hitbottom || obj.hitleft || obj.hitright || obj.hittop)
		obj.active = obj.obclass = 0;
	QueueSprite (obj.x,obj.y,obj.shapenum);
}


/*
=================
=
= SpawnPlayer
=
=================
*/

void SpawnPlayer (unsigned tilex, unsigned tiley)
{
	memset (&objlist[0],0,sizeof(objtype));
	objlist[0].obclass = daveobj;
	objlist[0].xdir = 1;
	objlist[0].active = 1;
	objlist[0].x = tilex<<8;
	objlist[0].y = (tiley<<8)+16-1;
	objlist[0].temp2 = 0;
	obj = objlist[0];
	ChangeState (&s_davestand);
	objlist[0] = obj;
}


/*
=================
=
= PlayerControl
=
=================
*/


void PlayerControl (void)
{
	switch (c.dir)
	{
	case north:
		if (doorx)
		{
			if (doortype == 1)
			{
				obj.x = doorx<<8;
				ChangeState (&s_daveopendoor);
			}
			else
			{
				obj.x = ((doorx-1)<<8)+0x40;
				ChangeState (&s_davedoor1);
			}
		}
		else
		{
			obj.ydir = -1;
			ChangeState (&s_daveaimup);
		}
		break;
	case northeast:
		obj.xdir = 1;
		obj.ydir = -1;
		ChangeState (&s_daveaimup);
		break;
	case northwest:
		obj.xdir = -1;
		obj.ydir = -1;
		ChangeState (&s_daveaimup);
		break;
	case south:
		obj.ydir = 1;
		ChangeState (&s_daveaimdown);
		break;
	case southeast:
		obj.xdir = 1;
		obj.ydir = 1;
		ChangeState (&s_daveaimdown);
		break;
	case southwest:
		obj.xdir = -1;
		obj.ydir = 1;
		ChangeState (&s_daveaimdown);
		break;
	case east:
		obj.xdir = 1;
		ChangeState (&s_davewalk1);
		break;
	case west:
		obj.xdir = -1;
		ChangeState (&s_davewalk1);
		break;
	case nodir:
		ChangeState (&s_davestand);
		break;
	}
	lastdir = c.dir;
}


/*
=================
=
= PlayerThink
=
=================
*/


void PlayerThink (void)
{
	if (c.button1)
	{
		if (!jumpheld)
		{
			jumpheld = 1;
			jumptime = 14;
			if (c.dir == west || c.dir == east)
				obj.xspeed = obj.xdir*24;
			else
				obj.xspeed = 0;
			obj.yspeed = -40;
			ChangeState (&s_davejump1);
			PlaySound (JUMPSND);
			return;
		}
	}
	else
		jumpheld = 0;

	if (c.button2 && fireready)
	{
		fireready = 0;
		obj.ydir = 0;
		ChangeState (&s_daveshoot1);
		return;
	}

	if (c.dir != lastdir)
		PlayerControl ();

	if (obj.state == &s_davestand && shells < 8)
	{
		idletime = idletime + tics;
		if (idletime > 30)
			ChangeState (&s_davereload1);
	}
	else
		idletime = 0;
}


/*
=================
=
= door routines
=
=================
*/

extern unsigned	tilesegs[];
void	CenterScreen (void);
void	DrawTileAt (unsigned source, unsigned dest);

void DoorThink (void)
{
	int type,tilex,tiley;
	unsigned info;
	int tile;
	register unsigned x,y;

	info = doorinfo;
	if (info == 0x8000)
	{
		playstate = ex_completed;
		obj.active = 0;
		return;
	}
	tilex = (info>>8)&0x7f;
	tiley = info&0x7f;
	obj.x = tilex<<8;
	obj.y = tiley<<8;
	tile = mapplane[0][tiley*mapwwide+tilex];
	if (intile[tile] != 2)
	{
		if (tile == 0x27a)
			type = 3;
		else if (tile == 0x2e4)
			type = 4;
		else
			type = 2;
		for (y=0;y<3;y++)
			for (x=0;x<2;x++)
			{
				info = doortiles[type][y*2+x];
				mapplane[0][(tiley-1+y)*mapwwide+(tilex-1+x)] = info;
			}
	}
	CalcBounds ();
	memcpy (&obj.oldleft,&obj.left,16);
	objlist[0] = obj;
	CenterScreen ();
}



void OpenDoorThink (void)
{
	int tile,type;
	register int x,y;

	PlaySound (OPENDOORSND);
	statuscount = 2;
	statusofs = ((doory-originytile-1)<<4)*linewidth + (doorx-originxtile-1)*2;
	if (doorinfo <= 0x8000)
		type = 2;
	else
		type = 0;
	if (doorinfo > 0x8000)
		SpawnBonus (doorx,doory-1,doorinfo-0x8001);
	if (doortile == 0x27a)
		type++;
	else if (doortile == 0x2e4)
		type += 2;

	outport (GC_INDEX,GC_MODE);
	for (y=0;y<3;y++)
		for (x=0;x<2;x++)
		{
			tile = doortiles[type][y*2+x];
			mapplane[0][(doory-1+y)*mapwwide+(doorx-1+x)] = tile;
			DrawTileAt (tilesegs[tile],bufferofs+statusofs+(y<<4)*linewidth+x*2+0xa000);
		}
}


/*
=================
=
= UpDownThink
=
= Looking up or ducking: jump straight up, drop through a ledge, or shoot that way
=
=================
*/

void UpDownThink (void)
{
	register int x;

	if (c.button1)
	{
		if (obj.ydir < 0)
		{
			jumptime = 14;
			obj.xspeed = 0;
			obj.yspeed = -40;
			ChangeState (&s_davejump1);
			PlaySound (JUMPSND);
			return;
		}
		for (x=obj.tileleft;x<=obj.tileright;x++)
			if (southwall[mapplane[0][(obj.tilebottom+1)*mapwwide+x]])
				return;
		obj.y += 0x40;
		obj.oldtilebottom++;
		ChangeState (&s_davefall1);
		PlaySound (JUMPSND);
	}
	else if (c.button2 && fireready)
	{
		fireready = 0;
		if (obj.ydir < 0)
			ChangeState (&s_daveshootup1);
		else
			ChangeState (&s_daveshootdown1);
	}
	else if (c.dir != lastdir)
		PlayerControl ();
}


/*
=================
=
= FidgetThink
=
=================
*/

void FidgetThink (void)
{
	PlaySound (RELOADSND);
	if (++shells == 8)
		ChangeState (&s_davestand);
}


/*
=================
=
= TraceShot
=
= Follows a shot from x,y by xstep,ystep until it hits an object that can be shot (1) or
= a wall (-1), or leaves the screen (0); shotx and shoty say where
=
=================
*/

int TraceShot (register unsigned x, unsigned y, int xstep, int ystep)
{
	int tilex,tiley,tile;
	register objtype *ob;

	for (;;)
	{
		for (ob=&objlist[1];ob<lastobj;ob++)
			if (ob->active && ob->temp2 && ob->left <= x && ob->right >= x
			&& ob->top <= y && ob->bottom >= y)
			{
				if (ob->temp3)
				{
					ob->temp1 = 20;
					if (!--ob->hitpoints)
						Explode (ob);
				}
				shotx = x;
				shoty = y;
				return 1;
			}

		tilex = (x+xstep)>>8;
		tiley = (y+ystep)>>8;

		if (x>>8 != tilex)
		{
			tile = mapplane[0][tiley*mapwwide+tilex];
			if (xstep > 0)
			{
				if (tilex-originxtile > 25)
					return 0;
				if (westwall[tile])
				{
					x += 15-(x&15);
					goto hitwall;
				}
			}
			else
			{
				if (originxtile-tilex > 5)
					return 0;
				if (eastwall[tile])
				{
					x += -(x&15);
					goto hitwall;
				}
			}
		}
		x += xstep;

		if (y>>8 != tiley)
		{
			tile = mapplane[0][tiley*mapwwide+tilex];
			if (ystep > 0)
			{
				if (tiley-originytile > 14)
					return 0;
				if (northwall[tile])
				{
					y += 15-(y&15);
					goto hitwall;
				}
			}
			else
			{
				if (originytile-tiley > 3)
					return 0;
				if (southwall[tile])
				{
					y += -(y&15);
					goto hitwall;
				}
			}
		}
		y += ystep;
	}

hitwall:
	shotx = x;
	shoty = y;
	return -1;
}


/*
=================
=
= ShootThink
=
= The player fires the shotgun in the direction held: up, level or down
=
=================
*/


void ShootThink (void)
{
	unsigned y;
	int result;
	unsigned last;
	statetype *flash;
	register unsigned dir;
	register unsigned x;

	if (!shells)
	{
		obj.x += obj.xdir<<6;
		ChangeState (&s_davestand);
		PlaySound (NOAMMOSND);
		return;
	}

	switch (c.dir)
	{
	case north:
		obj.ydir = -1;
		break;
	case northeast:
		obj.xdir = 1;
		obj.ydir = -1;
		break;
	case east:
		obj.ydir = 0;
		obj.xdir = 1;
		break;
	case southeast:
		obj.xdir = 1;
		obj.ydir = 1;
		break;
	case south:
		obj.ydir = 1;
		break;
	case southwest:
		obj.xdir = -1;
		obj.ydir = 1;
		break;
	case west:
		obj.ydir = 0;
		obj.xdir = -1;
		break;
	case northwest:
		obj.xdir = -1;
		obj.ydir = -1;
		break;
	}

	switch (obj.ydir)
	{
	case -1:
		if (obj.state != &s_daveshootup1)
			ChangeState (&s_daveshootup2);
		break;
	case 0:
		if (obj.state != &s_daveshoot1)
			ChangeState (&s_daveshoot2);
		break;
	case 1:
		if (obj.state != &s_daveshootdown1)
			ChangeState (&s_daveshootdown2);
		break;
	}

	if (!godmode)
		shells--;

	switch (obj.ydir)
	{
	case -1:
		flash = &s_flashup1;
		dir = 0;
		last = 2;
		y = obj.y+0x10;
		if (obj.xdir > 0)
			x = obj.x+0x120;
		else
			x = obj.x+0x30;
		break;
	case 0:
		flash = &s_flash1;
		y = obj.y+0xa0;
		if (obj.xdir > 0)
			x = obj.x+0x140;
		else
			x = obj.x+0xa0;
		break;
	case 1:
		flash = &s_flashdown1;
		dir = 3;
		last = 7;
		y = obj.y+0x150;
		if (obj.xdir > 0)
			x = obj.x+0x120;
		else
			x = obj.x+0x30;
		break;
	}

	SpawnNewObj (inertobj,obj.x,obj.y,obj.xdir,flash);
	new->ydir = -1;

	shotx = shoty = 0;
	if (obj.ydir == 0)
	{
		result = TraceShot (x,y,obj.xdir<<7,0);
		if (!result)
		{
			result = TraceShot (x,y+0x40,obj.xdir<<7,0);
			if (!result)
				result = TraceShot (x,y-0x80,obj.xdir<<7,0);
		}
	}
	else
	{
		do
		{
			result = TraceShot (x,y,shotxstep[dir]*obj.xdir,shotystep[dir]);
		} while (result != 1 && ++dir <= last);
	}

	if (shotx)
	{
		SpawnNewObj (inertobj,shotx-0x40,shoty-0x40,0,&s_shothit);
		new->active = 1;
		PlaySound (SHOTMONSTERSND);
	}
	else
		PlaySound (SHOTMISSSND);
}


void ResumeControlThink (void)
{
	PlayerControl ();
}


/*
=================
=
= AirThink
=
= Steering in the air, and how long the jump keeps rising while button 1 is held
=
=================
*/

void AirThink (void)
{
	register unsigned i;

	switch (c.dir)
	{
	case northeast:
	case east:
	case southeast:
		for (i=0;i<tics;i++)
		{
			obj.xspeed += 2;
			if (obj.xspeed > 24)
				obj.xspeed = 24;
			obj.xmove += obj.xspeed;
		}
		if (obj.xmove > 0)
		{
			obj.xdir = 1;
			obj.shapenum = obj.state->rightshapenum;
		}
		break;

	case southwest:
	case west:
	case northwest:
		for (i=0;i<tics;i++)
		{
			obj.xspeed -= 2;
			if (obj.xspeed < -24)
				obj.xspeed = -24;
			obj.xmove += obj.xspeed;
		}
		if (obj.xmove < 0)
		{
			obj.xdir = -1;
			obj.shapenum = obj.state->leftshapenum;
		}
		break;

	default:
		if (obj.xmove > 0)
		{
			for (i=0;i<tics;i++)
			{
				obj.xspeed--;
				obj.xmove += obj.xspeed;
				if (obj.xspeed < 0)
					obj.xspeed = 0;
			}
		}
		else if (obj.xmove < 0)
		{
			for (i=0;i<tics;i++)
			{
				obj.xspeed++;
				obj.xmove += obj.xspeed;
				if (obj.xspeed < 0)		// sic: never above 0 either way
					obj.xspeed = 0;
			}
		}
	}

	if (jumptime)
	{
		if (jumptime < tics)
		{
			obj.ymove = obj.yspeed*jumptime;
			jumptime = 0;
		}
		else
		{
			obj.ymove = obj.yspeed*tics;
			jumptime = jumptime - tics;
		}
		if (!c.button1)
			jumptime = 0;
		if (!jumptime)
			ChangeState (&s_davejump2);
	}
	else
	{
		DoGravity ();
		if (obj.state != &s_davefall1 && obj.state != &s_davefall2 && obj.yspeed > 0)
			ChangeState (&s_davefall1);
	}
}


/*
=================
=
= PlayerContact
=
= What touching each class does: a negative playstate kills the player (the number says
= how); bonuses (class 9) score
=
=================
*/

void PlayerContact (objtype *ob, register objtype *hit)
{
	register int type;

	switch (hit->obclass)
	{
	case knifeobj:
		playstate = ex_knifed;
		break;
	case slimeobj:
		playstate = ex_slimed;
		break;
	case werewolfobj:
		playstate = ex_werewolf;
		break;
	case spiderobj:
		playstate = ex_spider;
		break;
	case skullobj:
	case frankobj:
	case handobj:
	case mastershotobj:
	case fireballobj:
		playstate = ex_hit;
		break;
	case zombieobj:
		if (hit->state != &s_zombiegrab3)
			break;
		if (hit->xdir == 1 && hit->x > ob->x)
			return;
		if (hit->xdir == -1 && hit->x < ob->x)
			return;
		playstate = ex_grabbed;
		break;
	case ghostobj:
		if (!hit->temp2)
			break;
		playstate = ex_ghost;
		break;
	case bonusobj:
		switch (hit->temp4)
		{
		case 0:
			type = 1;
			PlaySound (TREASURE1SND);
			break;
		case 1:
			type = 2;
			PlaySound (TREASURE2SND);
			break;
		case 2:
			type = 3;
			PlaySound (TREASURE3SND);
			break;
		case 3:
			type = 4;
			PlaySound (TREASURE4SND);
			break;
		case 4:
			type = 5;
			PlaySound (TREASURE5SND);
			break;
		case 5:
			type = 0;
			PlaySound (EXTRADAVESND);
			lives++;
			break;
		}
		GivePoints (hit->x,hit->y,type);
		hit->active = hit->obclass = 0;
		break;
	}

	if (godmode && playstate < 0)
		playstate = ex_playing;
}


/*
=================
=
= player reacts
=
=================
*/

void ScrollScreen (void);

void PlayerWalkReact (void)
{
	ClipXMove ();
	CheckGround ();
	if ((obj.hitleft || obj.hitright) && obj.state != &s_davewalk1)
		ChangeState (&s_davestand);
	if (!obj.hitbottom)
		ChangeState (&s_davefall1);
	QueueSprite (obj.x,obj.y,obj.shapenum);
}

void PlayerStandReact (void)
{
	ClipXMove ();
	CheckGround ();
	if (!obj.hitbottom)
		ChangeState (&s_davefall1);
	QueueSprite (obj.x,obj.y,obj.shapenum);
}

void PlayerAirReact (void)
{
	ClipToWalls ();
	if (obj.hitleft || obj.hitright)
		obj.xspeed = 0;
	if (obj.hitbottom)
	{
		ResumeControlThink ();
		PlaySound (LANDSND);
		obj.x += obj.xdir<<6;
		CalcBounds ();
		ClipToWalls ();
	}
	else if (obj.hittop)
	{
		obj.yspeed = 0;
		PlaySound (HITHEADSND);
		ChangeState (&s_davefall1);
	}
	ScrollScreen ();
	QueueSprite (obj.x,obj.y,obj.shapenum);
}


// Nothing calls this: a leftover that restarts Dave's state timer.

void ClearDaveTics (void)
{
	objlist[0].ticcount = 0;
}


/*
=================
=
= CheckTiles
=
= The tiles the player covers: doors (intile 1 and 2, with the door's info in map plane
= 1), death (13) and the exit (99)
=
=================
*/

void CheckTiles (void)
{
	int type,tile,info;
	register int x,y;

	doorx = 0;
	for (x=obj.tileleft;x<=obj.tileright;x++)
		for (y=obj.tiletop;y<=obj.tilebottom;y++)
		{
			tile = mapplane[0][y*mapwwide+x];
			type = intile[tile];
			if (type)
			switch (type)
			{
			case 1:
			case 2:
				info = mapplane[1][y*mapwwide+x];
				if (info)
				{
					doortile = tile;
					doorinfo = info;
					doortype = type;
					doorx = x;
					doory = y;
				}
				break;
			case 13:
				playstate = ex_deadlytile;
				break;
			case 99:
				playstate = ex_completed;
				break;
			}
		}
}


/*
=================
=
= zombies (class 2)
=
=================
*/

void SpawnZombie (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (zombieobj,tilex<<8,(tiley<<8)-0x80,-1,zombiestart[RndT()/64]);
	new->temp2 = new->temp3 = 1;
	new->hitpoints = 2;
	new->ydir = 1;
}


void ZombieThink (void)
{
	int dy,dir;
	register int x,dx;

	dy = obj.bottom-objlist[0].bottom;
	dy = dy > 0 ? dy : -dy;
	if (obj.x > objlist[0].x)
	{
		dx = obj.left-objlist[0].right;
		dir = -1;
	}
	else
	{
		dx = objlist[0].left-obj.right;
		dir = 1;
	}
	if (dy < 0x200)
	{
		obj.xdir = dir;
		if (dx <= 0x80)
			ChangeState (&s_zombiegrab1);
	}

	dy = objlist[0].tilebottom-obj.tilebottom;
	if (dy > 0 && dy < 10)
	{
		dx = obj.tileleft-objlist[0].tileleft;
		dx = dx > 0 ? dx : -dx;
		if (dx < 6)
		{
			for (x=obj.tileleft;x<=obj.tileright;x++)
			{
				if (southwall[mapplane[0][(obj.tilebottom+1)*mapwwide+x]])
					return;
				if (!northwall[mapplane[0][(obj.tilebottom+2)*mapwwide+x]])
					return;
			}
			obj.ymove = 0x80;
			ChangeState (&s_zombiefall1);
		}
	}
}


void ZombieFallReact (void)
{
	if (southwall[mapplane[0][obj.tilebottom*mapwwide+obj.tileleft]]
	|| obj.tilebottom > objlist[0].tilebottom)
	{
		ClipYMove ();
		ChangeState (&s_zombiewalk1);
	}

	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
}


/*
=================
=
= throwers (class 3) and their bolts (class 16)
=
=================
*/

void SpawnThrower (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (throwerobj,tilex<<8,(tiley<<8)+0x80,-1,&s_throwerwalk1);
	new->temp2 = new->temp3 = 1;
	new->hitpoints = 2;
}


void ThrowerThink (void)
{
	int dir;
	register int dy,dx;

	dy = obj.bottom-objlist[0].bottom;
	dy = dy > 0 ? dy : -dy;
	if (obj.x > objlist[0].x)
	{
		dx = obj.left-objlist[0].right;
		dir = -1;
	}
	else
	{
		dx = objlist[0].left-obj.right;
		dir = 1;
	}
	if (dy <= 0x100)
	{
		if (dx > 0x700)
		{
			obj.xdir = dir;
			if (RndT() < 50)
			{
				PlaySound (KNIFETHROWSND);
				ChangeState (&s_throwerthrow1);
			}
		}
		else if (dx < 0x300)
			obj.xdir = -dir;
	}
	else if (RndT() < 2)
		obj.xdir = -obj.xdir;
}


void ThrowThink (void)
{
	if (obj.xdir > 0)
		SpawnNewObj (knifeobj,obj.x+0x200,obj.y,1,&s_knife);
	else
		SpawnNewObj (knifeobj,obj.x,obj.y,-1,&s_knife);
	new->temp2 = 0;
	new->active = 1;
	obj.thinkdelay = 4;
}


void BoltReact (void)
{
	ClipXMove ();
	if (obj.hitleft || obj.hitright)
		ChangeState (&s_shothit);
	QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= crawlers (class 4): on the floor, or on the ceiling when there is no floor
=
=================
*/

void SpawnSlime (register unsigned tilex, register unsigned tiley)
{
	if (northwall[mapplane[0][(tiley+1)*mapwwide+tilex]])
	{
		SpawnNewObj (slimeobj,tilex<<8,(tiley<<8)+0x80,1,&s_slimefloor1);
		new->ydir = 1;
	}
	else
	{
		SpawnNewObj (slimeobj,tilex<<8,tiley<<8,1,&s_slimeceiling1);
		new->ydir = -1;
	}
	new->temp2 = new->temp3 = 1;
	new->hitpoints = 1;
}


void SlimeThink (void)
{
	if (RndT() < 32)
		obj.xdir = -obj.xdir;
	if (RndT() < 16)
	{
		if (obj.ydir > 0)
		{
			obj.ydir = -1;
			ChangeState (&s_slimedrop1);
		}
		else
		{
			obj.ydir = 1;
			ChangeState (&s_slimerise1);
		}
	}
}


void SlimeFlipReact (void)
{
	ClipToWalls ();
	if (obj.ydir == -1 && obj.hittop)
	{
		ChangeState (&s_slimeceilingland);
		PlaySound (SLIMESLOPSND);
	}
	else if (obj.ydir == 1 && obj.hitbottom)
	{
		ChangeState (&s_slimefloorland);
		PlaySound (SLIMESLOPSND);
	}
	QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= class 6
=
=================
*/

void SpawnGhost (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (ghostobj,tilex<<8,(tiley<<8)+0x80,1,&s_ghostfly1);
	new->temp2 = 0;
	new->hitpoints = 1;
}


void SpawnHiddenGhost (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (ghostobj,tilex<<8,(tiley<<8)+0x80,1,&s_ghostwait);
	new->ydir = 1;
	new->temp2 = 0;
	new->hitpoints = 1;
}


void GhostWaitThink (void)
{
	register unsigned dx;

	if (ABS(objlist[0].y-obj.y) < 0x200)
	{
		if (objlist[0].xdir > 0)
			dx = objlist[0].x-obj.x;
		else
			dx = obj.x-objlist[0].x;
		if (dx >= 0x200 && dx <= 0x500)
		{
			PlaySound (GHOSTFADESND);
			obj.ticcount = 0;
			ChangeState (&s_ghostappear1);
		}
	}
}


void GhostAppearThink (void)
{
	obj.temp2 = obj.temp3 = 1;
	PlaySound (GHOSTREALSND);
}


void GhostContact (register objtype *ob, register objtype *hit)
{
	if (hit->obclass == daveobj)
	{
		ob->state = &s_ghostappear4;
		ob->shapenum = s_ghostappear4.rightshapenum;
		ob->react = s_ghostappear4.react;
		ob->contact = s_ghostappear4.contact;
	}
}


void GhostFlyThink (void)
{
	register unsigned i;
	int xaccel,yaccel;

	xaccel = objlist[0].x > obj.x ? 1 : -1;
	yaccel = objlist[0].y > obj.y ? 1 : -1;
	for (i=0;i<tics;i++)
	{
		obj.xspeed += xaccel;
		if (obj.xspeed < -20)
			obj.xspeed = -20;
		else if (obj.xspeed > 20)
			obj.xspeed = 20;
		obj.xmove += obj.xspeed;
		obj.yspeed += yaccel;
		if (obj.yspeed < -20)
			obj.yspeed = -20;
		else if (obj.yspeed > 20)
			obj.yspeed = 20;
		obj.ymove += obj.yspeed;
	}
	if (obj.xspeed > 0)
		obj.xdir = 1;
	else
		obj.xdir = -1;
}


/*
=================
=
= class 7
=
=================
*/

void SpawnWerewolf (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (werewolfobj,tilex<<8,tiley<<8,1,&s_werewolfwalk1);
	new->temp2 = new->temp3 = 1;
	new->hitpoints = 3;
}


void WerewolfThink (void)
{
	int dir;
	register int dy,dx;

	dy = obj.bottom-objlist[0].bottom;
	dy = dy > 0 ? dy : -dy;
	if (obj.x > objlist[0].x)
	{
		dx = obj.left-objlist[0].right;
		dir = -1;
	}
	else
	{
		dx = objlist[0].left-obj.right;
		dir = 1;
	}
	if (dy < 0x300)
	{
		obj.xdir = dir;
		if (dx < 0x600)
		{
			ChangeState (&s_werewolfjump1);
			obj.yspeed = -65;
		}
	}
}


void WerewolfWalkReact (void)
{
	register int tile;

	ClipXMove ();
	tile = northwall[mapplane[0][obj.xdir > 0 ? (obj.tilebottom+1)*mapwwide+obj.tileright
		: (obj.tilebottom+1)*mapwwide+obj.tileleft]];

	if ((obj.hitleft && obj.xdir == -1) || (obj.hitright && obj.xdir == 1))
	{
		obj.xdir = -obj.xdir;
		memcpy (&obj.oldleft,&obj.left,16);
		ChangeState (obj.state);
		CalcBounds ();
		ClipXMove ();
		obj.thinkdelay = 0;
	}
	else if (!tile)
	{
		ChangeState (&s_werewolfjump1);
		obj.yspeed = -65;
		obj.thinkdelay = 0;
	}

	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


void WerewolfFallThink (void)
{
	DoGravity ();
}


void WerewolfJumpReact (void)
{
	ClipToWalls ();
	if (obj.hitleft || obj.hitright)
	{
		obj.thinkdelay = 0;
		obj.temp7 = 1;
	}
	if (obj.hitbottom)
	{
		obj.thinkdelay = 7;
		if (obj.temp7)
		{
			obj.xdir = -obj.xdir;
			obj.temp7 = obj.thinkdelay = 0;
		}
		ChangeState (&s_werewolfwalk1);
		memcpy (&obj.oldleft,&obj.left,16);
		ChangeState (&s_werewolfwalk1);
		CalcBounds ();
		ClipToWalls ();
	}

	if (obj.temp1)
	{
		obj.temp1 = obj.temp1 - tics;
		if (obj.temp1 < 0)
			obj.temp1 = 0;
		QueueXorSprite (obj.x,obj.y,obj.shapenum);
	}
	else
		QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= class 8: a flier that homes on the player
=
=================
*/

void SpawnSkull (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (skullobj,tilex<<8,tiley<<8,1,&s_skullfly1);
	new->temp2 = 1;
	new->temp3 = 0;
}


void SkullFlyThink (void)
{
	register unsigned i;
	int xaccel,yaccel;

	xaccel = objlist[0].x > obj.x ? 1 : -1;
	yaccel = obj.y-0xc0 < objlist[0].y ? 1 : -1;
	for (i=0;i<tics;i++)
	{
		obj.xspeed += xaccel;
		if (obj.xspeed < -4)
			obj.xspeed = -4;
		else if (obj.xspeed > 4)
			obj.xspeed = 4;
		obj.xmove += obj.xspeed;
		obj.yspeed += yaccel;
		if (obj.yspeed < -4)
			obj.yspeed = -4;
		else if (obj.yspeed > 4)
			obj.yspeed = 4;
		obj.ymove += obj.yspeed;
	}
	if (obj.xspeed > 0)
		obj.xdir = 1;
	else
		obj.xdir = -1;
}


void SkullReact (void)		// the same as ClipDrawReact
{
	ClipToWalls ();
	QueueSprite (obj.x,obj.y,obj.shapenum);
	CheckOffScreen ();
}


/*
=================
=
= class 5: drops on a thread when the player passes below (temp4: where it hangs from)
=
=================
*/

void SpawnSpider (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (spiderobj,tilex<<8,tiley<<8,1,&s_spiderwalk1);
	new->temp2 = new->temp3 = 1;
	new->hitpoints = 1;
	new->temp4 = new->y;
}


void SpiderThink (void)
{
	register int dx;

	dx = objlist[0].x-obj.x;
	if (objlist[0].y-obj.y < 0x600 && ABS(dx) < 0xa0)
	{
		obj.ydir = 1;
		ChangeState (&s_spiderdrop);
	}
	else if (objlist[0].x > obj.x && obj.xdir == -1)
	{
		obj.xdir = 1;
		ChangeState (obj.state);
	}
	else if (objlist[0].x < obj.x && obj.xdir == 1)
	{
		obj.xdir = -1;
		ChangeState (obj.state);
	}
}


void SpiderDropReact (void)
{
	register int x;
	register unsigned y;

	ClipXMove ();
	for (x=obj.tileleft;x<=obj.tileright;x++)
		if (southwall[mapplane[0][obj.tilebottom*mapwwide+x]])
		{
			obj.ydir = -1;
			ChangeState (&s_spiderrise);
			break;
		}
	if (obj.y-obj.temp4 > 0x600)
	{
		obj.ydir = -1;
		ChangeState (&s_spiderrise);
	}
	for (y=obj.temp4;obj.y+0x80 > y;y+=0x80)
		QueueSprite (obj.x+0x40,y,0x9a);
	QueueSprite (obj.x,obj.y,0x99);
}


void SpiderRiseReact (void)
{
	register unsigned y;

	ClipXMove ();
	if (obj.y <= obj.temp4)
	{
		obj.y = obj.temp4;
		ChangeState (&s_spiderwalk1);
	}
	else
	{
		for (y=obj.temp4;obj.y+0x80 > y;y+=0x80)
			QueueSprite (obj.x+0x40,y,0x9a);
		QueueSprite (obj.x,obj.y,0x99);
	}
}


/*
=================
=
= class 10: the first boss
=
=================
*/

void SpawnFrank (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (frankobj,tilex<<8,tiley<<8,-1,&s_frankwalk1);
	new->temp2 = 1;
	new->temp3 = 1;
	new->hitpoints = 15;
}


void FrankThink (void)
{
	obj.temp4++;
	if (obj.temp4 > 10)
	{
		obj.temp4 -= 10;
		ChangeState (&s_frankthrow1);
	}
}


void FrankFireThink (void)
{
	if (obj.xdir > 0)
		SpawnNewObj (fireballobj,obj.right,obj.y+0x100,1,&s_fireballstart1);
	else
		SpawnNewObj (fireballobj,obj.left-0x100,obj.y+0x100,-1,&s_fireballstart1);
	new->ydir = 1;
	new->temp2 = new->temp3 = new->hitpoints = 1;
}


/*
=================
=
= fireballs (class 17): bounce a dozen times
=
=================
*/

void FireballThink (void)
{
	obj.xmove = obj.xdir*tics*24;
	obj.ymove = obj.ydir*tics*24;
}


void FireballTimeThink (void)
{
	if (++obj.temp5 >= 24)
		ChangeState (&s_fireball1);
}


void FireballReact (void)
{
	ClipToWalls ();
	if (obj.hittop)
	{
		obj.ydir = 1;
		obj.temp4++;
	}
	else if (obj.hitbottom)
	{
		obj.ydir = -1;
		obj.temp4++;
	}
	if (obj.hitleft)
	{
		obj.xdir = 1;
		obj.temp4++;
	}
	else if (obj.hitright)
	{
		obj.xdir = -1;
		obj.temp4++;
	}
	if (obj.temp4 > 12)
		obj.active = obj.obclass = 0;
	QueueSprite (obj.x,obj.y,obj.shapenum);
}


/*
=================
=
= class 11: the second boss, with its minions (class 14) and shots (class 15)
=
=================
*/

void SpawnMaster (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (masterobj,tilex<<8,tiley<<8,1,&s_master1);
	new->temp2 = 1;
	new->temp3 = 1;
	new->hitpoints = 15;
}


void MasterThink (void)
{
	if (obj.xdir > 0)
	{
		if (obj.x > 0xf00)
			obj.xdir = -1;
	}
	else if (obj.x < 0x200)
		obj.xdir = 1;
	obj.x += tics*obj.xdir*12;

	if (((objtype *)obj.temp4)->obclass != handobj
	&& (obj.temp8 = obj.temp8+tics) > 300)
	{
		obj.temp8 = 0;
		SpawnNewObj (handobj,obj.x,obj.y+0x300,-1,&s_handappear);
		new->temp2 = new->temp3 = 1;
		new->hitpoints = 2;
		new->temp4 = 8;
		obj.temp4 = (int)new;
		ChangeState (&s_master3);
	}

	if (((objtype *)obj.temp5)->obclass != handobj
	&& (obj.temp7 = obj.temp7+tics) > 300)
	{
		obj.temp8 = 0;			// sic: temp7 is this one's timer
		SpawnNewObj (handobj,obj.x+0x400,obj.y+0x300,-1,&s_handappear);
		new->temp2 = new->temp3 = 1;
		new->hitpoints = 2;
		new->temp4 = 6;
		obj.temp5 = (int)new;
		ChangeState (&s_master3);
	}

	obj.temp6 = obj.temp6+tics;
	if (obj.temp6 > 330)
	{
		obj.temp7 = obj.temp6 = 0;
		ChangeState (&s_masterspit1);
		PlaySound (MASTFIRESND);
	}
}


void MasterSpitThink (void)
{
	obj.temp7 = obj.temp7+tics;
	if (obj.temp7 > 60)
	{
		SpawnNewObj (mastershotobj,obj.x,obj.y+0x300,-1,&s_mastershot1);
		new->xspeed = -30;
		new->yspeed = 30;
		SpawnNewObj (mastershotobj,obj.x+0x100,obj.y+0x400,-1,&s_mastershot1);
		new->xspeed = -16;
		new->yspeed = 40;
		SpawnNewObj (mastershotobj,obj.x+0x300,obj.y+0x400,-1,&s_mastershot1);
		new->xspeed = 16;
		new->yspeed = 40;
		SpawnNewObj (mastershotobj,obj.x+0x400,obj.y+0x300,-1,&s_mastershot1);
		new->xspeed = 30;
		new->yspeed = 30;
		ChangeState (&s_master1);
	}
}


void HandThink (void)
{
	register unsigned i;
	int xaccel,yaccel;

	xaccel = objlist[0].x > obj.x ? 1 : -1;
	yaccel = objlist[0].y > obj.y ? 1 : -1;
	for (i=0;i<tics;i++)
	{
		obj.xspeed += xaccel;
		if (obj.xspeed < -obj.temp4)
			obj.xspeed = -obj.temp4;
		else if (obj.xspeed > obj.temp4)
			obj.xspeed = obj.temp4;
		obj.xmove += obj.xspeed;
		obj.yspeed += yaccel;
		if (obj.yspeed < -obj.temp4)
			obj.yspeed = -obj.temp4;
		else if (obj.yspeed > obj.temp4)
			obj.yspeed = obj.temp4;
		obj.ymove += obj.yspeed;
	}
	if (obj.xspeed > 0)
		obj.xdir = 1;
	else
		obj.xdir = -1;
}


void MasterShotThink (void)
{
	obj.xmove = obj.xspeed*tics;
	obj.ymove = obj.yspeed*tics;
}


/*
=================
=
= class 12
=
=================
*/

void SpawnDelbert (unsigned tilex, unsigned tiley)
{
	SpawnNewObj (delbertobj,tilex<<8,tiley<<8,1,&s_delbert1);
	new->xdir = -1;
	new->ydir = 1;
}


void DrawOnlyReact (void)
{
	QueueSprite (obj.x,obj.y,obj.shapenum);
}


void DelbertFallReact (void)
{
	ClipYMove ();
	if (obj.hitbottom)
		ChangeState (&s_spherebreak1);
	QueueSprite (obj.x,obj.y,obj.shapenum);
}


void WinThink (void)
{
	playstate = ex_completed;
}


/*
=================
=
= ScanInfoPlane
=
= Spawns the objects map plane 1 places, and notes which sprite sets they need
=
=================
*/


void ScanInfoPlane (void)
{
	unsigned info;
	register unsigned x,y;

	memset (needsprites,0,sizeof(needsprites));
	needsprites[0] = 1;
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwwide;x++)
		{
			info = mapplane[1][y*mapwwide+x];
			switch (info)
			{
			case 1:
				needsprites[1] = 1;
				SpawnZombie (x,y);
				break;
			case 2:
				needsprites[2] = 1;
				SpawnThrower (x,y);
				break;
			case 3:
				needsprites[4] = 1;
				SpawnSlime (x,y);
				break;
			case 4:
				needsprites[3] = 1;
				SpawnGhost (x,y);
				break;
			case 5:
				needsprites[3] = 1;
				SpawnHiddenGhost (x,y);
				break;
			case 6:
				needsprites[5] = 1;
				SpawnSpider (x,y);
				break;
			case 7:
				needsprites[6] = 1;
				SpawnSkull (x,y);
				break;
			case 9:
				needsprites[7] = 1;
				SpawnWerewolf (x,y);
				break;
			case 10:
				needsprites[4] = 1;
				break;
			case 11:
				needsprites[8] = 1;
				SpawnFrank (x,y);
				break;
			case 12:
				needsprites[9] = 1;
				SpawnMaster (x,y);
				break;
			case 13:
				needsprites[9] = 1;
				SpawnDelbert (x,y);
				break;
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				SpawnBonus (x,y,info-14);
				break;
			case 255:
				SpawnPlayer (x,y);
				break;
			}
		}
}
