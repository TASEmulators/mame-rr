/*************************************************************************

    Operation Wolf

*************************************************************************/

class opwolf_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, opwolf_state(machine)); }

	opwolf_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *      cchip_ram;

	/* video-related */
	UINT16       sprite_ctrl;
	UINT16       sprites_flipscreen;

	/* misc */
	UINT8        adpcm_b[0x08];
	UINT8        adpcm_c[0x08];
	UINT32       adpcm_pos[2], adpcm_end[2];
	int          adpcm_data[2];

	int          opwolf_gun_xoffs, opwolf_gun_yoffs;

	/* c-chip */
	int          opwolf_region;

	UINT8        current_bank;
	UINT8        current_cmd;
	UINT8        cchip_last_7a;
	UINT8        cchip_last_04;
	UINT8        cchip_last_05;
	UINT8        cchip_coins_for_credit[2];
	UINT8        cchip_credits_for_coin[2];
	UINT8        cchip_coins[2];
	UINT8        c588, c589, c58a; // These variables derived from the bootleg

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	running_device *pc080sn;
	running_device *pc090oj;
	running_device *msm1;
	running_device *msm2;
};


/*----------- defined in machine/opwolf.c -----------*/

void opwolf_cchip_init(running_machine *machine);

READ16_HANDLER( opwolf_cchip_status_r );
READ16_HANDLER( opwolf_cchip_data_r );
WRITE16_HANDLER( opwolf_cchip_status_w );
WRITE16_HANDLER( opwolf_cchip_data_w );
WRITE16_HANDLER( opwolf_cchip_bank_w );


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( opwolf_spritectrl_w );

VIDEO_UPDATE( opwolf );
