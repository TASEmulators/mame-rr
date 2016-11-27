/***************************************************************************

    Seibu Sound System v1.02, games using this include:

    Cross Shooter    1987   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Cabal            1988   * "Michel/Seibu    sound 11/04/88" (YM2151 substituted for YM3812, unknown ADPCM)
    Dead Angle       1988   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (2xYM2203 substituted for YM3812, unknown ADPCM)
    Dynamite Duke    1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden           1990   * "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Blood Brothers   1990     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    D-Con            1992     "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    Related sound programs (not implemented yet):

    Zero Team                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Legionaire                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812)
    Raiden 2                  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Raiden DX                 "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    Cup Soccer                "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC." (YM2151 substituted for YM3812, plus extra MSM6205)
    SD Gundam Psycho Salamander "Copyright by King Bee Sol 1991"
    * = encrypted

***************************************************************************/

#include "devlegcy.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/2151intf.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"

ADDRESS_MAP_EXTERN(seibu_sound_map, 8);
ADDRESS_MAP_EXTERN(seibu2_sound_map, 8);
ADDRESS_MAP_EXTERN(seibu2_raiden2_sound_map, 8);
ADDRESS_MAP_EXTERN(seibu3_sound_map, 8);
ADDRESS_MAP_EXTERN(seibu3_adpcm_sound_map, 8);

READ16_HANDLER( seibu_main_word_r );
READ8_HANDLER( seibu_main_v30_r );
WRITE16_HANDLER( seibu_main_word_w );
WRITE8_HANDLER( seibu_main_v30_w );

WRITE16_HANDLER( seibu_main_mustb_w );

WRITE8_HANDLER( seibu_irq_clear_w );
WRITE8_HANDLER( seibu_rst10_ack_w );
WRITE8_HANDLER( seibu_rst18_ack_w );
WRITE8_HANDLER( seibu_bank_w );
WRITE8_HANDLER( seibu_coin_w );
void seibu_ym3812_irqhandler(running_device *device, int linestate);
void seibu_ym2151_irqhandler(running_device *device, int linestate);
void seibu_ym2203_irqhandler(running_device *device, int linestate);
READ8_HANDLER( seibu_soundlatch_r );
READ8_HANDLER( seibu_main_data_pending_r );
WRITE8_HANDLER( seibu_main_data_w );
MACHINE_RESET( seibu_sound );
void seibu_sound_decrypt(running_machine *machine,const char *cpu,int length);

void seibu_adpcm_decrypt(running_machine *machine, const char *region);
WRITE8_DEVICE_HANDLER( seibu_adpcm_adr_w );
WRITE8_DEVICE_HANDLER( seibu_adpcm_ctl_w );

DECLARE_LEGACY_SOUND_DEVICE(SEIBU_ADPCM, seibu_adpcm);

extern const ym3812_interface seibu_ym3812_interface;
extern const ym2151_interface seibu_ym2151_interface;
extern const ym2203_interface seibu_ym2203_interface;

/**************************************************************************/

#define SEIBU_COIN_INPUTS											\
	PORT_START("COIN")											\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(4)		\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(4)


#define SEIBU_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD("audiocpu", Z80, freq)								\
	MDRV_CPU_PROGRAM_MAP(seibu_sound_map)							\

#define SEIBU2_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD("audiocpu", Z80, freq)								\
	MDRV_CPU_PROGRAM_MAP(seibu2_sound_map)						\

#define SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(freq)						\
	MDRV_CPU_ADD("audiocpu",  Z80, freq)								\
	MDRV_CPU_PROGRAM_MAP(seibu2_raiden2_sound_map)				\

#define SEIBU3_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD("audiocpu", Z80, freq)								\
	MDRV_CPU_PROGRAM_MAP(seibu3_sound_map)						\

#define SEIBU3A_SOUND_SYSTEM_CPU(freq)								\
	MDRV_CPU_ADD("audiocpu", Z80, freq)								\
	MDRV_CPU_PROGRAM_MAP(seibu3_adpcm_sound_map)					\

#define SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(freq1,freq2)			\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD("ymsnd", YM3812, freq1)								\
	MDRV_SOUND_CONFIG(seibu_ym3812_interface)						\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\
																	\
	MDRV_OKIM6295_ADD("oki", freq2, OKIM6295_PIN7_LOW)				\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\

#define SEIBU_SOUND_SYSTEM_YM3812_RAIDEN_INTERFACE(freq1,freq2)		\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD("ymsnd", YM3812, freq1)								\
	MDRV_SOUND_CONFIG(seibu_ym3812_interface)						\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\
																	\
	MDRV_OKIM6295_ADD("oki", freq2, OKIM6295_PIN7_HIGH)				\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)						\

#define SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(freq1,freq2)			\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD("ymsnd", YM2151, freq1)								\
	MDRV_SOUND_CONFIG(seibu_ym2151_interface)						\
	MDRV_SOUND_ROUTE(0, "mono", 0.50)								\
	MDRV_SOUND_ROUTE(1, "mono", 0.50)								\
																	\
	MDRV_OKIM6295_ADD("oki", freq2, OKIM6295_PIN7_LOW)				\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\


#define SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(freq1,freq2,regiona, regionb)		\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD("ymsnd", YM2151, freq1)								\
	MDRV_SOUND_CONFIG(seibu_ym2151_interface)						\
	MDRV_SOUND_ROUTE(0, "mono", 0.50)								\
	MDRV_SOUND_ROUTE(1, "mono", 0.50)								\
																	\
	MDRV_OKIM6295_ADD("oki1", freq2, OKIM6295_PIN7_HIGH)			\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\
																	\
	MDRV_OKIM6295_ADD("oki2", freq2, OKIM6295_PIN7_HIGH)			\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\


#define SEIBU_SOUND_SYSTEM_YM2203_INTERFACE(freq)					\
	MDRV_SPEAKER_STANDARD_MONO("mono")								\
																	\
	MDRV_SOUND_ADD("ym1", YM2203, freq)								\
	MDRV_SOUND_CONFIG(seibu_ym2203_interface)						\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)						\
																	\
	MDRV_SOUND_ADD("ym2", YM2203, freq)								\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)						\

#define SEIBU_SOUND_SYSTEM_ADPCM_INTERFACE							\
	MDRV_SOUND_ADD("adpcm1", SEIBU_ADPCM, 8000)						\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40) 					\
																	\
	MDRV_SOUND_ADD("adpcm2", SEIBU_ADPCM, 8000)						\
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)						\


/**************************************************************************/

