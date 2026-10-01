/* ARM9-resident MAP0_S00 state. The PSX version keeps these in the map
 * overlay; the DS build links the starting map into the base ARM9 image. */

q19_12 g_Cutscene_Timer;
VECTOR3 g_Cutscene_CameraPositionTarget;
VECTOR3 g_Cutscene_CameraLookAtTarget;
VECTOR3 g_WarpCamera;

/* Per-room BGM flags and current mix transition state. */
u32 D_800DF300[7];
q19_12 D_800DF2F8;

/* MAP0_S00's 35x40 fallback chunk grid and its 5x5 exterior-grid override.
 * The original bytes are map-overlay data; retain deterministic safe values
 * until the map asset loader supplies its decoded room table. */
u8 MAP_ROOM_IDXS[35 * 40];
u8 sharedData_800DF2DC_0_s00[5 * 5];

/* Cutscene message audio state used by the opening-map event scripts. */
u16 g_Cutscene_MapMsgAudioIdx;
u8 g_Cutscene_MapMsgAudioCmds[64];

/* Map-event working bytes referenced by the opening alley sequence. */
u8 D_800DFAC4;
u8 D_800DFAC8;
u8 D_800DFB61;
