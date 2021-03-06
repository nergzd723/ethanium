#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define VGA_SEQ_INDEX_PORT 0x3C4
#define VGA_SEQ_DATA_PORT 0x3C5

#define VGA_GC_INDEX_PORT 0x3CE
#define VGA_GC_DATA_PORT 0x3CF

#define VGA_CRTC_INDEX_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 0x3D5

#define VGA_SEQ_MAP_MASK_REG 0x02
#define VGA_SEQ_CHARSET_REG 0x03
#define VGA_SEQ_MEMORY_MODE_REG 0x04

#define VGA_GC_READ_MAP_SELECT_REG 0x04
#define VGA_GC_GRAPHICS_MODE_REG 0x05
#define VGA_GC_MISC_REG 0x06
void write_regs(unsigned char *regs, _Bool isVmode);
