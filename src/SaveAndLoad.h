#pragma once

#include "Common.h"

struct Chunk;

void SaveThreadWork(void* data);

bool SaveChunk(Chunk* chunk);
bool TryLoadChunk(Chunk* chunk);

bool SaveWorld(GameWorld* world);
