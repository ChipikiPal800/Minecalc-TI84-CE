// ============================================================================
//  MINECALC 0.4.0   FILE 3/3: src/main_part3.h  (gameplay, HUD, world list, title, credits, main)
//  Included by main_part2.h.
// ============================================================================

// ============================================================================
//  GAMEPLAY
// ============================================================================
static bool soul_skip=false;
static void cross_chunk(int dx,int dz,int newx,int newz)
{
	flint keepy=ply;
	save_chunk(false);
	chunkx+=dx; chunkz+=dz;
	loadnewchunk();
	if(newx>=0) plx=newx;
	if(newz>=0) plz=newz;
	settle_player(keepy);
}
// walking into a one-block step with head room: hop up it
static void autostep(int cx,int cz,int ix,int iy,int iz)
{
	if(!set_autojump||flying||grav.data!=0||iy<2||iy>=WY-2) return;
	if(walkable(vis_map(cx,iy-1,cz))) return;
	if(!walkable(vis_map(cx,iy,cz))||!walkable(vis_map(cx,iy+1,cz))) return;
	if(!walkable(vis_map(ix,iy+1,iz))) return;
	if(walkable(vis_map(ix,iy-2,iz))) return;
	grav=jump_vel;
}
static void try_walk(flint dirx,flint dirz)
{
	if(gamemode==0&&randInt(0,100)==0){ if(playerhunger>0) playerhunger--; updatehotbar=2; }
	int iplx=(int)plx, iply=(int)ply, iplz=(int)plz;
	if(!flying&&iply>=2&&vis_map(iplx,iply-2,iplz)==SOULSAND){ soul_skip=!soul_skip; if(soul_skip) return; }
	if(plx+dirx>30&&chunkx<61) cross_chunk(1,0,1,-1);
	else if(plx+dirx<1&&chunkx>0) cross_chunk(-1,0,30,-1);
	else if(plz+dirz>=32&&chunkz<61) cross_chunk(0,1,-1,1);
	else if(plz+dirz<1&&chunkz>0) cross_chunk(0,-1,-1,31);
	else
	{
		int nx=(int)(dirx+plx), nz=(int)(dirz+plz);
		if((walkable(vis_map(nx,iply,iplz))&&walkable(vis_map(nx,iply-1,iplz)))||vis_map(nx,iply,iplz)==DOORTOPOPEN) plx+=dirx;
		else autostep(nx,iplz,iplx,iply,iplz);
		iplx=(int)plx;
		if((walkable(vis_map(iplx,iply,nz))&&walkable(vis_map(iplx,iply-1,nz)))||vis_map(iplx,iply,nz)==DOORTOPCLOSED) plz+=dirz;
		else autostep(iplx,nz,iplx,iply,iplz);
	}
}
static void travel_portal()
{
	loading_screen(realm==0?"Entering the Nether...":"Returning to the Overworld...");
	realmcooldown=30;
	portal_must_exit=true;
	save_chunk(false);
	realm=realm?0:1;
	loadnewchunk();
	change_bg(realm==1?NETHERBGCOLOR:bg_colors[world_time()]);
	uint8_t x,y,z;
	if(find_nether_portal(x,y,z)){ plx=x; plz=z; settle_player(flint((int)y)); }
	else
	{
		uint8_t ry=0;
		spawn_nether_portal((uint8_t)(int)plx,ry,(uint8_t)(int)plz);
		plx+=1;
		settle_player(flint((int)ry+2));
	}
	compute_world_top();
	updatehotbar=2;
}
static void check_portal()
{
	int x=(int)plx, y=(int)ply, z=(int)plz;
	bool in=vis_map(x,y,z)==NETHERPORTAL||(y>0&&vis_map(x,y-1,z)==NETHERPORTAL);
	if(!in){ portal_must_exit=false; return; }
	if(portal_must_exit||realmcooldown) return;
	travel_portal();
}
static void respawn_player()
{
	playerhp=20; playerhunger=20; playerair=MAXAIR; flying=false; updatehotbar=2;
	if(chunkx!=respawnchunkx||chunkz!=respawnchunkz||realm!=0)
	{
		loading_screen("Respawning...");
		save_chunk(false);
		realm=0;
		chunkx=respawnchunkx; chunkz=respawnchunkz;
		loadnewchunk();
	}
	change_bg(bg_colors[world_time()]);
	plx=respawnx; plz=respawnz;
	settle_player(flint(respawny));
}
static int tool_level(int id)
{
	switch(id){ case WOODPIC: case GOLDPIC: return 2; case STONEPIC: return 3; case IRONPIC: return 4; case DIAMONDPIC: return 5; }
	return 1;
}
static void do_place()
{
	int id=ghotbar[hotbarblock];
	updatehotbar=2;
	if(id>=ITEMBASE)
	{
		int ii=id-ITEMBASE;
		if(ii<item_num&&use_item[ii]){ use_item[ii](); keyup(key_place); return; }
	}
	uint8_t* before;
	uint8_t* target=crosshair(before);
	if(!target) return;
	if(*target>reg_block_num&&*target<MOBSTART&&use_block(target)){ ch_valid=false; return; }
	if(id<=0||id>invenblocknum) return;
	if(gamemode==0&&inventory[id]<=0){ ghotbar[hotbarblock]=0; return; }
	uint8_t* block=before;
	if(block==target) return;
	if(!walkable(*block)||*block>=MOBSTART||*block==NETHERPORTAL) return;
	if(id==SAPLING){ uint8_t under=*(block-YJ); if(under!=GRASS&&under!=SNOWGRASS){ toast("Saplings need grass"); return; } }
	int ipx=(int)plx, ipy=(int)ply, ipz=(int)plz;
	const uint8_t look=look_dir();
	uint8_t* top=block;
	uint8_t* bottom=nullptr;
	if(id==DOORTOPOPEN)
	{
		bottom=block-YJ;
		if(!walkable(*bottom)||*bottom>=MOBSTART)
		{
			bottom=block;
			top=block+YJ;
			if(top>=world+WORLD_BYTES||!walkable(*top)||*top>=MOBSTART) return;
		}
	}
	uint8_t x,y,z;
	address2coords(top,x,y,z);
	bool same=(ipx==x&&ipz==z);
	if(same&&(y==ipy||(y==ipy-1&&vis_map(ipx,ipy+1,ipz)))) return;
	if(bottom)
	{
		uint8_t bx,by,bz;
		address2coords(bottom,bx,by,bz);
		if(ipx==bx&&ipz==bz&&(by==ipy||by==ipy-1)) return;
		*bottom=(look&1)?DOORBOTTOMCLOSED:DOORBOTTOMOPEN;
		*top=(look&1)?DOORTOPCLOSED:DOORTOPOPEN;
	}
	else *top=orient_place((uint8_t)id);
	add_updates_fully(top);
	ch_valid=false;
	stats.placed++;
	if(y>=world_top) world_top=y;
	if(thin_id[id]) thin_world=1;
	if(gamemode==0){ inventory[id]--; if(inventory[id]<=0) ghotbar[hotbarblock]=0; }
}
static void do_physics()
{
	int ipx=(int)plx, ipz=(int)plz;
	uint8_t feet=vis_map(ipx,(int)ply-1,ipz);
	if(!walkable(feet)&&feet!=DOORBOTTOMCLOSED&&feet!=DOORBOTTOMOPEN&&(int)ply<WY-1) ply+=1;
	int ipy=(int)ply;
	feet=vis_map(ipx,ipy-1,ipz);
	uint8_t head=vis_map(ipx,ipy,ipz);
	uint8_t below=(ipy>=2)?vis_map(ipx,ipy-2,ipz):BARRIER;
	uint8_t above=(ipy<WY-1)?vis_map(ipx,ipy+1,ipz):BARRIER;
	bool inwater=iswater(feet)||iswater(head);
	bool ladder=(bflags[feet]&F_CLIMB)||(bflags[head]&F_CLIMB);
	if(flying)
	{
		grav=0; air_peak=ply;
		if(kb_IsDown(key_jump)&&walkable(above)) ply+=fly_speed;
		else if(kb_IsDown(kb_KeyWindow)&&walkable(below)) ply-=fly_speed;
	}
	else if(ladder||inwater)
	{
		grav=0; air_peak=ply;
		if(kb_IsDown(key_jump)){ if(walkable(above)) ply+=ladder?climb_speed:swim_up; }
		else if(walkable(below)) ply-=sink_speed;
	}
	else if(walkable(below)||grav<0)
	{
		if(ply>air_peak) air_peak=ply;
		grav+=grav_amount;
		if(grav>grav_max) grav=grav_max;
		ply-=grav;
		uint8_t h2=vis_map(ipx,(int)ply,ipz);
		if(!walkable(h2)&&h2!=DOORTOPCLOSED&&h2!=DOORTOPOPEN){ ply-=1; grav=0; }
	}
	else
	{
		if(air_peak>ply)
		{
			int fall=(int)(air_peak-ply);
			if(fall>3&&gamemode==0){ playerhp-=fall-3; updatehotbar=2; }
		}
		air_peak=ply;
		if(kb_IsDown(key_jump))
		{
			grav=jump_vel;
			if(gamemode==0&&randInt(0,40)==0&&playerhunger>0){ playerhunger--; updatehotbar=2; }
		}
		else grav=0;
	}
	if(ply>WY-1) ply=WY-1;
	if(ply<1) ply=1;
}
static const char* biome_name()
{
	if(realm==1) return "Nether";
	if(world_type==1) return "Flat";
	switch(biome){ case 0: return "Plains"; case 1: return "Snowy Plains"; case 2: return "Desert"; case 3: return "Forest"; case 4: return "Snowy Forest"; }
	return "?";
}
static const char* const look_names[4]={"+Z","-X","-Z","+X"};
static const char* target_name()
{
	uint8_t* before;
	uint8_t* t=crosshair(before);
	if(!t) return "-";
	uint8_t id=*t;
	if(id>=MOBSTART) return "Mob";
	return block_names[id];
}
// black wireframe on the 9 visible edges of the targeted block
static void draw_outline()
{
	uint8_t* before;
	uint8_t* t=crosshair(before);
	if(!t||*t>=MOBSTART) return;
	uint8_t bx,by,bz;
	address2coords(t,bx,by,bz);
	const int ix=(int)plx, iy=(int)ply, iz=(int)plz;
	const flint cxz=COSR(rotxz), sxz=SINR(rotxz), cyz=COSR(rotyz), syz=SINR(rotyz);
	int sx[8], sy[8];
	bool ok[8];
	for(int i=0;i<8;i++)
	{
		flint wx=fraw((2*(bx-ix)+2*(i&1)-1)*256);
		flint wy=fraw((2*(by-iy)+2*((i>>1)&1)-1)*256);
		flint wz=fraw((2*(bz-iz)+2*((i>>2)&1)-1)*256);
		flint cx=cxz*wx+sxz*wz;
		flint nz1=cxz*wz-sxz*wx;
		flint cy=cyz*wy+syz*nz1;
		flint cz=cyz*nz1-syz*wy;
		ok[i]=cz.data>=64;
		if(ok[i]){ sx[i]=160+(cx.data*focal_length)/cz.data; sy[i]=120-(cy.data*focal_length)/cz.data; }
	}
	const int hc=((ix>bx)?0:1)|(((iy>by)?0:1)<<1)|(((iz>bz)?0:1)<<2);//far corner: its 3 edges are hidden
	gfx_SetClipRegion(0,0,320,184);
	gfx_SetColor(COL_BLACK);
	for(int a=0;a<8;a++) for(int k=0;k<3;k++)
	{
		if(a&(1<<k)) continue;
		int b=a|(1<<k);
		if(a==hc||b==hc||!ok[a]||!ok[b]) continue;
		gfx_Line(sx[a],sy[a],sx[b],sy[b]);
	}
	gfx_SetClipRegion(0,0,320,240);
}
static int mine_num=0, mine_den=0, tm_r=0, tm_h=0, tm_l=0;
static void draw_hud(int wtime)
{
	if(set_held){ int hid=ghotbar[hotbarblock]; if(hid) draw_item(hid,284,146,0); }
	if(mine_den>0)
	{
		int done=mine_den-imax(mine_num,0);
		int w=imin((40*done)/mine_den,40);
		gfx_SetColor(0); gfx_FillRectangle_NoClip(140,128,40,4);
		if(w>0){ gfx_SetColor(231); gfx_FillRectangle_NoClip(140,128,w,4); }
	}
	gfx_SetColor(75);
	gfx_HorizLine_NoClip(0,184,320);
	if(updatehotbar>0)
	{
		updatehotbar--;
		gfx_SetColor(COL_GRAY_D);
		gfx_FillRectangle_NoClip(0,184,320,56);
		gfx_SetColor(COL_BLACK);
		gfx_FillRectangle_NoClip(0,184,320,1);
		for(int i=0;i<HOTBAR_N;i++){ int sx=2+i*35; draw_slot(sx,204,34); if(ghotbar[i]) draw_item(ghotbar[i],sx+1,205,1); }
		{
			int sx=2+hotbarblock*35;
			gfx_SetColor(COL_WHITE); gfx_Rectangle_NoClip(sx-1,202,36,36);
			gfx_SetColor(COL_BLACK); gfx_Rectangle_NoClip(sx,203,34,35);
		}
		int hp=imax(0,imin(20,playerhp));
		int x=0, val=(hp>>1)*15;
		for(;x<val;x+=15) gfx_Sprite_NoClip(heart,x,185);
		int oldx=x;
		for(;x<150;x+=15) gfx_Sprite_NoClip(emptyheart,x,185);
		if(hp&1) gfx_Sprite_NoClip(halfheart,oldx,185);
		int h=imax(0,imin(20,playerhunger));
		int a=(10-(h>>1))*15;
		for(int xx=169+a;xx<305;xx+=15) gfx_Sprite_NoClip(hunger,xx,185);
		for(int xx=169;xx<169+a;xx+=15) gfx_Sprite_NoClip(emptyhunger,xx,185);
	}
	if(set_cross==0){ gfx_SetColor(0); gfx_FillRectangle_NoClip(157,119,6,2); gfx_FillRectangle_NoClip(159,117,2,6); }
	else if(set_cross==1){ gfx_SetColor(0); gfx_FillRectangle_NoClip(158,118,4,4); gfx_SetColor(255); gfx_FillRectangle_NoClip(159,119,2,2); }
	if(playerair<MAXAIR)
	{
		for(int i=0;i<MAXAIR;i++)
		{
			int bx=308-i*12;
			if(i<playerair){ gfx_SetColor(159); gfx_FillCircle(bx,176,4); gfx_SetColor(255); gfx_SetPixel(bx-1,174); }
			else { gfx_SetColor(74); gfx_Circle(bx,176,4); }
		}
	}
	if(hurt_timer)
	{
		hurt_timer--;
		gfx_SetColor(224);
		gfx_Rectangle_NoClip(0,0,320,184);
		gfx_Rectangle_NoClip(1,1,318,182);
		gfx_Rectangle_NoClip(2,2,316,180);
	}
	menu_text_setup();
	if(name_timer)
	{
		name_timer--;
		int id=ghotbar[hotbarblock];
		if(id&&set_names) shadow_text_centered(item_name(id),160,COL_WHITE);
	}
	if(toast_timer){ toast_timer--; shadow_text_centered(toast_msg,140,COL_YELLOW); }
	if(flying) shadow_text("Flying",272,4,COL_WHITE);
	if(set_debug)
	{
		gfx_SetTextFGColor((wtime>12||realm==1)?255:0);
		gfx_SetTextXY(0,0);
		gfx_PrintString("X: "); gfx_PrintInt((int)plx,1);
		gfx_PrintString(", Y: "); gfx_PrintInt((int)ply,1);
		gfx_PrintString(", Z: "); gfx_PrintInt((int)plz,1);
		gfx_SetTextXY(0,10);
		gfx_PrintString("Chunk: "); gfx_PrintInt((int)chunkx-32,1);
		gfx_PrintString(", "); gfx_PrintInt((int)chunkz-32,1);
		gfx_SetTextXY(0,20);
		gfx_PrintString(biome_name());
		gfx_PrintString("  T"); gfx_PrintInt(wtime,1);
		static int fps_frames=0;
		static clock_t fps_mark=clock();
		static int fps_shown=0;
		++fps_frames;
		clock_t now=clock();
		if((clock_t)(now-fps_mark)>=32768){ fps_shown=fps_frames; fps_frames=0; fps_mark=now; }
		gfx_SetTextXY(0,30);
		gfx_PrintString("FPS: ");
		gfx_PrintInt(fps_shown,1);
		gfx_PrintString("  R"); gfx_PrintInt(tm_r,1);
		gfx_PrintString(" H"); gfx_PrintInt(tm_h,1);
		gfx_PrintString(" L"); gfx_PrintInt(tm_l,1);
		gfx_PrintString("ms");
		gfx_SetTextXY(0,40);
		gfx_PrintString("Facing ");
		gfx_PrintString(look_names[look_dir()]);
		gfx_PrintString("  ");
		gfx_PrintString(target_name());
	}
	gfx_SetTextBGColor(255);
	gfx_SetTextFGColor(0);
}
static void hardcore_delete_current_world();
static void play_game()
{
	gfx_SetDrawBuffer();
	gfx_SetTextTransparentColor(26);
	start_time=clock();
	session_mark=clock();
	srand(rtc_Time());
	clock_t last_update=clock(), last_place=clock(), last_world_tick=clock(), last_autosave=clock();
	unsigned int frame=AVERAGEFPS*5-1, wtick=0;
	int mining_timer=0;
	uint8_t* mining_block=nullptr;
	bool prevkeydel=true;
	uint8_t attack_cd=0;
	int lspeed=0;
	bool prev_fly=false;
	int prev_hb=hotbarblock;
	int8_t prev_hp=playerhp;
	int air_ticks=0;
	int wtime=world_time();
	bool quit=false;
	mine_den=0;
	ch_valid=false;
	compute_dir();
	compute_world_top();
	portal_must_exit=true;
	grav=0; air_peak=ply;
	change_bg(realm==1?NETHERBGCOLOR:bg_colors[wtime]);
	gfx_FillScreen(bg_color);
	updatehotbar=2;
	toast(gamemode==1?"Creative mode - press Enter for items":"Survival mode - good luck!");
	static const kb_lkey_t numkeys[HOTBAR_N]={kb_Key1,kb_Key2,kb_Key3,kb_Key4,kb_Key5,kb_Key6,kb_Key7,kb_Key8,kb_Key9};
	while(!quit)
	{
		frame++;
		if(gamemode==1){ playerhp=20; playerhunger=20; playerair=MAXAIR; }
		if(realmcooldown) realmcooldown--;
		if(playerhp<1)
		{
			stats.deaths++;
			death_x=(int)plx; death_y=(int)ply; death_z=(int)plz;
			bool hc=(difficulty==3&&gamemode==0);
			int c=death_screen(hc,death_x,death_y,death_z);
			if(hc){ hardcore_delete_current_world(); return; }
			respawn_player();
			prev_hp=playerhp;
			mining_block=nullptr;
			mine_den=0;
			gfx_SetDrawBuffer();
			gfx_FillScreen(bg_color);
			updatehotbar=2;
			if(c==1) break;
			continue;
		}
		if((frame&15)==0)
		{
			if(gamemode==0)
			{
				if((playerhunger>17||difficulty==0)&&playerhp<20)
				{
					playerhp+=2; if(playerhp>20) playerhp=20;
					if(difficulty) playerhunger-=randInt(0,2);
					updatehotbar=2;
				}
				else if(playerhunger<=0&&(playerhp>1||difficulty>=2)){ playerhp--; updatehotbar=2; }
				if(playerhunger<0) playerhunger=0;
			}
			wtime=world_time();
			if(realm==0)
			{
				change_bg(bg_colors[wtime]);
				if((biome==0||biome==3||world_type==1)&&mob_count<10)
				{
					if(!randInt(0,4)) spawn_sheep();
					if(!randInt(0,4)) spawn_pig();
				}
				if(set_mobspawn&&difficulty&&wtime>9&&mob_count<14&&!randInt(0,set_mobspawn==2?1:2))
				{
					int r=randInt(0,5);
					if(r<2) spawn_creeper();
					else if(r==2) spawn_skeleton();
					else spawn_zombie();
				}
			}
			else if(set_mobspawn&&difficulty&&mob_count<10&&!randInt(0,6)) spawn_piglin();
		}
		gfx_Wait();
		const clock_t tk0=clock();
		kb_Scan();
		if(kb_IsDown(key_lookfast)){ keyup(key_lookfast); lspeed=!lspeed; toast(lspeed?"Fast look ON":"Fast look OFF"); }
		bool turned=true;
		const int lr=(set_look+1)<<lspeed, ud=(set_look==2?2:1)<<lspeed;
		if(kb_IsDown(kb_KeyRight)) rotxz=(rotxz-lr)&63;
		else if(kb_IsDown(kb_KeyLeft)) rotxz=(rotxz+lr)&63;
		else if(kb_IsDown(kb_KeyDown)||kb_IsDown(kb_KeyUp))
		{
			int pit=(rotyz>31)?rotyz-64:rotyz;//signed pitch: +down, -up, limited to +-15
			pit+=kb_IsDown(kb_KeyDown)?ud:-ud;
			pit=imax(-15,imin(15,pit));
			rotyz=pit&63;
		}
		else turned=false;
		if(turned) compute_dir();
		for(int i=0;i<HOTBAR_N;i++) if(kb_IsDown(numkeys[i])){ hotbarblock=i; updatehotbar=2; }
		if(hotbarblock!=prev_hb){ prev_hb=hotbarblock; name_timer=AVERAGEFPS*2; }
		bool fk=kb_IsDown(kb_KeyYequ);
		if(fk&&!prev_fly)
		{
			if(gamemode==1){ flying=!flying; grav=0; toast(flying?"Flying ON":"Flying OFF"); }
			else toast("Flying is creative-only");
		}
		prev_fly=fk;
		if(gamemode!=1) flying=false;
		clock_t ct=clock();
		// ---- world tick (4 Hz) ----
		if((clock_t)(ct-last_world_tick)>CLOCKS_PER_SEC/4)
		{
			last_world_tick=ct;
			wtick++;
			update_all_mobs();
			call_update();
			if((wtick&3)==0) compute_world_top();
			if(realm==0){ manage_nocturnal(vmob_pool,VMOB_POOL_SIZE,wtime,3); manage_nocturnal(skel_pool,SKEL_POOL_SIZE,wtime,3); }
			{
				const uint8_t** r=LTEX+LITTNT*TSTR;
				if(wtick&1){ r[TS_PX]=r[TS_NX]=r[TS_PZ]=r[TS_NZ]=GT(GT_TNT_SIDE); r[TS_TOP]=r[TS_BOT]=GT(GT_TNT_TOP); }
				else { for(int k=0;k<6;k++) r[k]=grey; }
			}
			int x=(int)plx, y=(int)ply, z=(int)plz;
			if(y>=2&&gamemode==0)
			{
				uint8_t under=vis_map(x,y-2,z), feet=vis_map(x,y-1,z), head=vis_map(x,y,z);
				if(under==MAGMABLOCK){ playerhp--; updatehotbar=2; }
				if(feet==FIRE||head==FIRE){ playerhp--; updatehotbar=2; }
				if((wtick&3)==0)
				{
					bool cac=false;
					for(int dy=0;dy<2;dy++)
					{
						int yy=y-dy;
						if(vis_map(x+1,yy,z)==CACTUS||vis_map(x-1,yy,z)==CACTUS||vis_map(x,yy,z+1)==CACTUS||vis_map(x,yy,z-1)==CACTUS) cac=true;
					}
					if(cac){ playerhp--; updatehotbar=2; }
				}
				if(iswater(head))
				{
					if(++air_ticks>=6){ air_ticks=0; if(playerair>0) playerair--; else { playerhp-=2; updatehotbar=2; } }
				}
				else { playerair=MAXAIR; air_ticks=0; }
			}
		}
		// ---- action tick (7 Hz) ----
		if((clock_t)(ct-last_update)>CLOCKS_PER_SEC/7)
		{
			last_update=ct;
			if(attack_cd) attack_cd--;
			if(kb_IsDown(key_fwd)) try_walk(pl_dirx,pl_dirz);
			else if(kb_IsDown(key_back)) try_walk(-pl_dirx,-pl_dirz);
			if(ply>WY-1) ply=WY-1;
			if(ply<1) ply=1;
			if(kb_IsDown(kb_KeyGraphVar)||open_crafting_table)
			{
				bool big=open_crafting_table;
				open_crafting_table=false;
				while(kb_IsDown(kb_KeyGraphVar)) kb_Scan();
				inventory_screen(big);
				while(kb_IsDown(kb_KeyGraphVar)||kb_IsDown(kb_KeyClear)) kb_Scan();
				gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2;
			}
			if(kb_IsDown(key_mine))
			{
				uint8_t* before;
				uint8_t* block=crosshair(before);
				if(block&&*block>=MOBSTART)
				{
					mine_den=0;
					if(!attack_cd){ hurtmob_at(block); attack_cd=2; }
				}
				else
				{
					int tool=tool_level(ghotbar[hotbarblock]);
					if(block&&*block!=BARRIER&&hardness[*block]<255)
					{
						mining_timer--;
						if(block!=mining_block) prevkeydel=true;
						uint8_t bv=*block;
						mining_block=block;
						if(prevkeydel)
						{
							mining_timer=(hardness[bv]*AVERAGEFPS)/tool;
							if(gamemode==1) mining_timer=0;
							mine_den=mining_timer;
						}
						prevkeydel=false;
						mine_num=mining_timer;
						if(mining_timer<=0)
						{
							prevkeydel=true;
							mining_block=nullptr;
							mine_den=0;
							int id=break_block(block);
							stats.mined++;
							if(tool<hardness[bv]) decrease_item(id);
						}
					}
					else mine_den=0;
				}
			}
			else { mining_timer=0; prevkeydel=true; mine_den=0; }
			if(kb_IsDown(kb_KeyAdd)){ hotbarblock=(hotbarblock+1)%HOTBAR_N; updatehotbar=2; }
			else if(kb_IsDown(kb_KeySub)){ hotbarblock=(hotbarblock+HOTBAR_N-1)%HOTBAR_N; updatehotbar=2; }
		}
		// ---- game menu ----
		if(kb_IsDown(kb_KeyClear))
		{
			while(kb_IsDown(kb_KeyClear)) kb_Scan();
			bool stay=true;
			while(stay)
			{
				raycast_screen();
				int g=game_menu_overlay();
				if(g==3){ quit=true; stay=false; }
				else if(g==1) settings_menu();
				else if(g==2) stats_screen();
				else stay=false;
			}
			gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2;
			continue;
		}
		if(gamemode==1&&kb_IsDown(kb_KeyEnter))
		{
			item_selection_menu();
			while(kb_IsDown(kb_KeyEnter)||kb_IsDown(kb_KeyClear)) kb_Scan();
			gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2;
		}
		if(kb_IsDown(kb_KeyGraph))
		{
			while(kb_IsDown(kb_KeyGraph)) kb_Scan();
			map_screen();
			gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2;
		}
		if(kb_IsDown(key_help)){ controls_screen(); gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2; }
		if((clock_t)(clock()-last_place)>CLOCKS_PER_SEC/4&&kb_IsDown(key_place)){ last_place=clock(); do_place(); }
		do_physics();
		check_portal();
		if((clock_t)(clock()-last_autosave)>(clock_t)9830400L)
		{
			loading_screen("Autosaving...");
			save_chunk(true);
			save_player();
			last_autosave=clock();
			gfx_SetDrawBuffer(); gfx_FillScreen(bg_color); updatehotbar=2;
			toast(save_failed?"Autosave failed (low RAM)":"World saved");
		}
		if(playerhp<prev_hp) hurt_timer=3;
		prev_hp=playerhp;
		clock_t tk1=clock();
		tm_l=(int)(((int)(tk1-tk0)*1000)>>15);
		raycast_screen();
		draw_celestial(wtime);
		if(set_outline) draw_outline();
		clock_t tk2=clock();
		tm_r=(int)(((int)(tk2-tk1)*1000)>>15);
		draw_hud(wtime);
		tm_h=(int)(((int)(clock()-tk2)*1000)>>15);
		gfx_SwapDraw();
	}
	loading_screen("Saving world...");
	save_chunk(false);
	save_player();
	loading_screen("Archiving...");
	archive_world_files();
	if(save_failed) message_screen("Warning: part of the world","could not be saved (low RAM).");
}

// ============================================================================
//  WORLD LIST (MNCFTD: u8 version, u8 count, count * {16 name, 6 appvar})
// ============================================================================
#define HEADER_BYTES (2+22*40)
static uint8_t world_header[HEADER_BYTES+24];
static void write_header()
{
	set_archive_fn("MNCFTD",false);
	ti_var_t s=ti_Open("MNCFTD","w");
	if(!s) return;
	ti_Write(world_header,1,HEADER_BYTES,s);
	ti_Close(s);
	set_archive_fn("MNCFTD",true);
}
static void load_header()
{
	memset(world_header,0,sizeof(world_header));
	ti_var_t s=ti_Open("MNCFTD","r");
	if(!s){ write_header(); return; }
	ti_Read(world_header,1,HEADER_BYTES,s);
	ti_Close(s);
	if(world_header[1]>40) world_header[1]=40;
}
static void remove_header_entry(const char* base6)
{
	load_header();
	int n=world_header[1];
	for(int i=0;i<n;i++)
	{
		int off=2+i*22;
		if(memcmp(&world_header[off+16],base6,6)==0)
		{
			memmove(&world_header[off],&world_header[off+22],(n-i-1)*22);
			world_header[1]--;
			memset(&world_header[2+world_header[1]*22],0,22);
			write_header();
			return;
		}
	}
}
static void hardcore_delete_current_world()
{
	loading_screen("Deleting world...");
	char base[7];
	memcpy(base,origwname,6);
	base[6]=0;
	delete_world_files(base);
	remove_header_entry(base);
}
static bool create_world()
{
	load_header();
	if(world_header[1]>=40){ message_screen("World limit reached (40).","Delete a world first."); return false; }
	char name[17]; memset(name,0,sizeof(name));
	if(!text_input("Name your world",name,16,false,false)) return false;
	char seedbuf[8]; memset(seedbuf,0,sizeof(seedbuf));
	if(!text_input("World seed (blank = random)",seedbuf,6,true,true)) return false;
	int seed=seedbuf[0]?atoi(seedbuf):randInt(0,32767);
	const char* wt[]={"Normal","Flat"};
	int t=mc_menu_t("World Type",wt,2); if(t<0) return false;
	const char* gm[]={"Survival","Creative"};
	int g=mc_menu_t("Game Mode",gm,2); if(g<0) return false;
	const char* df[]={"Peaceful","Normal","Hard","Hardcore"};
	int d=mc_menu_t("Difficulty",df,4,1); if(d<0) return false;
	int count=world_header[1];
	int num=0;
	for(;num<999;num++)
	{
		bool used=false;
		for(int i=0;i<count;i++){ char tc[4]; memcpy(tc,&world_header[2+i*22+19],3); tc[3]=0; if(atoi(tc)==num){ used=true; break; } }
		if(used) continue;
		char pn[9];
		snprintf(pn,9,"MCO%03d",num); if(appvar_exists(pn)) continue;
		snprintf(pn,9,"MCP%03d",num); if(appvar_exists(pn)) continue;
		break;
	}
	int off=2+count*22;
	memset(&world_header[off],0,22);
	memcpy(&world_header[off],name,16);
	char an[9];
	snprintf(an,9,"MCO%03d",num);
	memcpy(&world_header[off+16],an,6);
	world_header[1]=(uint8_t)(count+1);
	write_header();
	reset_game_state();
	memcpy(origwname,an,6);
	origwname[6]=0;
	wseed=seed&0xFFFF;
	world_type=(uint8_t)t; gamemode=(uint8_t)g; difficulty=(uint8_t)d;
	loading_screen("Generating world...");
	loadnewchunk();
	find_spawn();
	save_player();
	return true;
}
static int pick_world(const char* title)
{
	load_header();
	int n=world_header[1];
	if(n==0){ message_screen("No worlds yet!","Create one first."); return -1; }
	static char names[40][17];
	static const char* opts[41];
	for(int i=0;i<n;i++)
	{
		memcpy(names[i],&world_header[2+i*22],16);
		names[i][16]=0;
		if(!names[i][0]) strcpy(names[i],"(unnamed)");
		opts[i]=names[i];
	}
	opts[n]="Back";
	int c=mc_menu_t(title,opts,n+1);
	if(c<0||c==n) return -1;
	return c;
}
static bool open_world()
{
	int c=pick_world("Select World");
	if(c<0) return false;
	reset_game_state();
	memcpy(origwname,&world_header[2+c*22+16],6);
	origwname[6]=0;
	loading_screen("Loading world...");
	if(!load_player())
	{
		if(!load_legacy_player()){ message_screen("Couldn't read this world.",""); return false; }
		loading_screen("Upgrading old world (one time)...");
		scan_all_chunks(false);
	}
	flint keep=ply;
	loadnewchunk();
	settle_player(keep);
	return true;
}
static void delete_world_menu()
{
	int c=pick_world("Delete World");
	if(c<0) return;
	const char* yn[]={"Delete forever","Cancel"};
	if(mc_menu_t("Are you sure?",yn,2,1)!=0) return;
	char base[7];
	memcpy(base,&world_header[2+c*22+16],6);
	base[6]=0;
	loading_screen("Deleting world...");
	delete_world_files(base);
	remove_header_entry(base);
}

// ============================================================================
//  CREDITS + TITLE SCREEN
// ============================================================================
static void credits_screen()
{
	static const char* const L[5][2]={
		{"Made by","velkryn"},
		{"Raycaster and base by","imtree248"},
		{"Coded by Claude","(Opus 5.5, Fable 5.1, Sonnet 5.5)"},
		{"Files hosted on","Github & Cemetech"},
		{"Tested by velkryn,","Z.ai's GLM 5.3 & GLM 5.3-Flash Models"}};
	wait_key_release();
	gfx_SetDrawBuffer();
	draw_dirt_bg();
	menu_text_setup();
	draw_logo("CREDITS",6,3);
	gfx_SetColor(COL_BLACK); gfx_FillRectangle_NoClip(8,38,304,172);
	gfx_SetColor(COL_GRAY_L); gfx_Rectangle_NoClip(8,38,304,172);
	gfx_SetColor(COL_GRAY_D); gfx_Rectangle_NoClip(10,40,300,168);
	for(int i=0;i<5;i++)
	{
		shadow_text_centered(L[i][0],45+i*25,COL_GRAY_L);
		shadow_text_centered(L[i][1],56+i*25,COL_YELLOW);
	}
	shadow_text_centered("---Weekend & World Build(s)",172,COL_GRAY_M);
	gfx_SetTextScale(2,2);
	shadow_text_centered("Enjoy!",188,COL_YELLOW);
	gfx_SetTextScale(1,1);
	shadow_text_centered("Press any key",223,COL_WHITE);
	gfx_SwapDraw();
	wait_any_key();
}
static const char* splash_texts[10]={
	"Polished!","Punch trees!","100% calculator!","Also try Minecraft!","48 MHz of power!",
	"Nine hotbar slots!","Furnaces face you!","Press Graph for a map!","Block outlines!","Mipmapped!"
};
static void draw_title_screen(int row,int col,int splash)
{
	draw_dirt_bg();
	menu_text_setup();
	int right=draw_logo("MINECALC",26,4);
	const char* sp=splash_texts[splash];
	int sx=right-(int)gfx_GetStringWidth(sp);
	if(sx<4) sx=4;
	shadow_text(sp,sx,62,COL_YELLOW);
	draw_mc_button(60,104,200,20,row==0,"Singleplayer");
	draw_mc_button(60,146,98,20,row==1&&col==0,"Options...");
	draw_mc_button(162,146,98,20,row==1&&col==1,"Quit Game");
	draw_mc_button(232,210,84,20,row==2,"Credits");
	menu_text_setup();
	shadow_text("MineCalc 0.4.0",2,230,COL_WHITE);
}
static int title_screen()//0 singleplayer, 1 options, 2 quit, 3 credits
{
	int row=0, col=0;
	int splash=randInt(0,9);
	key_reset();
	bool dirty=true;
	while(true)
	{
		key_update();
		if(key_pressed(kb_KeyUp)&&row>0){ row--; dirty=true; }
		if(key_pressed(kb_KeyDown)&&row<2){ row++; dirty=true; }
		if((key_pressed(kb_KeyLeft)||key_pressed(kb_KeyRight))&&row==1){ col=1-col; dirty=true; }
		if(key_pressed(kb_KeyClear)){ wait_key_release(); return 2; }
		if(key_pressed(kb_KeyEnter)||key_pressed(kb_Key2nd))
		{
			wait_key_release();
			if(row==0) return 0;
			if(row==2) return 3;
			return col==0?1:2;
		}
		if(dirty){ dirty=false; gfx_SetDrawBuffer(); draw_title_screen(row,col,splash); gfx_SwapDraw(); }
	}
}
static bool title_flow()
{
	while(true)
	{
		int t=title_screen();
		if(t==2) return false;
		if(t==1){ settings_menu(); continue; }
		if(t==3){ credits_screen(); continue; }
		const char* sopts[]={"Create New World","Open World","Delete World","Back"};
		int s=mc_menu_t("Singleplayer",sopts,4);
		if(s==0){ if(create_world()) return true; }
		else if(s==1){ if(open_world()) return true; }
		else if(s==2) delete_world_menu();
	}
}

// ============================================================================
//  GARBAGE COLLECT HOOKS + MAIN
// ============================================================================
void gc_before(void){ gfx_End(); }
void gc_after(void)
{
	gfx_Begin();
	gfx_SetDefaultPalette(gfx_8bpp);
	gfx_palette[1]=gfx_RGBTo1555(40,40,40);
	gfx_SetDrawBuffer();
	gfx_SetTextFGColor(0);
	gfx_SetTextBGColor(255);
	gfx_SetTextTransparentColor(26);
	MNCFTIMG_init();
	refresh_sprite_tables();
	updatehotbar=2;
}
int main(void)
{
	if(os_GetAppVarData("MNCFTIMG",NULL)==NULL)
	{
		os_ClrHome(); os_HomeUp();
		printf("Missing MNCFTIMG.8xv!\nSend the assets file to\nyour calculator.");
		os_GetKey();
		return 1;
	}
	world=(uint8_t*)malloc(WORLD_BYTES);
	if(!world)
	{
		os_ClrHome(); os_HomeUp();
		printf("Not enough RAM!\nFree up some space.");
		os_GetKey();
		return 1;
	}
	ti_SetGCBehavior(gc_before,gc_after);
	MNCFTIMG_init();
	refresh_sprite_tables();
	init_block_tables();
	build_shade_lut();
	build_textures();
	build_iso_map();
	build_visible_ids();
	for(int i=1;i<INV_N;i++) inv_abs[i]=262144/i;
	inv_abs[0]=2605056;//== (300000<<9) wrapped to 24 bits, same value the old signed table used
	rtc_Enable(0);
	srand(rtc_Time());
	gfx_Begin();
	gfx_SetDefaultPalette(gfx_8bpp);
	gfx_palette[1]=gfx_RGBTo1555(40,40,40);
	gfx_SetDrawBuffer();
	gfx_SetTextTransparentColor(26);
	load_settings();
	apply_tex_mode();
	reset_game_state();
	while(title_flow()) play_game();
	gfx_End();
	free(world);
	return 0;
}