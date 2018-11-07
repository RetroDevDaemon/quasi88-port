#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED


extern	int	set_version;	/* �С���������ѹ� '0' �� '9'	*/
extern	int	rom_version;	/* (�ѹ�����) BASIC ROM�С������	*/

extern	int	use_extram;			/* 128KB��ĥRAM��Ȥ�	*/
extern	int	use_jisho_rom;			/* ����ROM��Ȥ�	*/
extern	int	use_built_in_font;		/* ��¢�ե���Ȥ�Ȥ�	*/
extern	int	use_pcg;			/* PCG-8100���ݡ���	*/
extern	int	font_type;			/* �ե���Ȥμ���	*/
extern	int	font_loaded;			/* ���ɤ����ե���ȼ�	*/


extern	int	memory_wait;			/* ���ꥦ������̵ͭ	*/

extern	char	*file_compatrom;		/* P88SR emu ��ROM��Ȥ�*/

extern	int	has_kanji_rom;			/* ����ROM��̵ͭ	*/

extern	int	linear_ext_ram;			/* ��ĥRAM��Ϣ³������	*/


extern	byte	*main_rom;			/* �ᥤ�� ROM (32KB)	*/
extern	byte	(*main_rom_ext)[0x2000];	/* ��ĥ ROM   (8KB *4)	*/
extern	byte	*main_rom_n;			/* N-BASIC    (32KB)	*/
extern	byte	*main_ram;			/* �ᥤ�� RAM (64KB)	*/
extern	byte	*main_high_ram;			/* ��® RAM(��΢) (4KB)	*/
extern	byte	*sub_romram;			/* ���� ROM/RAM (32KB)	*/

extern	byte	(*kanji_rom)[65536][2];		/* ���� ROM   (128KB*2)	*/

extern	byte	(*ext_ram)[0x8000];		/* ��ĥ RAM   (32KB*4��)*/
extern	byte	(*jisho_rom)[0x4000];		/* ���� ROM   (16KB*32)	*/

extern	byte	(*main_vram)[4];		/* VRAM[0x4000][4]	*/
extern	byte	*font_rom;			/* �ե���ȥ��᡼��     */
extern	byte	*font_pcg;			/* �ե���ȥ��᡼��(PCG)*/
extern	byte	*font_mem;			/* �ե���ȥ��᡼��(fix)*/
extern	byte	*font_mem2;			/* �ե���ȥ��᡼��(2nd)*/
extern	byte	*font_mem3;			/* �ե���ȥ��᡼��(3rd)*/


				/* ���꡼�������ˡ�ǥ��ꥢ����������	*/
#define	main_vram4 (bit32 *)main_vram		/* VRAM long word accrss*/

#define	ROM_VERSION	main_rom[0x79d7]



extern	byte	*dummy_rom;			/* ���ߡ�ROM (32KB)	*/
extern	byte	*dummy_ram;			/* ���ߡ�RAM (32KB)	*/
extern	byte	kanji_dummy_rom[16][2];		/* �������ߡ�ROM	*/



int	memory_allocate( void );
void	memory_free( void );

void	memory_reset_font( void );
void	memory_set_font( void );

int	memory_allocate_additional( void );

#endif	/* MEMORY_H_INCLUDED */
