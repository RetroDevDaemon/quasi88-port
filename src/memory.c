/************************************************************************/
/*									*/
/* ������� & ROM�ե��������					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "memory.h"
#include "pc88main.h"

#include "soundbd.h"		/* sound_board, sound2_adpcm	*/

#include "menu.h"		/* menu_lang	*/
#include "file-op.h"
#include "suspend.h"



int	set_version;	/* ��ư���ΥС���������ѹ� '0' �� '9'	*/
int	rom_version;	/* (�ѹ�����) BASIC ROM�С������		*/

int	use_extram	= DEFAULT_EXTRAM;	/* ��ĥRAM�Υ����ɿ�	*/
int	use_jisho_rom	= DEFAULT_JISHO;	/* ����ROM��Ȥ�	*/
int	use_built_in_font = FALSE;		/* ��¢�ե���Ȥ�Ȥ�	*/
int	use_pcg = FALSE;			/* PCG-8100���ݡ���	*/
int	font_type = 0;				/* �ե���Ȥμ���	*/
int	font_loaded = 0;			/* ���ɤ����ե���ȼ�	*/

int	memory_wait = FALSE;			/* ���ꥦ�����Ȥ�̵ͭ	*/

char	*file_compatrom = NULL;			/* P88SR emu ��ROM��Ȥ�*/

int	has_kanji_rom   = FALSE;		/* ����ROM��̵ͭ	*/

int	linear_ext_ram = TRUE;			/* ��ĥRAM��Ϣ³������	*/


/*----------------------------------------------------------------------*/
/* ��¢�ե���ȥǡ���							*/
/*----------------------------------------------------------------------*/
#include "font.h"


/*----------------------------------------------------------------------*/
/* �������ߡ�ROM (����ROM��̵�����Υ��ߡ�)				*/
/*----------------------------------------------------------------------*/
byte	kanji_dummy_rom[16][2] =
{
  { 0xaa, 0xaa, },	/* o o o o o o o o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0xaa, 0xaa, },	/* o o o o o o o o  */
  { 0x00, 0x00, },	/*                  */
};


/*----------------------------------------------------------------------*/
/* ROM�ե�����̾							*/
/*----------------------------------------------------------------------*/
enum {
  N88_ROM,  EXT0_ROM, EXT1_ROM,  EXT2_ROM, EXT3_ROM,  N_ROM,     SUB_ROM,
  KNJ1_ROM, KNJ2_ROM, JISHO_ROM, FONT_ROM, FONT2_ROM, FONT3_ROM, ROM_END
};
static char *rom_list[ ROM_END ][5] =
{
  {  "N88.ROM",		"n88.rom",	0,		0,		0, },
  {  "N88EXT0.ROM",	"n88ext0.rom",	"N88_0.ROM",	"n88_0.rom",	0, },
  {  "N88EXT1.ROM",	"n88ext1.rom",	"N88_1.ROM",	"n88_1.rom",	0, },
  {  "N88EXT2.ROM",	"n88ext2.rom",	"N88_2.ROM",	"n88_2.rom",	0, },
  {  "N88EXT3.ROM",	"n88ext3.rom",	"N88_3.ROM",	"n88_3.rom",	0, },
  {  "N88N.ROM",	"n88n.rom",	"N80.ROM",	"n80.rom",	0, },
  {  "N88SUB.ROM",	"n88sub.rom",	"DISK.ROM",	"disk.rom",	0, },
  {  "N88KNJ1.ROM",	"n88knj1.rom",	"KANJI1.ROM",	"kanji1.rom",	0, },
  {  "N88KNJ2.ROM",	"n88knj2.rom",	"KANJI2.ROM",	"kanji2.rom",	0, },
  {  "N88JISHO.ROM",	"n88jisho.rom",	"JISYO.ROM",	"jisyo.rom",	0, },
  {  "FONT.ROM",	"font.rom",	0,		0,		0, },
  {  "FONT2.ROM",	"font2.rom",	0,		0,		0, },
  {  "FONT3.ROM",	"font3.rom",	0,		0,		0, },
};



byte	*main_rom;			/* �ᥤ�� ROM [0x8000] (32KB)	*/
byte	(*main_rom_ext)[0x2000];	/* ��ĥ ROM [4][0x2000](8KB *4)	*/
byte	*main_rom_n;			/* N-BASIC [0x8000]    (32KB)	*/
byte	*main_ram;			/* �ᥤ�� RAM [0x10000](64KB)	*/
byte	*main_high_ram;			/* ��® RAM(��΢)[0x1000] (4KB)	*/
byte	(*kanji_rom)[65536][2];		/* �����ңϣ�[2][65536][2]	*/
byte	*sub_romram;			/* ���� ROM/RAM [0x8000] (32KB)	*/

byte	(*ext_ram)[0x8000];		/* ��ĥ RAM[4][0x8000](32KB*4��)*/
byte	(*jisho_rom)[0x4000];		/* ���� ROM[32][0x4000](16KB*32)*/

bit8	(*main_vram)[4];		/* VRAM[0x4000][4](=G/R/G/pad)	*/
bit8	*font_rom;			/* �ե���ȥ��᡼��ROM[8*256*2]	*/

bit8	*font_pcg;			/* �ե���ȥ��᡼��ROM(PCG��)	*/
bit8	*font_mem;			/* �ե���ȥ��᡼��ROM(���� )	*/
bit8	*font_mem2;			/* �ե���ȥ��᡼��ROM(����2)	*/
bit8	*font_mem3;			/* �ե���ȥ��᡼��ROM(����3)	*/

byte	*dummy_rom;			/* ���ߡ�ROM (32KB)		*/
byte	*dummy_ram;			/* ���ߡ�RAM (32KB)		*/





/*----------------------------------------------------------------------
 * ���ꥢ������
 *----------------------------------------------------------------------*/
static	int	mem_alloc_result;

static	void	mem_alloc_start( const char *msg )	/* ������ݳ��� */
{
  if( verbose_proc ){ printf( "%s", msg ); }

  mem_alloc_result = TRUE;
}

static	void	*mem_alloc( size_t size )		/* ������� */
{
  void	*ptr = malloc( size );

  if( ptr == NULL ){ 
    mem_alloc_result = FALSE;
  }
  return ptr;
}

static	int	mem_alloc_finish( void )		/* ������ݴ�λ(���Ǽ���) */
{
  if( verbose_proc ){ 
    if( mem_alloc_result == FALSE ){ printf( "FAILED\n" ); }
    else                           { printf( "OK\n" );     }
  }

  return mem_alloc_result;
}



/*----------------------------------------------------------------------
 * ROM ���᡼���ե���������˥���
 *----------------------------------------------------------------------*/

/*
 * �̾��ROM���᡼������
 *	int load_rom( char *filelist[], byte *ptr, int size, int disp_flag )
 *		filelist �� �����ץ󤹤�ե�����̾�����Υꥹ��
 *		ptr      �� ���Υݥ��󥿤���˥��ɤ���
 *		size     �� ���ɤ��륵����
 *		disp_flag�� verbose_proc ����ɽ�������å���������
 *				DISP_FNAME	�������ե�����̾����ɽ��
 *							"filename ..."
 *						����ͤ�ߤ�"OK\n"�����ղä���
 *						�ե����뤬�����ʤ��ä��顢
 *							"filename not found"
 *						ɬ�פ˱����� "\n" ���ղä���
 *
 *				DISP_RESULT	�������ե�����̾�ȷ�̤�ɽ��
 *							"filename ... OK\n"
 *						�ե����뤬�����ʤ��ä��顢
 *							"filename not found\n"
 *
 *				DISP_IF_EXIST	�������ե�����̾�ȷ�̤�ɽ��
 *							"filename ..."
 *						����ͤ�ߤ�"OK\n"�����ղä���
 *						�ե����뤬�����ʤ��ä��顢
 *							�ʤˤ�ɽ�����ʤ���
 *
 *		����     �� �ɤ߹�������������ե����뤬�����ʤ��ä��� -1
 */
#define		DISP_FNAME	(0)
#define		DISP_RESULT	(1)
#define		DISP_IF_EXIST	(2)

static	int	load_rom( char *filelist[], byte *ptr, int size, int disp )
{
  OSD_FILE *fp;
  int i=0, load_size = -1;
  char buf[ OSD_MAX_FILENAME ];
  const char *dir = osd_dir_rom();

  if( dir ){
    for( i=0; filelist[i] ; i++ ){

      if( osd_path_join( dir, filelist[i], buf, OSD_MAX_FILENAME )==FALSE )
	break;

      if( (fp = osd_fopen( FTYPE_ROM, buf, "rb" )) ){

	load_size = osd_fread( ptr, sizeof(byte), size, fp );
	osd_fclose( fp );

	break;
      }

    }
  }

  if( load_size < 0 ){			/* �ե����븫�Ĥ��餺 */
    memset( ptr, 0xff, size );
  }else if( load_size < size ){		/* ���Ĥ��ä����ɡ���������­��ʤ� */
    memset( &ptr[load_size], 0xff, size-load_size );
  }


  if( verbose_proc ){
    if( load_size < 0 ){
      if( disp == DISP_FNAME  ) printf( "  %-12s ... ",           filelist[0]);
      if( disp == DISP_RESULT ) printf( "  %-12s ... Not Found\n",filelist[0]);
    }else{
      printf( "  Found %-12s : Load...", filelist[i] );
      if( disp == DISP_RESULT ){
	if( load_size == size ){ printf( "OK\n" );     }
	else                   { printf( "FAILED\n" ); }
      }
    }
  }

  return load_size;
}



/*
 * ��������ROM���᡼������
 *	OSD_FILE *load_compat_rom_open( void )
 *	void load_compat_rom_close( OSD_FILE *fp )
 *		�ե�����򳫤� / �Ĥ���
 *	int load_compat_rom( byte *ptr, long pos, int size, OSD_FILE *fp )
 *		�ե�����fp ����Ƭ pos�Х����ܤ��� size�Х��Ȥ� ptr�˥��ɡ�
 *		���ͤϥ��ɤ��������������������Ԥʤ� -1��
 *	�����1�٤Ǥ�����˼��Ԥ����顢 load_compat_rom_success==FALSE �ˤʤ�
 */
static	int	load_compat_rom_success;

static	OSD_FILE *load_compat_rom_open( void )
{
  OSD_FILE *fp = osd_fopen( FTYPE_ROM, file_compatrom, "rb" );

  if( verbose_proc ){
    if( fp ){ printf( "  Found %-12s : Load...", file_compatrom ); }
    else    { printf( "  %-12s ... Not Found\n", file_compatrom ); }
  }

  if( fp==NULL ) load_compat_rom_success = FALSE;
  else           load_compat_rom_success = TRUE;

  return fp;
}

static	int	load_compat_rom( byte *ptr, long pos, int size, OSD_FILE *fp )
{
  int load_size = -1;

  if( fp ){
    if( osd_fseek( fp, pos, SEEK_SET ) == 0 ){

      load_size = osd_fread( ptr, 1, size, fp );

      if( load_size < size ){
	memset( &ptr[load_size], 0xff, size-load_size );
      }
    }

    if( load_size != size ) load_compat_rom_success = FALSE;
  }

  return load_size;
}

static	void	load_compat_rom_close( OSD_FILE *fp )
{
  if( fp ) osd_fclose( fp );
}










/****************************************************************************
 * ���ߥ�졼�����˻��Ѥ������γ��ݤȡ�ROM���᡼���Υ���
 *
 *	peach����ˤ�ꡢM88 �� ROM�ե������Ȥ���褦�˳�ĥ����ޤ���	
 *	�� ���θ�������˥������������ޤ�����
 *****************************************************************************/

#define	FONT_SZ	(8*256*1)

int	memory_allocate( void )
{
  int	size;


		/* ɸ��������� */

  mem_alloc_start( "Allocating memory for standard ROM/RAM..." );
  {
    main_rom     = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );
    main_rom_ext = (byte(*)[0x2000])  mem_alloc( sizeof(byte) *  0x2000 *4 );
    main_rom_n   = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );
    sub_romram   = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );

    main_ram     = (byte *)           mem_alloc( sizeof(byte) * 0x10000 );
    main_high_ram= (byte *)           mem_alloc( sizeof(byte) *  0x1000 );
    main_vram    = (byte(*)[0x4])     mem_alloc( sizeof(byte) *  0x4000 *4 );

    kanji_rom    = (byte(*)[65536][2])mem_alloc( sizeof(byte)*2*65536*2 );

    font_pcg     = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem     = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem2    = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem3    = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
  }
  if( mem_alloc_finish()==FALSE ){
    return 0;
  }




		/* ROM���᡼����ե����뤫���ɤ߹��� */

  if( file_compatrom == NULL ){		/* �̾��ROM���᡼���ե����� */

    load_rom( rom_list[ N88_ROM ],  main_rom,        0x8000, DISP_RESULT );
    load_rom( rom_list[ EXT0_ROM ], main_rom_ext[0], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT1_ROM ], main_rom_ext[1], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT2_ROM ], main_rom_ext[2], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT3_ROM ], main_rom_ext[3], 0x2000, DISP_RESULT );
    load_rom( rom_list[ N_ROM ],    main_rom_n,      0x8000, DISP_RESULT );

    size = load_rom( rom_list[ SUB_ROM ], sub_romram, 0x2000, DISP_FNAME );
    {
      if( verbose_proc ){
	if     ( size <       0 ){ printf( "Not Found\n" );    }
	else if( size ==  0x800 ){ printf( "OK(2D-type)\n" );  }
	else if( size == 0x2000 ){ printf( "OK(2HD-type)\n" ); }
	else                     { printf( "FAILED\n");        }
      }
      if( size <= 0x800 ){
	memcpy( &sub_romram[0x0800], &sub_romram[0x0000], 0x0800 );
	memcpy( &sub_romram[0x1000], &sub_romram[0x0000], 0x1000 );
      }
    }

  }else{				/* ������ROM���᡼���ե����� */

    OSD_FILE *fp = load_compat_rom_open();
    if( fp ){
      load_compat_rom( main_rom,                  0, 0x8000, fp );
      load_compat_rom( main_rom_ext[0],     0x0c000, 0x2000, fp );
      load_compat_rom( main_rom_ext[1],     0x0e000, 0x2000, fp );
      load_compat_rom( main_rom_ext[2],     0x10000, 0x2000, fp );
      load_compat_rom( main_rom_ext[3],     0x12000, 0x2000, fp );
      load_compat_rom( &main_rom_n[0x6000], 0x08000, 0x2000, fp );
      load_compat_rom( sub_romram,          0x14000, 0x2000, fp );

      if( load_compat_rom_success == FALSE ){	/* �������Ǽ��Ԥ�����н�λ */
	if( verbose_proc ){ printf( "FAILED\n"); }
      }
      else{					/* �����ʤ� N-BASIC ���� */
	size = load_compat_rom( main_rom_n, 0x16000, 0x6000, fp );
	if( verbose_proc ){
	  if     ( size ==      0 ){ printf( "OK (Without N-BASIC)\n" ); }
	  else if( size == 0x6000 ){ printf( "OK (With N-BASIC)\n" );    }
	  else                     { printf( "FAILED\n");                }
	}
      }

      load_compat_rom_close( fp );
    }
  }
					/* SUB¦ ROM �ߥ顼������RAM���ꥢ */
  memcpy( &sub_romram[0x2000], &sub_romram[0x0000], 0x2000 );
  memset( &sub_romram[0x4000], 0xff, 0x4000 );
  


		/* ����ROM���᡼����ե����뤫���ɤ߹��� */

  size=load_rom( rom_list[ KNJ1_ROM ], kanji_rom[0][0], 0x20000, DISP_RESULT );

  if( size == 0x20000 ){
    has_kanji_rom = TRUE;
  }else{
    has_kanji_rom = FALSE;
    menu_lang = MENU_ENGLISH;
  }

  load_rom( rom_list[ KNJ2_ROM ], kanji_rom[1][0], 0x20000, DISP_RESULT );



		/* �ե����ROM���᡼����ե����뤫���ɤ߹��� */
		/*   (���ߥ���ե��å�ʸ������¢ʸ�������)  */

  if( use_built_in_font ){

    memcpy( &font_mem[0], &built_in_font_ANK[0], 0x100*8 );

  }else{

    size = load_rom( rom_list[ FONT_ROM ],  font_mem,  FONT_SZ, DISP_FNAME );
    font_loaded |= 1;

    if( verbose_proc ){
      if( size == FONT_SZ ){ printf( "OK\n" ); }
      else{
	if( size < 0 ){ printf( "Not Found " ); }
	else          { printf( "FAILED ");     }
	if( has_kanji_rom ){ printf( "(Use KANJI-ROM font)\n" ); }
	else               { printf( "(Use built-in font)\n" );  }
      }
    }

    if( size != FONT_SZ ){	/* �۾���ϡ�����ROM����¢�ե���Ȥ���� */
      if( has_kanji_rom ){
	memcpy( &font_mem[0], &kanji_rom[0][(1<<11)][0], 0x100*8 );
      }else{
	memcpy( &font_mem[0], &built_in_font_ANK[0], 0x100*8 );
	font_loaded &= ~1;
      }
    }
  }

  memcpy( &font_mem[0x100*8], &built_in_font_graph[0], 0x100*8 );



		/* ��2�ե���ȥ��᡼����ե����뤫���ɤ߹��� */

  if( use_built_in_font ){

    memcpy( &font_mem2[0],       &built_in_font_ANH[0],   0x100*8 );
    memcpy( &font_mem2[0x100*8], &built_in_font_graph[0], 0x100*8 );

  }else{

    size = load_rom( rom_list[FONT2_ROM], font_mem2, FONT_SZ*2, DISP_IF_EXIST);
    font_loaded |= 2;

    if( verbose_proc ){
      if     ( size == -1 ) ;
      else if( size == FONT_SZ*2 ){ printf( "OK(with semi-graphic-font)\n" ); }
      else if( size == FONT_SZ   ){ printf( "OK\n" );                         }
      else                        { printf( "FAILED\n" ); }
    }

    if( size == FONT_SZ*2 ){
      ;
    }else{
      if( size == FONT_SZ ){
	;
      }else{			/* ¸�ߤ��ʤ����ϡ���¢�ե���Ȥ���� */
	memcpy( &font_mem2[0],     &built_in_font_ANH[0],   0x100*8 );
	font_loaded &= ~2;
      }
      memcpy( &font_mem2[0x100*8], &built_in_font_graph[0], 0x100*8 );
    }

  }


		/* ��3�ե���ȥ��᡼����ե����뤫���ɤ߹��� */

  if( use_built_in_font ){

    memset( &font_mem3[0], 0, FONT_SZ*2 );

  }else{

    size = load_rom( rom_list[FONT3_ROM], font_mem3, FONT_SZ*2, DISP_IF_EXIST);
    font_loaded |= 4;

    if( verbose_proc ){
      if     ( size == -1 ) ;
      else if( size == FONT_SZ*2 ){ printf( "OK(with semi-graphic-font)\n" ); }
      else if( size == FONT_SZ   ){ printf( "OK\n" );                         }
      else                        { printf( "FAILED\n" ); }
    }

    if( size == FONT_SZ*2 ){
      ;
    }else{
      if( size == FONT_SZ ){
	memcpy( &font_mem3[0x100*8], &built_in_font_graph[0], 0x100*8 );

      }else{			/* ¸�ߤ��ʤ����ϡ�Ʃ���ե���Ȥ���� */
	memset( &font_mem3[0], 0, FONT_SZ*2 );
	font_loaded &= ~4;
      }
    }

  }


		/* �ե���Ȥ�ʸ�������� 0 �����Ф˶��� */
  memset( &font_mem[0],  0, 8 );
  memset( &font_mem2[0], 0, 8 );
  memset( &font_mem3[0], 0, 8 );


  memory_reset_font();



		/* ROM�ΥС���������¸ */
  rom_version = ROM_VERSION;

		/* ���ץ���ʥ�ʥ������� */

  if( memory_allocate_additional()== FALSE ){
    return 0;
  }


  return 1;
}



/****************************************************************************
 *
 *
 *****************************************************************************/
static	int	alloced_extram = 0;		/* ���ݤ�����ĥRAM�ο�	*/

int	memory_allocate_additional( void )
{

		/* ��ĥ�������� */

  if( use_extram ){

    if (use_extram <= 4 ||
	BETWEEN(8, use_extram, 10) ||
	use_extram == 16) {
	;
    } else {
	linear_ext_ram = TRUE;
    }

				/* ���ݺѤߥ���������������С����ݤ��ʤ��� */
    if( ext_ram && alloced_extram < use_extram ){
      free( ext_ram );
      ext_ram = NULL;
    }

    if( ext_ram == NULL ){

      char msg[80];
      sprintf( msg, "Allocating memory for Extended RAM(%dKB)...",
			use_extram * 128 );

      mem_alloc_start( msg );

      ext_ram = (byte(*)[0x8000])mem_alloc( sizeof(byte)*0x8000 *4*use_extram);

      if( dummy_rom == NULL )
	dummy_rom = (byte *)     mem_alloc( sizeof(byte) * 0x8000 );
      if( dummy_ram == NULL )
	dummy_ram = (byte *)     mem_alloc( sizeof(byte) * 0x8000 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

      alloced_extram = use_extram;
    }

    memset( &ext_ram[0][0], 0xff, 0x8000 * 4*use_extram );
    memset( &dummy_rom[0],  0xff, 0x8000 );
  }


		/* ����ROM�ѥ������� */
		/* ����ROM���᡼����ե����뤫���ɤ߹��� */

  if( use_jisho_rom ){

    if( jisho_rom == NULL ){

      mem_alloc_start( "Allocating memory for Jisho ROM..." );

      jisho_rom = (byte(*)[0x4000])mem_alloc( sizeof(byte) * 0x4000*32 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

      load_rom( rom_list[ JISHO_ROM ], jisho_rom[0], 0x4000*32, DISP_RESULT );

    }
  }


		/* ������ɥܡ���II ADPCM��RAM����� */

  if( sound_board==SOUND_II ){

    if( sound2_adpcm == NULL ){

      mem_alloc_start( "Allocating memory for ADPCM RAM..." );

      sound2_adpcm = (byte *)mem_alloc( sizeof(byte) * 0x40000 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

    }

    memset( &sound2_adpcm[0],  0xff, 0x40000 );
  }

  return 1;
}







/****************************************************************************
 * �ե���Ƚ����
 *	PCG�ե���ȥǡ������̾�Υե���Ȥǽ����
 *****************************************************************************/
void	memory_reset_font( void )
{
  memcpy( font_pcg, font_mem, sizeof(byte)*8*256*2 );

  memory_set_font();
}



/****************************************************************************
 * ���Ѥ���ե���Ȥ����
 *
 *****************************************************************************/
void	memory_set_font( void )
{
  if( use_pcg ){
    font_rom = font_pcg;
  }else{
    if     ( font_type == 0 ) font_rom = font_mem;
    else if( font_type == 1 ) font_rom = font_mem2;
    else if( font_type == 2 ) font_rom = font_mem3;
  }
}



/****************************************************************************
 * ���ݤ�������β���
 *	��λ�����ʤ��������ɬ�פ�ʤ����ɡ�
 *****************************************************************************/
void	memory_free( void )
{
  if( main_rom )     free( main_rom );
  if( main_rom_ext ) free( main_rom_ext );
  if( main_rom_n )   free( main_rom_n );
  if( sub_romram)    free( sub_romram );

  if( main_ram)      free( main_ram );
  if( main_high_ram )free( main_high_ram );
  if( main_vram )    free( main_vram );

  if( kanji_rom )    free( kanji_rom );

  if( font_pcg )     free( font_pcg );
  if( font_mem )     free( font_mem );
  if( font_mem2 )    free( font_mem2 );
  if( font_mem3 )    free( font_mem3 );

  if( use_extram )    free( ext_ram );
  if( use_jisho_rom ) free( jisho_rom );
  if( sound_board==SOUND_II ) free( sound2_adpcm );
}







/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID		"MEM "
#define	SID_MAIN	"MEM0"
#define	SID_HIGH	"MEM1"
#define	SID_SUB		"MEM2"
#define	SID_VRAM	"MEM3"
#define	SID_PCG		"MEM4"
#define	SID_ADPCM	"MEMA"
#define	SID_ERAM	"MEMB"
#define	SID2		"MEM5"

static	T_SUSPEND_W	suspend_memory_work[]=
{
  { TYPE_INT,	&set_version,		},
  { TYPE_INT,	&use_extram,		},
  { TYPE_INT,	&use_jisho_rom,		},

  { TYPE_INT,	&use_pcg,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_memory_work2[]=
{
  { TYPE_INT,	&linear_ext_ram,	},
  { TYPE_END,	0			},
};


int	statesave_memory( void )
{
  if( statesave_table( SID, suspend_memory_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2, suspend_memory_work2 ) != STATE_OK ) return FALSE;

  /* �̾���� */

  if( statesave_block( SID_MAIN, main_ram,           0x10000  ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_HIGH, main_high_ram,      0x1000   ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_SUB, &sub_romram[0x4000], 0x4000   ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_VRAM, main_vram,          4*0x4000 ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_PCG,  font_pcg,           8*256*2  ) != STATE_OK )
								return FALSE;

  /* ���ץ���ʥ�ʥ��� */

  if( sound_board==SOUND_II ){
    if( statesave_block( SID_ADPCM, sound2_adpcm,    0x40000  ) != STATE_OK )
								return FALSE;
  }
  if( use_extram ){
    if( statesave_block( SID_ERAM, ext_ram, 0x8000*4*use_extram ) != STATE_OK )
								return FALSE;
  }
  return TRUE;
}

int	stateload_memory( void )
{
  if( stateload_table( SID, suspend_memory_work ) != STATE_OK ) return FALSE;

  /* ��0.6.3 �ϥ��ơ��ȥ�������¸���ˡ� ROM�С�������ͤ�С���������ѹ���
     �Ȥ��� ��¸���Ƥ��롣(�Ĥޤꡢ���ץ���� -server X ������֤ˤʤäƤ���)
     0.6.4�� �Ǥϡ����Τ褦�ʤ��ȤϹԤ�ʤ���(���� ROM �С����������)
     �ä˸ߴ����˼³��Ϥʤ��Ȼפ����ġ��ʲ���ä���Ф���˰���? */
  /* if (set_version == rom_version) set_version = 0; */


  if( stateload_table( SID2, suspend_memory_work2 ) != STATE_OK ) {

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );

    linear_ext_ram = TRUE;
  }


  /* �̾���� */

  if( stateload_block( SID_MAIN, main_ram,           0x10000  ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_HIGH, main_high_ram,      0x1000   ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_SUB, &sub_romram[0x4000], 0x4000   ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_VRAM, main_vram,          4*0x4000 ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_PCG,  font_pcg,           8*256*2  ) != STATE_OK )
								return FALSE;

  /* ���ץ���ʥ�ʥ��� */

  if( memory_allocate_additional()== FALSE ){
    return FALSE;
  }
  if( sound_board==SOUND_II ){
    if( stateload_block( SID_ADPCM, sound2_adpcm,    0x40000  ) != STATE_OK )
								return FALSE;
  }
  if( use_extram ){
    if( stateload_block( SID_ERAM, ext_ram, 0x8000*4*use_extram ) != STATE_OK )
								return FALSE;
  }
  return TRUE;
}
