#include <stdlib.h>
#include "SDL3/SDL.h"
#include "ui.h"

void *UI_MemAlloc(size_t size) {
    return malloc(size);
}

void *UI_MemExpand(void *block, size_t size) {
    return realloc(block, size);
}

void *UI_MemShrink(void *block, size_t size) {
    void *new_block = realloc(block, size);
    return new_block == NULL ? block : new_block;
}

void UI_MemFree(void *block) {
    free(block);
}

bool UI_DrawRect(UIContext *ctx, UIRect rect, UIColor color) {
    SDL_Renderer *renderer = (SDL_Renderer *)ctx->userData;
    if (!SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a))
        return false;
    SDL_FRect sdlRect = {
        .x = rect.x,
        .y = rect.y,
        .w = rect.w,
        .h = rect.h,
    };
    if (!SDL_RenderFillRect(renderer, &sdlRect))
        return false;
    return true;
}
