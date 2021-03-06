/*
 * BLGRPFNT
 */

#include <stddef.h>
#include <stdio.h>
/*#include <string.h>*/
#include <io.h>
#include <msxbdos.h>
#include <blstd.h>
#include <blstdvdp.h>
#include <blgrp.h>
#include <blgrpcmd.h>
#include <blgrpfnt.h>
#include <blgrpdat.h>

#ifdef NO_BLGRPFNT
void bl_grp_fnt_init(void)
{
}

void bl_grp_set_font_size(uint8_t w, uint8_t h)
{
	w = h;
}

void bl_grp_set_font_color(uint8_t fg, uint8_t bg)
{
	fg = bg;
}

void bl_grp_setup_font_draw_func(void)
{
}

#else

uint8_t font_8x8_org[] = {
#include "font_e.h"
};

uint8_t font_8x8_center[] = {
/*#include "font_e.h"*/
0,
};

uint8_t font_8x8_bold[] = {
/*#include "font_e.h"*/
0,
};

uint8_t font_8x8_wide[] = {
/*#include "font_e.h"*/
0,
};

void draw_font_null(uint8_t *font)
{
}

uint8_t font_text_mode;
uint8_t font_width_byte;
uint8_t font_8x8_init = 0;
extern uint8_t *font_8x8;
void (*font_draw_func)(uint8_t *font) = draw_font_null;

void (*font_draw_func_table[2][10])(uint8_t *font) = {
	/* non-interlace mode */
	{
		draw_font_null,			/* T1 */
		draw_font_null,			/* T2 */
		bl_draw_font_mc,		/* MC */
		draw_font_null,			/* G1 */
		draw_font_null,			/* G2 */
		draw_font_null,			/* G3 */
		bl_draw_font_g4,		/* G4 */
		bl_draw_font_g5,		/* G5 */
		bl_draw_font_g6,		/* G6 */
		bl_draw_font_g7			/* G7 */
	},
	/* interlace mode */
	{
		draw_font_null,			/* T1 */
		draw_font_null,			/* T2 */
		bl_draw_font_mc,		/* MC */
		draw_font_null,			/* G1 */
		draw_font_null,			/* G2 */
		draw_font_null,			/* G3 */
		bl_draw_font_g4i,		/* G4 */
		bl_draw_font_g5i,		/* G5 */
		bl_draw_font_g6i,		/* G6 */
		bl_draw_font_g7i		/* G7 */
	}
};

void bl_grp_fnt_init(void)
{
#if 0
	uint16_t n;
	uint8_t *org, *center, *bold, *wide;
#endif
	font_8x8 = font_8x8_org;
#if 0
	if (!font_8x8_init) {
		font_8x8_init = 1;			/* init once! */
		org = font_8x8_org;
		center = font_8x8_center;
		bold = font_8x8_bold;
		wide = font_8x8_wide;
		for (n = 0; n < sizeof(font_8x8_org); n++) {
			*center >>= 1;
			*bold = *center | (*center >> 1);
			*wide = (*center & 0xF0) | ((*center & 0x1E) >> 1);

			org++;
			center++;
			bold++;
			wide++;
		}
	}
#endif
}

void bl_grp_load_font(char *filename)
{
	uint8_t fh;

	fh = open(filename, O_RDONLY);
	if (fh == 0xFF)
		return;

	read(fh, font_8x8_org, 2048);

	close(fh);
}

static void bl_grp_copy_font_to_pattern_gen(uint16_t addr)
{
	bl_vdp_vram_h = (uint8_t)(addr >> 14);
/*	bl_vdp_vram_h |= bl_grp.active_page_a16_a14;*/
	bl_vdp_vram_m = (uint8_t)((addr >> 8) & 0x3F);
	bl_vdp_vram_l = (uint8_t)addr;
	bl_vdp_vram_cnt = 2048;
	bl_copy_to_vram_nn(font_8x8);
}

void bl_grp_setup_text_font(void)
{
	uint16_t vram_addr = bl_grp.pattern_gen_addr;

	switch (bl_grp.screen_mode) {
	case GRP_SCR_T1:		/* only for Pattern based mode */
	case GRP_SCR_T2:
	case GRP_SCR_G1:
		bl_grp_copy_font_to_pattern_gen(vram_addr);
		break;
	case GRP_SCR_G2:
	case GRP_SCR_G3:
		bl_grp_copy_font_to_pattern_gen(vram_addr);
		vram_addr += 2048;
		bl_grp_copy_font_to_pattern_gen(vram_addr);
		vram_addr += 2048;
		bl_grp_copy_font_to_pattern_gen(vram_addr);
		break;
	default:
		break;
	}
}

void bl_grp_setup_font_draw_func(void)
{
	font_draw_func = font_draw_func_table[bl_grp.interlace_on][bl_grp.screen_mode];

	switch (bl_grp.screen_mode) {
	case GRP_SCR_T1:
	case GRP_SCR_T2:
	case GRP_SCR_G1:
	case GRP_SCR_G2:
	case GRP_SCR_G3:
		font_text_mode = 1;
		break;
	default:
		font_text_mode = 0;
		break;
	}

#asm
	; Setup function pointer
	ld hl,(_font_draw_func)
	ld (_bl_grp_print_chr_addr + 1),hl
	ld (_bl_grp_print_bitmap_addr + 1),hl
#endasm
}

void bl_grp_set_font_style(uint8_t style)
{
	switch (style) {
	case GRP_FONT_CENTER:
		font_8x8 = font_8x8_center;
		break;
	case GRP_FONT_BOLD:
		font_8x8 = font_8x8_bold;
		break;
	case GRP_FONT_WIDE:
		font_8x8 = font_8x8_wide;
		break;
	case GRP_FONT_ORGIN:
	default:
		font_8x8 = font_8x8_org;
		break;
	}
}

void bl_grp_set_font_size(uint8_t w, uint8_t h)
{
	bl_grp.font_width = w & 0xFE;		/* even */
	bl_grp.font_height= h;
	bl_draw_font_w = bl_grp.font_width;
	bl_draw_font_h = bl_grp.font_height;
	font_width_byte = (bl_grp.font_width) >> (bl_grp.bpp_shift);

	switch (bl_grp.screen_mode) {
	case GRP_SCR_T1:
		bl_grp.text_width = 40;
		break;
	case GRP_SCR_T2:
		bl_grp.text_width = 80;
		break;
	case GRP_SCR_G1:
	case GRP_SCR_G2:
	case GRP_SCR_G3:
		bl_grp.text_width = 32;
		break;
	case GRP_SCR_MC:
		bl_grp.text_width = (uint8_t)(64 / bl_grp.font_width);
		break;
	case GRP_SCR_G5:
	case GRP_SCR_G6:
		bl_grp.text_width = (uint8_t)(512 / bl_grp.font_width);
		break;
	case GRP_SCR_G4:
	case GRP_SCR_G7:
	default:
		bl_grp.text_width = (uint8_t)(256 / bl_grp.font_width);
		break;
	}
}

void bl_grp_set_font_color(uint8_t fg, uint8_t bg)
{
	bl_draw_font_fgc = bl_grp.font_fgc = fg;
	bl_draw_font_bgc = bl_grp.font_bgc = bg;

	/* pre-calculation for G4,G6 */
	bl_draw_font_g4c();
	bl_draw_font_g6c();
}

static uint16_t vram_faddr;
void bl_grp_print_pos(uint16_t x, uint16_t y)
{
	if (font_text_mode) {
		vram_faddr = bl_grp.pattern_name_addr;
		vram_faddr += y * bl_grp.row_byte + x;
		bl_vdp_vram_h = 0;
	} else {
		if (bl_grp.screen_mode == GRP_SCR_MC) {	/* MC */
			vram_faddr = bl_grp.pattern_gen_addr;
			vram_faddr += (y & 0xF8) << 5;		/* (y / 8) * 256 */
			vram_faddr += y & 0x07;
			vram_faddr += (x & 0xFE) << 2;		/* (x / 2) * 8 */
		} else {
			if (bl_grp.interlace_on)
				y >>= 1;

			vram_faddr = y * bl_grp.row_byte;
			vram_faddr += x >> (bl_grp.bpp_shift);
		}

		bl_vdp_vram_h = (uint8_t)(vram_faddr >> 14);
		bl_vdp_vram_h |= bl_grp.active_page_a16_a14;
	}

	bl_vdp_vram_m = (uint8_t)((vram_faddr >> 8) & 0x3F);
	bl_vdp_vram_l = (uint8_t)vram_faddr;
}

#asm
;void bl_grp_print_str(char *str)
	global	_bl_grp_print_str
	global	_bl_write_vram
	global	_font_8x8

_bl_grp_print_str:
	pop bc			; return addr
	pop de			; char *str
	push de
	push bc

	ld a,(_font_text_mode)
	and a
	jp z,_bl_grp_print_bitmap

_bl_grp_print_pattern:		; for T0, T1, G1, G2, G3
	ld a,(de)
	and a
	ret z			; string end?

	ld l,a
	call _bl_write_vram

_bl_grp_print_pattern_lp:
	inc de
	ld a,(de)
	and a
	ret z			; string end?
	out (098h),a		; write vram
	jp _bl_grp_print_pattern_lp

_bl_grp_print_bitmap:		; for bitmap graphic
	ld a,(de)
	and a
	ret z			; string end?
	push de			; backup str

	ld h,0
	ld l,a
	add hl,hl
	add hl,hl
	add hl,hl		; font idx = (uint16_t)(*str) * 8

	defb 001h		; ld bc,NN
_font_8x8:			; uint8_t *font_8x8;
	defw 0
	add hl,bc		; hl = font addr

_bl_grp_print_bitmap_addr:
	call 00000h		; call (_font_draw_func)
	pop de			; restore str
	inc de			; str++
	jp _bl_grp_print_bitmap
#endasm

/* C version
void bl_grp_print_str(char *str)
{
	while (*str) {
		fcode_idx = (uint16_t)(*str) << 3;
		font_draw_func(font_8x8 + fcode_idx);
		str++;
	}
}
*/

void bl_grp_print(uint16_t x, uint16_t y, char *str)
{
	bl_grp_print_pos(x, y);
	bl_grp_print_str(str);
}

#asm
;void bl_grp_print_chr(char chr)
	global	_bl_grp_print_chr

_bl_grp_print_chr:
	pop de			; return addr
	pop bc			; char chr
	push bc
	push de

	ld a,c			; char chr
	ld b,000h
	rla
	rl b
	rla
	rl b
	rla
	rl b
	and 0f8h
	ld c,a			; font idx = (uint16_t)(chr) << 3

	ld hl,(_font_8x8)
	add hl,bc		; hl = font addr
_bl_grp_print_chr_addr:
	jp 00000h		; jp (_font_draw_func)
#endasm

/* C version
void bl_grp_print_chr(char chr)
{
	fcode_idx = (uint16_t)chr << 3;
	font_draw_func(font_8x8 + fcode_idx);
}
*/

void bl_grp_print_cursor(void)
{
	uint16_t vram_faddr_bak = vram_faddr;
	uint8_t bl_vdp_vram_l_bak = bl_vdp_vram_l;

	bl_grp_print_chr(0xDB);

	vram_faddr = vram_faddr_bak;
	bl_vdp_vram_l = bl_vdp_vram_l_bak;
}

void bl_grp_print_backspace(void)
{
	uint8_t addr_delta = font_width_byte << 1;

	bl_grp_print_chr(0x20);

	vram_faddr -= addr_delta;
	bl_vdp_vram_l -= addr_delta;
}

void bl_grp_print_back(char count)
{
	uint8_t addr_delta = font_width_byte * count;

	vram_faddr -= addr_delta;
	bl_vdp_vram_l -= addr_delta;
}
#endif
;
