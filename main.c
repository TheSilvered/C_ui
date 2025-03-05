#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SDL3/SDL.h"

#define UI_IMPLEMENTATION
#include "ui.h"
#include "SDL3_impl.c"

bool generateLayout(UIElement *root);
void logErrorAndExit(void);

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO))
        logErrorAndExit();

    SDL_Window *window = SDL_CreateWindow("C UI", 800, 600, SDL_WINDOW_RESIZABLE);
    if (window == NULL) logErrorAndExit();
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) logErrorAndExit();

    UIContext context;
    if (!UIContext_Init(&context, UI_DEFAULT_MAX_ELEMENT_COUNT, (void *)renderer))
        return 1;

    if (!generateLayout(context.root))
        return 1;

    bool running = true;
    while (running) {
        for (SDL_Event event; SDL_PollEvent(&event);) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        UIContext_UpdateWindow(&context, w, h);

        if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE))
            logErrorAndExit();
        if (!SDL_RenderClear(renderer))
            logErrorAndExit();

        if (!UIContext_Draw(&context))
            return 1;

        if (!SDL_RenderPresent(renderer))
            logErrorAndExit();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void logErrorAndExit(void)  {
    fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
    exit(1);
}

bool generateLayout(UIElement *root) {
    UI_BackgroundColor(root, (UIColor) { 0, 0, 0, 255 });
    UI_Padding(root, 40.0f);

    UIElement *outer = UIElement_New(root);
    UI_BackgroundColor(outer, (UIColor) { 255, 0, 0, 255 });
    UI_AlignX(outer, UIAlignX_center);
    UI_Padding(outer, 10.0f);

    UIElement *child1 = UIElement_New(outer);
    UI_BackgroundColor(child1, (UIColor) { 0, 255, 0, 255 });
    UI_FixedWidth(child1, 300.0f);
    UI_FixedHeight(child1, 300.0f);

    UIElement *child2 = UIElement_New(outer);
    UI_BackgroundColor(child2, (UIColor) { 0, 0, 255, 255 });
    UI_FixedWidth(child2, 200.0f);
    UI_FixedHeight(child2, 200.0f);
    return true;
}
