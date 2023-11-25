#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "debugmalloc.h"
#include "text.h"
#include "custom_renders.h"
#include "constants.h"
#include "leaderboard.h"


/*A programozás alapjai 1 házi feladat: Typeracer
Gaál Botond
CRQEYD
2023.11.26.*/

/*
Az összes SDL-lel kapcsolatos változó inicializálása:
-ablak nyitás
-betűtípusok megnyitása
-renderer inicializálás
int szeles, magas: a keletkező ablak méretei
const char* tipus: a betűtípust tartalmazó fájl neve
SDL_Window** pwindow: az ablak pointere (pontosabban a pointerének pointere, de az ablakra a pointerével hivatkozunk)
SDL_Renderer** prenderer: a renderer pointerének pointere (a rendererre a pointerével hivatkozunk)
SDL_Font** pfont, punderlined, ptitle: a betűtípus pointerének pointerei (a betűtípusokra a pointerükkel hivatkozunk), 'simán', aláhúzva, és nagybetűvel félkövérrel
*/
void sdl_init(int szeles, int magas, const char* tipus, SDL_Window** pwindow, SDL_Renderer** prenderer, TTF_Font** pfont, TTF_Font** punderlined, TTF_Font** ptitle) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("Typeracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(1);
    }
    TTF_Init();
    TTF_Font *font = TTF_OpenFont(tipus, 30);
    if (font == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_Font *title = TTF_OpenFont(tipus, 80);
    if (font == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontStyle(title, TTF_STYLE_BOLD);
    TTF_Font *underlined = TTF_OpenFont(tipus, 30);
    if (underlined == NULL) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontStyle(underlined, TTF_STYLE_UNDERLINE);
    *pwindow = window;
    *prenderer = renderer;
    *pfont = font;
    *ptitle = title;
    *punderlined = underlined;
    #ifdef __WIN32__
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    #endif
}

/*
Az összes SDL-lel kapcsolatos változó bezárása/felszabadítása, amelyek dinamikusan foglaltak.
Ezek mind pointerek pointerei, mivel az egyes SDL objektumokra a pointerükkel hivatkozunk:
SDL_Window **pwindow: az ablak
SDL_Renderer **prenderer: a renderer
TTF_Font **pfont: a 'normál' betűtípus
TTF_Font **punderlined: az aláhúzott betűtípus
TTF_Font **ptitle: a nagybetűs betűtípus
*/
void sdl_close(SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **pfont, TTF_Font **punderlined, TTF_Font **ptitle) {
    SDL_DestroyRenderer(*prenderer);
    *prenderer = NULL;

    SDL_DestroyWindow(*pwindow);
    *pwindow = NULL;

    TTF_CloseFont(*pfont);
    *pfont = NULL;

    TTF_CloseFont(*punderlined);
    *pfont = NULL;

    TTF_CloseFont(*ptitle);
    *pfont = NULL;

    SDL_Quit();
}

/*
A progra, fölülről nézve egy event loop, amiben két féle eventet keresünk:
SDL_Quit: ki akar lépni a játékos a programból
GAME_VIEW_CHANGED_EVENT: a játéknézet megváltozását jelző event
A main()-ben lvő event loop várja, hogy megváltozzon a játéknézet, és a játéknézetnek megfelelő
kezelő függvényt hívja meg. Ezen felül inicializálja és felszabadítja azokat a változókat, amikre
a játéknézeteknek szüksége van.
*/
int main(int argc, char *argv[]) {
    srand(time(0)); //inicializáljuk a randomszám generátort
    const int magas = 600;
    const int szeles = 1000;
    const int margo = 40;
    Bot bots[BOT_NUM];
    init_bots(bots);
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    TTF_Font *underlined;
    TTF_Font *title;
    LeaderBoard leaderboard;
    if (!load_leaderboard(&leaderboard)) {
        perror("Nem sikerult megnyitni a ranglistat.");
        return 0;
    }
    TextArray textarray = parse_file("hobbit_long.txt"); //töltsük be a fájlt, amiből generáljuk a szövegeket
    if (textarray.texts == NULL) {
        perror("Nem sikerult beolvasni a textarray-t.");
        return 0;
    }
    sdl_init(szeles, magas, FONT, &window, &renderer, &font, &underlined, &title);
    Car player_car = {0, 0, 0, 0, {255, 0, 0, 0}, {0, 0, 0}, "You "}; // a játékos kocsija mindig piros
    GameView game_view = MainMenu;
    /*
    game_data fogja tartalmazni az összes olyan változót, amire bármely játéknézetnek szüksége lehet,
    és memóriában tárolandók. Mindig cím szerint van átadva, hiszen a játéknézeteknek módosítaniuk kell
    */
    GameData game_data = {{0, 0}, leaderboard, renderer, font, underlined, title, &textarray, game_view, magas, szeles, margo, bots, NULL, 0, player_car};
    SDL_Event event;
    event.type = GAME_VIEW_CHANGED_EVENT;
    SDL_PushEvent(&event);
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case GAME_VIEW_CHANGED_EVENT:
                switch (game_data.game_view){
                    case MainMenu:
                        main_menu(&game_data);
                        break;
                    case Settings:
                        settings(&game_data);
                        break;
                    case SingleGame:
                        run_single_game(&game_data);
                        break;
                    case BotGame:
                        run_bot_game(&game_data);
                        break;
                    case MultiGame:
                        run_multi_game(&game_data);
                        break;
                    case Statistics:
                        statistics(&game_data);
                        break;
                    case MultiStatistics:
                        multi_statistics(&game_data);
                        break;
                    case AskName:
                        ask_name(&game_data);
                        break;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
        }
    }
    free(game_data.multis);
    game_data.multis = NULL;
    save_leaderboard(game_data.leaderboard);
    free_textarray(&textarray);
    sdl_close(&window, &renderer, &font, &underlined, &title);
    return 0;
}
