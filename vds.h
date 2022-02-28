/*
 * VBMouse - Interface to the Virtual DMA Service (VDS)
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

#ifndef VDS_H
#define VDS_H

#include <stdbool.h>
#include <stdint.h>

typedef unsigned char vdserr;
enum {
	VDS_SUCCESSFUL = 0,
	VDS_REGION_NOT_CONTIGUOUS = 1,
	VDS_REGION_NOT_ALIGNED    = 2,
	VDS_UNABLE_TO_LOCK        = 3,
	VDS_NO_BUFFER_AVAIL       = 4,
	VDS_REGION_TOO_LARGE      = 5,
	VDS_BUFFER_IN_USE         = 6,
	VDS_INVALID_REGION        = 7,
	VDS_REGION_NOT_LOCKED     = 8,
	VDS_TOO_MANY_PAGES        = 9,
	VDS_INVALID_BUFFER_ID     = 0xA,
	VDS_BUFFER_BOUNDARY       = 0xB,
	VDS_INVALID_DMA_CHANNEL   = 0xC,
	VDS_DISABLE_COUNT_OVRFLOW = 0xD,
	VDS_DISABLE_COUNT_UNDFLOW = 0xE,
	VDS_NOT_SUPPORTED         = 0xF,
	VDS_FLAGS_NOT_SUPPORTED   = 0x10,
};

enum {
/*
   Bit 1 = Automatically copy to/from buffer
   Bit 2 = Disable automatic buffer allocation
   Bit 3 = Disable automatic remap feature
   Bit 4 = Region must not cross 64K physical alignment boundary
   Bit 5 = Region must not cross 128K physical alignment boundary
   Bit 6 = Copy page-table for scatter/gather remap
   Bit 7 = Allow non-present pages for scatter/gather remap
*/
	VDS_AUTO_COPY_DATA = 1 << 1,
	VDS_NO_AUTO_ALLOC  = 1 << 2,
	VDS_NO_AUTO_REMAP  = 1 << 3,
	VDS_ALIGN_64K      = 1 << 4,
	VDS_ALIGN_128K     = 1 << 5
};

/** DMA Descriptor structure. Describes a potentially DMA-lockable buffer. */
typedef _Packed struct VDS_DDS
{
	/** Size of this buffer. */
	uint32_t regionSize;
	/** Logical/segmented address of this buffer, offset part */
	uint32_t offset;
	/** Segment of this buffer. */
	uint16_t segOrSelector;
	/** Internal VDS buffer ID. */
	uint16_t bufferId;
	/** Physical address of this buffer. */
	uint32_t physicalAddress;
} VDS_DDS;

static bool vds_available(void);
#pragma aux vds_available = \
	"mov ax, 0x40" \
	"mov es, ax" \
	"mov al, es:[0x7B]" \
	"and al, 0x20" \
	"setnz al" \
	__value [al] \
	__modify [ax es]

/** Locks an already allocated buffer into a physical memory location.
  * regionSize, offset and segment must be valid in the DDS,
  * while the physical address is returned. */
static vdserr vds_lock_dma_buffer_region(VDS_DDS __far * dds, unsigned char flags);
#pragma aux vds_lock_dma_buffer_region = \
	"stc"              /* If nothing happens, assume failure. */ \
	"mov ax, 0x8103" \
	"int 0x4B" \
	"jc fail" \
	"mov ah, 0" \
	"jmp end" \
	"fail: test ah, ah" \
	"jnz end" \
	"mov ah, 0xFF"       /* Force a error code if there was none. */ \
	"end:" \
	__parm [es di] [dx] \
	__value [ah] \
	__modify [ax]

/** Unlocks a locked buffer. */
static vdserr vds_unlock_dma_buffer_region(VDS_DDS __far * dds, unsigned char flags);
#pragma aux vds_unlock_dma_buffer_region = \
	"stc" \
	"mov ax, 0x8104" \
	"int 0x4B" \
	"jc fail" \
	"mov ah, 0" \
	"jmp end" \
	"fail: test ah, ah" \
	"jnz end" \
	"mov ah, 0xFF" \
	"end:" \
	__parm [es di] [dx] \
	__value [ah] \
	__modify [ax]

#endif
