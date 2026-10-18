// ============================================================================
//  src/dda_asm.cpp  -  eZ80 raycaster routines (replaces stuff.asm / stuffs.s)
//  The assembly is a file-scope asm block, so the normal [cc] rule builds it.
//  Only the routines the game still uses are here. Fog is computed in C now,
//  so the DDA returns just the face: 0 = x, 23 = y, 46 = z, 3 = miss.
// ============================================================================
#include <stdint.h>

extern "C" {
uint8_t scratch[3];            // scratch[2] = cell color (set from C via getscratch3)
uint8_t* rectaddr;             // VRAM address of the next 5x5 cell
uint8_t* dda_currblock;
int dda_deltax, dda_deltay, dda_deltaz;
int dda_tmaxx, dda_tmaxy, dda_tmaxz;
int runfill_n, runfill_advance, runfill_cnt;
uint8_t* getscratch3(){ return scratch+2; }
uint8_t** getrectaddr(){ return &rectaddr; }
int* getddadeltax(){ return &dda_deltax; }
int* getddadeltay(){ return &dda_deltay; }
int* getddadeltaz(){ return &dda_deltaz; }
uint8_t** getddacurrblock(){ return &dda_currblock; }
int* getddatmaxx(){ return &dda_tmaxx; }
int* getddatmaxy(){ return &dda_tmaxy; }
int* getddatmaxz(){ return &dda_tmaxz; }
}

__asm__(R"ASM(
	.assume adl=1
	.text
	.global _ASM_DDA_FULL
	.global _optomized_raycast_rect_5x5_no2
	.global _optomized_raycast_rect_run

	.macro ASM_DDA_STEP SX, SY, SZ
	or a, a
	sbc hl, bc
	jp p, 1f
	add hl, bc
	or a, a
	sbc hl, de
	jp p, 2f
	add hl, de
	push de
	ld de, \SX
	add ix, de
	ld de, (_dda_deltax)
	add hl, de
	pop de
	cp a, (ix + 0)
	jp nz, dda_end_x
	jp 4f
2:
	add hl, de
	push bc
	lea ix, ix + \SZ
	ex de, hl
	ld bc, (_dda_deltaz)
	add hl, bc
	ex de, hl
	pop bc
	cp a, (ix + 0)
	jp nz, dda_end_z
	jp 4f
1:
	add hl, bc
	ex de, hl
	or a, a
	sbc hl, bc
	jp m, 3f
	add hl, bc
	ex de, hl
	push hl
	ld h, b
	ld l, c
	lea ix, ix + \SY
	ld bc, (_dda_deltay)
	add hl, bc
	ld b, h
	ld c, l
	pop hl
	cp a, (ix + 0)
	jp nz, dda_end_y
	jp 4f
3:
	add hl, bc
	push bc
	lea ix, ix + \SZ
	ld bc, (_dda_deltaz)
	add hl, bc
	ex de, hl
	pop bc
	cp a, (ix + 0)
	jp nz, dda_end_z
4:
	.endm

_ASM_DDA_FULL:
	push ix
	ld ix, 0
	add ix, sp
	ld hl, 0
	ld l, (ix + 6)
	add hl, hl
	add hl, hl
	ld de, dda_jump_table
	add hl, de
	ld ix, (_dda_currblock)
	ld bc, (_dda_tmaxy)
	ld de, (_dda_tmaxz)
	xor a, a
	jp (hl)

dda_end_x:
	xor a, a
	jp dda_exit
dda_end_y:
	ld a, 23
	jp dda_exit
dda_miss:
	ld a, 3
	jp dda_exit
dda_end_z:
	ld a, 46
dda_exit:
	ld (_dda_tmaxx), hl
	ld (_dda_tmaxy), bc
	ld (_dda_tmaxz), de
	ld (_dda_currblock), ix
	pop ix
	ret

dda_jump_table:
	jp dda_q0
	jp dda_q1
	jp dda_q2
	jp dda_q3
	jp dda_q4
	jp dda_q5
	jp dda_q6
	jp dda_q7

dda_q0:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP -1024, -32, -1
	.endr
	jp dda_miss
dda_q1:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP 1024, -32, -1
	.endr
	jp dda_miss
dda_q2:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP -1024, 32, -1
	.endr
	jp dda_miss
dda_q3:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP 1024, 32, -1
	.endr
	jp dda_miss
dda_q4:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP -1024, -32, 1
	.endr
	jp dda_miss
dda_q5:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP 1024, -32, 1
	.endr
	jp dda_miss
dda_q6:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP -1024, 32, 1
	.endr
	jp dda_miss
dda_q7:
	ld hl, (_dda_tmaxx)
	.rept 15
	ASM_DDA_STEP 1024, 32, 1
	.endr
	jp dda_miss

	.macro ROW5
	ld (ix + 0), de
	ld (ix + 2), de
	add ix, bc
	.endm

_optomized_raycast_rect_5x5_no2:
	push ix
	ld ix, (_rectaddr)
	ld a, (_scratch + 2)
	ld de, (_scratch)
	ld d, a
	ld e, a
	ld bc, 320
	ROW5
	ROW5
	ROW5
	ROW5
	ld (ix + 0), de
	ld (ix + 2), de
	ld ix, (_rectaddr)
	lea ix, ix + 5
	ld (_rectaddr), ix
	pop ix
	ret

	.macro RUN_ROW
	ld de, (_scratch)
	ld (ix + 0), de
	ld bc, (_runfill_cnt)
	push ix
	pop hl
	lea de, ix + 3
	ldir
	ld bc, 320
	add ix, bc
	.endm

_optomized_raycast_rect_run:
	push ix
	ld hl, (_runfill_n)
	add hl, hl
	add hl, hl
	ld bc, (_runfill_n)
	add hl, bc
	dec hl
	dec hl
	dec hl
	ld (_runfill_cnt), hl
	ld a, (_scratch + 2)
	ld (_scratch + 1), a
	ld (_scratch), a
	ld ix, (_rectaddr)
	RUN_ROW
	RUN_ROW
	RUN_ROW
	RUN_ROW
	RUN_ROW
	ld hl, (_runfill_advance)
	ld de, (_rectaddr)
	add hl, de
	ld (_rectaddr), hl
	pop ix
	ret
)ASM");