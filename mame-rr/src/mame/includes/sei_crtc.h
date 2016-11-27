/*----------- defined in video/sei_crtc.c -----------*/

extern UINT16 *seibucrtc_sc0vram,*seibucrtc_sc1vram,*seibucrtc_sc2vram,*seibucrtc_sc3vram;
extern UINT16 *seibucrtc_vregs;
extern UINT16 seibucrtc_sc0bank;

WRITE16_HANDLER( seibucrtc_sc0vram_w );
WRITE16_HANDLER( seibucrtc_sc1vram_w );
WRITE16_HANDLER( seibucrtc_sc2vram_w );
WRITE16_HANDLER( seibucrtc_sc3vram_w );
WRITE16_HANDLER( seibucrtc_vregs_w );
void seibucrtc_sc0bank_w(UINT16 data);
VIDEO_START( seibu_crtc );
VIDEO_UPDATE( seibu_crtc );
