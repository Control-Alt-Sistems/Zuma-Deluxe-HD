#include "Application.h"

#include "global/Consts.h"
#include "global/Engine.h"
#include "global/Enums.h"


#include "gameplay/Game.h"
#include "menu/MenuMgr.h"
#include <stdio.h>

/* the game logic was written for 60 updates per second, so it always runs at that rate */
#define TICKS_PER_SECOND 60
#define TICK_MS (1000.0 / TICKS_PER_SECOND)
/* after a long stall (window drag, breakpoint) drop the backlog instead of fast-forwarding */
#define MAX_TICKS_PER_FRAME 5

Engine engine;

typedef struct {
  Game game;
  MenuMgr menuMgr;

  int curLvl;
  int curDifficulty;

  int inMenu;
  int mouseClicked; /* kept until a tick consumes it */

  bool skipDraw; /* the game just went back to the menu: skip one draw, as before */

  bool isRunning;
} App;

static int ApplicationZuma_LoadResources(LevelMgr *levelMgr, Engine *engine) {
  if (!Engine_LoadSettings(engine))
    return 9;

  if (!Engine_TexturesLoad(engine, filesTextures, TEXTURES_COUNT)) {
    return 3;
  }
  if (!Engine_FontsLoad(engine, filesFonts, FONTS_COUNT))
    return 4;
  const int soundCopies = 4;
  const int sfxCopies = 1;

  if (!Engine_SoundsLoad(engine, filesSounds, SOUNDS_COUNT, soundCopies))
    return 5;
  if (!Engine_SoundsSfxLoad(engine, filesSoundsSfx, SOUNDS_SFX_COUNT, sfxCopies))
    return 6;
  if (!Engine_MusicLoad(engine, fileMusic))
    return 7;

  if (!LevelMgr_LoadLevels(levelMgr, "levels.xml"))
    return 8;

  return 0;
}

static int ApplicationZuma_ShowStartupImage(Engine *engine) {
  char *basePath = SDL_GetBasePath();

  char resPath[512];
  SDL_snprintf(resPath, sizeof(resPath), "%simages/disclaimer.jpg", basePath ? basePath : "");

  SDL_free(basePath);

  SDL_Texture *texDisc = Engine_TextureLoad(engine, resPath);
  if (!texDisc)
    return 10;

  SDL_RenderCopy(engine->render, texDisc, NULL, NULL);
  SDL_RenderPresent(engine->render);
  SDL_Delay(2000);

  SDL_DestroyTexture(texDisc);
  return 0;
}

static void ApplicationZuma_HandleEvents(App *app) {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
    case SDL_QUIT:
      app->isRunning = false;
      break;
    case SDL_MOUSEBUTTONDOWN:

      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        app->mouseClicked = 1;
        break;
      case SDL_BUTTON_RIGHT:
        app->mouseClicked = 2;
        break;
      }
      break;
    }
  }
}

/* one fixed step of game logic (1/60 s) */
static void ApplicationZuma_Tick(LevelMgr *levelMgr, Engine *engine, App *app) {
  if (app->inMenu) {
    MenuMgr_Update(engine, &app->menuMgr);

    if (app->menuMgr.roomID == MR_GAME && app->inMenu) {
      app->inMenu = 0;
      Game_Init(levelMgr, engine, &app->game, app->curLvl, app->curDifficulty);
    }
  } else {
    Game_Update(levelMgr, engine, &app->game, &app->inMenu, app->mouseClicked);
    if (app->inMenu) {
      MenuMgr_Set(engine, &app->menuMgr, MR_GAUNTLET);
      app->skipDraw = true;
    }
  }

  app->mouseClicked = 0;
}

static void ApplicationZuma_Draw(LevelMgr *levelMgr, Engine *engine, App *app) {
  SDL_RenderClear(engine->render);

  if (app->inMenu)
    MenuMgr_Draw(levelMgr, engine, &app->menuMgr);
  else
    Game_Draw(levelMgr, engine, &app->game);

  SDL_RenderPresent(engine->render);
}

int ApplicationZuma_Start(void) {
  Engine engine;
  LevelMgr levelMgr;
  if (!Engine_Init(&engine))
    return 1;

  if (!Engine_CreateWindow(&engine, "Zuma HD. By GalaxyShad and s4lat", WINDOW_WIDTH, WINDOW_HEIGHT))
    return 2;

  int err = ApplicationZuma_ShowStartupImage(&engine);
  if (err) {
    Engine_Destroy(&engine);
    return err;
  }

  err = ApplicationZuma_LoadResources(&levelMgr, &engine);
  if (err) {
    Engine_Destroy(&engine);
    return err;
  }

  srand(time(NULL));

  App app;
  app.curLvl = 0;
  app.curDifficulty = 0;
  app.inMenu = 1;
  app.mouseClicked = 0;
  app.skipDraw = false;
  app.isRunning = true;

  MenuMgr_Init(&app.menuMgr, &app.curLvl, &app.curDifficulty);
  MenuMgr_Set(&engine, &app.menuMgr, MR_MAIN);

  const double counterToMs = 1000.0 / (double)SDL_GetPerformanceFrequency();
  Uint64 prevCounter = SDL_GetPerformanceCounter();
  double accumulatorMs = TICK_MS; /* run the first tick right away */

  while (app.isRunning) {
    ApplicationZuma_HandleEvents(&app);

    Uint64 now = SDL_GetPerformanceCounter();
    accumulatorMs += (double)(now - prevCounter) * counterToMs;
    prevCounter = now;

    int ticks = 0;
    while (accumulatorMs >= TICK_MS && ticks < MAX_TICKS_PER_FRAME && app.isRunning) {
      ApplicationZuma_Tick(&levelMgr, &engine, &app);
      accumulatorMs -= TICK_MS;
      ticks++;
    }

    if (ticks == MAX_TICKS_PER_FRAME)
      accumulatorMs = 0;

    if (ticks == 0) {
      /* nothing to simulate yet: give the CPU back until the next tick is due */
      SDL_Delay(1);
      continue;
    }

    /* draw only after logic ran: some animations still advance inside the draw functions,
       so drawing more often than 60 times per second would speed them up */
    if (app.skipDraw)
      app.skipDraw = false;
    else
      ApplicationZuma_Draw(&levelMgr, &engine, &app);
  }

  LevelMgr_Free(&levelMgr);
  Engine_Destroy(&engine);

  return 0;
}