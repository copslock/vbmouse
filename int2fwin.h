/*
 * VBMouse - Interface to the Windows int 2Fh services
 * Copyright (C) 2022 Javier S. Pedro
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef INT2FWIN_H
#define INT2FWIN_H

#include <stdbool.h>
#include <stdint.h>
#include <dos.h>

typedef void (__far *LPFN)(void);

enum int2f_functions
{
	INT2F_NOTIFY_BACKGROUND_SWITCH = 0x4001,
	INT2F_NOTIFY_FOREGROUND_SWITCH = 0x4002
};

enum vxd_device_ids
{
	VMD_DEVICE_ID = 0xC
};

static bool windows_386_enhanced_mode(void);
#pragma aux windows_386_enhanced_mode = \
	"mov ax, 0x1600" \
	"int 0x2F" \
	"test al, 0x7F" /* return value is either 0x80 or 0x00 if win386 is not running */ \
	"setnz al" \
	__value [al] \
	__modify [ax]

static LPFN win_get_vxd_api_entry(uint16_t devid);
#pragma aux win_get_vxd_api_entry = \
	"mov ax, 0x1684" /* Get Device Entry Point Address */  \
	"int 0x2F" \
	__parm [bx] \
	__value [es di] \
	__modify [ax]

/* VMD (Virtual Mouse Device) API */

enum vmd_apis {
	VMD_GET_VERSION    = 0x0,
	VMD_SET_MOUSE_TYPE = 0x100,
};

enum vmd_mouse_type {
	VMD_TYPE_UNDEFINED = 0,
	VMD_TYPE_BUS       = 1,
	VMD_TYPE_SERIAL    = 2,
	VMD_TYPE_INPORT    = 3,
	VMD_TYPE_PS2       = 4,
	VMD_TYPE_HP        = 5
};

static inline int vmd_get_version(LPFN *vmd_entry);
#pragma aux vmd_get_version = \
	"mov ax, 0x000" \
	"call dword ptr es:[di]" \
	__parm [es di] \
	__value [ax]

static inline bool vmd_set_mouse_type(LPFN *vmd_entry, uint8_t mousetype, int8_t interrupt, int8_t com_port);
#pragma aux vmd_set_mouse_type = \
	"stc"                    /* If nothing happens, assume failure */ \
	"mov ax, 0x100" \
	"call dword ptr es:[di]" \
	"setnc al" \
	__parm [es di] [bl] [bh] [cl] \
	__value [al] \
	__modify [ax]

/* Miscelaneous helpers. */

static inline void hook_int2f(LPFN *prev, LPFN new)
{
	*prev = _dos_getvect(0x2F);
	_dos_setvect(0x2F, new);
}

static inline void unhook_int2f(LPFN prev)
{
	_dos_setvect(0x2F, prev);
}

#endif
