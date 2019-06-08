
/* PCE.C */
void pce_write_header(FILE *f, int banks);
int  pce_pack_8x8_tile(unsigned char *buffer, void *data, int line_offset, int format);
int  pce_pack_16x16_tile(unsigned char *buffer, void *data, int line_offset, int format);
int  pce_pack_16x16_sprite(unsigned char *buffer, void *data, int line_offset, int format);
void pce_defchr(int *ip);
void pce_defpal(int *ip);
void pce_defspr(int *ip);
void pce_incbat(int *ip);
void pce_incpal(int *ip);
void pce_incspr(int *ip);
void pce_inctile(int *ip);
void pce_incmap(int *ip);
void pce_vram(int *ip);
void pce_pal(int *ip);
void pce_develo(int *ip);
void pce_mml(int *ip);

/* MML.C */
int mml_start(unsigned char *buffer);
int mml_stop(unsigned char *buffer);
int mml_parse(unsigned char *buffer, int bufsize, char *ptr);

/* PCE specific instructions */
struct t_opcode pce_inst[82] = {
	{NULL, "BBR",  class10,0, 0x0F, 0, 1},
	{NULL, "BBR0", class5, 0, 0x0F, 0, 1},
	{NULL, "BBR1", class5, 0, 0x1F, 0, 1},
	{NULL, "BBR2", class5, 0, 0x2F, 0, 1},
	{NULL, "BBR3", class5, 0, 0x3F, 0, 1},
	{NULL, "BBR4", class5, 0, 0x4F, 0, 1},
	{NULL, "BBR5", class5, 0, 0x5F, 0, 1},
	{NULL, "BBR6", class5, 0, 0x6F, 0, 1},
	{NULL, "BBR7", class5, 0, 0x7F, 0, 1},
	{NULL, "BBS",  class10,0, 0x8F, 0, 1},
	{NULL, "BBS0", class5, 0, 0x8F, 0, 1},
	{NULL, "BBS1", class5, 0, 0x9F, 0, 1},
	{NULL, "BBS2", class5, 0, 0xAF, 0, 1},
	{NULL, "BBS3", class5, 0, 0xBF, 0, 1},
	{NULL, "BBS4", class5, 0, 0xCF, 0, 1},
	{NULL, "BBS5", class5, 0, 0xDF, 0, 1},
	{NULL, "BBS6", class5, 0, 0xEF, 0, 1},
	{NULL, "BBS7", class5, 0, 0xFF, 0, 1},
	{NULL, "BRA",  class2, 0, 0x80, 0, 1},
	{NULL, "BSR",  class2, 0, 0x44, 0, 1},
	{NULL, "CLA",  class1, 0, 0x62, 0, 1},
	{NULL, "CLX",  class1, 0, 0x82, 0, 1},
	{NULL, "CLY",  class1, 0, 0xC2, 0, 1},
	{NULL, "CSH",  class1, 0, 0xD4, 0, 1},
	{NULL, "CSL",  class1, 0, 0x54, 0, 1},
	{NULL, "PHX",  class1, 0, 0xDA, 0, 1},
	{NULL, "PHY",  class1, 0, 0x5A, 0, 1},
	{NULL, "PLX",  class1, 0, 0xFA, 0, 1},
	{NULL, "PLY",  class1, 0, 0x7A, 0, 1},
	{NULL, "RMB",  class9, 0, 0x07, 0, 1},
	{NULL, "RMB0", class4, ZP, 0x03, 0, 1},
	{NULL, "RMB1", class4, ZP, 0x13, 0, 1},
	{NULL, "RMB2", class4, ZP, 0x23, 0, 1},
	{NULL, "RMB3", class4, ZP, 0x33, 0, 1},
	{NULL, "RMB4", class4, ZP, 0x43, 0, 1},
	{NULL, "RMB5", class4, ZP, 0x53, 0, 1},
	{NULL, "RMB6", class4, ZP, 0x63, 0, 1},
	{NULL, "RMB7", class4, ZP, 0x73, 0, 1},
	{NULL, "SAX",  class1, 0, 0x22, 0, 1},
	{NULL, "SAY",  class1, 0, 0x42, 0, 1},
	{NULL, "SET",  class1, 0, 0xF4, 0, 1},
	{NULL, "SMB",  class9, 0, 0x87, 0, 1},
	{NULL, "SMB0", class4, ZP, 0x83, 0, 1},
	{NULL, "SMB1", class4, ZP, 0x93, 0, 1},
	{NULL, "SMB2", class4, ZP, 0xA3, 0, 1},
	{NULL, "SMB3", class4, ZP, 0xB3, 0, 1},
	{NULL, "SMB4", class4, ZP, 0xC3, 0, 1},
	{NULL, "SMB5", class4, ZP, 0xD3, 0, 1},
	{NULL, "SMB6", class4, ZP, 0xE3, 0, 1},
	{NULL, "SMB7", class4, ZP, 0xF3, 0, 1},
	{NULL, "ST0",  class4, IMM, 0x03, 1, 1},
	{NULL, "ST1",  class4, IMM, 0x13, 1, 1},
	{NULL, "ST2",  class4, IMM, 0x23, 1, 1},
	{NULL, "STZ",  class4, ZP|ZP_X|ABS|ABS_X, 0x00, 0x05, 1},
	{NULL, "SXY",  class1, 0, 0x02, 0, 1},
	{NULL, "TAI",  class6, 0, 0xF3, 0, 1},
	{NULL, "TAM",  class8, 0, 0x53, 0, 1},
	{NULL, "TAM0", class3, 0, 0x53, 0x01, 1},
	{NULL, "TAM1", class3, 0, 0x53, 0x02, 1},
	{NULL, "TAM2", class3, 0, 0x53, 0x04, 1},
	{NULL, "TAM3", class3, 0, 0x53, 0x08, 1},
	{NULL, "TAM4", class3, 0, 0x53, 0x10, 1},
	{NULL, "TAM5", class3, 0, 0x53, 0x20, 1},
	{NULL, "TAM6", class3, 0, 0x53, 0x40, 1},
	{NULL, "TAM7", class3, 0, 0x53, 0x80, 1},
	{NULL, "TDD",  class6, 0, 0xC3, 0, 1},
	{NULL, "TIA",  class6, 0, 0xE3, 0, 1},
	{NULL, "TII",  class6, 0, 0x73, 0, 1},
	{NULL, "TIN",  class6, 0, 0xD3, 0, 1},
	{NULL, "TMA",  class8, 0, 0x43, 0, 1},
	{NULL, "TMA0", class3, 0, 0x43, 0x01, 1},
	{NULL, "TMA1", class3, 0, 0x43, 0x02, 1},
	{NULL, "TMA2", class3, 0, 0x43, 0x04, 1},
	{NULL, "TMA3", class3, 0, 0x43, 0x08, 1},
	{NULL, "TMA4", class3, 0, 0x43, 0x10, 1},
	{NULL, "TMA5", class3, 0, 0x43, 0x20, 1},
	{NULL, "TMA6", class3, 0, 0x43, 0x40, 1},
	{NULL, "TMA7", class3, 0, 0x43, 0x80, 1},
	{NULL, "TRB",  class4, ZP|ABS, 0x10, 0, 1},
	{NULL, "TSB",  class4, ZP|ABS, 0x00, 0, 1},
	{NULL, "TST",  class7, 0, 0x00, 0, 1},
	{NULL, NULL, NULL, 0, 0, 0, 0}
};

/* PCE specific pseudos */
struct t_opcode pce_pseudo[23] = {
	{NULL,  "DEFCHR", pce_defchr, PSEUDO, P_DEFCHR, 0, 1},
	{NULL,  "DEFPAL", pce_defpal, PSEUDO, P_DEFPAL, 0, 1},
	{NULL,  "DEFSPR", pce_defspr, PSEUDO, P_DEFSPR, 0, 1},
	{NULL,  "INCBAT", pce_incbat, PSEUDO, P_INCBAT, 0xD5, 1},
	{NULL,  "INCSPR", pce_incspr, PSEUDO, P_INCSPR, 0xEA, 1},
	{NULL,  "INCPAL", pce_incpal, PSEUDO, P_INCPAL, 0xF8, 1},
	{NULL,  "INCTILE",pce_inctile,PSEUDO, P_INCTILE,0xEA, 1},
	{NULL,  "INCMAP", pce_incmap, PSEUDO, P_INCMAP, 0xD5, 1},
	{NULL,  "MML",    pce_mml,    PSEUDO, P_MML,    0, 1},
	{NULL,  "PAL",    pce_pal,    PSEUDO, P_PAL,    0, 1},
	{NULL,  "VRAM",   pce_vram,   PSEUDO, P_VRAM,   0, 1},
					             
	{NULL, ".DEFCHR", pce_defchr, PSEUDO, P_DEFCHR, 0, 1},
	{NULL, ".DEFPAL", pce_defpal, PSEUDO, P_DEFPAL, 0, 1},
	{NULL, ".DEFSPR", pce_defspr, PSEUDO, P_DEFSPR, 0, 1},
	{NULL, ".INCBAT", pce_incbat, PSEUDO, P_INCBAT, 0xD5, 1},
	{NULL, ".INCSPR", pce_incspr, PSEUDO, P_INCSPR, 0xEA, 1},
	{NULL, ".INCPAL", pce_incpal, PSEUDO, P_INCPAL, 0xF8, 1},
	{NULL, ".INCTILE",pce_inctile,PSEUDO, P_INCTILE,0xEA, 1},
	{NULL, ".INCMAP", pce_incmap, PSEUDO, P_INCMAP, 0xD5, 1},
	{NULL, ".MML",    pce_mml,    PSEUDO, P_MML,    0, 1},
	{NULL, ".PAL",    pce_pal,    PSEUDO, P_PAL,    0, 1},
	{NULL, ".VRAM",   pce_vram,   PSEUDO, P_VRAM,   0, 1},
	{NULL, NULL, NULL, 0, 0, 0, 0}
};

/* PCE machine description */
struct t_machine pce = {
	MACHINE_PCE,   /* type */
	"PCEAS-S", /* asm_name */
	"Extended PC-Engine Assembler (v2.51)", /* asm_title */
	".pce",  /* rom_ext */
	"PCE_INCLUDE", /* include_env */
	0xD8,    /* zp_limit */
	0x2000,  /* ram_limit */
	0x2000,  /* ram_base */
	1,       /* ram_page */
	0xF8,    /* ram_bank */
	pce_inst,   /* inst */
	pce_pseudo, /* pseudo_inst */
	pce_pack_8x8_tile,     /* pack_8x8_tile */
	pce_pack_16x16_tile,   /* pack_16x16_tile */
	pce_pack_16x16_sprite, /* pack_16x16_sprite */
	pce_write_header /* write_header */
};

