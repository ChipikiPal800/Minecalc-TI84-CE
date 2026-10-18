// ============================================================================
//  MINECALC 0.4.0   FILE 2/3: src/main_part2.h  (save/load, settings, recipes, screens)
//  Included by main.cpp. Ends with: #include "main_part3.h"
// ============================================================================

// ============================================================================
//  SAVE / LOAD
//  chunk file MC{O|N}nnn[xz]: u16 seed, u16 0xFEED, u8 ver, u8 nblocks, u8 itembase,
//    u16 sizes[nblocks], per-id lists of u16 indices (65535 ends each)  -- unchanged since 0.2.0
//  player file MCPnnn: v4 = 9 hotbar bytes; v3 (8 bytes) still loads
// ============================================================================
char number2char(uint8_t num)
{
	if(num<10) return num+'0';
	if(num<36) return num+55;
	if(num<62) return num+61;
	return 0;
}
bool appvar_exists(const char* name)
{
	ti_var_t s=ti_Open(name,"r");
	if(s){ ti_Close(s); return true; }
	return false;
}
static bool set_archive_fn(const char* name,bool archived)
{
	ti_var_t s=ti_Open(name,"r");
	if(!s) return false;
	bool ok=ti_SetArchiveStatus(archived,s);
	ti_Close(s);
	return ok;
}
static void make_chunk_name(char* out,uint8_t cx,uint8_t cz,uint8_t rlm)
{
	memcpy(out,origwname,6);
	out[2]=rlm?'N':'O';
	if(cx==32&&cz==32){ out[6]=0; out[7]=0; out[8]=0; }
	else { out[6]=number2char(cx); out[7]=number2char(cz); out[8]=0; }
}
static void player_name(char* out){ memcpy(out,origwname,6); out[2]='P'; out[6]=0; }

#define MAX_VISITED 256
static uint16_t visited[MAX_VISITED];
static int nvisited=0;
static bool visited_overflow=false;
static int unarchived_saves=0;
static void add_visited_raw(uint16_t e)
{
	for(int i=0;i<nvisited;i++) if(visited[i]==e) return;
	if(nvisited<MAX_VISITED) visited[nvisited++]=e; else visited_overflow=true;
}
static void add_visited(uint8_t cx,uint8_t cz,uint8_t r){ add_visited_raw((uint16_t)((r<<12)|(cx<<6)|cz)); }
static void visited_name(char* out,uint16_t e){ make_chunk_name(out,(uint8_t)((e>>6)&63),(uint8_t)(e&63),(uint8_t)(e>>12)); }
void srandchunkseed(unsigned short seed)
{
	unsigned int loadseed=seed&0xFFF;
	loadseed|=chunkx<<12;
	loadseed|=chunkz<<18;
	srand(loadseed);
}
static void archive_name(const char* n){ if(appvar_exists(n)) set_archive_fn(n,true); }
static void archive_visited(const char* except)
{
	char n[9];
	for(int i=0;i<nvisited;i++)
	{
		visited_name(n,visited[i]);
		if(except&&!strcmp(n,except)) continue;
		archive_name(n);
	}
	unarchived_saves=0;
}
static void archive_world_files()
{
	archive_visited(nullptr);
	char p[9];
	player_name(p);
	archive_name(p);
}
static uint8_t* realworld_ptr=nullptr;
// copies the live world into VRAM, regenerates the seed world into world[], builds per-id difference lists in VRAM
static uint8_t* build_change_lists(int* sumout)
{
	if((uint8_t*)gfx_vbuffer!=VRAM_BUF_A) gfx_SwapDraw();
	uint8_t* realworld=VRAM_BUF_A+1300;
	realworld_ptr=realworld;
	memcpy(realworld,world,WORLD_BYTES);
	int savedtop=bstacktop;
	uint8_t savedbiome=biome;
	srandchunkseed(wseed);
	generate_world();
	bstacktop=savedtop;
	biome=savedbiome;
	memset(world_changes_sizes,0,sizeof(world_changes_sizes));
	for(int i=0;i<WORLD_BYTES;i++){ uint8_t v=save_map[realworld[i]]; if(v!=world[i]) world_changes_sizes[v]++; }
	int sum=0;
	uint16_t* next=(uint16_t*)(VRAM_BUF_A+1310+WORLD_BYTES);
	for(int i=0;i<MOBSTART;i++)
	{
		world_changes_sizes[i]++;
		world_changes[i]=next;
		next+=world_changes_sizes[i];
		world_changes[i][world_changes_sizes[i]-1]=65535;
		sum+=world_changes_sizes[i];
	}
	for(int i=0;i<WORLD_BYTES;i++){ uint8_t v=save_map[realworld[i]]; if(v!=world[i]) *world_changes[v]++=(uint16_t)i; }
	for(int i=0;i<MOBSTART;i++) world_changes[i]-=world_changes_sizes[i]-1;
	*sumout=sum;
	return (uint8_t*)next;
}
// restore=true keeps the live world in memory afterwards (autosave)
static bool save_chunk(bool restore)
{
	if(!restore) update_cleanup();
	char name[9];
	make_chunk_name(name,chunkx,chunkz,realm);
	bool ok=false;
	for(int attempt=0;attempt<2&&!ok;++attempt)
	{
		size_t oldsize=0;
		bool existed=false;
		{
			ti_var_t f=ti_Open(name,"r");
			if(f){ existed=true; oldsize=ti_GetSize(f); ti_Close(f); }
		}
		if(attempt>0||unarchived_saves>=4) archive_visited(name);
		if(existed&&!set_archive_fn(name,false)) continue;
		int sum=0;
		uint8_t* end=build_change_lists(&sum);
		uint8_t* realworld=realworld_ptr;
		size_t total=7+2*MOBSTART+2*(size_t)sum;
		bool havebackup=false;
		if(existed&&oldsize&&end+oldsize<=VRAM_END)
		{
			ti_var_t f=ti_Open(name,"r");
			if(f){ havebackup=(ti_Read(end,1,oldsize,f)==oldsize); ti_Close(f); }
		}
		ti_var_t f=ti_Open(name,"w");
		if(f)
		{
			if(total<65000&&ti_Resize(total,f)>0)
			{
				ti_Rewind(f);
				uint16_t s16=(uint16_t)wseed, mg=SAVE_MAGIC;
				uint8_t hdr[3]={SAVE_VERSION,MOBSTART,ITEMBASE};
				ok=ti_Write(&s16,2,1,f)==1&&ti_Write(&mg,2,1,f)==1&&ti_Write(hdr,1,3,f)==3&&ti_Write(world_changes_sizes,2,MOBSTART,f)==MOBSTART;
				for(int i=0;i<MOBSTART&&ok;i++) ok=(ti_Write(world_changes[i],2,world_changes_sizes[i],f)==world_changes_sizes[i]);
			}
			ti_Close(f);
		}
		if(!ok&&havebackup){ f=ti_Open(name,"w"); if(f){ ti_Write(end,1,oldsize,f); ti_Close(f); } }
		if(!ok||restore) memcpy(world,realworld,WORLD_BYTES);
	}
	if(ok){ add_visited(chunkx,chunkz,realm); unarchived_saves++; }
	else save_failed=true;
	return ok;
}
static bool save_player_once()
{
	char n[9];
	player_name(n);
	if(appvar_exists(n)) set_archive_fn(n,false);
	ti_var_t f=ti_Open(n,"w");
	if(!f) return false;
	stats.play_ticks+=(uint32_t)(clock()-session_mark);
	session_mark=clock();
	bool ok=true;
	uint8_t ver=PLAYER_VERSION, ib=ITEMBASE;
	uint16_t ninv=INV_SIZE;
	ok=ok&&ti_Write(&ver,1,1,f)==1;
	ok=ok&&ti_Write(&ib,1,1,f)==1;
	ok=ok&&ti_Write(&ninv,2,1,f)==1;
	ok=ok&&ti_Write(inventory,sizeof(int),INV_SIZE,f)==INV_SIZE;
	ok=ok&&ti_Write(ghotbar,1,HOTBAR_N,f)==HOTBAR_N;
	int p[3]={plx.data,ply.data,plz.data};
	ok=ok&&ti_Write(p,sizeof(int),3,f)==3;
	uint32_t t=world_ticks()%(32768UL*60*20);
	ok=ok&&ti_Write(&t,4,1,f)==1;
	int r[3]={respawnx,respawny,respawnz};
	ok=ok&&ti_Write(r,sizeof(int),3,f)==3;
	uint8_t b[10]={chunkx,chunkz,respawnchunkx,respawnchunkz,realm,(uint8_t)playerhp,(uint8_t)playerhunger,world_type,gamemode,difficulty};
	ok=ok&&ti_Write(b,1,10,f)==10;
	uint16_t sd=(uint16_t)wseed;
	ok=ok&&ti_Write(&sd,2,1,f)==1;
	uint8_t ovf=visited_overflow;
	ok=ok&&ti_Write(&ovf,1,1,f)==1;
	uint16_t nv=(uint16_t)nvisited;
	ok=ok&&ti_Write(&nv,2,1,f)==1;
	if(nv) ok=ok&&ti_Write(visited,2,nv,f)==nv;
	uint8_t hs=1;
	ok=ok&&ti_Write(&hs,1,1,f)==1;
	ok=ok&&ti_Write(&stats,sizeof(Stats),1,f)==1;
	ti_Close(f);
	return ok;
}
static bool save_player()
{
	if(save_player_once()) return true;
	archive_visited(nullptr);
	if(save_player_once()) return true;
	save_failed=true;
	return false;
}
static bool load_chunk_file(const char* name)
{
	ti_var_t f=ti_Open(name,"r");
	if(!f) return false;
	uint16_t seed=0;
	ti_Read(&seed,2,1,f);
	srandchunkseed(wseed);
	generate_world();
	uint16_t first=0;
	if(ti_Read(&first,2,1,f)!=1){ ti_Close(f); return true; }
	int nb;
	if(first==SAVE_MAGIC)
	{
		uint8_t hdr[3];
		if(ti_Read(hdr,1,3,f)!=3){ ti_Close(f); return true; }
		nb=hdr[1];
		if(nb>MOBSTART||ti_Read(world_changes_sizes,2,nb,f)!=(size_t)nb){ ti_Close(f); return true; }
	}
	else
	{
		nb=LEGACY_MOBSTART;
		world_changes_sizes[0]=first;
		if(ti_Read(world_changes_sizes+1,2,nb-1,f)!=(size_t)(nb-1)){ ti_Close(f); return true; }
	}
	uint16_t buf[64];
	for(int i=0;i<nb;i++)
	{
		int left=world_changes_sizes[i];
		while(left>0)
		{
			int n=left>64?64:left;
			size_t got=ti_Read(buf,2,n,f);
			for(size_t j=0;j<got;j++){ uint16_t c=buf[j]; if(c<WORLD_BYTES&&i<MOBSTART) world[c]=(uint8_t)i; }
			if((int)got<n){ ti_Close(f); return true; }
			left-=n;
		}
	}
	ti_Close(f);
	return true;
}
static int remap_id(int i,int oldbase){ return i<oldbase?i:i-oldbase+ITEMBASE; }
static void sanitize_player()
{
	if(playerhp<=0||playerhp>20) playerhp=20;
	if(playerhunger<0||playerhunger>20) playerhunger=20;
	if(chunkx>61) chunkx=32;
	if(chunkz>61) chunkz=32;
	if(realm>1) realm=0;
	if(world_type>1) world_type=0;
	if(gamemode>1) gamemode=0;
	if(difficulty>3) difficulty=1;
	if(plx<1||plx>30) plx=16;
	if(plz<1||plz>31) plz=16;
	if(ply<2||ply>WY-1) ply=WY-1;
	if(hotbarblock<0||hotbarblock>=HOTBAR_N) hotbarblock=0;
}
static bool load_player()
{
	char n[9];
	player_name(n);
	ti_var_t f=ti_Open(n,"r");
	if(!f) return false;
	uint8_t ver=0, ib=ITEMBASE;
	uint16_t ninv=0;
	ti_Read(&ver,1,1,f);
	if(ver!=3&&ver!=PLAYER_VERSION){ ti_Close(f); return false; }//v3 (0.2/0.3 worlds, 8 hotbar slots) still loads
	ti_Read(&ib,1,1,f);
	ti_Read(&ninv,2,1,f);
	memset(inventory,0,sizeof(inventory));
	for(int i=0;i<ninv;i++)
	{
		int v=0;
		ti_Read(&v,sizeof(int),1,f);
		int ni=remap_id(i,ib);
		if(ni>=0&&ni<INV_SIZE) inventory[ni]=v;
	}
	uint8_t hb[HOTBAR_N]={};
	ti_Read(hb,1,ver>=4?HOTBAR_N:8,f);
	for(int i=0;i<HOTBAR_N;i++){ int v=hb[i]?remap_id(hb[i],ib):0; ghotbar[i]=(v>0&&v<INV_SIZE)?(uint8_t)v:0; }
	int p[3]={};
	ti_Read(p,sizeof(int),3,f);
	plx.data=p[0]; ply.data=p[1]; plz.data=p[2];
	ti_Read(&offsettime,4,1,f);
	int r[3]={16,WY-1,16};
	ti_Read(r,sizeof(int),3,f);
	respawnx=r[0]; respawny=r[1]; respawnz=r[2];
	uint8_t b[10]={32,32,32,32,0,20,20,0,0,1};
	ti_Read(b,1,10,f);
	chunkx=b[0]; chunkz=b[1]; respawnchunkx=b[2]; respawnchunkz=b[3]; realm=b[4];
	playerhp=(int8_t)b[5]; playerhunger=(int8_t)b[6]; world_type=b[7]; gamemode=b[8]; difficulty=b[9];
	uint16_t sd=0;
	ti_Read(&sd,2,1,f);
	wseed=sd;
	uint8_t ovf=0;
	ti_Read(&ovf,1,1,f);
	visited_overflow=ovf;
	uint16_t nv=0;
	ti_Read(&nv,2,1,f);
	nvisited=0;
	for(int i=0;i<nv;i++){ uint16_t e=0; if(ti_Read(&e,2,1,f)!=1) break; add_visited_raw(e); }
	uint8_t hs=0;
	memset(&stats,0,sizeof(stats));
	if(ti_Read(&hs,1,1,f)==1&&hs==1) ti_Read(&stats,sizeof(Stats),1,f);
	ti_Close(f);
	sanitize_player();
	return true;
}
static bool load_legacy_player()
{
	char n[9];
	make_chunk_name(n,32,32,0);
	ti_var_t f=ti_Open(n,"r");
	if(!f) return false;
	uint16_t seed=0;
	ti_Read(&seed,2,1,f);
	wseed=seed;
	uint16_t first=0;
	if(ti_Read(&first,2,1,f)!=1||first==SAVE_MAGIC){ ti_Close(f); return true; }
	world_changes_sizes[0]=first;
	if(ti_Read(world_changes_sizes+1,2,LEGACY_MOBSTART-1,f)!=LEGACY_MOBSTART-1){ ti_Close(f); return true; }
	int skip=0;
	for(int i=0;i<LEGACY_MOBSTART;i++) skip+=world_changes_sizes[i];
	ti_Seek(skip*2,SEEK_CUR,f);
	int num=0;
	if(ti_Read(&num,sizeof(int),1,f)!=1){ ti_Close(f); return true; }
	memset(inventory,0,sizeof(inventory));
	for(int i=0;i<num;i++)
	{
		int v=0;
		ti_Read(&v,sizeof(int),1,f);
		int ni=remap_id(i,LEGACY_ITEMBASE);
		if(ni>=0&&ni<INV_SIZE) inventory[ni]=v;
	}
	uint8_t hb[8]={};
	ti_Read(hb,1,8,f);
	for(int i=0;i<8;i++){ int v=hb[i]?remap_id(hb[i],LEGACY_ITEMBASE):0; ghotbar[i]=(v>0&&v<INV_SIZE)?(uint8_t)v:0; }
	int p[3]={};
	ti_Read(p,sizeof(int),3,f);
	plx.data=p[0]; ply.data=p[1]; plz.data=p[2];
	ti_Read(&offsettime,4,1,f);
	ti_Read(&respawnx,sizeof(int),1,f);
	ti_Read(&respawny,sizeof(int),1,f);
	ti_Read(&respawnz,sizeof(int),1,f);
	uint8_t b[7]={32,32,32,32,0,20,20};
	ti_Read(b,1,7,f);
	chunkx=b[0]; chunkz=b[1]; respawnchunkx=b[2]; respawnchunkz=b[3]; realm=b[4];
	playerhp=(int8_t)b[5]; playerhunger=(int8_t)b[6];
	uint8_t o[3];
	if(ti_Read(o,1,3,f)==3){ world_type=o[0]; gamemode=o[1]; difficulty=o[2]; }
	else { world_type=0; gamemode=0; difficulty=1; }
	ti_Close(f);
	sanitize_player();
	return true;
}
static void scan_all_chunks(bool del)
{
	char n[9];
	for(uint8_t r=0;r<2;r++) for(uint8_t x=0;x<62;x++) for(uint8_t z=0;z<62;z++)
	{
		make_chunk_name(n,x,z,r);
		if(appvar_exists(n)){ if(del) ti_Delete(n); else add_visited(x,z,r); }
	}
}
static void delete_world_files(const char* base6)
{
	char saved[9];
	memcpy(saved,origwname,9);
	memcpy(origwname,base6,6);
	origwname[6]=0;
	char n[9];
	if(load_player()&&!visited_overflow){ for(int i=0;i<nvisited;i++){ visited_name(n,visited[i]); ti_Delete(n); } }
	else scan_all_chunks(true);
	make_chunk_name(n,32,32,0); ti_Delete(n);
	make_chunk_name(n,32,32,1); ti_Delete(n);
	player_name(n); ti_Delete(n);
	memcpy(origwname,saved,9);
}

// ============================================================================
//  CHUNK SWITCHING / PLAYER PLACEMENT
// ============================================================================
static void spawn_initial_mobs()
{
	if(realm==1){ for(int i=0;i<8;i++) spawn_piglin(); }
	else if(world_type==1||biome==0||biome==3){ for(int i=0;i<3;i++){ spawn_sheep(); spawn_pig(); } }
}
static void loadnewchunk()
{
	bstacktop=0;
	ch_valid=false;
	reset_mobs();
	updatehotbar=2;
	make_chunk_name(wname,chunkx,chunkz,realm);
	if(!(appvar_exists(wname)&&load_chunk_file(wname))){ srandchunkseed(wseed); generate_world(); }
	if(realm==1) biome=255;
	srand(rtc_Time());
	spawn_initial_mobs();
	requeue_block_updates();
	compute_world_top();
}
static void settle_player(flint keepy)
{
	ply=keepy;
	if(ply>WY-1) ply=WY-1;
	if(ply<2) ply=2;
	int x=(int)plx, z=(int)plz, guard=0;
	while(guard++<WY&&(int)ply<WY-1&&(!walkable(vis_map(x,(int)ply,z))||!walkable(vis_map(x,(int)ply-1,z)))) ply+=1;
	grav=0;
	air_peak=ply;
}
static void find_spawn()
{
	int x=16, z=16, y;
	for(y=WY-2;y>1;y--){ uint8_t b=vis_map(x,y,z); if(b&&!walkable(b)&&b!=LEAF&&b!=WOOD) break; }
	if(y+1<WY) vis_map(x,y+1,z)=0;
	if(y+2<WY) vis_map(x,y+2,z)=0;
	plx=x; plz=z;
	ply=imin(y+2,WY-1);
	respawnx=x; respawny=(int)ply; respawnz=z;
	respawnchunkx=32; respawnchunkz=32;
	grav=0; air_peak=ply;
	compute_world_top();
}
static void reset_game_state()
{
	memset(inventory,0,sizeof(inventory));
	memset(ghotbar,0,sizeof(ghotbar));
	memset(&stats,0,sizeof(stats));
	hotbarblock=0;
	playerhp=20; playerhunger=20; playerair=MAXAIR;
	plx=16; ply=WY-1; plz=16; rotxz=0; rotyz=0;
	compute_dir();
	grav=0; air_peak=ply; flying=false;
	chunkx=32; chunkz=32; respawnchunkx=32; respawnchunkz=32;
	respawnx=16; respawny=WY-1; respawnz=16;
	realm=0; biome=0; offsettime=3932160UL; wseed=0;
	world_type=0; gamemode=0; difficulty=1;
	nvisited=0; visited_overflow=false; unarchived_saves=0;
	bstacktop=0; ch_valid=false;
	reset_mobs();
	realmcooldown=0; portal_must_exit=false; open_crafting_table=false;
	toast_timer=0; name_timer=0; hurt_timer=0; save_failed=false;
	change_bg(bg_colors[4]);
}

// ============================================================================
//  SETTINGS  (MNCFSET is append-only: new fields have defaults when missing)
// ============================================================================
static void apply_fov(){ focal_length=(set_fov==0)?250:((set_fov==2)?170:200); }
void save_settings(bool arch)
{
	set_archive_fn("MNCFSET",false);
	ti_var_t s=ti_Open("MNCFSET","w");
	if(!s) return;
	ti_Write(&set_fog,1,1,s);
	ti_Write(&set_debug,1,1,s);
	ti_Write(&set_mobspawn,1,1,s);
	ti_Write(&key_fwd,2,1,s);
	ti_Write(&key_back,2,1,s);
	ti_Write(&key_jump,2,1,s);
	ti_Write(&key_mine,2,1,s);
	ti_Write(&key_place,2,1,s);
	ti_Write(&key_lookfast,2,1,s);
	ti_Write(&key_help,2,1,s);
	ti_Write(&set_names,1,1,s);
	ti_Write(&set_gfx,1,1,s);
	ti_Write(&set_tex,1,1,s);
	uint8_t nv[7]={set_dist,set_fov,set_cross,set_outline,set_held,set_autojump,set_look};
	ti_Write(nv,1,7,s);
	ti_Close(s);
	if(arch) set_archive_fn("MNCFSET",true);
}
void load_settings()
{
	ti_var_t s=ti_Open("MNCFSET","r");
	if(!s){ apply_fov(); return; }
	ti_Read(&set_fog,1,1,s);
	ti_Read(&set_debug,1,1,s);
	ti_Read(&set_mobspawn,1,1,s);
	ti_Read(&key_fwd,2,1,s);
	ti_Read(&key_back,2,1,s);
	ti_Read(&key_jump,2,1,s);
	ti_Read(&key_mine,2,1,s);
	ti_Read(&key_place,2,1,s);
	ti_Read(&key_lookfast,2,1,s);
	ti_Read(&key_help,2,1,s);
	if(ti_Read(&set_names,1,1,s)!=1) set_names=1;
	if(ti_Read(&set_gfx,1,1,s)!=1) set_gfx=0;
	if(ti_Read(&set_tex,1,1,s)!=1) set_tex=1;
	uint8_t nv[7]={1,1,0,1,1,1,1};
	if(ti_Read(nv,1,7,s)==7){ set_dist=nv[0]; set_fov=nv[1]; set_cross=nv[2]; set_outline=nv[3]; set_held=nv[4]; set_autojump=nv[5]; set_look=nv[6]; }
	ti_Close(s);
	if(set_mobspawn>2) set_mobspawn=1;
	if(set_gfx>3) set_gfx=0;
	if(set_tex>1) set_tex=1;
	if(set_dist>1) set_dist=1;
	if(set_fov>2) set_fov=1;
	if(set_cross>2) set_cross=0;
	if(set_look>2) set_look=1;
	set_outline&=1; set_held&=1; set_autojump&=1;
	haze_enabled=set_fog;
	apply_fov();
}
const char* keyname(kb_lkey_t k)
{
	switch(k)
	{
		case kb_Key2nd: return "2nd";       case kb_KeyAlpha: return "Alpha";   case kb_KeyMode: return "Mode";
		case kb_KeyDel: return "Del";       case kb_KeyStat: return "Stat";     case kb_KeyMath: return "Math";
		case kb_KeyApps: return "Apps";     case kb_KeyGraphVar: return "X,T,O,n"; case kb_KeyEnter: return "Enter";
		case kb_KeyAdd: return "+";         case kb_KeySub: return "-";         case kb_KeyMul: return "*";
		case kb_KeyDiv: return "/";         case kb_KeyUp: return "Up";         case kb_KeyDown: return "Down";
		case kb_KeyLeft: return "Left";     case kb_KeyRight: return "Right";   case kb_KeyGraph: return "Graph";
		case kb_KeyTrace: return "Trace";   case kb_KeyZoom: return "Zoom";     case kb_KeyWindow: return "Window";
		case kb_KeyYequ: return "Y=";       case kb_KeyPrgm: return "Prgm";     case kb_KeyVars: return "Vars";
		case kb_KeyClear: return "Clear";   case kb_KeySto: return "Sto";       case kb_KeyLn: return "Ln";
		case kb_KeyLog: return "Log";
	}
	return "Key";
}
kb_lkey_t capture_key()
{
	wait_key_release();
	while(true)
	{
		kb_Scan();
		for(int g=1;g<8;++g) for(int b=0;b<8;++b)
			if(kb_Data[g]&(1<<b)) return (kb_lkey_t)((g<<8)|(1<<b));
	}
}
static void draw_key_prompt(const char* name)
{
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	shadow_text_centered("Press a key for",96,COL_WHITE);
	shadow_text_centered(name,112,COL_YELLOW);
	shadow_text_centered("(Clear to cancel)",140,COL_GRAY_L);
	gfx_SwapDraw();
}
void keybinds_menu()
{
	int sel=0;
	while(true)
	{
		kb_lkey_t binds[7]={key_fwd,key_back,key_jump,key_mine,key_place,key_lookfast,key_help};
		const char* names[7]={"Forward","Backward","Jump","Mine/Attack","Place/Use","Look Speed","Help"};
		char lines[7][28];
		const char* opts[8];
		for(int i=0;i<7;++i){ snprintf(lines[i],28,"%s: %s",names[i],keyname(binds[i])); opts[i]=lines[i]; }
		opts[7]="Done";
		int c=mc_menu_t("Keybinds",opts,8,sel);
		if(c<0||c==7) return;
		sel=c;
		draw_key_prompt(names[c]);
		kb_lkey_t nk=capture_key();
		if(nk!=kb_KeyClear)
		{
			switch(c)
			{
				case 0: key_fwd=nk; break;  case 1: key_back=nk; break;  case 2: key_jump=nk; break;
				case 3: key_mine=nk; break; case 4: key_place=nk; break; case 5: key_lookfast=nk; break;
				case 6: key_help=nk; break;
			}
			save_settings(false);
		}
		wait_key_release();
	}
}
static void controls_screen()
{
	char l[12][40];
	snprintf(l[0],40,"Move: %s / %s",keyname(key_fwd),keyname(key_back));
	snprintf(l[1],40,"Jump / swim / climb: %s",keyname(key_jump));
	snprintf(l[2],40,"Mine / attack: %s",keyname(key_mine));
	snprintf(l[3],40,"Place / use / eat: %s",keyname(key_place));
	snprintf(l[4],40,"Look: arrows   Fast look: %s",keyname(key_lookfast));
	snprintf(l[5],40,"Hotbar: + / -  or keys 1-9");
	snprintf(l[6],40,"Inventory + crafting: X,T,O,n");
	snprintf(l[7],40,"Map: Graph    Help: %s",keyname(key_help));
	snprintf(l[8],40,"Creative item picker: Enter");
	snprintf(l[9],40,"Creative fly: Y=  (descend: Window)");
	snprintf(l[10],40,"Inventory: Mode=recipes  x=craft all");
	snprintf(l[11],40,"Game menu / save: Clear");
	wait_key_release();
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	gfx_SetTextScale(2,2);
	shadow_text_centered("Controls",8,COL_GRAY_L);
	gfx_SetTextScale(1,1);
	for(int i=0;i<12;i++) shadow_text(l[i],16,34+i*15,COL_WHITE);
	shadow_text_centered("Press any key",222,COL_YELLOW);
	gfx_SwapDraw();
	wait_any_key();
}
static const char* const o_gfx[4]={"Fancy","Fast","Fastest","Ultra"};
static const char* const o_dist[2]={"Sharp","Smooth"};
static const char* const o_fov[3]={"Narrow","Normal","Wide"};
static const char* const o_cross[3]={"Cross","Dot","Off"};
static const char* const o_look[3]={"Slow","Normal","Fast"};
static const char* const o_mob[3]={"Off","Normal","Frequent"};
static const char* onoff(uint8_t v){ return v?"ON":"OFF"; }
#define OPT(i,n,v) do{ snprintf(b[i],28,"%s: %s",n,v); o[i]=b[i]; }while(0)
static void video_menu()
{
	static const char* d[10]={
		"Fancy: smart reuse. Ultra: all rays",
		"Textured blocks, or flat colors",
		"Mip detail at medium distance",
		"Haze toward the sky color",
		"Field of view",
		"Crosshair style",
		"Black outline on the target block",
		"Show the item in your hand",
		"FPS, position and timings",
		0 };
	int sel=0;
	while(true)
	{
		char b[9][28]; const char* o[10];
		OPT(0,"Graphics",o_gfx[set_gfx&3]);
		OPT(1,"Block Textures",onoff(set_tex));
		OPT(2,"Distant Textures",o_dist[set_dist&1]);
		OPT(3,"Fog",onoff(set_fog));
		OPT(4,"FOV",o_fov[set_fov%3]);
		OPT(5,"Crosshair",o_cross[set_cross%3]);
		OPT(6,"Block Outline",onoff(set_outline));
		OPT(7,"Held Item",onoff(set_held));
		OPT(8,"Debug Info",onoff(set_debug));
		o[9]="Back";
		int c=mc_menu_t("Video",o,10,sel,d);
		if(c<0||c==9) return;
		sel=c;
		switch(c)
		{
			case 0: set_gfx=(set_gfx+1)&3; break;
			case 1: set_tex=!set_tex; apply_tex_mode(); break;
			case 2: set_dist=!set_dist; break;
			case 3: set_fog=!set_fog; haze_enabled=set_fog; refresh_fog(); break;
			case 4: set_fov=(set_fov+1)%3; apply_fov(); break;
			case 5: set_cross=(set_cross+1)%3; break;
			case 6: set_outline=!set_outline; break;
			case 7: set_held=!set_held; break;
			case 8: set_debug=!set_debug; break;
		}
		save_settings(false);
	}
}
static void gameplay_menu()
{
	static const char* d[5]={"Hostile mob frequency at night","Hop up one-block steps by walking","Arrow-key turn speed","Name popup when switching items",0};
	int sel=0;
	while(true)
	{
		char b[4][28]; const char* o[5];
		OPT(0,"Mob Spawning",o_mob[set_mobspawn%3]);
		OPT(1,"Auto-jump",onoff(set_autojump));
		OPT(2,"Look Speed",o_look[set_look%3]);
		OPT(3,"Item Names",onoff(set_names));
		o[4]="Back";
		int c=mc_menu_t("Gameplay",o,5,sel,d);
		if(c<0||c==4) return;
		sel=c;
		switch(c)
		{
			case 0: set_mobspawn=(set_mobspawn+1)%3; break;
			case 1: set_autojump=!set_autojump; break;
			case 2: set_look=(set_look+1)%3; break;
			case 3: set_names=!set_names; break;
		}
		save_settings(false);
	}
}
void settings_menu()
{
	static const char* o[5]={"Video Settings...","Gameplay Settings...","Keybinds...","View Controls","Done"};
	int sel=0;
	while(true)
	{
		int c=mc_menu_t("Options",o,5,sel);
		if(c<0||c==4){ save_settings(true); return; }
		sel=c;
		if(c==0) video_menu();
		else if(c==1) gameplay_menu();
		else if(c==2) keybinds_menu();
		else controls_screen();
	}
}
static void stats_screen()
{
	uint32_t ticks=stats.play_ticks+(uint32_t)(clock()-session_mark);
	uint32_t mins=(ticks>>15)/60;
	const char* names[5]={"Blocks mined","Blocks placed","Mobs killed","Deaths","Minutes played"};
	unsigned int vals[5]={(unsigned int)stats.mined,(unsigned int)stats.placed,(unsigned int)stats.killed,(unsigned int)stats.deaths,(unsigned int)mins};
	wait_key_release();
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	gfx_SetTextScale(2,2);
	shadow_text_centered("Statistics",8,COL_GRAY_L);
	gfx_SetTextScale(1,1);
	for(int i=0;i<5;i++)
	{
		shadow_text(names[i],60,56+i*22,COL_WHITE);
		gfx_SetTextFGColor(COL_BLACK); gfx_SetTextXY(221,57+i*22); gfx_PrintUInt(vals[i],1);
		gfx_SetTextFGColor(COL_YELLOW); gfx_SetTextXY(220,56+i*22); gfx_PrintUInt(vals[i],1);
	}
	shadow_text_centered("Press any key",200,COL_YELLOW);
	gfx_SwapDraw();
	wait_any_key();
}

// ============================================================================
//  ON-SCREEN TEXT ENTRY
// ============================================================================
static char csc_digit(uint8_t k)
{
	switch(k)
	{
		case sk_0: return '0'; case sk_1: return '1'; case sk_2: return '2'; case sk_3: return '3';
		case sk_4: return '4'; case sk_5: return '5'; case sk_6: return '6'; case sk_7: return '7';
		case sk_8: return '8'; case sk_9: return '9';
	}
	return 0;
}
static bool text_input(const char* prompt,char* buf,int maxlen,bool digits,bool allow_empty)
{
	static const char letters[]="\0\0\0\0\0\0\0\0\0\0\"WRMH\0\0?[VQLG\0\0:ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
	int len=strlen(buf);
	bool alpha=!digits;
	bool dirty=true;
	wait_key_release();
	while(os_GetCSC());
	while(true)
	{
		if(dirty)
		{
			dirty=false;
			gfx_SetDrawBuffer();
			draw_dirt_bg();
			menu_text_setup();
			shadow_text_centered(prompt,70,COL_WHITE);
			gfx_SetColor(COL_BLACK);
			gfx_FillRectangle_NoClip(38,96,244,24);
			gfx_SetColor(COL_GRAY_L);
			gfx_Rectangle_NoClip(38,96,244,24);
			gfx_SetTextFGColor(COL_WHITE);
			gfx_SetTextXY(44,104);
			gfx_PrintString(buf);
			gfx_PrintString("_");
			shadow_text_centered(digits?"Numbers only":(alpha?"[alpha] switches to 123":"[alpha] switches to ABC"),132,COL_YELLOW);
			shadow_text_centered("[del] erase  [enter] done  [clear] back",150,COL_GRAY_L);
			gfx_SwapDraw();
		}
		uint8_t k=os_GetCSC();
		if(!k) continue;
		if(k==sk_Enter){ if(len>0||allow_empty){ wait_key_release(); return true; } continue; }
		if(k==sk_Clear){ wait_key_release(); return false; }
		if(k==sk_Del){ if(len>0){ buf[--len]=0; dirty=true; } continue; }
		if(k==sk_Alpha&&!digits){ alpha=!alpha; dirty=true; continue; }
		char ch=0;
		if(alpha){ if(k<sizeof(letters)){ ch=letters[k]; if(!((ch>='A'&&ch<='Z')||ch==' ')) ch=0; } }
		else ch=csc_digit(k);
		if(ch&&len<maxlen){ buf[len++]=ch; buf[len]=0; dirty=true; }
	}
}

// ============================================================================
//  RECIPES  (position independent: shapes match anywhere in the grid)
// ============================================================================
#define RECIPIENUM 33
static const uint8_t recipes[RECIPIENUM][3][3]={
	{{WOOD,0,0},{0,0,0},{0,0,0}},
	{{PLANKS,0,0},{PLANKS,0,0},{0,0,0}},
	{{PLANKS,PLANKS,PLANKS},{0,STICK,0},{0,STICK,0}},
	{{STONE,STONE,STONE},{0,STICK,0},{0,STICK,0}},
	{{IRONINGOT,IRONINGOT,IRONINGOT},{0,STICK,0},{0,STICK,0}},
	{{DIAMONDITEM,DIAMONDITEM,DIAMONDITEM},{0,STICK,0},{0,STICK,0}},
	{{0,PLANKS,0},{0,PLANKS,0},{0,STICK,0}},
	{{0,STONE,0},{0,STONE,0},{0,STICK,0}},
	{{0,IRONINGOT,0},{0,IRONINGOT,0},{0,STICK,0}},
	{{0,DIAMONDITEM,0},{0,DIAMONDITEM,0},{0,STICK,0}},
	{{PLANKS,PLANKS,0},{PLANKS,PLANKS,0},{0,0,0}},
	{{PLANKS,PLANKS,0},{PLANKS,PLANKS,0},{PLANKS,PLANKS,0}},
	{{STONE,STONE,STONE},{STONE,0,STONE},{STONE,STONE,STONE}},
	{{WOOL,WOOL,WOOL},{PLANKS,PLANKS,PLANKS},{0,0,0}},
	{{GOLDINGOT,GOLDINGOT,GOLDINGOT},{0,STICK,0},{0,STICK,0}},
	{{0,GOLDINGOT,0},{0,GOLDINGOT,0},{0,STICK,0}},
	{{IRONINGOT,0,0},{0,FLINT,0},{0,0,0}},
	{{GUNPOWDER,SAND,GUNPOWDER},{SAND,GUNPOWDER,SAND},{GUNPOWDER,SAND,GUNPOWDER}},
	{{IRONINGOT,0,IRONINGOT},{0,IRONINGOT,0},{0,0,0}},
	{{IRONINGOT,IRONINGOT,IRONINGOT},{IRONINGOT,IRONINGOT,IRONINGOT},{IRONINGOT,IRONINGOT,IRONINGOT}},
	{{GOLDINGOT,GOLDINGOT,GOLDINGOT},{GOLDINGOT,GOLDINGOT,GOLDINGOT},{GOLDINGOT,GOLDINGOT,GOLDINGOT}},
	{{DIAMONDITEM,DIAMONDITEM,DIAMONDITEM},{DIAMONDITEM,DIAMONDITEM,DIAMONDITEM},{DIAMONDITEM,DIAMONDITEM,DIAMONDITEM}},
	{{REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT},{REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT},{REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT,REDSTONEDUSTUNLIT}},
	{{IRONBLOCK,0,0},{0,0,0},{0,0,0}},
	{{GOLDBLOCK,0,0},{0,0,0},{0,0,0}},
	{{DIAMONDBLOCK,0,0},{0,0,0},{0,0,0}},
	{{REDSTONEBLOCK,0,0},{0,0,0},{0,0,0}},
	{{STONE,STONE,0},{STONE,STONE,0},{0,0,0}},
	{{SAND,SAND,0},{SAND,SAND,0},{0,0,0}},
	{{PLANKS,PLANKS,PLANKS},{STICK,STICK,STICK},{PLANKS,PLANKS,PLANKS}},
	{{STICK,0,STICK},{STICK,STICK,STICK},{STICK,0,STICK}},
	{{GOLDINGOT,GOLDINGOT,GOLDINGOT},{GOLDINGOT,APPLE,GOLDINGOT},{GOLDINGOT,GOLDINGOT,GOLDINGOT}},
	{{COALITEM,0,0},{STICK,0,0},{0,0,0}},
};
static const uint8_t recipe_result[RECIPIENUM]={PLANKS,STICK,WOODPIC,STONEPIC,IRONPIC,DIAMONDPIC,WOODSWORD,STONESWORD,IRONSWORD,DIAMONDSWORD,
	CRAFTTABLE,DOORTOPOPEN,FURNACE,BEDTOP,GOLDPIC,GOLDSWORD,FLINTANDSTEEL,TNT,BUCKET,IRONBLOCK,GOLDBLOCK,DIAMONDBLOCK,REDSTONEBLOCK,
	IRONINGOT,GOLDINGOT,DIAMONDITEM,REDSTONEDUSTUNLIT,STONEBRICK,SANDSTONE,BOOKSHELF,LADDER,GOLDENAPPLE,TORCH};
static const uint8_t recipe_count[RECIPIENUM]={4,4,1,1,1,1,1,1,1,1, 1,3,1,1,1,1,1,1,1,1,1,1,1, 9,9,9,9,4,1,1,3,1,4};
static bool match_recipe(int grid[3][3],const uint8_t r[3][3])
{
	int gx=3,gy=3,rx=3,ry=3;
	for(int x=0;x<3;x++) for(int y=0;y<3;y++)
	{
		if(grid[x][y]){ if(x<gx) gx=x; if(y<gy) gy=y; }
		if(r[y][x]){ if(x<rx) rx=x; if(y<ry) ry=y; }
	}
	if(gx==3||rx==3) return false;
	for(int x=0;x<3;x++) for(int y=0;y<3;y++)
	{
		int a=(x+gx<3&&y+gy<3)?grid[x+gx][y+gy]:0;
		int b=(x+rx<3&&y+ry<3)?r[y+ry][x+rx]:0;
		if(a!=b) return false;
	}
	return true;
}
static int find_recipe(int grid[3][3],int* count)
{
	for(int i=0;i<RECIPIENUM;i++) if(match_recipe(grid,recipes[i])){ *count=recipe_count[i]; return recipe_result[i]; }
	return -1;
}
static int grid_uses(int grid[3][3],int s){ int n=0; for(int x=0;x<3;x++) for(int y=0;y<3;y++) if(grid[x][y]==s) n++; return n; }
static bool recipe_can(int rn)//can the player craft recipe rn right now?
{
	if(gamemode==1) return true;
	for(int y=0;y<3;y++) for(int x=0;x<3;x++)
	{
		int s=recipes[rn][y][x];
		if(!s) continue;
		int need=0;
		for(int yy=0;yy<3;yy++) for(int xx=0;xx<3;xx++) if(recipes[rn][yy][xx]==s) need++;
		if(inventory[s]<need) return false;
	}
	return true;
}
static bool craft_once(int grid[3][3],char* msg)
{
	int cnt=0;
	int result=find_recipe(grid,&cnt);
	if(result<=0){ strcpy(msg,"Nothing to craft"); return false; }
	if(gamemode==0)
		for(int x=0;x<3;x++) for(int y=0;y<3;y++)
		{
			int s=grid[x][y];
			if(!s) continue;
			int need=grid_uses(grid,s);
			if(inventory[s]<need){ snprintf(msg,32,"Need %d %s",need,item_name(s)); return false; }
		}
	for(int x=0;x<3;x++) for(int y=0;y<3;y++) if(grid[x][y]) decrease_item(grid[x][y]);
	inventory[result]+=cnt;
	add2hotbar(result);
	if(gamemode==0)
	{
		int clr[9]; int nc=0;
		for(int x=0;x<3;x++) for(int y=0;y<3;y++){ int s=grid[x][y]; if(s&&inventory[s]<grid_uses(grid,s)) clr[nc++]=s; }
		for(int i=0;i<nc;i++) for(int x=0;x<3;x++) for(int y=0;y<3;y++) if(grid[x][y]==clr[i]) grid[x][y]=0;
	}
	snprintf(msg,32,"+%d %s",cnt,item_name(result));
	return true;
}

// ============================================================================
//  SCREENS: recipe book, inventory/crafting, furnace, creative picker, map, menus
// ============================================================================
#define ISLOT 34
static uint8_t owned_ids[INV_SIZE];
static int build_owned()
{
	int n=0;
	for(int i=0;i<num_visible;i++){ int id=visible_ids[i]; if(inventory[id]>0) owned_ids[n++]=(uint8_t)id; }
	return n;
}
static void draw_grid_cursor(int x,int y)
{
	gfx_SetColor(COL_YELLOW);
	gfx_Rectangle_NoClip(x,y,ISLOT,ISLOT);
	gfx_Rectangle_NoClip(x+1,y+1,ISLOT-2,ISLOT-2);
}
static void draw_inv_grid(int n,int scroll,int gx,int gy)
{
	for(int i=0;i<25;i++)
	{
		int idx=scroll+i;
		int sx=gx+(i%5)*ISLOT, sy=gy+(i/5)*ISLOT;
		draw_slot(sx,sy,ISLOT);
		if(idx<n) draw_item(owned_ids[idx],sx+1,sy+1,1);
	}
	gfx_SetTextFGColor(COL_GRAY_D);
	if(scroll>0) gfx_PrintStringXY("^",gx+5*ISLOT+2,gy+2);
	if(scroll+25<n) gfx_PrintStringXY("v",gx+5*ISLOT+2,gy+5*ISLOT-10);
}
static void fix_scroll(int icur,int& scroll)
{
	if(icur<scroll) scroll=(icur/5)*5;
	if(icur>=scroll+25) scroll=(icur/5-4)*5;
	if(scroll<0) scroll=0;
}
static void view_recipes()
{
	static char labels[RECIPIENUM][24];
	static const char* opts[RECIPIENUM+1];
	for(int i=0;i<RECIPIENUM;i++){ snprintf(labels[i],24,"%c%s x%d",recipe_can(i)?'*':' ',item_name(recipe_result[i]),recipe_count[i]); opts[i]=labels[i]; }
	opts[RECIPIENUM]="Back";
	int sel=0;
	while(true)
	{
		int c=mc_menu_t("Recipes (* = can craft)",opts,RECIPIENUM+1,sel);
		if(c<0||c==RECIPIENUM) return;
		sel=c;
		gfx_SetDrawBuffer();
		draw_dirt_bg();
		draw_panel(50,26,220,180);
		menu_text_setup();
		gfx_SetTextFGColor(COL_GRAY_D);
		gfx_PrintStringXY(labels[c]+1,160-(int)gfx_GetStringWidth(labels[c]+1)/2,34);
		for(int x=0;x<3;x++) for(int y=0;y<3;y++)
		{
			int sx=62+x*ISLOT, sy=52+y*ISLOT;
			draw_slot(sx,sy,ISLOT);
			int id=recipes[c][y][x];
			if(id) draw_item(id,sx+1,sy+1,0);
		}
		gfx_PrintStringXY("=>",174,96);
		draw_slot(200,86,ISLOT);
		draw_item(recipe_result[c],201,87,0);
		draw_count(201,87,recipe_count[c]);
		gfx_PrintStringXY("Any key: back",62,190);
		gfx_SwapDraw();
		wait_any_key();
	}
}
static void inventory_screen(bool big)
{
	int grid[3][3];
	memset(grid,0,sizeof(grid));
	const int size=big?3:2;
	int area=0, icur=0, tx=0, ty=0, hcur=hotbarblock, scroll=0, held=0;
	char msg[32]; msg[0]=0;
	const int IGX=6, IGY=18, CGX=big?190:206, CGY=18, HX=7, HY=203;
	key_reset();
	bool dirty=true;
	while(true)
	{
		key_update();
		int n=build_owned();
		if(icur>=n) icur=n>0?n-1:0;
		if(key_pressed(kb_KeyGraphVar)||key_pressed(kb_KeyClear)) break;
		if(key_pressed(kb_KeyMode)){ view_recipes(); key_reset(); dirty=true; continue; }
		if(key_pressed(kb_KeyAdd)){ area=(area+1)%3; dirty=true; }
		if(key_pressed(kb_KeySub)){ area=(area+2)%3; dirty=true; }
		bool L=key_pressed(kb_KeyLeft), R=key_pressed(kb_KeyRight), U=key_pressed(kb_KeyUp), D=key_pressed(kb_KeyDown);
		if(L||R||U||D) dirty=true;
		if(area==0)
		{
			if(L&&(icur%5)>0) icur--;
			else if(R){ if((icur%5)<4&&icur+1<n) icur++; else { area=1; tx=0; ty=0; } }
			else if(U&&icur>=5) icur-=5;
			else if(D){ if(icur+5<n) icur+=5; else { area=2; hcur=icur%5; } }
		}
		else if(area==1)
		{
			if(L){ if(tx>0) tx--; else area=0; }
			else if(R&&tx<size-1) tx++;
			else if(U&&ty>0) ty--;
			else if(D){ if(ty<size-1) ty++; else { area=2; hcur=4; } }
		}
		else
		{
			if(L) hcur=(hcur+HOTBAR_N-1)%HOTBAR_N;
			else if(R) hcur=(hcur+1)%HOTBAR_N;
			else if(U) area=0;
		}
		if(key_pressed(kb_Key2nd))
		{
			dirty=true; msg[0]=0;
			if(area==0){ if(n>0){ int id=owned_ids[icur]; held=(held==id)?0:id; } }
			else if(area==1)
			{
				if(held) grid[tx][ty]=held;
				else if(grid[tx][ty]){ held=grid[tx][ty]; grid[tx][ty]=0; }
			}
			else
			{
				if(held){ for(int j=0;j<HOTBAR_N;j++) if(ghotbar[j]==held) ghotbar[j]=ghotbar[hcur]; ghotbar[hcur]=(uint8_t)held; held=0; }
				else if(ghotbar[hcur]){ held=ghotbar[hcur]; ghotbar[hcur]=0; }
			}
		}
		if(key_pressed(kb_KeyAlpha)){ dirty=true; if(area==1&&grid[tx][ty]) grid[tx][ty]=0; else held=0; }
		if(key_pressed(kb_KeyDel)){ dirty=true; memset(grid,0,sizeof(grid)); }
		if(key_pressed(kb_KeyEnter)){ dirty=true; craft_once(grid,msg); }
		if(key_pressed(kb_KeyMul))//craft everything the ingredients allow
		{
			dirty=true;
			int total=0;
			while(total<40&&craft_once(grid,msg)) total++;
			if(total) snprintf(msg,32,"+%d crafts",total);
		}
		if(dirty)
		{
			dirty=false;
			n=build_owned();
			if(icur>=n) icur=n>0?n-1:0;
			fix_scroll(icur,scroll);
			gfx_SetDrawBuffer();
			gfx_FillScreen(COL_BLACK);
			draw_panel(0,0,320,240);
			menu_text_setup();
			gfx_SetTextFGColor(COL_GRAY_D);
			gfx_PrintStringXY("Inventory",IGX+2,6);
			gfx_PrintStringXY(big?"Crafting Table":"Crafting",CGX,6);
			draw_inv_grid(n,scroll,IGX,IGY);
			for(int x=0;x<size;x++) for(int y=0;y<size;y++)
			{
				int sx=CGX+x*ISLOT, sy=CGY+y*ISLOT;
				draw_slot(sx,sy,ISLOT);
				if(grid[x][y]) draw_item(grid[x][y],sx+1,sy+1,1);
			}
			int cnt=0;
			int result=find_recipe(grid,&cnt);
			int ry=CGY+size*ISLOT+6;
			gfx_SetTextFGColor(COL_GRAY_D);
			gfx_PrintStringXY("=>",CGX+10,ry+13);
			draw_slot(CGX+36,ry,ISLOT);
			if(result>0){ draw_item(result,CGX+37,ry+1,0); draw_count(CGX+37,ry+1,cnt); }
			gfx_SetTextFGColor(COL_GRAY_D);
			gfx_PrintStringXY("Enter=1  x=all",CGX,ry+ISLOT+20);
			for(int i=0;i<HOTBAR_N;i++)
			{
				int sx=HX+i*ISLOT;
				draw_slot(sx,HY,ISLOT);
				if(ghotbar[i]) draw_item(ghotbar[i],sx+1,HY+1,1);
			}
			int cxp,cyp;
			if(area==0){ int r=icur-scroll; cxp=IGX+(r%5)*ISLOT; cyp=IGY+(r/5)*ISLOT; }
			else if(area==1){ cxp=CGX+tx*ISLOT; cyp=CGY+ty*ISLOT; }
			else { cxp=HX+hcur*ISLOT; cyp=HY; }
			draw_grid_cursor(cxp,cyp);
			int hov=0;
			if(area==0&&n>0) hov=owned_ids[icur];
			else if(area==1) hov=grid[tx][ty];
			else hov=ghotbar[hcur];
			char line[48];
			if(held) snprintf(line,48,"Holding: %s",item_name(held));
			else snprintf(line,48,"%s",item_name(hov));
			if(msg[0]){ gfx_SetTextFGColor(msg[0]=='+'?COL_DKGREEN:192); gfx_PrintStringXY(msg,CGX,ry+ISLOT+6); }
			if(line[0]) draw_tip(IGX,189,line);
			gfx_SwapDraw();
		}
	}
	updatehotbar=2;
}
static const uint8_t fuel_ids[]={WOOD,PLANKS,CRAFTTABLE,BOOKSHELF,LADDER,WOODPIC,WOODSWORD,STICK,COALITEM,SAPLING};
static const int fuel_times[]={75,75,75,75,37,50,50,25,400,25};
static const uint8_t smelt_in[]={SAND,IRON,GOLD,RAWMUTTON,RAWPORKCHOP,WOOD};
static const uint8_t smelt_out[]={GLASS,IRONINGOT,GOLDINGOT,COOKEDMUTTON,COOKEDPORKCHOP,COALITEM};
static int fuel_time(int id){ for(unsigned i=0;i<sizeof(fuel_ids);i++) if(fuel_ids[i]==id) return fuel_times[i]; return 0; }
static int smelt_result(int id){ for(unsigned i=0;i<sizeof(smelt_in);i++) if(smelt_in[i]==id) return smelt_out[i]; return 0; }
static void furnace_screen()
{
	int cook=0, fuel=0, progress=0, burn=0, burn_total=1, icur=0, scroll=0;
	char msg[32]; msg[0]=0;
	const int IGX=6, IGY=18;
	clock_t last=clock();
	key_reset();
	while(true)
	{
		key_update();
		if(key_pressed(kb_KeyGraphVar)||key_pressed(kb_KeyClear)) break;
		int n=build_owned();
		if(icur>=n) icur=n>0?n-1:0;
		if(key_pressed(kb_KeyLeft)&&(icur%5)>0) icur--;
		if(key_pressed(kb_KeyRight)&&(icur%5)<4&&icur+1<n) icur++;
		if(key_pressed(kb_KeyUp)&&icur>=5) icur-=5;
		if(key_pressed(kb_KeyDown)&&icur+5<n) icur+=5;
		fix_scroll(icur,scroll);
		if(key_pressed(kb_Key2nd)&&n>0)
		{
			int id=owned_ids[icur];
			if(smelt_result(id)){ if(cook!=id){ cook=id; progress=0; } msg[0]=0; }
			else strcpy(msg,"Can't smelt that");
		}
		if(key_pressed(kb_KeyAlpha)&&n>0)
		{
			int id=owned_ids[icur];
			if(fuel_time(id)){ fuel=id; msg[0]=0; }
			else strcpy(msg,"That's not fuel");
		}
		while((clock_t)(clock()-last)>=3277)
		{
			last+=3277;
			if(cook&&inventory[cook]>0)
			{
				if(burn==0&&fuel&&inventory[fuel]>0){ burn=burn_total=fuel_time(fuel); decrease_item(fuel); }
				if(burn>0&&++progress>=50)
				{
					progress=0;
					int r=smelt_result(cook);
					decrease_item(cook);
					inventory[r]++;
					add2hotbar(r);
				}
			}
			else progress=0;
			if(burn>0) burn--;
		}
		n=build_owned();
		if(icur>=n) icur=n>0?n-1:0;
		gfx_SetDrawBuffer();
		gfx_FillScreen(COL_BLACK);
		draw_panel(0,0,320,240);
		menu_text_setup();
		gfx_SetTextFGColor(COL_GRAY_D);
		gfx_PrintStringXY("Inventory",IGX+2,6);
		gfx_PrintStringXY("Furnace",204,6);
		draw_inv_grid(n,scroll,IGX,IGY);
		draw_slot(204,22,ISLOT);
		if(cook) draw_item(cook,205,23,1);
		int k=burn>0?1+(burn*4)/burn_total:0;
		if(k>5) k=5;
		if(flame_imgs[k]) gfx_Sprite_NoClip(flame_imgs[k],212,60);
		draw_slot(204,92,ISLOT);
		if(fuel) draw_item(fuel,205,93,1);
		draw_arrow_nfull(242,62,36,22,(progress*36)/50);
		draw_slot(282,55,ISLOT);
		int r=cook?smelt_result(cook):0;
		if(r) draw_item(r,283,56,1);
		if(n>0) draw_grid_cursor(IGX+((icur-scroll)%5)*ISLOT,IGY+((icur-scroll)/5)*ISLOT);
		gfx_SetTextFGColor(COL_GRAY_D);
		gfx_PrintStringXY("2nd: smelt",200,140);
		gfx_PrintStringXY("alpha: fuel",200,152);
		gfx_PrintStringXY("X,T,O,n: close",200,164);
		if(msg[0]){ gfx_SetTextFGColor(192); gfx_PrintStringXY(msg,200,180); }
		if(n>0) draw_tip(IGX,189,item_name(owned_ids[icur]));
		gfx_SwapDraw();
	}
	updatehotbar=2;
}
static void item_selection_menu()
{
	const int COLS=8, ROWS=5, PAGE=COLS*ROWS, SLOT=34, GX=10, GY=18, HX=7;
	int cursor=0;
	key_reset();
	bool dirty=true;
	while(true)
	{
		key_update();
		if(key_pressed(kb_KeyLeft)&&cursor>0){ cursor--; dirty=true; }
		if(key_pressed(kb_KeyRight)&&cursor<num_visible-1){ cursor++; dirty=true; }
		if(key_pressed(kb_KeyUp)&&cursor-COLS>=0){ cursor-=COLS; dirty=true; }
		if(key_pressed(kb_KeyDown)&&cursor+COLS<num_visible){ cursor+=COLS; dirty=true; }
		if(key_pressed(kb_KeyAdd)){ hotbarblock=(hotbarblock+1)%HOTBAR_N; dirty=true; }
		if(key_pressed(kb_KeySub)){ hotbarblock=(hotbarblock+HOTBAR_N-1)%HOTBAR_N; dirty=true; }
		if(key_pressed(kb_KeyEnter)||key_pressed(kb_Key2nd))
		{
			int id=visible_ids[cursor];
			if(inventory[id]<64) inventory[id]=64;
			for(int j=0;j<HOTBAR_N;j++) if(ghotbar[j]==id) ghotbar[j]=0;
			ghotbar[hotbarblock]=(uint8_t)id;
			updatehotbar=2;
			dirty=true;
		}
		if(key_pressed(kb_KeyClear)||key_pressed(kb_KeyGraphVar)) return;
		if(dirty)
		{
			dirty=false;
			gfx_SetDrawBuffer();
			gfx_FillScreen(COL_BLACK);
			draw_panel(4,2,312,236);
			menu_text_setup();
			gfx_SetTextFGColor(COL_GRAY_D);
			char title[40];
			snprintf(title,40,"Items: %s",item_name(visible_ids[cursor]));
			gfx_PrintStringXY(title,GX,6);
			int page=cursor/PAGE;
			for(int i=0;i<PAGE;++i)
			{
				int sx=GX+(i%COLS)*SLOT, sy=GY+(i/COLS)*SLOT;
				draw_slot(sx,sy,SLOT);
				int idx=page*PAGE+i;
				if(idx<num_visible) draw_item(visible_ids[idx],sx+1,sy+1,0);
			}
			int ci=cursor-page*PAGE;
			draw_grid_cursor(GX+(ci%COLS)*SLOT,GY+(ci/COLS)*SLOT);
			int pages=(num_visible+PAGE-1)/PAGE;
			int track=ROWS*SLOT;
			int thumbh=(track-4)/(pages>0?pages:1);
			draw_inset(GX+COLS*SLOT+6,GY,14,track);
			gfx_SetColor(COL_PANEL);
			gfx_FillRectangle_NoClip(GX+COLS*SLOT+8,GY+2+page*thumbh,10,thumbh);
			int hy=GY+ROWS*SLOT+8;
			for(int i=0;i<HOTBAR_N;++i)
			{
				int sx=HX+i*SLOT;
				draw_slot(sx,hy,SLOT);
				if(ghotbar[i]) draw_item(ghotbar[i],sx+1,hy+1,1);
				if(i==hotbarblock){ gfx_SetColor(COL_WHITE); gfx_Rectangle_NoClip(sx-1,hy-1,SLOT+2,SLOT+2); }
			}
			gfx_SwapDraw();
		}
	}
}
static void map_screen()
{
	uint8_t prev[WZ];//height of the previous column (for the shading step)
	const int CS=6, OX=(320-WX*CS)/2, OY=26;
	gfx_SetDrawBuffer();
	gfx_FillScreen(COL_BLACK);
	draw_panel(OX-6,4,WX*CS+12,232);
	int starty=(realm==1)?imin((int)ply+1,WY-2):WY-1;
	for(int x=0;x<WX;x++) for(int z=0;z<WZ;z++)
	{
		uint8_t id=0;
		int y;
		for(y=starty;y>0;y--){ id=vis_map(x,y,z); if(id&&id!=BARRIER) break; }
		uint8_t c;
		if(id==0||id==BARRIER) c=COL_BLACK;
		else if(id<TEXTURED_REG_START) c=colors[id+RBP1];
		else if(id>reg_block_num&&id<=FLOORVOXELSEND) c=floorcolors[id];
		else { c=LTEX[id*TSTR+TS_TOP][27]; if(c==26) c=LTEX[id*TSTR+TS_PX][27]; if(c==26) c=0; }
		if(x>0&&y<prev[z]) c=shade1(c);
		prev[z]=(uint8_t)y;
		gfx_SetColor(c);
		gfx_FillRectangle_NoClip(OX+x*CS,OY+(WZ-1-z)*CS,CS,CS);
	}
	int px=OX+(int)plx*CS+3, pz=OY+(WZ-1-(int)plz)*CS+3;
	int ex=px+(int)(pl_dirx*10), ey=pz-(int)(pl_dirz*10);
	gfx_SetColor(COL_WHITE);
	gfx_Line(px,pz,ex,ey);
	gfx_SetColor(224);
	gfx_FillCircle(px,pz,3);
	menu_text_setup();
	gfx_SetTextFGColor(COL_GRAY_D);
	gfx_PrintStringXY(realm?"Map - Nether":"Map - Overworld",OX,10);
	char line[48];
	snprintf(line,48,"Chunk %d,%d   X%d Y%d Z%d",chunkx-32,chunkz-32,(int)plx,(int)ply,(int)plz);
	gfx_PrintStringXY(line,OX,OY+WZ*CS+4);
	gfx_SwapDraw();
	wait_any_key();
}
int game_menu_overlay()//0 back, 1 options, 2 statistics, 3 save+quit
{
	int choice=0;
	bool dirty=true, first=true;
	menu_text_setup();
	dim_rows(0,184,COL_BLACK);
	key_reset();
	while(true)
	{
		key_update();
		if(key_pressed(kb_KeyUp)){ choice=(choice+3)%4; dirty=true; }
		if(key_pressed(kb_KeyDown)){ choice=(choice+1)%4; dirty=true; }
		if(key_pressed(kb_KeyClear)){ wait_key_release(); return 0; }
		if(key_pressed(kb_KeyEnter)||key_pressed(kb_Key2nd)){ wait_key_release(); return choice; }
		if(dirty)
		{
			dirty=false;
			if(!first) copy_displayed_to_draw();
			first=false;
			menu_text_setup();
			shadow_text_centered("Game Menu",30,COL_WHITE);
			draw_mc_button(60,54,200,20,choice==0,"Back to Game");
			draw_mc_button(60,80,200,20,choice==1,"Options...");
			draw_mc_button(60,106,200,20,choice==2,"Statistics");
			draw_mc_button(60,142,200,20,choice==3,"Save and Quit to Title");
			gfx_SwapDraw();
		}
	}
}
static int death_screen(bool hardcore,int dx,int dy,int dz)
{
	gfx_Wait();
	copy_displayed_to_draw();
	dim_rows(0,240,160);
	gfx_SwapDraw();
	int choice=0;
	char where[32];
	snprintf(where,32,"Died at X%d Y%d Z%d",dx,dy,dz);
	wait_key_release();
	key_reset();
	bool dirty=true;
	while(true)
	{
		key_update();
		if(!hardcore&&(key_pressed(kb_KeyUp)||key_pressed(kb_KeyDown))){ choice^=1; dirty=true; }
		if(key_pressed(kb_KeyEnter)||key_pressed(kb_Key2nd)){ wait_key_release(); return hardcore?1:choice; }
		if(dirty)
		{
			dirty=false;
			gfx_Wait();
			copy_displayed_to_draw();
			menu_text_setup();
			gfx_SetTextScale(2,2);
			shadow_text_centered(hardcore?"Game Over!":"You Died!",56,COL_WHITE);
			gfx_SetTextScale(1,1);
			shadow_text_centered(where,84,COL_GRAY_L);
			if(hardcore){ shadow_text_centered("Hardcore: this world is gone.",98,COL_GRAY_L); draw_mc_button(60,130,200,20,true,"Title Screen"); }
			else { draw_mc_button(60,120,200,20,choice==0,"Respawn"); draw_mc_button(60,148,200,20,choice==1,"Title Screen"); }
			gfx_SwapDraw();
		}
	}
}

#include "main_part3.h"