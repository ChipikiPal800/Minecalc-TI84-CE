// ============================================================================
//  MI/NECALC  -  Minecraft for the TI-84 Plus CE            v0.4.0 "Polish"
//  FILE 1/3: src/main.cpp  (bottom: #include "main_part2.h" -> "main_part3.h")
//  Delete from src/: stuffs.s, haze_lut.h, _recovered_arrays.txt. Only stuff.asm is built.
//  ASM-pinned limits: WX=WY=WZ=32, VD=15, res=5, reg_block_num=22
// ============================================================================
#include <graphx.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <ti/vars.h>
#include <keypadc.h>
#include <sys/util.h>
#include <sys/timers.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fileioc.h>
#include "gfx/gfx.h"
#include "textures.h"
#define AUTO_GENERATE_GLOBALS
#include <tice.h>
#include <time.h>
#include <cstdlib>
#include <cstdint>

#define WX 32
#define WY 32
#define WZ 32
#define XJ (WY*WZ)
#define YJ WZ
#define ZJ 1
#define XWS 10
#define YWS 5
#define WORLD_BYTES (WX*WY*WZ)
#define vis_map(x, y, z) world[((x) << XWS) + ((y) << YWS) + (z)]
#define VD 15
#define res 5
#define AVERAGEFPS 7
#define INTERACTD 6
#define HOTBAR_N 9
uint8_t bg_color=125;

#define IRONNUM 75
#define DIAMONDNUM 20
#define REDSTONENUM 40
#define GOLDNUM 40
#define COALNUM 70
#define GRAVELNUM 7
#define OBSIDIANNUM 30
#define NETHERGOLDORENUM 50
#define TUNNELNUM 5
#define TUNNELWOBBLEF 10
#define TUNNELWOBBLEU 5
#define NETHERBGCOLOR 192

// ============================================================================
//  BLOCK / ITEM IDS  (append-only)
// ============================================================================
#define AIR 0
#define GRASS 1
#define STONE 2
#define WOOD 3
#define LEAF 4
#define BARRIER 5
#define UNLITLAMP 6
#define LITLAMP 7
#define UNLITBUTTON 8
#define LITBUTTON 9
#define SAND 10
#define WOOL 11
#define SNOWGRASS 12
#define SNOW 13
#define NETHERRACK 14
#define OBSIDIAN 15
#define WATERBLOCK 16
#define WATERFALL 17
#define IRONBLOCK 18
#define GOLDBLOCK 19
#define DIAMONDBLOCK 20
#define REDSTONEBLOCK 21
#define reg_block_num 22
#define RBP1 (reg_block_num+1)
#define FLOORNUM 5
#define FLOORVOXELS reg_block_num
#define SNOWCARPET (FLOORVOXELS+1)
#define REDSTONEDUSTUNLIT (FLOORVOXELS+2)
#define REDSTONEDUSTLIT (FLOORVOXELS+3)
#define WATERFLOOR (FLOORVOXELS+4)
#define WATERSLAB (FLOORVOXELS+5)
#define SLABSTART WATERSLAB
#define FLOORVOXELSEND WATERSLAB
#define CRAFTTABLE (FLOORVOXELSEND+1)
#define FURNACE (FLOORVOXELSEND+2)
#define PLANKS (FLOORVOXELSEND+3)
#define BEDTOP (FLOORVOXELSEND+4)
#define GLASS (FLOORVOXELSEND+5)
#define DOORTOPOPEN (FLOORVOXELSEND+6)
#define DOORBOTTOMOPEN (FLOORVOXELSEND+7)
#define DOORTOPCLOSED (FLOORVOXELSEND+8)
#define DOORBOTTOMCLOSED (FLOORVOXELSEND+9)
#define MAGMABLOCK (FLOORVOXELSEND+10)
#define FIRE (FLOORVOXELSEND+11)
#define SANDSTONE (FLOORVOXELSEND+12)
#define CACTUS (FLOORVOXELSEND+13)
#define IRON (FLOORVOXELSEND+14)
#define GOLD (FLOORVOXELSEND+15)
#define DIAMOND (FLOORVOXELSEND+16)
#define REDSTONE (FLOORVOXELSEND+17)
#define COAL (FLOORVOXELSEND+18)
#define TNT (FLOORVOXELSEND+19)
#define LITTNT (FLOORVOXELSEND+20)
#define SMOKE (FLOORVOXELSEND+21)
#define GRAVEL (FLOORVOXELSEND+22)
#define NETHERPORTAL (FLOORVOXELSEND+23)
#define NETHERGOLDORE (FLOORVOXELSEND+24)
#define STONEBRICK (FLOORVOXELSEND+25)
#define BOOKSHELF (FLOORVOXELSEND+26)
#define LADDER (FLOORVOXELSEND+27)
#define PUMPKIN (FLOORVOXELSEND+28)
#define GLOWSTONE (FLOORVOXELSEND+29)
#define SOULSAND (FLOORVOXELSEND+30)
#define SAPLING (FLOORVOXELSEND+31)
#define TORCH (FLOORVOXELSEND+32)
#define FURNACE_1 (FLOORVOXELSEND+33)
#define FURNACE_2 (FLOORVOXELSEND+34)
#define FURNACE_3 (FLOORVOXELSEND+35)
#define PUMPKIN_1 (FLOORVOXELSEND+36)
#define PUMPKIN_2 (FLOORVOXELSEND+37)
#define PUMPKIN_3 (FLOORVOXELSEND+38)
#define TEXTURENUM 38
#define TEXTURED_REG_START IRONBLOCK
#define invenblocknum (TEXTURENUM+reg_block_num+FLOORNUM)
#define ITEMBASE (invenblocknum+1)
#define WOODPIC (ITEMBASE+0)
#define STONEPIC (ITEMBASE+1)
#define GOLDPIC (ITEMBASE+2)
#define IRONPIC (ITEMBASE+3)
#define DIAMONDPIC (ITEMBASE+4)
#define NETHERITEPIC (ITEMBASE+5)
#define STICK (ITEMBASE+6)
#define WOODSWORD (ITEMBASE+7)
#define STONESWORD (ITEMBASE+8)
#define GOLDSWORD (ITEMBASE+9)
#define IRONSWORD (ITEMBASE+10)
#define DIAMONDSWORD (ITEMBASE+11)
#define IRONINGOT (ITEMBASE+12)
#define GOLDINGOT (ITEMBASE+13)
#define RAWMUTTON (ITEMBASE+14)
#define COOKEDMUTTON (ITEMBASE+15)
#define ROTTENFLESH (ITEMBASE+16)
#define DIAMONDITEM (ITEMBASE+17)
#define COALITEM (ITEMBASE+18)
#define FLINTANDSTEEL (ITEMBASE+19)
#define FLINT (ITEMBASE+20)
#define BUCKET (ITEMBASE+21)
#define WATERBUCKET (ITEMBASE+22)
#define RAWPORKCHOP (ITEMBASE+23)
#define COOKEDPORKCHOP (ITEMBASE+24)
#define GUNPOWDER (ITEMBASE+25)
#define APPLE (ITEMBASE+26)
#define GOLDENAPPLE (ITEMBASE+27)
#define NUM_ITEMS 28
#define item_num (NUM_ITEMS+1)
#define MOBSTART (reg_block_num+FLOORNUM+TEXTURENUM+1)
#define SKELETOPZ (MOBSTART+0)
#define SKELETOPX (MOBSTART+1)
#define SKELEBOTTOM (MOBSTART+2)
#define SHEEPFACEX (MOBSTART+3)
#define SHEEPFACEZ (MOBSTART+4)
#define SHEEPBACKX (MOBSTART+5)
#define SHEEPBACKZ (MOBSTART+6)
#define ZOMBIETOPX (MOBSTART+7)
#define ZOMBIETOPZ (MOBSTART+8)
#define ZOMBIELEGS (MOBSTART+9)
#define PIGFACEX (MOBSTART+10)
#define PIGFACEZ (MOBSTART+11)
#define PIGBACKX (MOBSTART+12)
#define PIGBACKZ (MOBSTART+13)
#define PIGLINTOPX (MOBSTART+14)
#define PIGLINTOPZ (MOBSTART+15)
#define PIGLINLEGS (MOBSTART+16)
#define CREEPERTOPX (MOBSTART+17)
#define CREEPERTOPZ (MOBSTART+18)
#define CREEPERLEGS (MOBSTART+19)
#define MOBHURT (MOBSTART+20)
#define MOBNUM 21
#define block_num (MOBSTART+MOBNUM)
#define INV_SIZE (block_num+item_num)

#define SAVE_MAGIC 0xFEED
#define SAVE_VERSION 3
#define PLAYER_VERSION 4
#define LEGACY_MOBSTART 52
#define LEGACY_ITEMBASE 52

static uint8_t floorcolorsconst[]={255,96,224,30,28};
uint8_t* floorcolors=floorcolorsconst-(FLOORVOXELS+1);
static inline int iabs(int v){return v<0?-v:v;}
static inline int imin(int a,int b){return a<b?a:b;}
static inline int imax(int a,int b){return a>b?a:b;}

// ============================================================================
//  BLOCK PROPERTY TABLES  (sized block_num: every world byte is a valid id)
// ============================================================================
#define F_WATER 1
#define F_WALK  2
#define F_BLAST 4
#define F_CLIMB 8
static uint8_t bflags[block_num];
static uint8_t hardness[block_num];
static uint8_t save_map[block_num];
static inline bool iswater(uint8_t v){return (bflags[v]&F_WATER)!=0;}
static inline bool walkable(uint8_t v){return (bflags[v]&F_WALK)!=0;}
static const uint8_t hardness_init[MOBSTART]=
{
	0,1,2,1,0,255,1,1,1,1, 1,1,1,1,2,5,255,255,3,4,4,4,
	0,
	0,0,0,255,255,
	1,2,1,1,1,1,1,1,1,2,0,2,1,
	3,4,4,4,2,1,255,0,1,0,2,
	2,1,1,1,1,1,0,
	0,2,2,2,1,1,1
};
static void init_block_tables()
{
	for(int i=0;i<block_num;i++)
	{
		bflags[i]=0;
		if(i==AIR||i>=MOBSTART||(i>reg_block_num&&i<=FLOORVOXELSEND)) bflags[i]|=F_WALK;
		save_map[i]=(i>=MOBSTART)?AIR:(uint8_t)i;
		hardness[i]=(i<MOBSTART)?hardness_init[i]:255;
	}
	bflags[FIRE]|=F_WALK; bflags[SMOKE]|=F_WALK; bflags[NETHERPORTAL]|=F_WALK; bflags[SAPLING]|=F_WALK; bflags[TORCH]|=F_WALK;
	bflags[WATERBLOCK]|=F_WALK|F_WATER|F_BLAST; bflags[WATERFALL]|=F_WALK|F_WATER|F_BLAST;
	bflags[WATERSLAB]|=F_WATER|F_BLAST; bflags[WATERFLOOR]|=F_WATER|F_BLAST;
	bflags[LADDER]|=F_WALK|F_CLIMB;
	bflags[OBSIDIAN]|=F_BLAST; bflags[BARRIER]|=F_BLAST;
	save_map[SMOKE]=AIR; save_map[LITTNT]=TNT;
}

// ============================================================================
//  GLOBAL GAME STATE
// ============================================================================
static uint8_t* world;
static unsigned short* world_changes[MOBSTART];
static uint16_t world_changes_sizes[MOBSTART];//also the chunk-file size table (save + load)
static int inventory[INV_SIZE];
int focal_length=200;
uint8_t world_type=0, gamemode=0, difficulty=1;
uint8_t set_fog=1, set_debug=1, set_mobspawn=1, set_names=1;
uint8_t set_gfx=0, set_tex=1, set_dist=1, set_fov=1, set_cross=0, set_outline=1, set_held=1, set_autojump=1, set_look=1;
uint8_t realm=0;
kb_lkey_t key_fwd=kb_Key2nd;
kb_lkey_t key_back=kb_KeyAlpha;
kb_lkey_t key_jump=kb_KeyMode;
kb_lkey_t key_mine=kb_KeyDel;
kb_lkey_t key_place=kb_KeyStat;
kb_lkey_t key_lookfast=kb_KeyMath;
kb_lkey_t key_help=kb_KeyApps;
int8_t playerhp=20, playerhunger=20;
#define MAXAIR 10
int8_t playerair=MAXAIR;
uint8_t ghotbar[HOTBAR_N]={};
int hotbarblock=0;
int updatehotbar=2;
int wseed=0;
uint8_t biome=0;
uint8_t chunkx=32, chunkz=32, respawnchunkx=32, respawnchunkz=32;
int respawnx=16, respawny=WY-1, respawnz=16;
int death_x=0, death_y=0, death_z=0;
uint32_t offsettime=3932160UL;
clock_t start_time=0;
clock_t session_mark=0;
char origwname[9]={};
char wname[9]={};
uint8_t realmcooldown=0;
bool portal_must_exit=false;
bool open_crafting_table=false;
bool flying=false;
uint8_t hurt_timer=0;
static char toast_msg[40]={};
static uint8_t toast_timer=0;
static uint8_t name_timer=0;
static bool save_failed=false;
struct Stats { uint32_t mined, placed, killed, deaths, play_ticks; };
static Stats stats={0,0,0,0,0};
static void toast(const char* s){ strncpy(toast_msg,s,39); toast_msg[39]=0; toast_timer=AVERAGEFPS*3; }

struct Mob;
static bool ch_valid=false;//crosshair cache (see crosshair())
void make_block_fall(uint8_t* block,uint8_t);
void add_update(uint8_t* block,uint8_t optionaldata=0);
void add_updates_fully(uint8_t* block);
int break_block(uint8_t* block);
void break_portal(uint8_t* block);
void explode(uint8_t* block,uint8_t r,Mob* src=nullptr);
void draw_tree(int x,int y,int z);
void address2coords(uint8_t* block, uint8_t&x,uint8_t&y,uint8_t&z)
{
	int nblock = block - world;
	z = nblock & (WZ-1); nblock >>= YWS;
	y = nblock & (WY-1); nblock >>= YWS;
	x = nblock;
}

// ============================================================================
//  BLOCK UPDATES (water, falling blocks, fire, tnt, smoke, saplings)
// ============================================================================
uint8_t countblockstaxi(uint8_t* block,uint8_t value)
{
	uint8_t n=0;
	if(*(block+XJ)==value) n++;
	if(*(block-XJ)==value) n++;
	if(*(block+1)==value) n++;
	if(*(block-1)==value) n++;
	return n;
}
void wateronespread(uint8_t* block, uint8_t value)
{
	uint8_t below=*(block-YJ);
	if(*block==0 && (below==WATERSLAB||below==WATERFLOOR))
	{
		*(block-YJ)=WATERFALL; *block=AIR; add_update(block-YJ); return;
	}
	if(below==WATERBLOCK) return;
	if(countblockstaxi(block,WATERBLOCK))
	{
		value=WATERSLAB;
		if(countblockstaxi(block,WATERBLOCK)>1)
		{
			value=WATERBLOCK;
			if(*block==WATERSLAB){ *block=WATERBLOCK; add_update(block); }
		}
		if(*block==WATERFLOOR||*block==0){ *block=value; add_update(block); return; }
	}
	if(*block==0){ *block=value; add_update(block); }
}
void waterplusspread(uint8_t* block, uint8_t value)
{
	wateronespread(block+XJ,value); wateronespread(block-XJ,value);
	wateronespread(block+1,value);  wateronespread(block-1,value);
}
void liquidblockspread(uint8_t* block,uint8_t)
{
	if(*(block-YJ)==0){ *(block-YJ)=WATERFALL; add_update(block-YJ); }
	waterplusspread(block,WATERSLAB);
}
void liquidslabspread(uint8_t* block,uint8_t)
{
	if(*(block-YJ)==0){ *(block-YJ)=WATERFALL; add_update(block-YJ); }
	uint8_t a=countblockstaxi(block,WATERFALL);
	uint8_t b=countblockstaxi(block,WATERBLOCK);
	a+=b;
	if(!a){ *block=AIR; add_updates_fully(block); return; }
	if(b>1){ *block=WATERBLOCK; add_update(block); return; }
	waterplusspread(block,WATERFLOOR);
}
void liquidfloorspread(uint8_t* block,uint8_t)
{
	if(*(block-YJ)==0){ *(block-YJ)=WATERFALL; add_update(block-YJ); }
	if(!countblockstaxi(block,WATERSLAB)){ *block=AIR; add_updates_fully(block); }
}
void liquidfallspread(uint8_t* block,uint8_t)
{
	uint8_t below=*(block-YJ);
	if(below==AIR||below==WATERFLOOR||below==WATERSLAB){ *(block-YJ)=WATERFALL; add_update(block-YJ); }
	uint8_t above=*(block+YJ);
	if(!iswater(above)){ *block=AIR; add_updates_fully(block); return; }
	if(!iswater(below)&&below!=AIR) waterplusspread(block,WATERSLAB);
}
void update_lit_tnt(uint8_t* block,uint8_t ttl)
{
	if(*(block-YJ)==0)
	{
		*block=0; add_updates_fully(block);
		block-=YJ; *block=LITTNT; add_updates_fully(block);
	}
	if(ttl==0) add_update(block,20);
	else if(ttl>1) add_update(block,ttl-1);
	else { *block=AIR; explode(block,3); }
}
void update_fire(uint8_t* block,uint8_t ttl)
{
	if(ttl==0) add_update(block,20);
	else if(ttl>1)
	{
		uint8_t blw=*(block-YJ);
		if(!(blw==WOOD||blw==PLANKS||blw==CRAFTTABLE||blw==NETHERRACK||blw==BOOKSHELF||blw==SOULSAND))
			add_update(block,ttl-1);
	}
	else { *block=AIR; add_updates_fully(block); }
}
void update_smoke(uint8_t* block,uint8_t ttl)
{
	if(ttl==0) add_update(block,4);
	else if(ttl>1) add_update(block,ttl-1);
	else { *block=AIR; add_updates_fully(block); }
}
void update_sapling(uint8_t* block,uint8_t ttl)
{
	uint8_t below=*(block-YJ);
	if(below!=GRASS&&below!=SNOWGRASS){ *block=AIR; return; }
	if(ttl==0){ add_update(block,(uint8_t)randInt(60,200)); return; }
	if(ttl>1){ add_update(block,ttl-1); return; }
	uint8_t x,y,z;
	address2coords(block,x,y,z);
	if(y<=WY-5&&x>=2&&x<=WX-3&&z>=2&&z<=WZ-3&&vis_map(x,y+1,z)==0&&vis_map(x,y+2,z)==0&&vis_map(x,y+3,z)==0)
		draw_tree(x,y,z);
	else add_update(block,100);
}
struct block2update { uint16_t block; uint8_t data; };
struct id2func
{
	uint8_t block;
	void (*bfunction)(uint8_t*, uint8_t);
	id2func(uint8_t a, void (*b)(uint8_t*,uint8_t)) : block(a), bfunction(b) {}
};
#define BSTACK_N 1024
static block2update bstack[BSTACK_N];
#define UPDATENUM 10
static id2func block_updates[UPDATENUM]={id2func(SAND,make_block_fall),id2func(GRAVEL,make_block_fall),id2func(FIRE,update_fire),id2func(LITTNT,update_lit_tnt),id2func(SMOKE,update_smoke),id2func(WATERBLOCK,liquidblockspread),id2func(WATERSLAB,liquidslabspread),id2func(WATERFLOOR,liquidfloorspread),id2func(WATERFALL,liquidfallspread),id2func(SAPLING,update_sapling)};
int bstacktop=0;
void make_block_fall(uint8_t* block,uint8_t)
{
	if(*(block-YJ)==0)
	{
		uint8_t id=*block; *block=0; add_updates_fully(block);
		block-=YJ; *block=id; add_updates_fully(block);
	}
}
void call_update();
static bool canupdatecleanup=true;
static bool updating_blocks=false;
void add_update(uint8_t* block,uint8_t optionaldata)
{
	canupdatecleanup=true;
	if(block>=world&&block<world+WORLD_BYTES)
	{
		if(bstacktop>BSTACK_N-2) call_update();
		if(bstacktop<BSTACK_N)
		{
			for(int i=0;i<UPDATENUM;++i)
			{
				if(block_updates[i].block==*block)
				{
					bstack[bstacktop].block=(uint16_t)(block-world);
					bstack[bstacktop].data=optionaldata;
					bstacktop++;
					return;
				}
			}
		}
	}
}
void add_updates_fully(uint8_t* block)
{
	add_update(block);
	add_update(block-XJ); add_update(block+XJ);
	add_update(block-YJ); add_update(block+YJ);
	add_update(block+1);  add_update(block-1);
}
void update_cleanup()
{
	uint8_t* end=world+WORLD_BYTES;
	for(uint8_t* w=world;w<end;++w)
	{
		if(*w==SMOKE) *w=AIR;
		else if(*w==LITTNT) *w=TNT;
	}
}
void call_update()
{
	if(updating_blocks)
	{
		if(canupdatecleanup){ update_cleanup(); canupdatecleanup=false; }
		return;
	}
	updating_blocks=true;
	canupdatecleanup=true;
	int countmax=bstacktop;
	for(int count=0;count<countmax;count++)
	{
		block2update temp=bstack[count];
		uint8_t curid=world[temp.block];
		for(int i=0;i<UPDATENUM;++i)
			if(block_updates[i].block==curid){ block_updates[i].bfunction(world+temp.block,temp.data); break; }
	}
	int newcount=bstacktop-countmax;
	if(newcount>0) memmove(bstack,bstack+countmax,newcount*sizeof(block2update));
	bstacktop=newcount>0?newcount:0;
	updating_blocks=false;
}
static void requeue_block_updates()
{
	for(int i=0;i<WORLD_BYTES;i++)
		if(world[i]==SAPLING||world[i]==LITTNT||world[i]==FIRE) add_update(world+i);
}

// ============================================================================
//  FIXED POINT  (9-bit fraction, integer only: no float anywhere)
// ============================================================================
#define shift_by 9
struct flint
{
	flint() {}
	flint(const flint& other) = default;
	flint(int other) { data = other << shift_by; }
	int data=0;
	inline flint& operator+=(int o){ data+=o<<shift_by; return *this; }
	inline flint& operator+=(flint o){ data+=o.data; return *this; }
	inline flint& operator-=(int o){ data-=o<<shift_by; return *this; }
	inline flint& operator-=(flint o){ data-=o.data; return *this; }
	inline flint& operator*=(int o){ data*=o; return *this; }
	inline flint& operator*=(flint o){ data=data*o.data>>shift_by; return *this; }
	inline flint& operator/=(int o){ data/=o; return *this; }
	inline flint& operator=(int o){ data=o<<shift_by; return *this; }
	inline flint& operator=(flint o){ data=o.data; return *this; }
	inline operator int() const { return data>>shift_by; }
	inline bool operator>(flint o) const { return data > o.data; }
	inline bool operator<(flint o) const { return data < o.data; }
	inline bool operator==(flint o) const { return data == o.data; }
	inline bool operator>=(flint o) const { return data >= o.data; }
	inline bool operator<=(flint o) const { return data <= o.data; }
	inline bool operator!=(flint o) const { return data != o.data; }
	inline bool operator>(int o) const { return data > (o << shift_by); }
	inline bool operator<(int o) const { return data < (o << shift_by); }
	inline bool operator==(int o) const { return (data>>shift_by) == o; }
	inline bool operator>=(int o) const { return (data>>shift_by) >= o; }
	inline bool operator<=(int o) const { return (data>>shift_by) <= o; }
	inline bool operator!=(int o) const { return (data>>shift_by) != o; }
	inline flint operator+(flint o) const { flint r=*this; return r+=o; }
	inline flint operator-(flint o) const { flint r=*this; return r-=o; }
	inline flint operator*(flint o) const { flint r=*this; return r*=o; }
	inline flint operator-() const { flint r; r.data=-data; return r; }
	inline flint operator+(int o) const { flint r=*this; return r+=o; }
	inline flint operator-(int o) const { flint r=*this; return r-=o; }
	inline flint operator*(int o) const { flint r=*this; return r*=o; }
	inline flint operator/(int o) const { flint r=*this; return r/=o; }
};
static inline flint fraw(int d){ flint f; f.data=d; return f; }
static flint cache[640]={};//per-column cos/sin*xdir for the current frame
#define INV_N 800
static int inv_abs[INV_N];//262144/|i|  (replaces the 1500-entry signed table)
#define INV(i) inv_abs[(i)<0?-(i):(i)]
static int rdiv(int a,int b){ return (a>=0?a+(b>>1):a-(b>>1))/b; }
static int xdir_d(int i){ return i==160?1:rdiv((i-160)*512,focal_length); }
static int ydir_d(int y){ return y==120?1:rdiv((120-y)*512,focal_length); }
// sin/cos of i*(2*pi/64) in 9-bit fixed, from a quarter table (identical to the old float tables)
static const uint16_t sqt[17]={0,50,100,149,196,241,284,325,362,396,426,452,473,490,502,510,512};
static int sin64(int i){ i&=63; int k=i&15; switch(i>>4){ case 0: return sqt[k]; case 1: return sqt[16-k]; case 2: return -sqt[k]; } return -sqt[16-k]; }
static inline flint SINR(int i){ return fraw(sin64(i)); }
static inline flint COSR(int i){ return fraw(sin64(i+16)); }

flint plx=16, ply=WY-1, plz=16;
flint pl_dirx=0, pl_dirz=1;
flint grav=0, air_peak=WY-1;
const flint grav_amount=fraw(153), grav_max=fraw(460), jump_vel=-1;
const flint climb_speed=fraw(256), swim_up=fraw(204), sink_speed=fraw(102), fly_speed=fraw(256);
int rotxz=0, rotyz=0;
#undef randInt
static int randInt(int lo, int hi){ return (int)((long)random() % (long)(hi-lo+1)) + lo; }//DO NOT CHANGE: world gen depends on it

inline void rot_point_2d(flint& x1,flint&y1,flint x2, flint y2,int angle)
{
	flint nx=x1-x2, ny=y1-y2, c=COSR(angle), s=SINR(angle);
	x1=x2+(c*nx-s*ny);
	y1=y2+(s*nx+c*ny);
}
inline void rot_point_3d(flint&x1,flint&y1,flint&z1,flint cxz, flint sxz, flint cyz,flint syz)
{
	flint ny=y1, nz=z1;
	y1=(cyz*ny-syz*nz);
	nz=(syz*ny+cyz*nz);
	flint nx=x1;
	x1=(cxz*nx-sxz*nz);
	z1=(sxz*nx+cxz*nz);
}
static void compute_dir()
{
	pl_dirx=0; pl_dirz=1;
	rot_point_2d(pl_dirx,pl_dirz,0,0,rotxz);
	if(pl_dirx>1) pl_dirx=1;
	if(pl_dirx<-1) pl_dirx=-1;
	if(pl_dirz>1) pl_dirz=1;
	if(pl_dirz<-1) pl_dirz=-1;
}
inline int fast_copysign_pos(int val, int src){ int m=src>>23; return (val^m)-m; }

uint8_t* cross_hair_pt(uint8_t*& blockbefore,bool countwaterblock=0,bool countallwater=0)
{
	int ix=plx, iy=ply, iz=plz;
	flint dx=fraw(1), dy=fraw(1), dz=1;
	rot_point_3d(dx,dy,dz,COSR(rotxz),SINR(rotxz),COSR(rotyz),SINR(rotyz));
	flint deltax=fraw(INV(dx.data)), deltay=fraw(INV(dy.data)), deltaz=fraw(INV(dz.data));
	int signdx=fast_copysign_pos(XJ,dx.data);
	int signdy=fast_copysign_pos(YJ,dy.data);
	int signdz=fast_copysign_pos(1,dz.data);
	flint tmaxx,tmaxy,tmaxz;
	tmaxx.data=deltax.data>>1; tmaxy.data=deltay.data>>1; tmaxz.data=deltaz.data>>1;
	uint8_t* curr=world+(ix<<XWS)+(iy<<YWS)+iz;
	blockbefore=curr;
	for(int i=0;i<INTERACTD;++i)
	{
		blockbefore=curr;
		if(tmaxx<tmaxy){
			if(tmaxx<tmaxz){ curr+=signdx; tmaxx+=deltax; }
			else { curr+=signdz; tmaxz+=deltaz; }
		} else {
			if(tmaxy<tmaxz){ curr+=signdy; tmaxy+=deltay; }
			else { curr+=signdz; tmaxz+=deltaz; }
		}
		if(curr<world||curr>=world+WORLD_BYTES) return NULL;
		uint8_t b=*curr;
		if(countallwater||((countwaterblock||b!=WATERBLOCK)&&(b!=WATERSLAB&&b!=WATERFLOOR&&b!=WATERFALL)))
			if(b!=0) return curr;
	}
	return NULL;
}
// one shared DDA per view/world state (outline, HUD, mining, placing all use this)
static uint8_t *ch_t,*ch_b;
static int ch_key[5];
static uint8_t* crosshair(uint8_t*& before)
{
	int a=(int)plx,b=(int)ply,c=(int)plz;
	if(!ch_valid||ch_key[0]!=a||ch_key[1]!=b||ch_key[2]!=c||ch_key[3]!=rotxz||ch_key[4]!=rotyz)
	{
		ch_t=cross_hair_pt(ch_b);
		ch_key[0]=a; ch_key[1]=b; ch_key[2]=c; ch_key[3]=rotxz; ch_key[4]=rotyz;
		ch_valid=true;
	}
	before=ch_b;
	return ch_t;
}

// ============================================================================
//  COLORS + TEXTURES
//  palette index = RRR BB GGG. 26 = transparent. Textures stored once at full
//  brightness; face shade + fog applied through 8 class rows (flp[hz+slot]).
// ============================================================================
static uint8_t colors[(reg_block_num + 1) << 2] = {
	0,130,106,65,6,125,0,0,0,0,205,140,130,215,64,0,25,25,139,229,30,160,0,
	0,5,172,238,103,125,0,0,0,0,239,247,255,255,128,74,27,27,222,231,159,224,0,
	0,131,139,97,7,125,0,0,0,0,238,181,131,247,96,1,25,27,213,230,31,192,0
};
static const uint8_t bg_colors[]={106,170,125,95,127,159,127,95,125,170, 106,73,41,8,0,0,0,8,41,73};
static bool haze_enabled=true;
static uint8_t pack332(uint8_t r,uint8_t b,uint8_t g){ return (uint8_t)((r<<5)|(b<<3)|g); }
static uint8_t g26(uint8_t c){ return c==26?27:c; }//26 is the transparent key: never emit it
static uint8_t shade1(uint8_t c)
{
	if(c==26) return 26;
	uint8_t r=c>>5, b=(c>>3)&3, g=c&7;
	uint8_t m=r>g?r:g;
	if(m==0){ if(b) b--; }
	else { uint8_t nm=m-1; b=(uint8_t)((b*nm)/m); if(r) r--; if(g) g--; }
	return g26((uint8_t)((r<<5)|(b<<3)|g));
}
// per-channel nearest level of c*f/16: the palette is a 3x3x2-bit product space, so for any
// per-channel weighted metric the nearest palette entry is exactly this rounding.
static uint8_t shade_f(uint8_t c,uint8_t f)
{
	uint8_t r=c>>5, b=(c>>3)&3, g=c&7;
	return pack332((uint8_t)((r*f+8)>>4),(uint8_t)((b*f+8)>>4),(uint8_t)((g*f+8)>>4));
}
static uint8_t lighten1(uint8_t c)
{
	uint8_t r=c>>5, b=(c>>3)&3, g=c&7;
	if(r<7) r++;
	if(b<3) b++;
	if(g<7) g++;
	return pack332(r,b,g);
}
static uint8_t mix50(uint8_t a,uint8_t t)
{
	return pack332((uint8_t)(((a>>5)+(t>>5)+1)>>1),(uint8_t)((((a>>3)&3)+((t>>3)&3)+1)>>1),(uint8_t)(((a&7)+(t&7)+1)>>1));
}
static uint8_t flc[8][256];          //rows 0-3: shade classes x,top,bottom,z ; rows 4-7: same + fog
static const uint8_t* flp[16];       //flp[hz+slot] -> class row (hz is 0 or 8)
static uint8_t sky_hi=125, sky_mid=125, fog_target=125, fog_realm=255;
static bool fog_built=false;
static void build_shade_lut()
{
	static const uint8_t cf[4]={10,16,8,13};
	static const uint8_t cls[8]={0,0,1,2,3,3,1,1};
	for(int k=0;k<4;k++) for(int c=0;c<256;c++) flc[k][c]=(k==1)?(uint8_t)c:g26(shade_f((uint8_t)c,cf[k]));
	for(int s=0;s<8;s++){ flp[s]=flc[cls[s]]; flp[8+s]=flc[4+cls[s]]; }
}
static void build_fog_lut()
{
	uint8_t fog[256];
	for(int c=0;c<256;c++) fog[c]=haze_enabled?g26(mix50((uint8_t)c,fog_target)):(uint8_t)c;
	for(int k=0;k<4;k++) for(int c=0;c<256;c++) flc[4+k][c]=fog[flc[k][c]];
}
void change_bg(uint8_t color)
{
	if(fog_built&&color==bg_color&&fog_realm==realm) return;
	bg_color=color;
	colors[BARRIER]=color; colors[BARRIER+RBP1]=color; colors[BARRIER+2*RBP1]=color;
	if(realm==1){ sky_hi=color; sky_mid=color; fog_target=color; }
	else if(((color>>5)+(color&7))>=6){ sky_mid=lighten1(color); sky_hi=lighten1(sky_mid); fog_target=sky_mid; }
	else { sky_mid=color; sky_hi=lighten1(color); fog_target=color; }
	fog_realm=realm; fog_built=true;
	build_fog_lut();
}
static void refresh_fog(){ fog_built=false; change_bg(bg_color); }

enum { TS_PX=0, TS_NX=1, TS_TOP=2, TS_BOT=3, TS_PZ=4, TS_NZ=5 };
#define TSTR 6
static const uint8_t* texturedata[block_num*TSTR];
#define LTEX texturedata
static uint8_t tex_on[block_num];   //1 = textured path
static uint8_t shade_on[block_num]; //1 = apply per-face shading
static uint8_t thin_id[block_num];  //1 = block has see-through texels
#define MIPMAX 80
static uint8_t mpool[MIPMAX][16];   //4x4 box-filtered mips
static uint8_t mavg[MIPMAX];        //average color
static uint8_t mipidx[block_num*TSTR];//0xFF = tile has transparent texels (no mip)
static void apply_tex_mode()
{
	for(int i=0;i<block_num;i++) tex_on[i]=(i>=TEXTURED_REG_START)?1:0;
	if(set_tex) for(int i=1;i<TEXTURED_REG_START;i++) if(i!=BARRIER) tex_on[i]=1;
}
enum {
	GT_PL, GT_CB, GT_CT_TOP, GT_CT_X, GT_CT_Z, GT_FU_FRONT, GT_GLASS,
	GT_ORE_FIRST, GT_TNT_SIDE=GT_ORE_FIRST+5, GT_TNT_TOP,
	GT_SB, GT_BK, GT_LAD, GT_PU_FACE, GT_PU_SIDE, GT_PU_TOP, GT_GLOW, GT_SS, GT_SAPLING, GT_APPLE, GT_GAPPLE,
	GT_DIRT, GT_GRASS_TOP, GT_GRASS_SIDE, GT_SNOW, GT_SNOW_SIDE, GT_STONE, GT_LOG, GT_LOG_TOP, GT_LEAF,
	GT_SAND, GT_WOOL, GT_NRACK, GT_OBS, GT_LAMP, GT_LAMP_LIT, GT_WATER, GT_TORCH, GT_TORCH_TOP, GT_HURT,
	GT_COUNT
};
static uint8_t gtex[GT_COUNT][64];
#define GT(n) ((const uint8_t*)gtex[(n)])
#define GT_ORE(o) GT(GT_ORE_FIRST+(o))

static void tex_build(uint8_t* out,const char* pat,const uint8_t* pal)
{
	for(int r=0;r<8;r++) for(int c=0;c<8;c++)
	{
		char ch=pat[r*8+c];
		out[((7-r)<<3)+c]=(ch=='.')?26:pal[ch-'0'];
	}
}
static uint16_t tseed=1;
static uint8_t trnd()
{
	uint16_t s=tseed;
	s^=(uint16_t)(s<<7); s^=(uint16_t)(s>>9); s^=(uint16_t)(s<<8);
	tseed=s;
	return (uint8_t)(s^(s>>8));
}
static void tex_noise(uint8_t* out,const uint8_t* pal,const uint8_t* cum,uint8_t n,uint16_t seed)
{
	tseed=(uint16_t)(seed|1);
	for(int i=0;i<64;i++)
	{
		uint8_t r=trnd(), k=0;
		while(k+1<n&&r>=cum[k]) k++;
		out[i]=pal[k];
	}
}
static void add_fringe(uint8_t* t,const uint8_t* pal,uint8_t n,uint8_t base)
{
	for(int c=0;c<8;c++)
	{
		int d=base+(trnd()<150?1:0)+(trnd()<50?1:0);
		for(int r=0;r<d;r++) t[((7-r)<<3)+c]=pal[trnd()%n];
	}
}
static const char P_PLANKS[]="00031000""20000020""11111111""03000100""00020000""11111111""00100030""11111111";
static const char P_COBBLE[]="02230112""00231003""33303333""12203220""10033201""33330033""02130122""00330013";
static const char P_CT_TOP[]="44444444""40230104""42000034""44444444""40302004""40020304""40000204""44444444";
static const char P_CT_X[]  ="44444444""45550604""40700664""40700604""40700704""41000014""11111111""02001020";
static const char P_CT_Z[]  ="44444444""40066604""40666604""40660004""40700004""41000014""11111111""00201002";
static const char P_FURN[]  ="02230112""33333333""21555512""33333333""02444420""14444441""14444441""33333333";
static const char P_GLASS[] ="00000000""0......0""0.1....0""0..1...0""0......0""0....1.0""0......0""00000000";
static const char P_ORE[]   ="00010000""02300010""02210000""00000230""10002210""00000020""03200100""02200000";
static const char P_TNTS[]  ="01001001""01001001""22222222""33233323""23223232""22222222""01001001""01001001";
static const char P_TNTT[]  ="00100100""01111110""11444411""01433410""01433410""11444411""01111110""00100100";
static const char P_SBRICK[]="10000003""00000203""33333333""00031000""02030000""33333333""10000003""00002003";
static const char P_BOOKX[] ="11111111""23452634""23452634""23452634""11111111""54326423""54326423""11111111";
static const char P_LADDER[]="0......0""01111110""0......0""0......0""01111110""0......0""0......0""01111110";
static const char P_PUFACE[]="10010010""14400441""14400441""10010010""04444440""14044041""10444401""10010010";
static const char P_PUSIDE[]="11011011""10010010""10010010""10010010""10010010""10010010""10010010""11011011";
static const char P_PUTOP[] ="10000001""01000010""00166100""00066000""00066000""00100100""01000010""10000001";
static const char P_GLOW[]  ="03120302""20031200""13002031""02310020""30021301""01300023""23021030""10230102";
static const char P_SOUL[]  ="00200100""01330210""00330000""20000102""00102000""02003310""10003300""00100020";
static const char P_SAPL[]  ="...00..."".00110.."".01101..""..0110.."".0.11.0.""...11...""...22...""..2222..";
static const char P_APPLE[] ="....122.""...1....""..000000""03000000""03000004""00000004"".000044.""..0440..";
static const char P_BARK[]  ="01020102""01020112""01120102""01020102""21020100""01020102""01021102""01020102";
static const char P_LOGTOP[]="00000000""01111110""01222210""01211210""01211210""01222210""01111110""00000000";
static const char P_LAMP[]  ="01111110""12322321""13233231""12322321""12322321""13233231""12322321""01111110";
static const char P_TORCH[] ="........""...00...""...01...""...22...""...22...""...22...""...22...""...22...";
static const char P_TORCHT[]="........""........""........""...00...""...01...""........""........""........";

static void set_row(uint8_t id,const uint8_t* px,const uint8_t* nx,const uint8_t* top,const uint8_t* bot,const uint8_t* pz,const uint8_t* nz,uint8_t sh)
{
	const uint8_t** r=texturedata+(int)id*TSTR;
	r[TS_PX]=px; r[TS_NX]=nx; r[TS_TOP]=top; r[TS_BOT]=bot; r[TS_PZ]=pz; r[TS_NZ]=nz;
	shade_on[id]=sh;
}
static void set4(uint8_t id,const uint8_t* x,const uint8_t* y,const uint8_t* z,const uint8_t* b,uint8_t sh){ set_row(id,x,x,y,b,z,z,sh); }
static void set1(uint8_t id,const uint8_t* t,uint8_t sh){ set_row(id,t,t,t,t,t,t,sh); }
// base id = front on -z (faces a player looking +z); var1..var1+2 = player looking -x, -z, +x
static void set_oriented(uint8_t base,uint8_t var1,const uint8_t* front,const uint8_t* side,const uint8_t* top,const uint8_t* bot)
{
	static const uint8_t fslot[4]={TS_NZ,TS_PX,TS_PZ,TS_NX};
	for(uint8_t l=0;l<4;l++)
	{
		uint8_t id=l?(uint8_t)(var1+l-1):base;
		set_row(id,side,side,top,bot,side,side,1);
		texturedata[(int)id*TSTR+fslot[l]]=front;
	}
}
static void build_lod()
{
	const uint8_t* src[MIPMAX];
	int n=0;
	for(int i=0;i<block_num*TSTR;i++)
	{
		const uint8_t* t=texturedata[i];
		mipidx[i]=0xFF;
		int k=0;
		for(;k<n;k++) if(src[k]==t) break;
		if(k<n){ mipidx[i]=(uint8_t)k; continue; }
		bool ok=n<MIPMAX;
		for(int j=0;j<64&&ok;j++) if(t[j]==26) ok=false;
		if(!ok) continue;
		int tr=0,tb=0,tg=0;
		for(int r=0;r<4;r++) for(int c=0;c<4;c++)
		{
			int o=(r<<4)+(c<<1), sr=0,sb=0,sg=0;
			for(int q=0;q<4;q++)
			{
				uint8_t p=t[o+(q&1)+((q>>1)<<3)];
				sr+=p>>5; sb+=(p>>3)&3; sg+=p&7;
			}
			mpool[n][(r<<2)+c]=pack332((uint8_t)((sr+2)>>2),(uint8_t)((sb+2)>>2),(uint8_t)((sg+2)>>2));
			tr+=sr; tb+=sb; tg+=sg;
		}
		mavg[n]=pack332((uint8_t)((tr+32)>>6),(uint8_t)((tb+32)>>6),(uint8_t)((tg+32)>>6));
		src[n]=t; mipidx[i]=(uint8_t)n; n++;
	}
	for(int id=0;id<MOBSTART;id++)
	{
		thin_id[id]=0;
		if(id<=AIR||id==BARRIER) continue;
		for(int s=0;s<6;s++) if(mipidx[id*TSTR+s]==0xFF){ thin_id[id]=1; break; }
	}
}
static void init_texturedata()
{
	for(int i=0;i<block_num*TSTR;i++) texturedata[i]=gapblack;
	memset(shade_on,0,sizeof(shade_on));
	set4(GRASS,GT(GT_GRASS_SIDE),GT(GT_GRASS_TOP),GT(GT_GRASS_SIDE),GT(GT_DIRT),1);
	set1(STONE,GT(GT_STONE),1);
	set4(WOOD,GT(GT_LOG),GT(GT_LOG_TOP),GT(GT_LOG),GT(GT_LOG_TOP),1);
	set1(LEAF,GT(GT_LEAF),1);
	set1(BARRIER,clear,0);
	set1(UNLITLAMP,GT(GT_LAMP),1);
	set1(LITLAMP,GT(GT_LAMP_LIT),0);
	set1(UNLITBUTTON,GT(GT_STONE),1);
	set1(LITBUTTON,GT(GT_STONE),1);
	set1(SAND,GT(GT_SAND),1);
	set1(WOOL,GT(GT_WOOL),1);
	set4(SNOWGRASS,GT(GT_SNOW_SIDE),GT(GT_SNOW),GT(GT_SNOW_SIDE),GT(GT_DIRT),1);
	set1(SNOW,GT(GT_SNOW),1);
	set1(NETHERRACK,GT(GT_NRACK),1);
	set1(OBSIDIAN,GT(GT_OBS),1);
	set1(WATERBLOCK,GT(GT_WATER),1);
	set1(WATERFALL,GT(GT_WATER),1);
	set4(IRONBLOCK,ironblockx,ironblocky,ironblockz,ironblocky,0);
	set4(GOLDBLOCK,goldblockx,goldblocky,goldblockz,goldblocky,0);
	set4(DIAMONDBLOCK,diamondblockx,diamondblocky,diamondblockz,diamondblocky,0);
	set4(REDSTONEBLOCK,redstoneblockx,redstoneblocky,redstoneblockz,redstoneblocky,0);
	set1(FLOORVOXELS,gapblack,0);
	for(int i=FLOORVOXELS+1;i<WATERSLAB;i++) set1((uint8_t)i,clear,0);
	set4(WATERSLAB,waterslabside,clear,waterslabside,clear,0);
	set4(CRAFTTABLE,GT(GT_CT_X),GT(GT_CT_TOP),GT(GT_CT_Z),GT(GT_PL),1);
	set_oriented(FURNACE,FURNACE_1,GT(GT_FU_FRONT),GT(GT_CB),GT(GT_CB),GT(GT_CB));
	set1(PLANKS,GT(GT_PL),1);
	set4(BEDTOP,bedsidetop,bedtop,bedsidefront,bedtop,0);
	set1(GLASS,GT(GT_GLASS),1);
	set4(DOORTOPOPEN,doorarch,brown,doorfronttop,brown,1);
	set4(DOORBOTTOMOPEN,doorbottomarch,clear,brown,brown,1);
	set4(DOORTOPCLOSED,doorfronttop,brown,doorarch,brown,1);
	set4(DOORBOTTOMCLOSED,brown,clear,doorbottomarch,brown,1);
	set4(MAGMABLOCK,magmablockx,magmablocky,magmablockx,magmablocky,0);
	set4(FIRE,fire,firetop,fire,firetop,0);
	set4(SANDSTONE,sandstonesidedarker,sandstonetop,sandstoneside,sandstonetop,0);
	set4(CACTUS,cactusside,cactustop,cactusside,cactustop,1);
	for(int o=0;o<5;o++) set1((uint8_t)(IRON+o),GT_ORE(o),1);
	set4(TNT,GT(GT_TNT_SIDE),GT(GT_TNT_TOP),GT(GT_TNT_SIDE),GT(GT_TNT_TOP),1);
	set4(LITTNT,GT(GT_TNT_SIDE),GT(GT_TNT_TOP),GT(GT_TNT_SIDE),GT(GT_TNT_TOP),1);
	set1(SMOKE,explosionxyz,0);
	set1(GRAVEL,graveltxt,1);
	set1(NETHERPORTAL,netherportaltxt,0);
	set4(NETHERGOLDORE,nethergoldorex,nethergoldorey,nethergoldorez,nethergoldorey,0);
	set1(STONEBRICK,GT(GT_SB),1);
	set4(BOOKSHELF,GT(GT_BK),GT(GT_PL),GT(GT_BK),GT(GT_PL),1);
	set4(LADDER,GT(GT_LAD),clear,GT(GT_LAD),clear,1);
	set_oriented(PUMPKIN,PUMPKIN_1,GT(GT_PU_FACE),GT(GT_PU_SIDE),GT(GT_PU_TOP),GT(GT_PU_TOP));
	set1(GLOWSTONE,GT(GT_GLOW),0);
	set1(SOULSAND,GT(GT_SS),1);
	set4(SAPLING,GT(GT_SAPLING),clear,GT(GT_SAPLING),clear,0);
	set4(TORCH,GT(GT_TORCH),GT(GT_TORCH_TOP),GT(GT_TORCH),clear,0);
	set4(SKELETOPZ,skeletope,grey,skeletopf,grey,0);
	set4(SKELETOPX,skeletopf,grey,skeletope,grey,0);
	set1(SKELEBOTTOM,skelebottom,0);
	set4(SHEEPFACEX,sheepface,grey,sheepside,grey,0);
	set4(SHEEPFACEZ,sheepside,grey,sheepface,grey,0);
	set4(SHEEPBACKX,sheepback,grey,sheepside,grey,0);
	set4(SHEEPBACKZ,sheepside,grey,sheepback,grey,0);
	set4(ZOMBIETOPX,zombieface,zombieheadtop,zombiehead,zombieheadtop,0);
	set4(ZOMBIETOPZ,zombiehead,zombieheadtop,zombieface,zombieheadtop,0);
	set4(ZOMBIELEGS,zombielegs,zombieheadtop,zombielegs,zombieheadtop,0);
	set4(PIGFACEX,pigface,pigtop,pigfrontside,pigtop,0);
	set4(PIGFACEZ,pigfrontside,pigtop,pigface,pigtop,0);
	set4(PIGBACKX,pigback,pigtop,pigbackside,pigtop,0);
	set4(PIGBACKZ,pigbackside,pigtop,pigback,pigtop,0);
	set4(PIGLINTOPX,piglinface,pigtop,piglintopside,pigtop,0);
	set4(PIGLINTOPZ,piglintopside,pigtop,piglinface,pigtop,0);
	set4(PIGLINLEGS,piglinlegs,pigtop,piglinlegs,pigtop,0);
	set4(CREEPERTOPX,creeperface,creeperbody,creeperbody,creeperbody,0);
	set4(CREEPERTOPZ,creeperbody,creeperbody,creeperface,creeperbody,0);
	set1(CREEPERLEGS,creeperlegs,0);
	set1(MOBHURT,GT(GT_HURT),0);
	build_lod();
}
static void build_textures()
{
	static const uint8_t pal_planks[]={204,130,171,237};
	static const uint8_t pal_cobble[]={148,107,181,74,0,33};
	static const uint8_t pal_ct[]={204,171,130,237,97,148,74,65};
	static const uint8_t pal_glass[]={223,255};
	static const uint8_t pal_tnt[]={224,192,255,0,148};
	static const uint8_t pal_sb[]={148,181,107,74};
	static const uint8_t pal_book[]={204,130,192,58,45,229,97};
	static const uint8_t pal_ladder[]={163,195};
	static const uint8_t pal_pumpkin[]={227,195,226,0,231,130,44};
	static const uint8_t pal_glow[]={230,255,196,239};
	static const uint8_t pal_soul[]={98,65,139,33};
	static const uint8_t pal_sapl[]={44,5,130};
	static const uint8_t pal_apple[]={224,130,44,255,192};
	static const uint8_t pal_gapple[]={231,130,44,255,229};
	static const uint8_t ore_cols[5][2]={{237,246},{231,229},{30,159},{224,192},{0,41}};
	static const uint8_t pal_dirt[]={139,171,106,130},   cum_dirt[]={100,170,225};
	static const uint8_t pal_grass[]={77,110,68,101},    cum_grass[]={110,175,225};
	static const uint8_t pal_snow[]={255,223,222},       cum_snow[]={170,235};
	static const uint8_t pal_stone[]={148,181,107,116},  cum_stone[]={115,166,217};
	static const uint8_t pal_leaf[]={68,36,101,2},       cum_leaf[]={95,170,220};
	static const uint8_t pal_sand[]={214,246,206,173},   cum_sand[]={110,180,230};
	static const uint8_t pal_wool[]={255,223,222},       cum_wool[]={150,220};
	static const uint8_t pal_nrack[]={128,96,161,137},   cum_nrack[]={100,165,215};
	static const uint8_t pal_obs[]={41,0,8,72},          cum_obs[]={110,185,230};
	static const uint8_t pal_water[]={25,27,59},         cum_water[]={140,210};
	static const uint8_t pal_hurt[]={224,192,225,160},   cum_hurt[]={100,170,220};
	static const uint8_t pal_bark[]={98,65,131};
	static const uint8_t pal_logtop[]={98,204,171};
	static const uint8_t pal_lamp[]={65,98,130,163};
	static const uint8_t pal_lamplit[]={98,163,230,255};
	static const uint8_t pal_torch[]={230,255,130};
	tex_build(gtex[GT_PL],P_PLANKS,pal_planks);
	tex_build(gtex[GT_CB],P_COBBLE,pal_cobble);
	tex_build(gtex[GT_CT_TOP],P_CT_TOP,pal_ct);
	tex_build(gtex[GT_CT_X],P_CT_X,pal_ct);
	tex_build(gtex[GT_CT_Z],P_CT_Z,pal_ct);
	tex_build(gtex[GT_FU_FRONT],P_FURN,pal_cobble);
	tex_build(gtex[GT_GLASS],P_GLASS,pal_glass);
	for(int o=0;o<5;o++)
	{
		uint8_t pt[4]={148,116,ore_cols[o][0],ore_cols[o][1]};
		tex_build(gtex[GT_ORE_FIRST+o],P_ORE,pt);
	}
	tex_build(gtex[GT_TNT_SIDE],P_TNTS,pal_tnt);
	tex_build(gtex[GT_TNT_TOP],P_TNTT,pal_tnt);
	tex_build(gtex[GT_SB],P_SBRICK,pal_sb);
	tex_build(gtex[GT_BK],P_BOOKX,pal_book);
	tex_build(gtex[GT_LAD],P_LADDER,pal_ladder);
	tex_build(gtex[GT_PU_FACE],P_PUFACE,pal_pumpkin);
	tex_build(gtex[GT_PU_SIDE],P_PUSIDE,pal_pumpkin);
	tex_build(gtex[GT_PU_TOP],P_PUTOP,pal_pumpkin);
	tex_build(gtex[GT_GLOW],P_GLOW,pal_glow);
	tex_build(gtex[GT_SS],P_SOUL,pal_soul);
	tex_build(gtex[GT_SAPLING],P_SAPL,pal_sapl);
	tex_build(gtex[GT_APPLE],P_APPLE,pal_apple);
	tex_build(gtex[GT_GAPPLE],P_APPLE,pal_gapple);
	tex_noise(gtex[GT_DIRT],pal_dirt,cum_dirt,4,0x1D3);
	tex_noise(gtex[GT_GRASS_TOP],pal_grass,cum_grass,4,0x2B7);
	memcpy(gtex[GT_GRASS_SIDE],gtex[GT_DIRT],64); tseed=0x3C5; add_fringe(gtex[GT_GRASS_SIDE],pal_grass,4,1);
	tex_noise(gtex[GT_SNOW],pal_snow,cum_snow,3,0x4E1);
	memcpy(gtex[GT_SNOW_SIDE],gtex[GT_DIRT],64); tseed=0x5A9; add_fringe(gtex[GT_SNOW_SIDE],pal_snow,3,2);
	tex_noise(gtex[GT_STONE],pal_stone,cum_stone,4,0x6B3);
	for(int i=0;i<6;i++){ uint8_t p=trnd()&63; gtex[GT_STONE][p]=107; if((p&7)<7) gtex[GT_STONE][p+1]=107; }
	tex_build(gtex[GT_LOG],P_BARK,pal_bark);
	tex_build(gtex[GT_LOG_TOP],P_LOGTOP,pal_logtop);
	tex_noise(gtex[GT_LEAF],pal_leaf,cum_leaf,4,0x7C1);
	tex_noise(gtex[GT_SAND],pal_sand,cum_sand,4,0x8D5);
	tex_noise(gtex[GT_WOOL],pal_wool,cum_wool,3,0x9E3);
	tex_noise(gtex[GT_NRACK],pal_nrack,cum_nrack,4,0xA17);
	tex_noise(gtex[GT_OBS],pal_obs,cum_obs,4,0xB29);
	tex_build(gtex[GT_LAMP],P_LAMP,pal_lamp);
	tex_build(gtex[GT_LAMP_LIT],P_LAMP,pal_lamplit);
	tex_noise(gtex[GT_WATER],pal_water,cum_water,3,0xC3B);
	tex_build(gtex[GT_TORCH],P_TORCH,pal_torch);
	tex_build(gtex[GT_TORCH_TOP],P_TORCHT,pal_torch);
	tex_noise(gtex[GT_HURT],pal_hurt,cum_hurt,4,0xD4D);
	init_texturedata();
}

// ============================================================================
//  NAMES
// ============================================================================
static const char* const block_names[invenblocknum+1]={
	"Air","Grass Block","Stone","Oak Log","Leaves","Barrier","Lamp","Lit Lamp","Button","Lit Button",
	"Sand","Wool","Snowy Grass","Snow Block","Netherrack","Obsidian","Water","Water",
	"Block of Iron","Block of Gold","Block of Diamond","Block of Redstone","?",
	"Snow Layer","Redstone Dust","Redstone Dust","Water","Water",
	"Crafting Table","Furnace","Oak Planks","Bed","Glass","Door","Door","Door","Door",
	"Magma Block","Fire","Sandstone","Cactus",
	"Iron Ore","Gold Ore","Diamond Ore","Redstone Ore","Coal Ore","TNT","TNT","Smoke","Gravel",
	"Nether Portal","Nether Gold Ore",
	"Stone Bricks","Bookshelf","Ladder","Jack o'Lantern","Glowstone","Soul Sand","Oak Sapling",
	"Torch","Furnace","Furnace","Furnace","Jack o'Lantern","Jack o'Lantern","Jack o'Lantern"
};
static const char* const item_names[NUM_ITEMS]={
	"Wooden Pickaxe","Stone Pickaxe","Golden Pickaxe","Iron Pickaxe","Diamond Pickaxe","Netherite Pick",
	"Stick","Wooden Sword","Stone Sword","Golden Sword","Iron Sword","Diamond Sword",
	"Iron Ingot","Gold Ingot","Raw Mutton","Cooked Mutton","Rotten Flesh","Diamond","Coal",
	"Flint and Steel","Flint","Bucket","Water Bucket","Raw Porkchop","Cooked Porkchop","Gunpowder",
	"Apple","Golden Apple"
};
static const char* item_name(int id)
{
	if(id<=0) return "";
	if(id<=invenblocknum) return block_names[id];
	int i=id-ITEMBASE;
	if(i>=0&&i<NUM_ITEMS) return item_names[i];
	return "?";
}

// ============================================================================
//  SPRITE TABLES + ICON DRAWING
// ============================================================================
static gfx_sprite_t* item_imgs[item_num];
static gfx_sprite_t* flame_imgs[6];
static void refresh_sprite_tables()
{
	gfx_sprite_t* t[26]={woodpic,stonepic,goldpic,ironpic,diamondpic,NULL,stick,woodsword,stonesword,goldsword,ironsword,diamondsword,ironingot,goldingot,rawmutton,cookedmutton,rottenflesh,diamond,coal,flintandsteeeel,flintitem,bucket,waterbucket,rawporkchop,cookedporkchop,gunpowder};
	for(int i=0;i<item_num;i++) item_imgs[i]=(i<26)?t[i]:NULL;
	flame_imgs[0]=emptyfurnaceflames; flame_imgs[1]=furnaceflames1; flame_imgs[2]=furnaceflames2;
	flame_imgs[3]=furnaceflames3; flame_imgs[4]=furnaceflames4; flame_imgs[5]=furnaceflames5;
}
static uint8_t iso_map[32*32];
static void build_iso_map()//integer version of the isometric cube projection (no float)
{
	for(int py=0;py<32;py++) for(int px=0;px<32;px++)
	{
		uint8_t v=255;
		int dx2=2*px-31, dy2=2*py-1;
		int a=dx2+2*dy2, b=2*dy2-dx2;
		if(a>=0&&a<56&&b>=0&&b<56) v=(uint8_t)((1<<6)|((b/7)<<3)|(a/7));
		else if(px>=2&&px<16)
		{
			int s=2*px-3, t=4*py-2*px-27;
			if(t>=0&&t<60) v=(uint8_t)(((7-(t*2)/15)<<3)|((s*2)/7));
		}
		else if(px>=16&&px<30)
		{
			int s=2*px-31, t=4*py+2*px-89;
			if(t>=0&&t<60) v=(uint8_t)((2<<6)|((7-(t*2)/15)<<3)|((s*2)/7));
		}
		iso_map[py*32+px]=v;
	}
}
static void draw_tex_scaled(const uint8_t* t,int x,int y,int sx,int sy)
{
	uint8_t* base=(uint8_t*)gfx_vbuffer;
	for(int r=0;r<8;r++)
	{
		const uint8_t* row=t+((7-r)<<3);
		for(int c=0;c<8;c++)
		{
			uint8_t col=row[c];
			if(col==26) continue;
			uint8_t* p=base+(y+r*sy)*320+x+c*sx;
			for(int yy=0;yy<sy;yy++){ for(int xx=0;xx<sx;xx++) p[xx]=col; p+=320; }
		}
	}
}
static void draw_iso(int id,int x,int y)
{
	if(id<=0||id==BARRIER||id>=block_num) return;
	uint8_t* dst=(uint8_t*)gfx_vbuffer+y*320+x;
	const uint8_t** r=LTEX+id*TSTR;
	const uint8_t* f[3]={r[TS_NX],r[TS_TOP],r[TS_NZ]};
	const bool sh=shade_on[id]!=0;
	const uint8_t* s[3]={flp[sh?TS_NX:TS_TOP],flp[TS_TOP],flp[sh?TS_NZ:TS_TOP]};
	for(int py=0;py<32;py++)
	{
		const uint8_t* m=&iso_map[py*32];
		uint8_t* d=dst+py*320;
		for(int px=0;px<32;px++)
		{
			uint8_t v=m[px];
			if(v==255) continue;
			uint8_t face=v>>6;
			uint8_t c=f[face][v&63];
			if(c==26) continue;
			d[px]=s[face][c];
		}
	}
}
static void draw_count(int x,int y,int n)
{
	int digits=(n>=1000)?4:((n>=100)?3:((n>=10)?2:1));
	int tx=x+32-digits*8, ty=y+22;
	gfx_SetTextBGColor(26);
	gfx_SetTextFGColor(0);
	gfx_SetTextXY(tx+1,ty+1); gfx_PrintInt(n,0);
	gfx_SetTextFGColor(255);
	gfx_SetTextXY(tx,ty); gfx_PrintInt(n,0);
}
static void draw_item(int id,int x,int y,int show_num)
{
	if(id<=0) return;
	if(id==REDSTONEDUSTUNLIT||id==REDSTONEDUSTLIT) gfx_Sprite_NoClip(redstonedust,x,y);
	else if(id<=reg_block_num) draw_iso(id,x,y);
	else if(id<=FLOORVOXELSEND)
	{
		gfx_SetColor(floorcolors[id]);
		gfx_FillTriangle(x+7,y+7,x+2,y+28,x+28,y+7);
		gfx_FillTriangle(x+2,y+28,x+28,y+28,x+28,y+7);
	}
	else if(id<=invenblocknum)
	{
		if(id>=DOORTOPOPEN&&id<=DOORBOTTOMCLOSED){ draw_tex_scaled(doorfronttop,x,y,4,2); draw_tex_scaled(brown,x,y+16,4,2); }
		else if(id==LADDER) draw_tex_scaled(GT(GT_LAD),x,y,4,4);
		else if(id==FIRE) draw_tex_scaled(fire,x,y,4,4);
		else if(id==SAPLING) draw_tex_scaled(GT(GT_SAPLING),x,y,4,4);
		else if(id==TORCH) draw_tex_scaled(GT(GT_TORCH),x,y,4,4);
		else draw_iso(id,x,y);
	}
	else
	{
		int i=id-ITEMBASE;
		if(id==APPLE) draw_tex_scaled(GT(GT_APPLE),x,y,4,4);
		else if(id==GOLDENAPPLE) draw_tex_scaled(GT(GT_GAPPLE),x,y,4,4);
		else if(i>=0&&i<item_num&&item_imgs[i]) gfx_Sprite_NoClip(item_imgs[i],x,y);
	}
	if(show_num&&inventory[id]>0) draw_count(x,y,inventory[id]);
}

// ============================================================================
//  KEY HELPERS + MENU WIDGETS
// ============================================================================
static uint8_t kb_now[8], kb_prev[8];
static void key_update(){ memcpy(kb_prev,kb_now,8); kb_Scan(); for(uint8_t g=1;g<8;g++) kb_now[g]=(uint8_t)kb_Data[g]; }
static void key_reset(){ kb_Scan(); for(uint8_t g=1;g<8;g++){ kb_now[g]=(uint8_t)kb_Data[g]; kb_prev[g]=kb_now[g]; } }
static bool key_pressed(kb_lkey_t k){ uint8_t g=(uint8_t)(k>>8), m=(uint8_t)(k&0xFF); return (kb_now[g]&m)&&!(kb_prev[g]&m); }
static bool any_key_pressed(){ for(uint8_t g=1;g<8;g++) if(kb_now[g]&~kb_prev[g]) return true; return false; }
static void wait_key_release()
{
	do{ kb_Scan(); } while(kb_Data[1]||kb_Data[2]||kb_Data[3]||kb_Data[4]||kb_Data[5]||kb_Data[6]||kb_Data[7]);
}
static void wait_any_key(){ wait_key_release(); key_reset(); while(true){ key_update(); if(any_key_pressed()) break; } wait_key_release(); }
static bool keyup(kb_lkey_t key)
{
	if(kb_IsDown(key)){ while(kb_IsDown(key)) kb_Scan(); return true; }
	return false;
}
#define COL_BLACK  0
#define COL_WHITE  255
#define COL_YELLOW 231
#define COL_GRAY_D 106
#define COL_GRAY_M 139
#define COL_GRAY_L 172
#define COL_PANEL  214
#define COL_DKGREEN 4
#define VRAM_BUF_A ((uint8_t*)0xD40000)
#define VRAM_BUF_B ((uint8_t*)0xD52C00)
#define VRAM_END   ((uint8_t*)0xD65800)

static void menu_text_setup(){ gfx_SetTextTransparentColor(26); gfx_SetTextBGColor(26); }
static void shadow_text(const char* s,int x,int y,uint8_t fg)
{
	gfx_SetTextFGColor(COL_BLACK); gfx_SetTextXY(x+1,y+1); gfx_PrintString(s);
	gfx_SetTextFGColor(fg); gfx_SetTextXY(x,y); gfx_PrintString(s);
}
static void shadow_text_centered(const char* s,int y,uint8_t fg){ shadow_text(s,(320-(int)gfx_GetStringWidth(s))/2,y,fg); }
static void dim_rows(int y0,int y1,uint8_t color)
{
	uint8_t* p=(uint8_t*)gfx_vbuffer+y0*320;
	for(int y=y0;y<y1;++y){ for(int x=(y&1);x<320;x+=2) p[x]=color; p+=320; }
}
static void copy_displayed_to_draw()
{
	uint8_t* d=(uint8_t*)gfx_vbuffer;
	uint8_t* s=(d==VRAM_BUF_A)?VRAM_BUF_B:VRAM_BUF_A;
	memcpy(d,s,320*240);
}
static void draw_dirt_bg()
{
	for(int ty=0;ty<30;++ty) for(int tx=0;tx<40;++tx)
		gfx_Sprite_NoClip((const gfx_sprite_t*)menudirt,tx*8,ty*8);
	dim_rows(0,240,COL_BLACK);
}
static void draw_inset(int x,int y,int w,int h)
{
	gfx_SetColor(COL_WHITE); gfx_FillRectangle_NoClip(x,y,w,h);
	gfx_SetColor(COL_GRAY_D); gfx_FillRectangle_NoClip(x,y,w-1,h-1);
	gfx_SetColor(COL_GRAY_M); gfx_FillRectangle_NoClip(x+1,y+1,w-2,h-2);
}
static void draw_slot(int x,int y,int s){ draw_inset(x,y,s,s); }
static void draw_panel(int x,int y,int w,int h)
{
	gfx_SetColor(COL_BLACK); gfx_FillRectangle_NoClip(x,y,w,h);
	gfx_SetColor(COL_PANEL); gfx_FillRectangle_NoClip(x+1,y+1,w-2,h-2);
	gfx_SetColor(COL_WHITE); gfx_FillRectangle_NoClip(x+1,y+1,w-3,1); gfx_FillRectangle_NoClip(x+1,y+1,1,h-3);
	gfx_SetColor(COL_GRAY_M); gfx_FillRectangle_NoClip(x+2,y+h-2,w-3,1); gfx_FillRectangle_NoClip(x+w-2,y+2,1,h-3);
}
static void draw_tip(int x,int y,const char* s)//Minecraft-style tooltip box
{
	int w=(int)gfx_GetStringWidth(s)+8;
	gfx_SetColor(81); gfx_Rectangle_NoClip(x,y,w,13);
	gfx_SetColor(COL_BLACK); gfx_FillRectangle_NoClip(x+1,y+1,w-2,11);
	gfx_SetTextFGColor(COL_WHITE); gfx_SetTextXY(x+4,y+3); gfx_PrintString(s);
}
static void draw_mc_button(int x,int y,int w,int h,bool selected,const char* label)
{
	gfx_SetColor(selected?COL_WHITE:COL_BLACK); gfx_FillRectangle_NoClip(x,y,w,h);
	gfx_SetColor(selected?COL_GRAY_L:COL_GRAY_M); gfx_FillRectangle_NoClip(x+1,y+1,w-2,h-2);
	gfx_SetColor(selected?COL_PANEL:COL_GRAY_L);
	gfx_FillRectangle_NoClip(x+1,y+1,w-2,1); gfx_FillRectangle_NoClip(x+1,y+1,1,h-2);
	gfx_SetColor(COL_GRAY_D);
	gfx_FillRectangle_NoClip(x+1,y+h-2,w-2,1); gfx_FillRectangle_NoClip(x+w-2,y+1,1,h-2);
	menu_text_setup();
	int lw=gfx_GetStringWidth(label);
	shadow_text(label,x+(w-lw)/2,y+(h-8)/2,selected?COL_YELLOW:COL_WHITE);
}
static int draw_logo(const char* s,int y,int scale)
{
	gfx_SetTextScale(scale,scale);
	int x=(320-(int)gfx_GetStringWidth(s))/2;
	int right=x+(int)gfx_GetStringWidth(s);
	int d=scale;
	gfx_SetTextFGColor(COL_BLACK);  gfx_SetTextXY(x+d+1,y+d+1); gfx_PrintString(s);
	gfx_SetTextFGColor(COL_GRAY_D); gfx_SetTextXY(x+d,y+d); gfx_PrintString(s);
	gfx_SetTextFGColor(COL_GRAY_M); gfx_SetTextXY(x+(d>>1),y+(d>>1)); gfx_PrintString(s);
	gfx_SetTextFGColor(COL_PANEL);  gfx_SetTextXY(x-1,y-1); gfx_PrintString(s);
	gfx_SetTextFGColor(COL_GRAY_L); gfx_SetTextXY(x,y); gfx_PrintString(s);
	gfx_SetTextScale(1,1);
	return right;
}
static void draw_arrow_nfull(int x,int y,int w,int h,int howfull)
{
	gfx_SetColor(74);
	int w2=w>>1, h2=h>>1;
	gfx_FillRectangle_NoClip(x,y+(h2>>1),w2,h2);
	gfx_SetColor(255);
	for(int x2=0;x2<w2;x2++)
	{
		if(howfull<x2+w2) gfx_SetColor(74);
		gfx_VertLine(x+x2+w2,y+(x2>>1),h-x2);
	}
	gfx_SetColor(255);
	if(howfull>w2) howfull=w2;
	if(howfull>0) gfx_FillRectangle_NoClip(x,y+(h2>>1),howfull,h2);
}
static void loading_screen(const char* msg)
{
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	shadow_text_centered(msg,112,COL_WHITE);
	gfx_SwapDraw();
}
static void message_screen(const char* l1,const char* l2)
{
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	shadow_text_centered(l1,96,COL_WHITE);
	shadow_text_centered(l2,112,COL_WHITE);
	shadow_text_centered("Press any key",150,COL_YELLOW);
	gfx_SwapDraw();
	wait_any_key();
}
// scrollable menu. returns index or -1 on Clear. descs (optional): one-line help for the highlighted entry
static int mc_menu_t(const char* title,const char** options,int size,int start=0,const char* const* descs=nullptr)
{
	int choice=(start>=0&&start<size)?start:0;
	int top_i=0;
	const int MAXV=7;
	key_reset();
	bool dirty=true;
	while(true)
	{
		key_update();
		if(key_pressed(kb_KeyUp)){ choice=(choice+size-1)%size; dirty=true; }
		if(key_pressed(kb_KeyDown)){ choice=(choice+1)%size; dirty=true; }
		if(key_pressed(kb_KeyEnter)||key_pressed(kb_Key2nd)){ wait_key_release(); return choice; }
		if(key_pressed(kb_KeyClear)){ wait_key_release(); return -1; }
		if(dirty)
		{
			dirty=false;
			if(choice<top_i) top_i=choice;
			if(choice>=top_i+MAXV) top_i=choice-MAXV+1;
			int vis=size<MAXV?size:MAXV;
			const int step=24, bh=20;
			int top=44+(MAXV-vis)*step/2;
			gfx_SetDrawBuffer();
			draw_dirt_bg();
			menu_text_setup();
			gfx_SetTextScale(2,2);
			shadow_text_centered(title,12,COL_GRAY_L);
			gfx_SetTextScale(1,1);
			for(int i=0;i<vis;++i)
			{
				int idx=top_i+i;
				draw_mc_button(40,top+i*step,240,bh,idx==choice,options[idx]);
			}
			if(top_i>0) shadow_text("^",290,top+4,COL_WHITE);
			if(top_i+vis<size) shadow_text("v",290,top+(vis-1)*step+6,COL_WHITE);
			if(descs&&descs[choice]) shadow_text_centered(descs[choice],224,COL_GRAY_L);
			gfx_SwapDraw();
		}
	}
}

// ============================================================================
//  INVENTORY HELPERS
// ============================================================================
void decrease_item(int id)
{
	if(gamemode==1) return;
	if(id<=0||id>=INV_SIZE) return;
	if(inventory[id]>0)
	{
		inventory[id]--;
		if(inventory[id]==0)
			for(int i=0;i<HOTBAR_N;++i) if(ghotbar[i]==id){ ghotbar[i]=0; break; }
	}
	updatehotbar=2;
}
void add2hotbar(int id)
{
	updatehotbar=2;
	for(int i=0;i<HOTBAR_N;++i) if(ghotbar[i]==id) return;
	for(int i=0;i<HOTBAR_N;++i) if(ghotbar[i]==0){ ghotbar[i]=(uint8_t)id; return; }
}
static int num_visible=0;
static uint8_t visible_ids[INV_SIZE];
static bool hidden_id(int id)
{
	static const uint8_t hide[]={BARRIER,UNLITLAMP,LITLAMP,UNLITBUTTON,LITBUTTON,WATERBLOCK,WATERFALL,FLOORVOXELS,
		REDSTONEDUSTLIT,WATERSLAB,WATERFLOOR,DOORBOTTOMOPEN,DOORTOPCLOSED,DOORBOTTOMCLOSED,
		FIRE,LITTNT,SMOKE,NETHERPORTAL,NETHERITEPIC,
		FURNACE_1,FURNACE_2,FURNACE_3,PUMPKIN_1,PUMPKIN_2,PUMPKIN_3};
	for(unsigned i=0;i<sizeof(hide);i++) if(hide[i]==id) return true;
	return false;
}
static void build_visible_ids()
{
	num_visible=0;
	for(int i=1;i<ITEMBASE+NUM_ITEMS;i++) if(!hidden_id(i)) visible_ids[num_visible++]=(uint8_t)i;
}

// ============================================================================
//  MOBS  (stored as blocks in the world grid; static pools)
// ============================================================================
static inline bool inb(int x,int z){ return x>=1&&x<=WX-2&&z>=1&&z<=WZ-2; }
static inline int8_t sgn(int n){ return (n>0)-(n<0); }
struct Mob
{
	bool active=false;
	uint8_t x=0,y=0,z=0;
	int8_t hp=0;
	uint8_t hurt_ticks=0;
	virtual void clear()=0;
	virtual void draw()=0;
	virtual void update()=0;
	virtual bool at(uint8_t* block)=0;
	virtual void hurt(int amount)=0;
	virtual ~Mob(){}
};
#define MAX_MOBS 24
Mob* mob_list[MAX_MOBS];
int mob_count=0;
bool add_mob(Mob* m)
{
	if(mob_count>=MAX_MOBS) return false;
	mob_list[mob_count++]=m;
	m->active=true;
	return true;
}
void remove_mob(Mob* m)
{
	for(int i=0;i<mob_count;++i)
		if(mob_list[i]==m){ mob_list[i]=mob_list[mob_count-1]; mob_count--; break; }
	m->active=false;
}
void despawn_mob(Mob* m){ m->clear(); remove_mob(m); }
void update_all_mobs()
{
	ch_valid=false;
	for(int i=mob_count-1;i>=0;--i)
	{
		if(i>=mob_count) continue;
		Mob* m=mob_list[i];
		if(m->hurt_ticks&&--m->hurt_ticks==0){ m->clear(); m->draw(); }
		m->update();
	}
}
static const uint8_t attack_damage[item_num]={2,3,2,4,5,6,1,4,5,4,6,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
void hurtmob_at(uint8_t* block)
{
	int amount=1;
	int h=ghotbar[hotbarblock];
	if(h>=ITEMBASE&&h<ITEMBASE+NUM_ITEMS) amount=attack_damage[h-ITEMBASE];
	for(int i=0;i<mob_count;++i)
		if(mob_list[i]->at(block)){ mob_list[i]->hurt(amount); return; }
}
static void give_drop(uint8_t id,uint8_t cnt){ if(id&&cnt){ inventory[id]+=cnt; add2hotbar(id); } }

struct HMob : public Mob
{
	int8_t facingx=1, facingz=0, base_hp=8;
	uint8_t faceX=0,backX=0,faceZ=0,backZ=0, d1=0,c1=0,d2=0,c2=0;
	void setup(uint8_t fx,uint8_t bx,uint8_t fz,uint8_t bz,int8_t hp_,uint8_t a=0,uint8_t b=0,uint8_t c=0,uint8_t d=0)
	{ faceX=fx; backX=bx; faceZ=fz; backZ=bz; base_hp=hp_; hp=hp_; d1=a; c1=b; d2=c; c2=d; hurt_ticks=0; }
	bool spawn()
	{
		x=randInt(3,WX-4); z=randInt(3,WZ-4); facingx=1; facingz=0; hp=base_hp;
		int yy=WY-1; bool hitair=false;
		while(yy>1){ if(vis_map(x,yy,z)==0) hitair=true; if(hitair&&vis_map(x,yy-1,z)!=0) break; yy--; }
		if(yy<=1||yy>=WY-1) return false;
		y=yy;
		if(vis_map(x,y,z)!=0||vis_map(x-1,y,z)!=0) return false;
		draw();
		return true;
	}
	void clear() override { vis_map(x,y,z)=0; vis_map(x-facingx,y,z-facingz)=0; }
	void draw() override
	{
		uint8_t a,b;
		if(facingx!=0){ a=faceX; b=backX; } else { a=faceZ; b=backZ; }
		if(hurt_ticks){ a=MOBHURT; b=MOBHURT; }
		vis_map(x,y,z)=a; vis_map(x-facingx,y,z-facingz)=b;
	}
	void fall(){ if(y>1&&vis_map(x,y-1,z)==0&&vis_map(x-facingx,y-1,z-facingz)==0) y--; }
	void move()
	{
		clear();
		int nx=x+facingx, nz=z+facingz;
		if(inb(nx,nz))
		{
			if(vis_map(nx,y,nz)==0){ x=nx; z=nz; }
			else if(y<WY-2&&vis_map(nx,y+1,nz)==0&&vis_map(x,y+1,z)==0){ x=nx; z=nz; y++; }
		}
		fall();
		draw();
	}
	void randturn()
	{
		clear();
		int8_t fx=facingx, fz=facingz;
		switch(randInt(0,10)){ case 0: fx=1; fz=0; break; case 1: fx=-1; fz=0; break; case 2: fx=0; fz=1; break; case 3: fx=0; fz=-1; break; }
		if(inb(x-fx,z-fz)&&vis_map(x-fx,y,z-fz)==0){ facingx=fx; facingz=fz; }
		draw();
	}
	void knock()
	{
		int dx=x-(int)plx, dz=z-(int)plz;
		int8_t sx=0, sz=0;
		if(iabs(dx)>=iabs(dz)) sx=sgn(dx); else sz=sgn(dz);
		if(sx==0&&sz==0) return;
		int nx=x+sx, nz=z+sz, bx=nx-facingx, bz=nz-facingz;
		if(inb(nx,nz)&&inb(bx,bz)&&vis_map(nx,y,nz)==0&&vis_map(bx,y,bz)==0){ x=nx; z=nz; }
		fall();
	}
	void update() override { if(randInt(0,AVERAGEFPS)==0){ move(); randturn(); } }
	bool at(uint8_t* b) override { return &vis_map(x,y,z)==b||&vis_map(x-facingx,y,z-facingz)==b; }
	void hurt(int amount) override
	{
		if(!active) return;
		hp-=amount;
		if(hp<=0){ give_drop(d1,c1); give_drop(d2,c2); stats.killed++; despawn_mob(this); return; }
		clear(); knock(); hurt_ticks=2; draw();
	}
};
struct VMob : public Mob
{
	uint8_t facing=0, playerinsight=0;
	int8_t base_hp=20;
	uint8_t cooldown=0, damage=1;
	uint8_t legs=0,topX=0,topZ=0, d1=0,c1=0,d2=0,c2=0;
	int8_t ssx=0, ssz=0;
	uint8_t side_ticks=0;
	void setup(uint8_t l,uint8_t tx,uint8_t tz,int8_t hp_=20,uint8_t dmg=1,uint8_t a=0,uint8_t b=0,uint8_t c=0,uint8_t d=0)
	{ legs=l; topX=tx; topZ=tz; base_hp=hp_; hp=hp_; damage=dmg; d1=a; c1=b; d2=c; c2=d; playerinsight=0; cooldown=0; facing=0; hurt_ticks=0; side_ticks=0; ssx=0; ssz=0; }
	void draw() override
	{
		if(hurt_ticks){ vis_map(x,y,z)=MOBHURT; vis_map(x,y+1,z)=MOBHURT; return; }
		vis_map(x,y,z)=legs; vis_map(x,y+1,z)=facing?topZ:topX;
	}
	void clear() override { vis_map(x,y,z)=0; vis_map(x,y+1,z)=0; }
	bool spawn()
	{
		x=randInt(1,WX-2); z=randInt(1,WZ-2); hp=base_hp;
		int yy=WY-2; bool hitair=false;
		while(yy>1)
		{
			if(vis_map(x,yy,z)==0&&vis_map(x,yy+1,z)==0) hitair=true;
			if(hitair&&vis_map(x,yy-1,z)!=0) break;
			yy--;
		}
		if(yy<=1||yy>=WY-2) return false;
		y=yy;
		if(vis_map(x,y,z)!=0||vis_map(x,y+1,z)!=0) return false;
		if(iabs((int)x-(int)plx)+iabs((int)z-(int)plz)<6) return false;
		draw();
		return true;
	}
	bool step(int8_t sx,int8_t sz)
	{
		if(sx==0&&sz==0) return false;
		int nx=x+sx, nz=z+sz;
		if(!inb(nx,nz)) return false;
		if(vis_map(nx,y+1,nz)!=0) return false;
		if(vis_map(nx,y,nz)!=0)
		{
			if(y<WY-3&&vis_map(nx,y+2,nz)==0&&vis_map(x,y+2,z)==0){ x=nx; z=nz; y++; return true; }
			return false;
		}
		x=nx; z=nz;
		return true;
	}
	void move()
	{
		clear();
		if(playerinsight)
		{
			int iplx=(int)plx, iplz=(int)plz;
			int8_t sx=sgn(iplx-x), sz=sgn(iplz-z);
			bool xfirst=iabs(x-iplx)>iabs(z-iplz);
			bool moved=false;
			if(side_ticks)
			{
				side_ticks--;
				if(step(ssx,ssz)){ facing=(ssz!=0); moved=true; }
				else side_ticks=0;
			}
			if(!moved)
			{
				if(xfirst?step(sx,0):step(0,sz)){ facing=!xfirst; moved=true; }
				else if(xfirst?step(0,sz):step(sx,0)){ facing=xfirst; moved=true; }
			}
			if(!moved&&iabs(x-iplx)+iabs(z-iplz)>1)
			{
				if(xfirst){ ssx=0; ssz=randInt(0,1)?1:-1; }
				else { ssz=0; ssx=randInt(0,1)?1:-1; }
				if(step(ssx,ssz)){ facing=(ssz!=0); side_ticks=3; }
			}
		}
		else if(randInt(0,3)==0)
		{
			facing=!facing;
			if(iabs((int)plx-x)+iabs((int)plz-z)<12) playerinsight=1;
		}
		if(y>1&&vis_map(x,y-1,z)==0) y--;
		draw();
	}
	void update() override
	{
		uint8_t px=x, py=y, pz=z;
		move();
		if(cooldown) cooldown--;
		if((int)plx==x&&(int)plz==z&&iabs((int)ply-1-y)<2)
		{
			if(cooldown==0&&damage)
			{
				playerhp-=damage+(difficulty>=2?damage:0);
				cooldown=AVERAGEFPS;
				updatehotbar=2;
			}
			clear(); x=px; y=py; z=pz; draw();
		}
	}
	void knock()
	{
		int dx=x-(int)plx, dz=z-(int)plz;
		if(dx==0&&dz==0) return;
		if(iabs(dx)>=iabs(dz)) step(sgn(dx),0); else step(0,sgn(dz));
		if(y>1&&vis_map(x,y-1,z)==0) y--;
	}
	bool at(uint8_t* b) override { return &vis_map(x,y,z)==b||&vis_map(x,y+1,z)==b; }
	void hurt(int amount) override
	{
		if(!active) return;
		hp-=amount;
		if(hp<1){ give_drop(d1,c1); give_drop(d2,c2); stats.killed++; despawn_mob(this); return; }
		clear(); knock(); hurt_ticks=2; draw();
	}
};
struct Skeleton : public VMob
{
	void update() override
	{
		clear();
		int dx=(int)plx-x, dz=(int)plz-z;
		int adx=iabs(dx), adz=iabs(dz), dist=adx+adz;
		facing=adz>=adx;
		if(cooldown) cooldown--;
		if(dist<=14)
		{
			int8_t sx=0, sz=0;
			if(adx>adz) sx=sgn(dx); else sz=sgn(dz);
			if(dist>8) step(sx,sz);
			else if(dist<4) step(-sx,-sz);
			else if(randInt(0,3)==0) step(sz?1:0,sx?1:0);
			if(dist<=10&&cooldown==0&&randInt(0,2)==0)
			{
				playerhp-=damage+(difficulty>=2?damage:0);
				cooldown=AVERAGEFPS*2;
				updatehotbar=2;
			}
		}
		if(y>1&&vis_map(x,y-1,z)==0) y--;
		draw();
	}
};
struct Creeper : public VMob
{
	uint8_t fuse=0;
	void update() override
	{
		if(fuse)
		{
			clear();
			hurt_ticks=(fuse&1)?2:0;
			draw();
			fuse++;
			if(fuse>12)
			{
				clear();
				uint8_t* c=&vis_map(x,y,z);
				remove_mob(this);
				explode(c,3,this);
			}
			return;
		}
		VMob::update();
		if(active&&playerinsight&&iabs((int)plx-x)<=1&&iabs((int)plz-z)<=1&&iabs((int)ply-1-y)<3) fuse=1;
	}
};
#define HMOB_POOL_SIZE 16
#define VMOB_POOL_SIZE 16
#define SKEL_POOL_SIZE 8
#define CREEP_POOL_SIZE 6
static HMob hmob_pool[HMOB_POOL_SIZE];
static VMob vmob_pool[VMOB_POOL_SIZE];
static Skeleton skel_pool[SKEL_POOL_SIZE];
static Creeper creep_pool[CREEP_POOL_SIZE];
static void reset_mobs()
{
	for(int i=0;i<HMOB_POOL_SIZE;i++) hmob_pool[i].active=false;
	for(int i=0;i<VMOB_POOL_SIZE;i++) vmob_pool[i].active=false;
	for(int i=0;i<SKEL_POOL_SIZE;i++) skel_pool[i].active=false;
	for(int i=0;i<CREEP_POOL_SIZE;i++) creep_pool[i].active=false;
	mob_count=0;
}
HMob* spawn_hmob(uint8_t fx,uint8_t bx,uint8_t fz,uint8_t bz,int8_t hp,uint8_t a=0,uint8_t b=0,uint8_t c=0,uint8_t d=0)
{
	for(int i=0;i<HMOB_POOL_SIZE;++i)
		if(!hmob_pool[i].active)
		{
			HMob& m=hmob_pool[i];
			m.setup(fx,bx,fz,bz,hp,a,b,c,d);
			if(!m.spawn()) return NULL;
			if(!add_mob(&m)){ m.clear(); return NULL; }
			return &m;
		}
	return NULL;
}
VMob* spawn_vmob(uint8_t legs,uint8_t topX,uint8_t topZ,int8_t hp=20,uint8_t dmg=1,uint8_t a=0,uint8_t b=0,uint8_t c=0,uint8_t d=0)
{
	for(int i=0;i<VMOB_POOL_SIZE;++i)
		if(!vmob_pool[i].active)
		{
			VMob& m=vmob_pool[i];
			m.setup(legs,topX,topZ,hp,dmg,a,b,c,d);
			if(!m.spawn()) return NULL;
			if(!add_mob(&m)){ m.clear(); return NULL; }
			return &m;
		}
	return NULL;
}
Skeleton* spawn_skeleton()
{
	for(int i=0;i<SKEL_POOL_SIZE;++i)
		if(!skel_pool[i].active)
		{
			Skeleton& m=skel_pool[i];
			m.setup(SKELEBOTTOM,SKELETOPX,SKELETOPZ,12,2,STICK,2);
			if(!m.spawn()) return NULL;
			if(!add_mob(&m)){ m.clear(); return NULL; }
			return &m;
		}
	return NULL;
}
Creeper* spawn_creeper()
{
	for(int i=0;i<CREEP_POOL_SIZE;++i)
		if(!creep_pool[i].active)
		{
			Creeper& m=creep_pool[i];
			m.setup(CREEPERLEGS,CREEPERTOPX,CREEPERTOPZ,14,0,GUNPOWDER,1);
			m.fuse=0;
			if(!m.spawn()) return NULL;
			if(!add_mob(&m)){ m.clear(); return NULL; }
			return &m;
		}
	return NULL;
}
#define spawn_sheep() spawn_hmob(SHEEPFACEX,SHEEPBACKX,SHEEPFACEZ,SHEEPBACKZ,8,WOOL,2,RAWMUTTON,1)
#define spawn_pig() spawn_hmob(PIGFACEX,PIGBACKX,PIGFACEZ,PIGBACKZ,8,RAWPORKCHOP,1)
#define spawn_zombie() spawn_vmob(ZOMBIELEGS,ZOMBIETOPX,ZOMBIETOPZ,20,1,ROTTENFLESH,2)
VMob* spawn_piglin()
{
	VMob* s=spawn_vmob(PIGLINLEGS,PIGLINTOPX,PIGLINTOPZ,20,1,RAWPORKCHOP,1,GOLDINGOT,1);
	if(!s) return NULL;
	if(randInt(0,1)&&s->y>15)
	{
		s->clear();
		uint8_t oy=s->y;
		s->y-=2;
		while(vis_map(s->x,s->y,s->z)||vis_map(s->x,s->y+1,s->z)){ s->y--; if(s->y<2){ s->y=oy; break; } }
		s->draw();
	}
	return s;
}
template<class T>
void manage_nocturnal(T* pool,int n,int wtime,int dmg)
{
	for(int i=0;i<n;++i)
	{
		T& m=pool[i];
		if(!m.active) continue;
		if(dmg&&wtime<10&&m.hp>0)
		{
			m.hp-=dmg;
			if(m.y+2<WY)
			{
				uint8_t* f=&vis_map(m.x,m.y+2,m.z);
				if(*f==AIR){ *f=FIRE; add_update(f,3); }
			}
		}
		if(m.hp<1){ m.clear(); remove_mob(&m); }
	}
}

// ============================================================================
//  EXPLOSIONS, TIME, PORTALS
// ============================================================================
void explode(uint8_t* block,uint8_t r,Mob* src)
{
	ch_valid=false;
	uint8_t x,y,z;
	address2coords(block,x,y,z);
	int rr=r*r;
	for(int dx=-r;dx<=r;++dx)
	{
		int nx=x+dx; if((unsigned)nx>=WX) continue;
		for(int dy=-r;dy<=r;++dy)
		{
			int ny=y+dy; if((unsigned)ny>=WY) continue;
			for(int dz=-r;dz<=r;++dz)
			{
				int nz=z+dz; if((unsigned)nz>=WZ) continue;
				if(dx*dx+dy*dy+dz*dz>rr) continue;
				uint8_t* bl=&vis_map(nx,ny,nz);
				uint8_t t=*bl;
				if(bflags[t]&F_BLAST) continue;
				if(t==NETHERPORTAL){ break_portal(bl); continue; }
				if(t==TNT){ *bl=LITTNT; add_update(bl,(uint8_t)randInt(4,10)); continue; }
				if(t==LITTNT||t>=MOBSTART) continue;
				if(t!=0&&t!=SMOKE){ if(randInt(0,2)==0) break_block(bl); else *bl=0; }
				if(randInt(0,1)) *bl=SMOKE;
				add_updates_fully(bl);
			}
		}
	}
	for(int i=mob_count-1;i>=0;--i)
	{
		if(i>=mob_count) continue;
		Mob* m=mob_list[i];
		if(m==src) continue;
		if(iabs(m->x-x)+iabs(m->y-y)+iabs(m->z-z)<=r+2) m->hurt(8);
	}
	if(gamemode==0)
	{
		int d=iabs((int)plx-x)+iabs((int)ply-1-y)+iabs((int)plz-z);
		if(d<=r*2)
		{
			int dmg=r*2+1-d;
			if(difficulty>=2) dmg<<=1;
			else if(difficulty==0) dmg=(dmg+1)>>1;
			playerhp-=dmg;
			updatehotbar=2;
		}
	}
}
static uint32_t world_ticks(){ return (uint32_t)(clock()-start_time)+offsettime; }
static int world_time()
{
	uint32_t s=world_ticks()>>15;
	s=(s*17477UL)>>19;
	return (int)(s%20);
}
void draw_portal(uint8_t sx,uint8_t sy,uint8_t sz)
{
	if(sy>27) sy=27;
	for(uint8_t x=sx;x<sx+4;++x) for(uint8_t y=sy;y<sy+5;++y)
	{
		bool edge=(x==sx||x==sx+3||y==sy||y==sy+4);
		vis_map(x,y,sz)=edge?OBSIDIAN:NETHERPORTAL;
	}
}
bool is_portal(uint8_t bx,uint8_t by,uint8_t bz,uint8_t* out_sx,uint8_t* out_sy,uint8_t* out_sz,bool* z_oriented)
{
	for(int dx=0;dx<4;++dx) for(int dy=0;dy<5;++dy)
	{
		if(!(dx==0||dx==3||dy==0||dy==4)) continue;
		int sx=bx-dx, sy=by-dy;
		if(sx<0||sy<0||sx+3>=WX||sy+4>=WY) continue;
		bool ok=true;
		for(int x=0;x<4&&ok;++x) for(int y=0;y<5&&ok;++y)
		{
			bool corner=(x==0||x==3)&&(y==0||y==4);
			bool edge=(x==0||x==3||y==0||y==4)&&!corner;
			uint8_t b=vis_map(sx+x,sy+y,bz);
			if(edge&&b!=OBSIDIAN) ok=false;
			else if(!edge&&!corner&&b!=AIR) ok=false;
		}
		if(ok){ *out_sx=sx; *out_sy=sy; *out_sz=bz; *z_oriented=false; return true; }
	}
	for(int dz=0;dz<4;++dz) for(int dy=0;dy<5;++dy)
	{
		if(!(dz==0||dz==3||dy==0||dy==4)) continue;
		int sz=bz-dz, sy=by-dy;
		if(sz<0||sy<0||sz+3>=WZ||sy+4>=WY) continue;
		bool ok=true;
		for(int z=0;z<4&&ok;++z) for(int y=0;y<5&&ok;++y)
		{
			bool corner=(z==0||z==3)&&(y==0||y==4);
			bool edge=(z==0||z==3||y==0||y==4)&&!corner;
			uint8_t b=vis_map(bx,sy+y,sz+z);
			if(edge&&b!=OBSIDIAN) ok=false;
			else if(!edge&&!corner&&b!=AIR) ok=false;
		}
		if(ok){ *out_sx=bx; *out_sy=sy; *out_sz=sz; *z_oriented=true; return true; }
	}
	return false;
}
bool try_light_portal(uint8_t bx,uint8_t by,uint8_t bz)
{
	uint8_t sx,sy,sz; bool zo;
	if(!is_portal(bx,by,bz,&sx,&sy,&sz,&zo)) return false;
	if(!zo){ for(int x=1;x<3;++x) for(int y=1;y<4;++y) vis_map(sx+x,sy+y,sz)=NETHERPORTAL; }
	else   { for(int z=1;z<3;++z) for(int y=1;y<4;++y) vis_map(sx,sy+y,sz+z)=NETHERPORTAL; }
	return true;
}
void break_portal_helper(uint8_t* block)
{
	if(block<world||block>=world+WORLD_BYTES) return;
	if(*block==NETHERPORTAL)
	{
		*block=0;
		break_portal_helper(block+XJ); break_portal_helper(block-XJ);
		break_portal_helper(block+YJ); break_portal_helper(block-YJ);
		break_portal_helper(block+ZJ); break_portal_helper(block-ZJ);
	}
}
void break_portal(uint8_t* block)
{
	break_portal_helper(block+XJ); break_portal_helper(block-XJ);
	break_portal_helper(block+YJ); break_portal_helper(block-YJ);
	break_portal_helper(block+ZJ); break_portal_helper(block-ZJ);
}
void spawn_nether_portal(uint8_t x,uint8_t& y,uint8_t z)
{
	if(x<1) x=1;
	if(x>27) x=27;
	if(z<2) z=2;
	if(z>29) z=29;
	if(plx>27) plx=27;
	if(plx<1) plx=1;
	if(plz>29) plz=29;
	if(plz<2) plz=2;
	bool hit_air=false;
	for(y=WY-1;y>0;--y)
	{
		if(vis_map(x,y,z)==AIR) hit_air=true;
		if(hit_air&&vis_map(x,y,z)){ draw_portal(x,y,z); break; }
	}
}
bool find_nether_portal(uint8_t& x,uint8_t& y,uint8_t& z)
{
	for(x=0;x<WX;++x) for(z=0;z<WZ;++z) for(y=WY-1;y>0;y--)
		if(vis_map(x,y,z)==NETHERPORTAL) return true;
	return false;
}

// ============================================================================
//  BLOCK / ITEM USE
// ============================================================================
static inline uint8_t look_dir(){ return (uint8_t)(((rotxz+8)>>4)&3); }//0:+z 1:-x 2:-z 3:+x
static uint8_t orient_base(uint8_t id)
{
	if(id>=FURNACE_1&&id<=FURNACE_3) return FURNACE;
	if(id>=PUMPKIN_1&&id<=PUMPKIN_3) return PUMPKIN;
	return id;
}
static uint8_t orient_place(uint8_t id)
{
	uint8_t l=look_dir();
	if(l==0) return id;
	if(id==FURNACE) return (uint8_t)(FURNACE_1+l-1);
	if(id==PUMPKIN) return (uint8_t)(PUMPKIN_1+l-1);
	return id;
}
int break_block(uint8_t* block)
{
	ch_valid=false;
	uint8_t bv=*block;
	int iv=orient_base(bv);
	if(bv>=DOORTOPOPEN&&bv<=DOORBOTTOMCLOSED) iv=DOORTOPOPEN;
	switch(bv)
	{
		case DIAMOND: iv=DIAMONDITEM; break;
		case REDSTONE: iv=REDSTONEDUSTUNLIT; break;
		case REDSTONEDUSTLIT: iv=REDSTONEDUSTUNLIT; break;
		case COAL: iv=COALITEM; break;
		case FIRE: case SMOKE: case GLASS: case LITTNT:
		case WATERBLOCK: case WATERFALL: case WATERSLAB: case WATERFLOOR:
			iv=0; break;
		case GRAVEL: if(randInt(0,4)==0) iv=FLINT; break;
		case NETHERPORTAL: iv=0; break_portal(block); break;
		case OBSIDIAN: break_portal(block); break;
		case NETHERGOLDORE: iv=GOLDINGOT; break;
		case LEAF:
		{
			int r=randInt(0,9);
			if(r==0){ inventory[APPLE]++; add2hotbar(APPLE); toast("You found an apple!"); }
			else if(r<3){ inventory[SAPLING]++; add2hotbar(SAPLING); }
			break;
		}
	}
	if(iv){ inventory[iv]++; add2hotbar(iv); }
	*block=0;
	add_updates_fully(block);
	if(bv==DOORTOPCLOSED||bv==DOORTOPOPEN){ block-=YJ; if(*block>=DOORTOPOPEN&&*block<=DOORBOTTOMCLOSED) *block=0; }
	else if(bv==DOORBOTTOMCLOSED||bv==DOORBOTTOMOPEN){ block+=YJ; if(*block>=DOORTOPOPEN&&*block<=DOORBOTTOMCLOSED) *block=0; }
	return iv;
}
static void use_bed(uint8_t* block)
{
	if(realm==1){ explode(block,3); return; }
	respawnx=plx; respawny=ply; respawnz=plz;
	respawnchunkx=chunkx; respawnchunkz=chunkz;
	int wt=world_time();
	if(wt>9)
	{
		gfx_SetDrawBuffer();
		gfx_FillScreen(0);
		gfx_SetTextBGColor(26);
		gfx_SetTextFGColor(255);
		gfx_PrintStringXY("Sleeping...",124,110);
		gfx_SwapDraw();
		msleep(2000);
		uint32_t skip=(uint32_t)(22-wt)*30;
		offsettime+=skip<<15;
		toast("Good morning! Respawn point set");
	}
	else toast("Respawn point set");
	updatehotbar=2;
}
static void use_doorto(uint8_t* b){ *b+=2; b-=YJ; *b+=2; }
static void use_doortc(uint8_t* b){ *b-=2; b-=YJ; *b-=2; }
static void use_doorbo(uint8_t* b){ *b+=2; b+=YJ; *b+=2; }
static void use_doorbc(uint8_t* b){ *b-=2; b+=YJ; *b-=2; }
static void use_tnt(uint8_t* block){ if(ghotbar[hotbarblock]==FLINTANDSTEEL){ *block=LITTNT; add_updates_fully(block); } }
static void light_fire()
{
	keyup(key_place);
	uint8_t* before;
	uint8_t* t=cross_hair_pt(before);
	if(!t) return;
	if(*t==OBSIDIAN)
	{
		uint8_t x,y,z;
		address2coords(t,x,y,z);
		if(try_light_portal(x,y,z)){ toast("The portal hums..."); return; }
	}
	if(*t==TNT){ use_tnt(t); return; }
	if(*before==0){ *before=FIRE; add_updates_fully(before); ch_valid=false; }
}
static void use_bucket()
{
	uint8_t* before;
	uint8_t* t=cross_hair_pt(before,1);
	if(t&&*t==WATERBLOCK)
	{
		*t=0; add_updates_fully(t); ch_valid=false;
		decrease_item(BUCKET);
		inventory[WATERBUCKET]++; add2hotbar(WATERBUCKET);
	}
}
static void use_waterbucket()
{
	uint8_t* before;
	uint8_t* t=cross_hair_pt(before);
	if(t&&*before==0)
	{
		*before=WATERBLOCK; add_updates_fully(before); ch_valid=false;
		decrease_item(WATERBUCKET);
		inventory[BUCKET]++; add2hotbar(BUCKET);
	}
}
static void eat(int id,int food,int heal)
{
	if(playerhunger>=20&&heal==0&&gamemode==0){ toast("You're not hungry"); return; }
	playerhunger=(int8_t)imin(20,playerhunger+food);
	playerhp=(int8_t)imin(20,playerhp+heal);
	decrease_item(id);
	updatehotbar=2;
}
static void use_rawmutton(){ eat(RAWMUTTON,2,0); }
static void use_cookedmutton(){ eat(COOKEDMUTTON,8,0); }
static void use_rottenflesh(){ eat(ROTTENFLESH,3,0); }
static void use_rawpork(){ eat(RAWPORKCHOP,3,0); }
static void use_cookedpork(){ eat(COOKEDPORKCHOP,8,0); }
static void use_apple(){ eat(APPLE,4,0); }
static void use_goldenapple(){ eat(GOLDENAPPLE,4,8); toast("You feel much stronger!"); }
static void (*use_item[item_num])()={
	NULL,NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL, NULL,NULL,
	use_rawmutton,use_cookedmutton,use_rottenflesh, NULL,NULL,
	light_fire,NULL,use_bucket,use_waterbucket, use_rawpork,use_cookedpork,NULL,
	use_apple,use_goldenapple
};
static void furnace_screen();
static bool use_block(uint8_t* b)
{
	switch(*b)
	{
		case CRAFTTABLE: open_crafting_table=true; return true;
		case FURNACE: case FURNACE_1: case FURNACE_2: case FURNACE_3: furnace_screen(); updatehotbar=2; return true;
		case BEDTOP: use_bed(b); return true;
		case DOORTOPOPEN: use_doorto(b); return true;
		case DOORBOTTOMOPEN: use_doorbo(b); return true;
		case DOORTOPCLOSED: use_doortc(b); return true;
		case DOORBOTTOMCLOSED: use_doorbc(b); return true;
		case TNT: if(ghotbar[hotbarblock]==FLINTANDSTEEL){ use_tnt(b); return true; } return false;
	}
	return false;
}

// ============================================================================
//  RAYCASTER  (0.4.0)
//  One ray per 5x5 cell. Even rows: even columns traced, odd columns resolved
//  from neighbours' hit records (same block+face -> exact analytic texel, now
//  verified to land inside the block). Odd rows resolved from rows above/below.
//  Detail by distance: full (<5.5 blocks), 4x4 mip (5.5-12), average (>12).
//  Graphics: 0 Fancy, 1 Fast, 2 Fastest, 3 Ultra (every ray traced, reference).
// ============================================================================
extern "C" uint8_t ASM_DDA_FULL(uint8_t quadrant);
extern "C" int* getddadeltax();
extern "C" int* getddadeltay();
extern "C" int* getddadeltaz();
extern "C" uint8_t** getddacurrblock();
extern "C" int* getddatmaxx();
extern "C" int* getddatmaxy();
extern "C" int* getddatmaxz();
int* ddadeltax=getddadeltax();
int* ddadeltay=getddadeltay();
int* ddadeltaz=getddadeltaz();
uint8_t** ddacurrblock=getddacurrblock();
int* ddatmaxx=getddatmaxx();
int* ddatmaxy=getddatmaxy();
int* ddatmaxz=getddatmaxz();
extern "C" uint8_t* getscratch3();
extern "C" uint8_t** getrectaddr();
extern "C" void optomized_raycast_rect_5x5_no2();
extern "C" void optomized_raycast_rect_run();
extern "C" int runfill_n;
extern "C" int runfill_advance;

static uint8_t world_top=WY-1;   //highest non-air, non-barrier block
static uint8_t thin_world=0;     //1 = some block with see-through texels exists (disables sky fill in Fancy)
static void compute_world_top()
{
	uint8_t top=0, thin=0;
	for(int i=0;i<WORLD_BYTES;i++)
	{
		uint8_t b=world[i];
		if(!b||b==BARRIER) continue;
		thin|=thin_id[b];
		uint8_t y=(uint8_t)((i>>YWS)&(WY-1));
		if(y>top) top=y;
	}
	world_top=top; thin_world=thin;
}

#define HF_SKY   0xFF
#define HF_NOCOH 0x80   //retry/floor layer: never analytic
#define HF_FLAT  0x40   //color independent of the texel
#define HF_LOD   0x20   //far hit drawn with the average color
#define RG_MIP 2816     //5.5 blocks
#define RG_AVG 6144     //12 blocks (also the fog start)

static uint8_t** R_rect;
static uint8_t* R_col;
static uint8_t* R_w;
static int R_ix,R_iy,R_iz,R_ox,R_oy,R_oz,R_sxz,R_cxz,R_dy,R_tdyd;
static flint R_cosxz,R_sinxz,R_cosyz,R_sinyz;
static uint8_t R_tdybit,R_sky,R_smooth,R_nosky;
static uint16_t rec_i,res_i;
static uint8_t rec_f,res_f;
static uint8_t sky_row[38];
static uint16_t ri[2][64];
static uint8_t rf[2][64],rc[2][64],oc[64];

static void setup_row(int y)
{
	flint ny=fraw(ydir_d(y));
	flint tdy=(R_cosyz*ny-R_sinyz);
	flint tdz=(R_sinyz*ny+R_cosyz);
	R_tdyd=tdy.data;
	R_dy=INV(tdy.data);
	*ddadeltay=R_dy;
	R_sxz=(R_sinxz.data*tdz.data)>>shift_by;
	R_cxz=(R_cosxz.data*tdz.data)>>shift_by;
	R_tdybit=(uint8_t)((tdy.data>0)<<1);
	if(realm==1) R_sky=bg_color;
	else { int k=tdy.data>>6; R_sky=(k<2)?sky_hi:((k<4)?sky_mid:bg_color); }
	sky_row[y/5]=R_sky;
}
// hit point (u,v) in 9-bit world coordinates: (a*b)>>9 + (int)pl<<9 + half  (same ops as the DDA path)
static void uv_of(uint8_t slot,int t,int dxd,int dzd,int& u,int& v)
{
	if(slot<2){ u=((dzd*t)>>shift_by)+R_oz; v=((R_tdyd*t)>>shift_by)+R_oy; }
	else if(slot<4){ u=((dxd*t)>>shift_by)+R_ox; v=((dzd*t)>>shift_by)+R_oz; }
	else { u=((dxd*t)>>shift_by)+R_ox; v=((R_tdyd*t)>>shift_by)+R_oy; }
}
// final shaded+fogged color for a hit, or -1 if the texel is transparent
static int pick(uint8_t id,uint8_t slot,int texit,uint8_t hz,int u,int v)
{
	const int k=id*TSTR+slot;
	const uint8_t sc=shade_on[id]?slot:TS_TOP;
	const uint8_t m=mipidx[k];
	if(m!=0xFF)
	{
		if(texit>RG_AVG) return flp[8+sc][mavg[m]];
		if(R_smooth&&texit>=RG_MIP) return flp[hz+sc][mpool[m][((u&511)>>7)+((v>>5)&12)]];
	}
	const uint8_t tx=LTEX[k][((u&511)>>6)+((v>>3)&0x38)];
	return tx==26?-1:flp[hz+sc][tx];
}
static uint8_t trace_cell(int col)
{
	const int dxd=cache[col].data-R_sxz;
	const int dzd=cache[320+col].data+R_cxz;
	const int Dx=INV(dxd), Dz=INV(dzd);
	const uint8_t q=(uint8_t)((dxd>0)|R_tdybit|((dzd>0)<<2));
	*ddadeltax=Dx; *ddadeltaz=Dz;
	*ddatmaxx=Dx>>1; *ddatmaxy=R_dy>>1; *ddatmaxz=Dz>>1;
	uint8_t* curr=R_w;
	uint8_t clean=0;
	for(;;)
	{
		*ddacurrblock=curr;
		uint8_t d=ASM_DDA_FULL(q);
		if(d==3){ rec_f=HF_SKY; return R_sky; }
		d&=63;
		curr=*ddacurrblock;
		const uint8_t id=*curr;
		if(id==BARRIER){ rec_f=HF_SKY; return R_sky; }
		const uint8_t slot=(d==0)?(uint8_t)(q&1):((d==RBP1)?(uint8_t)(2+((q>>1)&1)):(uint8_t)(4+(q>>2)));
		const int tmx=*ddatmaxx, tmy=*ddatmaxy, tmz=*ddatmaxz;
		const int texit=(d==0)?tmx:((d==RBP1)?tmy:tmz);
		const uint8_t hz=(texit>0x1800)?8:0;
		rec_i=(uint16_t)(curr-world);
		if(!tex_on[id]){ rec_f=(uint8_t)(clean|HF_FLAT|slot); return flp[hz+TS_TOP][colors[id+d]]; }
		if(id>FLOORVOXELS&&id<=FLOORVOXELSEND)
		{
			if(id>=SLABSTART)
			{
				if(R_tdyd<0)
				{
					const int ty=tmy-(R_dy>>1);
					if(ty<tmx&&ty<tmz){ rec_f=HF_NOCOH|HF_FLAT; return flp[hz+TS_TOP][floorcolors[id]]; }
				}
				else if(d==RBP1){ rec_f=HF_NOCOH|HF_FLAT; return flp[hz+TS_TOP][floorcolors[id]]; }
			}
			else
			{
				if(tmy<tmx&&tmy<tmz&&R_tdyd<0){ rec_f=HF_NOCOH|HF_FLAT; return flp[hz+TS_TOP][floorcolors[id]]; }
				clean=HF_NOCOH;
				continue;
			}
		}
		const int D=(d==0)?Dx:((d==RBP1)?R_dy:Dz);
		int u,v;
		uv_of(slot,texit-D,dxd,dzd,u,v);
		const int c=pick(id,slot,texit,hz,u,v);
		if(c<0){ clean=HF_NOCOH; continue; }//transparent texel: the ray resumes from here
		const uint8_t lod=(texit>RG_AVG&&mipidx[id*TSTR+slot]!=0xFF)?HF_LOD:0;
		rec_f=(uint8_t)(clean|lod|slot);
		return (uint8_t)c;
	}
}
// resolve a cell whose neighbours both hit block idx on this face. -1 = must trace
static int analytic(int col,uint16_t idx,uint8_t f)
{
	const uint8_t slot=f&7;
	const uint8_t id=world[idx];
	const int dxd=cache[col].data-R_sxz;
	const int dzd=cache[320+col].data+R_cxz;
	int n,D,cu,cv;
	if(slot<2){ n=(int)(idx>>XWS)-R_ix; D=INV(dxd); cu=idx&(WZ-1); cv=(idx>>YWS)&(WY-1); }
	else if(slot<4){ n=(int)((idx>>YWS)&(WY-1))-R_iy; D=R_dy; cu=idx>>XWS; cv=idx&(WZ-1); }
	else { n=(int)(idx&(WZ-1))-R_iz; D=INV(dzd); cu=idx>>XWS; cv=(idx>>YWS)&(WY-1); }
	if(n<0) n=-n;
	if(n==0||n>VD) return -1;
	const int texit=n*D+(D>>1);//== the asm's post-step tmax on that axis
	int u,v;
	uv_of(slot,texit-D,dxd,dzd,u,v);
	if((u>>shift_by)!=cu||(v>>shift_by)!=cv) return -1;//ray does not actually land on this face: trace it
	const uint8_t hz=(texit>0x1800)?8:0;
	if(f&HF_FLAT)
	{
		const uint8_t d=(slot<2)?0:((slot<4)?RBP1:2*RBP1);
		return flp[hz+TS_TOP][colors[id+d]];
	}
	return pick(id,slot,texit,hz,u,v);
}
static int resolve(int col,uint8_t fa,uint16_t ia,uint8_t ca,uint8_t fb,uint16_t ib,uint8_t cb)
{
	if(fa==HF_SKY||fb==HF_SKY)
	{
		if(fa!=fb||R_nosky) return -1;
		res_f=HF_SKY; return R_sky;
	}
	if(fa==fb&&!(fa&HF_NOCOH)&&ia==ib)
	{
		const int v=analytic(col,ia,fa);
		if(v>=0){ res_f=fa; res_i=ia; }
		return v;
	}
	if((fa&(HF_FLAT|HF_LOD))&&(fb&(HF_FLAT|HF_LOD))&&ca==cb){ res_f=(uint8_t)(HF_NOCOH|HF_FLAT); res_i=ia; return ca; }
	return -1;
}
static void flush_run(uint8_t col,int n)
{
	*R_col=col;
	if(n>1){ runfill_n=n; runfill_advance=n*res; optomized_raycast_rect_run(); }
	else optomized_raycast_rect_5x5_no2();
}
static void emit_row(uint8_t* row,const uint8_t* cols)
{
	*R_rect=row;
	uint8_t prev=cols[0];
	int n=1;
	for(int c=1;c<64;c++)
	{
		uint8_t v=cols[c];
		if(v==prev){ n++; continue; }
		flush_run(prev,n);
		prev=v; n=1;
	}
	flush_run(prev,n);
}
void raycast_screen()
{
	__asm__ volatile ("di");
	R_rect=getrectaddr();
	R_col=getscratch3();
	R_ix=(int)plx; R_iy=(int)ply; R_iz=(int)plz;
	R_ox=(R_ix<<shift_by)+256; R_oy=(R_iy<<shift_by)+256; R_oz=(R_iz<<shift_by)+256;
	R_w=world+(R_ix<<XWS)+(R_iy<<YWS)+R_iz;
	const bool sky_fast=(R_iy>world_top);
	R_cosxz=COSR(rotxz); R_sinxz=SINR(rotxz);
	R_cosyz=COSR(rotyz); R_sinyz=SINR(rotyz);
	{
		flint* ca=&cache[0];
		flint* cb=ca+320;
		int dnx=xdir_d(res)-xdir_d(0);
		int dcos=(R_cosxz.data*dnx)>>shift_by;
		int dsin=(R_sinxz.data*dnx)>>shift_by;
		int vcos=(R_cosxz.data*xdir_d(0))>>shift_by;
		int vsin=(R_sinxz.data*xdir_d(0))>>shift_by;
		for(int i=0;i<320;i+=res){ ca->data=vcos; cb->data=vsin; vcos+=dcos; vsin+=dsin; ca+=res; cb+=res; }
	}
	const uint8_t mode=set_gfx;
	R_smooth=set_dist;
	R_nosky=(mode==0)&&thin_world;
	uint8_t* vb=(uint8_t*)gfx_vbuffer;
	int cur=0;
	for(int k=0;k<19;k++)
	{
		const int y=k*10;
		uint8_t* ci=rc[cur];
		uint8_t* fi=rf[cur];
		uint16_t* ii=ri[cur];
		setup_row(y);
		if(sky_fast&&R_tdyd>0){ memset(ci,R_sky,64); memset(fi,HF_SKY,64); }
		else
		{
			for(int c=0;c<64;c+=2){ ci[c]=trace_cell(c*res); fi[c]=rec_f; ii[c]=rec_i; }
			for(int c=1;c<64;c+=2)
			{
				int v=(mode<3&&c<63)?resolve(c*res,fi[c-1],ii[c-1],ci[c-1],fi[c+1],ii[c+1],ci[c+1]):-1;
				if(v>=0){ fi[c]=res_f; ii[c]=res_i; }
				else if(mode==2){ v=ci[c-1]; fi[c]=HF_NOCOH; ii[c]=0; }
				else { v=trace_cell(c*res); fi[c]=rec_f; ii[c]=rec_i; }
				ci[c]=(uint8_t)v;
			}
		}
		emit_row(vb+y*320,ci);
		if(k)
		{
			const int prv=cur^1;
			setup_row(y-res);
			if(sky_fast&&R_tdyd>0) memset(oc,R_sky,64);
			else for(int c=0;c<64;c++)
			{
				int v=(mode<3)?resolve(c*res,rf[prv][c],ri[prv][c],rc[prv][c],fi[c],ii[c],ci[c]):-1;
				if(v<0) v=(mode==1||mode==2)?rc[prv][c]:trace_cell(c*res);
				oc[c]=(uint8_t)v;
			}
			emit_row(vb+(y-res)*320,oc);
		}
		cur^=1;
	}
	__asm__ volatile ("ei");
}
// sun (day) / moon (night): projected through the inverse camera rotation, drawn only over this frame's sky pixels
static void draw_celestial(int wtime)
{
	if(realm==1) return;
	const bool day=wtime<10;
	const int a=((day?wtime:wtime-10)*32+16)/10;
	flint X=COSR(a), Y=SINR(a);
	flint cxz=COSR(rotxz), sxz=SINR(rotxz), cyz=COSR(rotyz), syz=SINR(rotyz);
	flint nx=cxz*X;
	flint nz1=-(sxz*X);
	flint ny=cyz*Y+syz*nz1;
	flint nz=cyz*nz1-syz*Y;
	if(nz.data<128) return;
	const int sx=160+(nx.data*focal_length)/nz.data;
	const int sy=120-(ny.data*focal_length)/nz.data;
	const int h=day?7:5;
	if(sx+h<0||sx-h>=320||sy+h<0||sy-h>=184) return;
	uint8_t* vb=(uint8_t*)gfx_vbuffer;
	const uint8_t outer=day?231:181, inner=255;
	for(int y=sy-h;y<=sy+h;y++)
	{
		if(y<0||y>=184) continue;
		const uint8_t sc=sky_row[y/5];
		uint8_t* row=vb+y*320;
		const bool coreY=(y>=sy-h+3&&y<=sy+h-3);
		for(int x=sx-h;x<=sx+h;x++)
		{
			if(x<0||x>=320||row[x]!=sc) continue;
			row[x]=(coreY&&x>=sx-h+3&&x<=sx+h-3)?inner:outer;
		}
	}
}

// ============================================================================
//  WORLD GENERATION  (untouched logic: same rand() order, same tunnel math => old saves regenerate identically)
// ============================================================================
void draw_tree(int x,int y,int z)
{
	for(int xo=-2;xo<=2;xo++) for(int zo=-2;zo<=2;zo++) vis_map(x+xo,y+2,z+zo)=LEAF;
	for(int xo=-1;xo<=1;xo++) for(int zo=-1;zo<=1;zo++) vis_map(x+xo,y+3,z+zo)=LEAF;
	vis_map(x,y,z)=WOOD; vis_map(x,y+1,z)=WOOD; vis_map(x,y+2,z)=WOOD;
}
void outline_world_with_barrier()
{
	for(int y=0;y<WY;y++) memset(world+(y<<YWS),BARRIER,WZ);
	for(int y=0;y<WY;y++) memset(world+((WX-1)<<XWS)+(y<<YWS),BARRIER,WZ);
	for(int x=0;x<WX;x++) for(int y=0;y<WY;y++) vis_map(x,y,0)=BARRIER;
	for(int x=0;x<WX;x++) memset(world+(x<<XWS),BARRIER,WZ);
}
#define hmap(x,z) hmap[((x)*(WZ+4))+(z)]
void generate_terrain(uint8_t surfblock,uint8_t underblock,uint8_t bottom,uint8_t top,uint8_t range)
{
	uint8_t kernel[5][5]={{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
	uint8_t* hmap=(uint8_t*)gfx_vbuffer;
	int tmr=top-range;
	for(uint8_t x=0;x<WX+4;x++) for(uint8_t z=0;z<WZ+4;z++) hmap(x,z)=tmr;
	for(uint8_t x=0;x<WX;x++) for(uint8_t z=0;z<WZ;z++)
	{
		int total=0;
		for(int xo=0;xo<5;xo++) for(int zo=0;zo<5;zo++) total+=kernel[xo][zo]*hmap(x+xo,z+zo);
		vis_map(x,(uint8_t)((total*241)>>16),z)=surfblock;
	}
	for(uint8_t x=0;x<WX;x++) for(uint8_t z=0;z<WZ;z++)
	{
		int yflag=0;
		for(int8_t y=top;y>=bottom;y--)
		{
			if(yflag==0){ if(vis_map(x,y,z)==surfblock) yflag=1; }
			else { vis_map(x,y,z)=surfblock; yflag++; }
			if(yflag>4) vis_map(x,y,z)=underblock;
		}
	}
}
void spawn_random_cluster(uint8_t block,uint8_t x,uint8_t y,uint8_t z,uint8_t minsize,uint8_t maxsize)
{
	uint8_t xlen=randInt(minsize,maxsize), ylen=randInt(minsize,maxsize), zlen=randInt(minsize,maxsize);
	for(int xo=0;xo<xlen;++xo) for(int yo=0;yo<ylen;++yo) for(int zo=0;zo<zlen;++zo)
	{
		if(x+xlen>WX) xlen=WX-x;
		if(y+ylen>WY) ylen=WY-y;
		if(z+zlen>WZ) zlen=WZ-z;
		vis_map(x+xo,y+yo,z+zo)=block;
	}
}
void generate_world_underground()
{
	for(int i=0;i<IRONNUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(4,12),z=randInt(0,WZ-1); spawn_random_cluster(IRON,x,y,z,1,3); }
	for(int i=0;i<GOLDNUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(2,10),z=randInt(0,WZ-1); spawn_random_cluster(GOLD,x,y,z,1,3); }
	for(int i=0;i<DIAMONDNUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(1,5),z=randInt(0,WZ-1); spawn_random_cluster(DIAMOND,x,y,z,1,2); }
	for(int i=0;i<REDSTONENUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(1,7),z=randInt(0,WZ-1); spawn_random_cluster(REDSTONE,x,y,z,1,3); }
	for(int i=0;i<COALNUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(6,17),z=randInt(0,WZ-1); spawn_random_cluster(COAL,x,y,z,2,3); }
	for(int i=0;i<OBSIDIANNUM;++i){ uint8_t x=randInt(0,WX-1),y=randInt(1,3),z=randInt(0,WZ-1); spawn_random_cluster(OBSIDIAN,x,y,z,1,2); }
	for(int i=0;i<GRAVELNUM;++i){ uint8_t x=randInt(0,WX-4),y=randInt(4,17),z=randInt(0,WZ-4); spawn_random_cluster(GRAVEL,x,y,z,2,4); }
	for(int i=0;i<TUNNELNUM;++i)
	{
		next:
		if(i>TUNNELNUM) break;
		flint tx=(int)randInt(0,WX-1);
		flint ty=(int)randInt(4,12);
		flint tz=(int)randInt(0,WZ-1);
		int ay=(int)randInt(0,63);
		int az=(int)randInt(-5,5);
		az=(64+az)&63;
		flint p5=fraw(256);
		int ix,iy,iz;
		int length=0;
		while(length<40)
		{
			length++;
			flint vx=COSR(ay)*COSR(az);
			flint vy=SINR(az);
			flint vz=SINR(ay)*COSR(az);
			tx+=vx; ty+=vy; tz+=vz;
			ix=(int)(tx+p5); iy=(int)(ty+p5); iz=(int)(tz+p5);
			for(int px=-1;px<2;++px) for(int py=-1;py<2;++py) for(int pz=-1;pz<2;++pz)
			{
				unsigned int ox=ix+px, oy=iy+py, oz=iz+pz;
				if(ox>WX-1||oy>WY-1||oz>WZ-1){ i++; goto next; }
				vis_map(ox,oy,oz)=0;
				if(!randInt(0,400)){ vis_map(ox,oy,oz)=WATERBLOCK; add_update(&vis_map(ox,oy,oz)); }
			}
			ay+=randInt(-TUNNELWOBBLEF,TUNNELWOBBLEF);
			az+=randInt(-TUNNELWOBBLEU,TUNNELWOBBLEU);
			ay&=63;
			az=az&63;
			if(az<0) az=0;
			if(az>5&&az<30) az=5;
			if(az>30&&az<59) az=59;
		}
	}
}
void generate_trees(int numtrees)
{
	for(uint8_t i=0;i<numtrees;i++)
	{
		uint8_t x=randInt(3,WX-3), z=randInt(3,WZ-3);
		int8_t y;
		for(y=WY-1;y>=0;y--) if(vis_map(x,y,z)&&vis_map(x,y,z)!=LEAF&&vis_map(x,y,z)!=WOOD) break;
		y++;
		if(y<WY-4) draw_tree(x,y,z);
	}
}
void generate_cacti(int n)
{
	for(int i=0;i<n;++i)
	{
		uint8_t x=randInt(1,WX-1), z=randInt(1,WZ-1);
		int8_t y;
		for(y=WY-1;y>=0;y--) if(vis_map(x,y,z)==SAND) break;
		vis_map(x,y+1,z)=CACTUS; vis_map(x,y+2,z)=CACTUS; vis_map(x,y+3,z)=CACTUS;
	}
}
static void generate_pumpkins(int n)
{
	for(int i=0;i<n;i++)
	{
		uint8_t x=randInt(2,WX-3), z=randInt(2,WZ-3);
		for(int y=WY-2;y>1;y--)
		{
			uint8_t b=vis_map(x,y,z);
			if(b){ if((b==GRASS||b==SNOWGRASS)&&vis_map(x,y+1,z)==0) vis_map(x,y+1,z)=PUMPKIN; break; }
		}
	}
}
void spawn_fire()
{
	for(int x=0;x<WX;++x) for(int z=0;z<WZ;++z) for(int y=28;y>0;--y)
		if(vis_map(x,y,z)){ if(randInt(0,64)==0) vis_map(x,y+1,z)=FIRE; break; }
}
static void generate_nether_extras()
{
	for(int i=0;i<14;i++)
	{
		uint8_t x=randInt(1,WX-2), z=randInt(1,WZ-2);
		for(int y=28;y>16;y--)
			if(vis_map(x,y,z)==0&&vis_map(x,y+1,z)==NETHERRACK)
			{
				vis_map(x,y,z)=GLOWSTONE;
				if(randInt(0,1)&&vis_map(x,y-1,z)==0) vis_map(x,y-1,z)=GLOWSTONE;
				break;
			}
	}
	for(int i=0;i<6;i++)
	{
		int cx=randInt(3,WX-4), cz=randInt(3,WZ-4);
		for(int dx=-2;dx<=2;dx++) for(int dz=-2;dz<=2;dz++)
		{
			if(iabs(dx)+iabs(dz)>3) continue;
			int x=cx+dx, z=cz+dz;
			for(int y=21;y>2;y--)
				if(vis_map(x,y,z)==NETHERRACK&&vis_map(x,y+1,z)==0){ vis_map(x,y,z)=SOULSAND; break; }
		}
	}
}
void generate_nether()
{
	for(int x=0;x<WX;++x) for(int z=0;z<WZ;++z)
	{
		vis_map(x,0,z)=MAGMABLOCK; vis_map(x,1,z)=MAGMABLOCK;
		vis_map(x,31,z)=NETHERRACK; vis_map(x,30,z)=NETHERRACK; vis_map(x,29,z)=NETHERRACK;
	}
	generate_terrain(NETHERRACK,NETHERRACK,6,21,10);
	spawn_fire();
	generate_terrain(NETHERRACK,NETHERRACK,20,25,2);
	spawn_fire();
	for(int j=0;j<2;++j)
	{
		for(int i=0;i<NETHERGOLDORENUM;++i)
		{
			uint8_t x=randInt(1,WX-2);
			uint8_t y=(j==0)?randInt(6,21):randInt(20,25);
			uint8_t z=randInt(1,WZ-2);
			if(vis_map(x,y,z))
			{
				vis_map(x,y,z)=NETHERGOLDORE;
				if(vis_map(x+1,y,z)) vis_map(x+1,y,z)=NETHERGOLDORE;
				if(vis_map(x-1,y,z)) vis_map(x-1,y,z)=NETHERGOLDORE;
				if(vis_map(x,y+1,z)) vis_map(x,y+1,z)=NETHERGOLDORE;
				if(vis_map(x,y-1,z)) vis_map(x,y-1,z)=NETHERGOLDORE;
				if(vis_map(x,y,z+1)) vis_map(x,y,z+1)=NETHERGOLDORE;
				if(vis_map(x,y,z-1)) vis_map(x,y,z-1)=NETHERGOLDORE;
			}
		}
		outline_world_with_barrier();
	}
	generate_nether_extras();
}
void generate_world()
{
	memset(world,0,WORLD_BYTES);
	if(realm==1){ generate_nether(); return; }
	if(world_type==1)
	{
		for(int x=0;x<WX;++x) for(int z=0;z<WZ;++z)
		{
			vis_map(x,20,z)=GRASS; vis_map(x,19,z)=GRASS; vis_map(x,18,z)=GRASS;
			for(int y=1;y<18;++y) vis_map(x,y,z)=STONE;
		}
		outline_world_with_barrier();
		return;
	}
	biome=randInt(0,4);
	switch(biome)
	{
		case 0: generate_terrain(GRASS,STONE,0,26,6); generate_trees(3); break;
		case 1: generate_terrain(SNOWGRASS,STONE,0,26,6); generate_trees(3); break;
		case 2: generate_terrain(SAND,SANDSTONE,0,26,6); generate_cacti(15); break;
		case 3: generate_terrain(GRASS,STONE,0,26,6); generate_trees(30); break;
		case 4: generate_terrain(SNOWGRASS,STONE,0,26,6); generate_trees(30); break;
	}
	if(biome==1||biome==4)
	{
		for(int i=0;i<15;++i)
		{
			uint8_t xs=randInt(3,WX-4), zs=randInt(3,WZ-4);
			uint8_t t1=xs+randInt(1,3);
			for(uint8_t x=xs-randInt(1,3);x<t1;++x)
			{
				uint8_t t2=zs+randInt(1,3);
				for(uint8_t z=zs-randInt(1,3);z<t2;++z)
					for(int8_t y=WY-1;y>=0;y--)
						if(vis_map(x,y,z)==SNOWGRASS){ vis_map(x,y,z)=SNOW; break; }
			}
		}
	}
	generate_world_underground();
	outline_world_with_barrier();
	if(biome!=2&&randInt(0,1)) generate_pumpkins(randInt(1,3));
}

#include "main_part2.h"