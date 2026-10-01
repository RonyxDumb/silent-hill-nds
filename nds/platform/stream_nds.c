#include "game.h"

/* The PSX STR/MDEC decoder is not an ARM9 renderer dependency. Keep the
 * original game-state flow alive by skipping FMVs and handing control back to
 * the next native engine state. */
void GameState_MovieIntroFadeIn_Update(void)
{
    Game_StateSetNext(GameState_MovieIntro);
}

void GameState_MovieIntro_Update(void)
{
    Game_StateSetNext(GameState_MainMenu);
}

void GameState_MovieOpening_Update(void)
{
    Game_StateSetNext(GameState_MainLoadScreen);
}

void GameState_ExitMovie_Update(void)
{
    Game_StateSetNext(GameState_InGame);
}

void GameState_DebugMoviePlayer_Update(void)
{
    Game_StateSetNext(GameState_MainMenu);
}

void GameState_MovieIntroAlternate_Update(void)
{
    Game_StateSetNext(GameState_MainMenu);
}

void open_main(s32 fileIdx, s32 frameOffset)
{
    (void)fileIdx;
    (void)frameOffset;
}
