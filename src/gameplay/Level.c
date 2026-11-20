#include "Level.h"
#include "SDL_stdinc.h"

int xmlDepth;

static void LevelParser_ParseGraphics(
    LevelMgr *levelMgr, void *data, const char *element, const char **attribute
) {

    LevelGraphics* ptr = realloc(
        levelMgr->graphics,
        sizeof(LevelGraphics) * (levelMgr->graphicsLen+1)
    );

    if (!ptr) {
        Engine_PushErrorFile("levels.xml", "Out of Memory!");
        return;
    }

    levelMgr->graphics = ptr;
    levelMgr->graphicsLen++;

    LevelGraphics* g = &levelMgr->graphics[levelMgr->graphicsLen-1];
    SDL_strlcpy(g->id, "none", sizeof(g->id));
    SDL_strlcpy(g->spiralFile, "none", sizeof(g->spiralFile));
    SDL_strlcpy(g->spiral2File, "none", sizeof(g->spiral2File));
    SDL_strlcpy(g->textureFile, "none", sizeof(g->textureFile));
    SDL_strlcpy(g->textureTopLayerFile, "none", sizeof(g->textureTopLayerFile));
    SDL_strlcpy(g->dispName, "none", sizeof(g->dispName));
    g->frogPos.x = 0;
    g->frogPos.y = 0;
    g->coinsLen = 0;
    g->coinsPos = NULL;

    for (int i = 0; attribute[i]; i += 2) {
        if (strcmp(attribute[i], "id") == 0)
            SDL_strlcpy(g->id, attribute[i+1], sizeof(g->id));
        else if (strcmp(attribute[i], "curve") == 0)
            SDL_strlcpy(g->spiralFile, attribute[i+1], sizeof(g->spiralFile));
        else if (strcmp(attribute[i], "curve2") == 0)
            SDL_strlcpy(g->spiral2File, attribute[i+1], sizeof(g->spiral2File));
        else if (strcmp(attribute[i], "image") == 0) 
            SDL_strlcpy(g->textureFile, attribute[i+1], sizeof(g->textureFile));
        else if (strcmp(attribute[i], "image-top") == 0)
            SDL_strlcpy(g->textureTopLayerFile, attribute[i+1], sizeof(g->textureTopLayerFile));
        else if (strcmp(attribute[i], "dispname") == 0)
            SDL_strlcpy(g->dispName, attribute[i+1], sizeof(g->dispName));
        else if (strcmp(attribute[i], "gx") == 0)
            g->frogPos.x = atof(attribute[i+1]);
        else if (strcmp(attribute[i], "gy") == 0)
            g->frogPos.y = atof(attribute[i+1]);
    }

}


static void LevelParser_ParseTreasurePoints(
    LevelMgr *levelMgr, void *data, const char *element, const char **attribute
) {
    LevelGraphics* g = &levelMgr->graphics[levelMgr->graphicsLen-1];
        
    SDL_FPoint* ptr = (SDL_FPoint*)realloc(
        g->coinsPos,
        sizeof(SDL_FPoint) * (g->coinsLen+1));
    if (!ptr) {
        Engine_PushErrorFile("levels.xml", "Out of Memory!");
        return;
    }

    g->coinsPos = ptr;
    g->coinsLen++;

    for (int i = 0; attribute[i]; i += 2) {
        if (strcmp(attribute[i], "x") == 0)
            g->coinsPos[g->coinsLen-1].x = atof(attribute[i+1]);
        else if (strcmp(attribute[i], "y") == 0)
            g->coinsPos[g->coinsLen-1].y = atof(attribute[i+1]);
    } 
}


static void LevelParser_ParseSettings(
    LevelMgr *levelMgr, void *data, const char *element, const char **attribute
) {
    LevelSettings* ptr = realloc(
        levelMgr->settings,
        sizeof(LevelSettings) * (levelMgr->settingsLen+1)
    );

    if (!ptr) {
        Engine_PushErrorFile("levels.xml", "Out of Memory!");
        return;
    }

    levelMgr->settings = ptr;
    levelMgr->settingsLen++;

    LevelSettings* s = &levelMgr->settings[levelMgr->settingsLen-1];

    SDL_strlcpy(s->id, "none", sizeof(s->id));
    s->ballSpd = 0.5;
    s->ballStartCount = 35;
    s->gaugeScore = 1000;
    s->repeatChance = 40;
    s->singleChance = 6;
    s->ballColors = 4;
    s->partTime = 30;
    s->slowFactor = 4;

    for (int i = 0; attribute[i]; i += 2) {
        if (strcmp(attribute[i], "id") == 0)
            SDL_strlcpy(s->id, attribute[i+1], sizeof(s->id));
        else if (strcmp(attribute[i], "speed") == 0)
            s->ballSpd = atof(attribute[i+1]);
        else if (strcmp(attribute[i], "start") == 0)
            s->ballStartCount = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "score") == 0)
            s->gaugeScore = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "repeat") == 0)
            s->repeatChance = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "colors") == 0)
            s->ballColors = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "single") == 0)
            s->singleChance = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "partime") == 0)
            s->partTime = atoi(attribute[i+1]);
        else if (strcmp(attribute[i], "slowfactor") == 0)
            s->slowFactor = atof(attribute[i+1]);
    }
}


static void LevelParser_ParseStageProgression(
    LevelMgr *levelMgr, void *data, const char *element, const char **attribute
) {

    for (int i = 0; i < STAGE_COUNT; i++) {
        levelMgr->stages[i].levelsLen = 0;
        levelMgr->stages[i].levels = NULL;
    }

    for (int i = 0; attribute[i]; i += 2) {
        if (strncmp(attribute[i], "stage", 5) == 0) {
            char* cid = strtok((char*)attribute[i], "stage");
            int stageID = cid ? atoi(cid)-1 : -1;

            if (stageID < 0 || stageID >= STAGE_COUNT) {
                continue;
            }

            char* gID = strtok((char*)attribute[i+1], ",");
            while (gID) {
                for (int i = 0; i < levelMgr->graphicsLen; i++) {
                    if (strcmp(gID, levelMgr->graphics[i].id) == 0) {
                        Stage* stage = &levelMgr->stages[stageID];

                        Level* ptr = realloc(
                            stage->levels,
                            sizeof(Level) * (stage->levelsLen+1));
                        if (!ptr) {
                            Engine_PushErrorFile("levels.xml", "Out of Memory!");
                            return;
                        }

                        stage->levels = ptr;
                        stage->levelsLen++;

                        Level* lv = &stage->levels[stage->levelsLen-1];
                        lv->settingsID = NULL;
                        lv->graphicsID = i;

                        break;
                    }
                }
                gID = strtok(NULL, ",");
            }
        } else if (strncmp(attribute[i], "diffi", 5) == 0) {
            char* cid = strtok((char*)attribute[i], "diffi");
            int stageID = cid ? atoi(cid)-1 : -1;

            if (stageID < 0 || stageID >= STAGE_COUNT) {
                continue;
            }

            Stage* stage = &levelMgr->stages[stageID];

            char* sID = strtok((char*)attribute[i+1], ",");
            int j = 0;
            while (sID && j < stage->levelsLen) {
                for (int i = 0; i < levelMgr->settingsLen; i++) {
                    if (strcmp(sID, levelMgr->settings[i].id) == 0) {
                        Level* lv = &stage->levels[j];
                        int* ids = realloc(lv->settingsID, sizeof(int));
                        if (!ids) {
                            Engine_PushErrorFile("levels.xml", "Out of Memory!");
                            return;
                        }
                        lv->settingsID = ids;
                        lv->settingsID[0] = i;
                        break;
                    }
                }
                sID = strtok(NULL, ",");
                j++;
            }
        }
    }
}


void XML_StartElement(void *data, const char *element, const char **attribute) {
    LevelMgr *levelMgr = (LevelMgr *)data; 
    if (strcmp(element, "Graphics") == 0) {
        LevelParser_ParseGraphics(levelMgr, data, element, attribute);
    } else if (xmlDepth == 2 && strcmp(element, "TreasurePoint") == 0) {
        LevelParser_ParseTreasurePoints(levelMgr, data, element, attribute);
    } else if (strcmp(element, "Settings") == 0) {
        LevelParser_ParseSettings(levelMgr, data, element, attribute);
    } else if (strcmp(element, "LevelProgression") == 0) {
        // TODO
    } else if (strcmp(element, "Level") == 0) {
        // TODO
    } else if (strcmp(element, "StageProgression") == 0) {
        LevelParser_ParseStageProgression(levelMgr, data, element, attribute);
    }

    xmlDepth++;
}

void XML_EndElement(void *data, const char *element) {
    xmlDepth--;
}

int LevelMgr_LoadLevels(LevelMgr *levelMgr, const char* fileName) {
    levelMgr->graphics = NULL;
    levelMgr->graphicsLen = 0;
    levelMgr->settings = NULL;
    levelMgr->settingsLen = 0;
    levelMgr->survivalLevels = NULL;
    levelMgr->survivalLevelsLen = 0;

    SDL_memset(levelMgr->bestScore, 0, sizeof(levelMgr->bestScore));
    SDL_memset(levelMgr->bestTime, 0, sizeof(levelMgr->bestTime));

    int done;

    char path[512];
    char* basePath = SDL_GetBasePath();
    SDL_snprintf(path, sizeof(path), "%s%s/%s", basePath ? basePath : "", PATH_LEVEL, fileName);
    SDL_free(basePath);

    char* buff = malloc(XML_BUFF_SIZE);
    if (!buff) {
        Engine_PushErrorFile(path, "Out of memory!");
        return 0;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        Engine_PushErrorFile(path, "Could not open file!");
        free(buff);
        return 0;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    if (!parser) {
        Engine_PushErrorFile(path, "Out of memory!");
        free(buff);
        fclose(fp);
        return 0;
    }
    XML_SetUserData(parser, levelMgr);
    XML_SetElementHandler(parser, XML_StartElement, XML_EndElement);

    do {
        int len = (int)fread(buff, 1, XML_BUFF_SIZE, fp);
        done = len < XML_BUFF_SIZE;
        if (XML_Parse(parser, buff, len, done) == XML_STATUS_ERROR) {
            Engine_PushErrorFile(path, XML_ErrorString(XML_GetErrorCode(parser)));
            free(buff);
            fclose(fp);
            XML_ParserFree(parser);
            return 0;
        }
    } while(!done);

    free(buff);
    fclose(fp);
    XML_ParserFree(parser);

    if (!LevelMgr_LoadProgress(levelMgr))
        return 0;

    return 1;
}

#define PROGRESS_FILE "progress.bin"

int LevelMgr_SaveProgress(LevelMgr *levelMgr) {
    char path[512];
    Engine_GetSavePath(path, sizeof(path), PROGRESS_FILE);

    FILE* file = fopen(path, "wb");
    if (!file) {
        return 0;
    }

    fputs("zumahdprog", file);
    for (int i = 0; i < LEVELS_COUNT; i++) {
        for (int j = 0; j < 4; j++) {
            fwrite(&levelMgr->bestScore[i][j],
                sizeof(int), 1, file);
            fwrite(&levelMgr->bestTime[i][j], 
                sizeof(int), 1, file);
        }
    }

    fclose(file);
    return 1;
}

int LevelMgr_LoadProgress(LevelMgr *levelMgr) {
    char path[512];
    Engine_GetSavePath(path, sizeof(path), PROGRESS_FILE);

    FILE* file = fopen(path, "rb");

    /* older versions saved progress.bin in the current folder: read it once,
       the save at the end writes it to the new place */
    bool migrating = false;
    if (!file) {
        file = fopen(PROGRESS_FILE, "rb");
        migrating = (file != NULL);
    }

    if (!file) {
        if (!LevelMgr_SaveProgress(levelMgr)) {
            Engine_PushError("Error reading file \"progress.bin\".",
                "Could not open file for reading.");
            return 0;
        }
        return 1;
    }

    char buff[11];
    fgets(buff, 11, file);
    if (strcmp(buff, "zumahdprog") != 0) {
        Engine_PushError("Error reading file \"progress.bin\".",
            "Wrong file type.");
        fclose(file);
        return 0;
    }

    for (int i = 0; i < LEVELS_COUNT; i++) {
        for (int j = 0; j < 4; j++) {
            if (fread(&levelMgr->bestScore[i][j],
                sizeof(int), 1, file) != 1) {
                    Engine_PushError("File read error \"progress.bin\".",
                        "The file is damaged.");
                    fclose(file);
                    return 0;
                }
            
            if (fread(&levelMgr->bestTime[i][j], 
                sizeof(int), 1, file) != 1) {
                    Engine_PushError("File read error \"progress.bin\".",
                        "The file is damaged.");
                    fclose(file);
                    return 0;
                }
        }
    }

    fclose(file);

    if (migrating)
        LevelMgr_SaveProgress(levelMgr);

    return 1;
}

Stage* LevelMgr_GetStage(LevelMgr *levelMgr, int stageID) {
    if (stageID < 0 || stageID >= STAGE_COUNT)
        return NULL;

    return &levelMgr->stages[stageID];
}

Level* LevelMgr_GetLevelFromStage(LevelMgr *levelMgr, int stageID, int lvID) {
    if (stageID < 0 || stageID >= STAGE_COUNT)
        return NULL;

    if (lvID < 0 || lvID >= levelMgr->stages[stageID].levelsLen)
        return NULL;

    return &levelMgr->stages[stageID].levels[lvID];
}

LevelGraphics* LevelMgr_GetLevelGraphics(LevelMgr *levelMgr, Level* lv) {
    if (!lv)
        return NULL;

    return &levelMgr->graphics[lv->graphicsID];
}

LevelSettings* LevelMgr_GetLevelSettings(LevelMgr *levelMgr, Level* lv, int settingID) {
    if (!lv || !lv->settingsID)
        return NULL;

    if (settingID < 0)
        return NULL;

    return &levelMgr->settings[lv->settingsID[settingID]];
}

void LevelMgr_Free(LevelMgr *levelMgr) {
    for (int i = 0; i < levelMgr->graphicsLen; i++)
        free(levelMgr->graphics[i].coinsPos);

    free(levelMgr->graphics);
    free(levelMgr->settings);

    for (int i = 0; i < STAGE_COUNT; i++)
        free(levelMgr->stages[i].levels);
}

////////////////////////////////////
////////////////////////////////////

/* the longest path a level may have; the original ones have 2500 to 3700 steps */
#define SPIRAL_MAX_LEN 100000
#define CURVE_CONTROL_POINT_SIZE 10

/* reads a curve file (.dat): "CURV", version, flags, size of the control point block,
   control point count and the control points (skipped), then the point count, the start
   point and one step per point (t1, t2, dx and dy in hundredths, signed). Little-endian. */
static int Level_LoadSpiral(const char* path, SDL_FPoint* start, SpiralDot** spiral, int* spiralLen) {
    *spiral = NULL;
    *spiralLen = 0;

    SDL_RWops* rw = SDL_RWFromFile(path, "rb");
    if (!rw) {
        Engine_PushErrorFile(path, "Could not open file.");
        return 0;
    }

    Sint64 fileSize = SDL_RWsize(rw);
    char magic[4];
    if (fileSize < 0x14 || SDL_RWread(rw, magic, 1, 4) != 4 || SDL_memcmp(magic, "CURV", 4) != 0) {
        Engine_PushErrorFile(path, "Not a curve file.");
        SDL_RWclose(rw);
        return 0;
    }

    SDL_RWseek(rw, 0x10, RW_SEEK_SET);
    Sint32 controlCount = (Sint32)SDL_ReadLE32(rw);
    Sint64 pointsOffset = 0x14 + (Sint64)controlCount * CURVE_CONTROL_POINT_SIZE;
    if (controlCount < 0 || pointsOffset + 12 > fileSize) {
        Engine_PushErrorFile(path, "The file is damaged.");
        SDL_RWclose(rw);
        return 0;
    }

    SDL_RWseek(rw, pointsOffset, RW_SEEK_SET);
    Sint32 pointCount = (Sint32)SDL_ReadLE32(rw);
    Uint32 bits = SDL_ReadLE32(rw);
    SDL_memcpy(&start->x, &bits, sizeof(float));
    bits = SDL_ReadLE32(rw);
    SDL_memcpy(&start->y, &bits, sizeof(float));

    int len = pointCount - 1;
    if (len < 1 || len > SPIRAL_MAX_LEN || pointsOffset + 12 + (Sint64)len * 4 > fileSize) {
        Engine_PushErrorFile(path, "The file is damaged.");
        SDL_RWclose(rw);
        return 0;
    }

    Uint8* raw = malloc((size_t)len * 4);
    SpiralDot* dots = malloc(sizeof(SpiralDot) * (size_t)len);
    if (!raw || !dots || SDL_RWread(rw, raw, 4, (size_t)len) != (size_t)len) {
        Engine_PushErrorFile(path, raw && dots ? "The file is damaged." : "Out of memory!");
        free(raw);
        free(dots);
        SDL_RWclose(rw);
        return 0;
    }
    SDL_RWclose(rw);

    for (int i = 0; i < len; i++) {
        dots[i].t1 = raw[i * 4] != 0;
        dots[i].t2 = raw[i * 4 + 1] != 0;
        /* signed on every platform (plain char is unsigned on ARM Linux) */
        dots[i].dx = (float)(Sint8)raw[i * 4 + 2] / 100.0f;
        dots[i].dy = (float)(Sint8)raw[i * 4 + 3] / 100.0f;
    }
    free(raw);

    *spiral = dots;
    *spiralLen = len;
    return 1;
}

int Level_Load(LevelMgr *levelMgr, Engine *engine, Level* level) {
    LevelGraphics* g = &levelMgr->graphics[level->graphicsID];

    level->texture = NULL;
    level->textureTopLayer = NULL;
    level->spiral = NULL;
    level->spiralLen = 0;
    level->spiral2 = NULL;
    level->spiral2Len = 0;

    char path[512];
    char* basePath = SDL_GetBasePath();
    const char* base = basePath ? basePath : "";

    SDL_snprintf(path, sizeof(path), "%s%s/%s/%s.jpg", base, PATH_LEVEL, g->id, g->textureFile);
    level->texture = Engine_TextureLoad(engine, path);

    if (level->texture && strcmp(g->textureTopLayerFile, "none") != 0) {
        SDL_snprintf(path, sizeof(path), "%s%s/%s/%s.png", base, PATH_LEVEL, g->id, g->textureTopLayerFile);
        level->textureTopLayer = Engine_TextureLoad(engine, path);
    }

    int ok = level->texture != NULL;
    if (ok) {
        SDL_snprintf(path, sizeof(path), "%s%s/%s/%s.dat", base, PATH_LEVEL, g->id, g->spiralFile);
        ok = Level_LoadSpiral(path, &level->spiralStart, &level->spiral, &level->spiralLen);
    }

    SDL_free(basePath);

    if (!ok)
        Level_Free(level);

    return ok;
}

void Level_Free(Level* level) {
    if (level->texture)
        SDL_DestroyTexture(level->texture);
    if (level->textureTopLayer)
        SDL_DestroyTexture(level->textureTopLayer);

    free(level->spiral);
    free(level->spiral2);

    level->texture = NULL;
    level->textureTopLayer = NULL;
    level->spiral = NULL;
    level->spiral2 = NULL;
    level->spiralLen = 0;
    level->spiral2Len = 0;
}

void Level_Draw(Engine *engine, Level* lvl) {
    int w, h;
    SDL_RenderGetLogicalSize(engine->render, &w, &h);
    Engine_DrawTextureSDL(engine, lvl->texture, w / 2, h / 2);
}

void Level_DrawTopLayer(Engine *engine, Level* lvl) {
    int w, h;
    SDL_RenderGetLogicalSize(engine->render, &w, &h);
    Engine_DrawTextureSDL(engine, lvl->textureTopLayer, w / 2, h / 2);
}

void Level_DrawDebug(LevelMgr *levelMgr, Engine *engine, Level* lvl) {
    float sx, sy;
    sx = (104 + lvl->spiralStart.x) * engine->scale_x;
    sy = lvl->spiralStart.y * engine->scale_y;
    SDL_SetRenderDrawColor(engine->render, 255, 255, 255, 160);
    for(int i = 0; i < lvl->spiralLen; i++) {
        if (lvl->spiral[i].t1)
            SDL_SetRenderDrawColor(engine->render,
                0, 255, 0, 255);
        else 
            SDL_SetRenderDrawColor(engine->render,
                255, 255, 255, 255);

        SDL_RenderDrawLineF(
            engine->render, 
            sx, 
            sy, 
            sx+lvl->spiral[i].dx * engine->scale_x, 
            sy+lvl->spiral[i].dy * engine->scale_y
        );

        /*if (lvl->spiral[i].t2) {
            SDL_SetRenderDrawColor(engine->render,
                0, 0, 255, 128);

            SDL_Rect rect = {sx, sy, 16, 16};
            SDL_RenderFillRect(engine->render, &rect);

            SDL_SetRenderDrawColor(engine->render,
                255, 255, 255, 255);
        }*/

        sx = sx+lvl->spiral[i].dx*engine->scale_x;
        sy = sy+lvl->spiral[i].dy*engine->scale_y;
    }

    LevelGraphics* g = LevelMgr_GetLevelGraphics(levelMgr, lvl);
    SDL_SetRenderDrawColor(engine->render, 0, 255, 0, 160);
    for(int i = 0; i < g->coinsLen; i++) {
        SDL_FRect rect = {
            (104 + g->coinsPos[i].x - 16)*engine->scale_x, 
            (g->coinsPos[i].y - 16)*engine->scale_y,
            32,
            32
        };
        SDL_RenderDrawRectF(engine->render, &rect);
    }
}

char* Level_GetInfo(LevelMgr *levelMgr, Level* lvl) {
    const size_t size = 1024;
    char* info = malloc(size);
    if (!info)
        return NULL;

    LevelGraphics* g = LevelMgr_GetLevelGraphics(levelMgr, lvl);
    int n = SDL_snprintf(info, size,
        " *- Graphics -*\n"
        " id: %s\n dispName: %s\n spiral: %s\n spiral2: %s\n"
        " texture: %s\n top texture: %s\n frog x: %g; y: %g\n\n",
        g->id, g->dispName, g->spiralFile, g->spiral2File,
        g->textureFile, g->textureTopLayerFile, g->frogPos.x, g->frogPos.y);

    LevelSettings* s = LevelMgr_GetLevelSettings(levelMgr, lvl, 0);
    if (s && n >= 0 && (size_t)n < size) {
        SDL_snprintf(info + n, size - (size_t)n,
            " *- Settings -*\n"
            " id: %s\n part time: %d\n repeat chance: %d\n single chance: %d\n"
            " slow factor: %f\n balls colors: %d\n balls spd: %f\n"
            " balls start count: %d\n score: %d\n",
            s->id, s->partTime, s->repeatChance, s->singleChance,
            s->slowFactor, s->ballColors, s->ballSpd, s->ballStartCount, s->gaugeScore);
    }

    return info;
}
