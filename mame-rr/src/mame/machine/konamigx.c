/**************************************************************************
 *
 * machine/konamigx.c - contains various System GX hardware abstractions
 *
 */

#include "emu.h"
#include "video/konamiic.h"
#include "includes/konamigx.h"

/***************************************************************************/
/*                                                                         */
/*                           GX/MW Protections                             */
/*                                                                         */
/***************************************************************************/

// K055550/K053990 protection chips, perform simple memset() and other game logic operations
static UINT16 prot_data[0x20];

static WRITE32_HANDLER(fantjour_dma_w);

READ16_HANDLER( K055550_word_r )
{
	return(prot_data[offset]);
}

WRITE16_HANDLER( K055550_word_w )
{
	UINT32 adr, bsize, count, i, lim;
	int src, tgt, srcend, tgtend, skip, cx1, sx1, wx1, cy1, sy1, wy1, cz1, sz1, wz1, c2, s2, w2;
	int dx, dy, angle;

	COMBINE_DATA(prot_data+offset);

	if (offset == 0 && ACCESSING_BITS_8_15)
	{
		data >>= 8;
		switch (data)
		{
			case 0x97: // memset() (Dadandarn at 0x639dc)
			case 0x9f: // memset() (Violent Storm at 0x989c)
				adr   = (prot_data[7] << 16) | prot_data[8];
				bsize = (prot_data[10] << 16) | prot_data[11];
				count = (prot_data[0] & 0xff) + 1;

				lim = adr+bsize*count;
				for(i=adr; i<lim; i+=2)
					memory_write_word(space, i, prot_data[0x1a/2]);
			break;

			// WARNING: The following cases are speculation based with questionable accuracy!(AAT)

			case 0x87: // unknown memory write (Violent Storm at 0x00b6ea)
				// Violent Storm writes the following data to the 55550 in mode 0x87.
				// All values are hardcoded and the write happens each frame during
				// gameplay. It refers to a 32x8-word list at 0x210e00 and seems to
				// be tied with another 13x128-byte table at 0x205080.
				// Both tables appear "check-only" and have little effect on gameplay.
				count =(prot_data[0] & 0xff) + 1;          // unknown ( byte 0x00)
				i     = prot_data[1];                      // unknown ( byte 0x1f)
				adr   = prot_data[7]<<16 | prot_data[8];   // address (dword 0x210e00)
				lim   = prot_data[9];                      // unknown ( word 0x0010)
				src   = prot_data[10]<<16 | prot_data[11]; // unknown (dword zero)
				tgt   = prot_data[12]<<16 | prot_data[13]; // unknown (dword zero)
			break;

			case 0xa0: // update collision detection table (Violent Storm at 0x018b42)
				count = prot_data[0] & 0xff;             // number of objects - 1
				skip  = prot_data[1]>>(8-1);             // words to skip in each entry to reach the "hit list"
				adr   = prot_data[2]<<16 | prot_data[3]; // where the table is located
				bsize = prot_data[5]<<16 | prot_data[6]; // object entry size in bytes

				srcend = adr + bsize * count;
				tgtend = srcend + bsize;

				// let's hope GCC will inline the mem24bew calls
				for (src=adr; src<srcend; src+=bsize)
				{
					cx1 = (short)memory_read_word(space, src);
					sx1 = (short)memory_read_word(space, src + 2);
					wx1 = (short)memory_read_word(space, src + 4);

					cy1 = (short)memory_read_word(space, src + 6);
					sy1 = (short)memory_read_word(space, src + 8);
					wy1 = (short)memory_read_word(space, src +10);

					cz1 = (short)memory_read_word(space, src +12);
					sz1 = (short)memory_read_word(space, src +14);
					wz1 = (short)memory_read_word(space, src +16);

					count = i = src + skip;
					tgt = src + bsize;

					for (; count<tgt; count++) memory_write_byte(space, count, 0);

					for (; tgt<tgtend; i++, tgt+=bsize)
					{
						c2 = (short)memory_read_word(space, tgt);
						s2 = (short)memory_read_word(space, tgt + 2);
						w2 = (short)memory_read_word(space, tgt + 4);
						if (abs((cx1+sx1)-(c2+s2))>=wx1+w2) continue; // X rejection

						c2 = (short)memory_read_word(space, tgt + 6);
						s2 = (short)memory_read_word(space, tgt + 8);
						w2 = (short)memory_read_word(space, tgt +10);
						if (abs((cy1+sy1)-(c2+s2))>=wy1+w2) continue; // Y rejection

						c2 = (short)memory_read_word(space, tgt +12);
						s2 = (short)memory_read_word(space, tgt +14);
						w2 = (short)memory_read_word(space, tgt +16);
						if (abs((cz1+sz1)-(c2+s2))>=wz1+w2) continue; // Z rejection

						memory_write_byte(space, i, 0x80); // collision confirmed
					}
				}
			break;

			case 0xc0: // calculate object "homes-in" vector (Violent Storm at 0x03da9e)
				dx = (short)prot_data[0xc];
				dy = (short)prot_data[0xd];

				// it's not necessary to use lookup tables because Violent Storm
				// only calls the service once per enemy per frame.
				if (dx)
				{
					if (dy)
					{
						angle = (atan((double)dy / dx) * 128.0) / M_PI;
						if (dx < 0) angle += 128;
						i = (angle - 0x40) & 0xff;
					}
					else
						i = (dx > 0) ? 0xc0 : 0x40;
				}
				else
					if (dy > 0) i = 0;
				else
					if (dy < 0) i = 0x80;
				else
					i = mame_rand(space->machine) & 0xff; // vector direction indeterminate

				prot_data[0x10] = i;
			break;

			default:
//              logerror("%06x: unknown K055550 command %02x\n", cpu_get_pc(space->cpu), data);
			break;
		}
	}
}

WRITE16_HANDLER( K053990_martchmp_word_w )
{
	int src_addr, src_count, src_skip;
	int dst_addr, /*dst_count,*/ dst_skip;
	int mod_addr, mod_count, mod_skip, mod_offs;
	int mode, i, element_size = 1;
	UINT16 mod_val, mod_data;

	COMBINE_DATA(prot_data+offset);

	if (offset == 0x0c && ACCESSING_BITS_8_15)
	{
		mode  = (prot_data[0x0d]<<8 & 0xff00) | (prot_data[0x0f] & 0xff);

		switch (mode)
		{
			case 0xffff: // word copy
				element_size = 2;
			case 0xff00: // byte copy
				src_addr  = prot_data[0x0];
				src_addr |= prot_data[0x1]<<16 & 0xff0000;
				dst_addr  = prot_data[0x2];
				dst_addr |= prot_data[0x3]<<16 & 0xff0000;
				src_count = prot_data[0x8]>>8;
				//dst_count = prot_data[0x9]>>8;
				src_skip  = prot_data[0xa] & 0xff;
				dst_skip  = prot_data[0xb] & 0xff;

				if ((prot_data[0x8] & 0xff) == 2) src_count <<= 1;
				src_skip += element_size;
				dst_skip += element_size;

				if (element_size == 1)
				for (i=src_count; i; i--)
				{
					memory_write_byte(space, dst_addr, memory_read_byte(space, src_addr));
					src_addr += src_skip;
					dst_addr += dst_skip;
				}
				else for (i=src_count; i; i--)
				{
					memory_write_word(space, dst_addr, memory_read_word(space, src_addr));
					src_addr += src_skip;
					dst_addr += dst_skip;
				}
			break;

			case 0x00ff: // sprite list modifier
				src_addr  = prot_data[0x0];
				src_addr |= prot_data[0x1]<<16 & 0xff0000;
				src_skip  = prot_data[0x1]>>8;
				dst_addr  = prot_data[0x2];
				dst_addr |= prot_data[0x3]<<16 & 0xff0000;
				dst_skip  = prot_data[0x3]>>8;
				mod_addr  = prot_data[0x4];
				mod_addr |= prot_data[0x5]<<16 & 0xff0000;
				mod_skip  = prot_data[0x5]>>8;
				mod_offs  = prot_data[0x8] & 0xff;
				mod_offs<<= 1;
				mod_count = 0x100;

				src_addr += mod_offs;
				dst_addr += mod_offs;

				for (i=mod_count; i; i--)
				{
					mod_val  = memory_read_word(space, mod_addr);
					mod_addr += mod_skip;

					mod_data = memory_read_word(space, src_addr);
					src_addr += src_skip;

					mod_data += mod_val;

					memory_write_word(space, dst_addr, mod_data);
					dst_addr += dst_skip;
				}
			break;

			default:
			break;
		}
	}
}

void konamigx_esc_alert(UINT32 *srcbase, int srcoffs, int count, int mode) // (WARNING: assumed big endianess)
{

// hand-filled but should be close
static const UINT8 ztable[7][8] =
{
	{5,4,3,2,1,7,6,0},
	{4,3,2,1,0,7,6,5},
	{4,3,2,1,0,7,6,5},
	{3,2,1,0,5,7,4,6},
	{6,5,1,4,3,7,0,2},
	{5,4,3,2,1,7,6,0},
	{5,4,3,2,1,7,6,0}
};

static const UINT8 ptable[7][8] =
{
	{0x00,0x00,0x00,0x10,0x20,0x00,0x00,0x30},
	{0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x20,0x20,0x00,0x00,0x00},
	{0x10,0x10,0x10,0x20,0x00,0x00,0x10,0x00},
	{0x00,0x00,0x20,0x00,0x10,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x10},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10}
};

	INT32 data1, data2, i, j, vpos, hpos, voffs, hoffs, vcorr, hcorr, vmask, hmask, magicid;
	UINT32 *src, *srcend, *obj, *objend;
	UINT16 *dst;
	const UINT8  *zcode, *pcode;

	if (!count || !srcbase) return;

	if (mode == 0)
	{
		src = srcbase + srcoffs;
		dst = K053247_ram;
		data1 = count<<2;
		data2 = count<<3;
		src += data1; dst += data2; i = -data1; j = -data2;
		do
		{
			data1 = src[i];
			data2 = src[i+1];
			i += 2;
			dst[j+1] = data1;
			dst[j+3] = data2;
			data1  >>= 16;
			data2  >>= 16;
			dst[j]   = data1;
			dst[j+2] = data2;
		}
		while (j += 4);
	}
	else
	{

#define EXTRACT_ODD         \
if((data1=obj[0])&0x8000)   \
{                           \
  i      = data1 & 7;       \
  data1 &= 0xff00;          \
  dst[0] = data1 | zcode[i];\
  data1  = obj[1];          \
  dst[1] = data1>>16;       \
  vpos   = data1 & 0xffff;  \
  data1  = obj[2];          \
  vpos  += voffs;           \
  dst[4] = data1;           \
  vpos  &= vmask;           \
  hpos   = data1>>16;       \
  data1  = obj[3];          \
  hpos  += hoffs;           \
  dst[2] = vpos;            \
  dst[3] = hpos;            \
  dst[5] = data1>>16;       \
  i      = pcode[i];        \
  dst[6] = data1| i<<4;     \
  dst += 8;                 \
  if (!(--j)) return;       \
}

#define EXTRACT_EVEN         \
if((data1=obj[0])&0x80000000)\
{                            \
  dst[1] = data1;            \
  data1>>= 16;               \
  i      = data1 & 7;        \
  data1 &= 0xff00;           \
  dst[0] = data1 | zcode[i]; \
  data1  = obj[1];           \
  hpos   = data1 & 0xffff;   \
  vpos   = data1>>16;        \
  hpos  += hoffs;            \
  vpos  += voffs;            \
  data1  = obj[2];           \
  vpos  &= vmask;            \
  dst[3] = hpos;             \
  dst[2] = vpos;             \
  dst[5] = data1;            \
  dst[4] = data1>>16;        \
  data1  = obj[3]>>16;       \
  i      = pcode[i];         \
  dst[6] = data1 | i<<4;     \
  dst += 8;                  \
  if (!(--j)) return;        \
}

		// These suspecious looking flags might tell the ESC chip what zcode/priority combos to use.
		// At the beginning of each sprite chunk there're at least three pointers to the main ROM but
		// I can't make out anything meaningful.
		magicid = srcbase[0x71f0/4];

		hmask = vmask = 0x3ff;
		if (magicid != 0x11010111)
		{
			switch (magicid)
			{
				case 0x10010801: i = 6; break;
				case 0x11010010: i = 5; vmask = 0x1ff; break;
				case 0x01111018: i = 4; break;
				case 0x10010011: i = 3;
					if ((srcbase[0x1c75]&0xff)==32) K055555_write_reg(K55_BLEND_ENABLES,36); // (TEMPORARY)
				break;
				case 0x11010811: i = 2; break;
				case 0x10000010: i = 1; break;
				default:         i = 0;
			}
			vcorr = srcbase[0x26a0/4] & 0xffff;
			hcorr = srcbase[0x26a4/4] >> 16;
			hcorr -= 10;
		}
		else
			hcorr = vcorr = i = 0;

		zcode = ztable[i];
		pcode = ptable[i];

		dst = K053247_ram;
		j = 256;

		// decode Vic-Viper
		if (srcbase[0x049c/4] & 0xffff0000)
		{
			hoffs = srcbase[0x0502/4] & 0xffff;
			voffs = srcbase[0x0506/4] & 0xffff;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x049e/4];
			EXTRACT_ODD
			obj = &srcbase[0x04ae/4];
			EXTRACT_ODD
			obj = &srcbase[0x04be/4];
			EXTRACT_ODD
		}

		// decode Lord British (the designer must be a Richard Garriot fan too:)
		if (srcbase[0x0848/4] & 0x0000ffff)
		{
			hoffs = srcbase[0x08b0/4]>>16;
			voffs = srcbase[0x08b4/4]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x084c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x085c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x086c/4];
			EXTRACT_EVEN
		}

		// decode common sprites
		src = srcbase + srcoffs;
		srcend = src + count * 0x30;
		do
		{
			if (!src[0]) continue;
			i = src[7] & 0xf;
			if (!i) continue; // reject retired or zero-element groups
			i <<= 2;
			hoffs = src[5]>>16;
			voffs = src[6]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = src + 8;
			objend = obj + i;

			do
			{
				EXTRACT_EVEN
			}
			while ((obj+=4)<objend);
		}
		while ((src+=0x30)<srcend);

		// clear residual data
		if (j) do { *dst = 0; dst += 8; } while (--j);
	}

#undef EXTRACT_ODD
#undef EXTRACT_EVEN
}

static UINT32 fantjour_dma[8];

void fantjour_dma_install(running_machine *machine)
{
	state_save_register_global_array(machine, fantjour_dma);
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xdb0000, 0xdb001f, 0, 0, fantjour_dma_w);
	memset(fantjour_dma, 0, sizeof(fantjour_dma));
}

static WRITE32_HANDLER(fantjour_dma_w)
{
	COMBINE_DATA(fantjour_dma + offset);
	if(!offset && ACCESSING_BITS_24_31) {
		UINT32 sa = fantjour_dma[1];
		//      UINT16 ss = (fantjour_dma[2] & 0xffff0000) >> 16;
		//      UINT32 sb = ((fantjour_dma[2] & 0xffff) << 16) | ((fantjour_dma[3] & 0xffff0000) >> 16);

		UINT32 da = ((fantjour_dma[3] & 0xffff) << 16) | ((fantjour_dma[4] & 0xffff0000) >> 16);
		//      UINT16 ds = fantjour_dma[4] & 0xffff;
		UINT32 db = fantjour_dma[5];

		//      UINT8 sz1 = fantjour_dma[0] >> 8;
		UINT8 sz2 = fantjour_dma[0] >> 16;
		UINT8 mode = fantjour_dma[0] >> 24;

		UINT32 x   = fantjour_dma[6];
		UINT32 i1, i2;

		if(mode == 0x93)
			for(i1=0; i1 <= sz2; i1++)
				for(i2=0; i2 < db; i2+=4) {
					memory_write_dword(space, da, memory_read_dword(space, sa) ^ x);
					da += 4;
					sa += 4;
				}
		else if(mode == 0x8f)
			for(i1=0; i1 <= sz2; i1++)
				for(i2=0; i2 < db; i2+=4) {
					memory_write_dword(space, da, x);
					da += 4;
				}
	}
}
