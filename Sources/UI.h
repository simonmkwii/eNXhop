#include "Includes.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

namespace UI
{
    // extern "C" Result nsInstallTitle(u64 tid, u32 unk, u8 storageId);
    static SDL_Window *sdl_wnd;
    static SDL_Surface *sdl_surf;
    static SDL_Renderer *sdl_render;
    static TTF_Font *fntLarge;
    static TTF_Font *fntMedium;
    static TTF_Font *fntSmall;

    static string Back = "romfs:/Graphics/Background.png";
    static SDL_Surface *sdls_Back;
    static SDL_Texture *sdlt_Back;
    static int TitleX = 30;
    static int TitleY = 22;
    static int Opt1X = 55;
    static int Opt1Y = 115;

    static uint selected = 0;
    static uint idselected = 0;
    static vector<string> options;
    static vector<string> idoptions;
    static vector<u64> ids;
    static string FooterText;

    SDL_Surface *InitSurface(string Path)
    {
        SDL_Surface *srf = IMG_Load(Path.c_str());
        if(srf)
        {
            Uint32 colorkey = SDL_MapRGB(srf->format, 0, 0, 0);
            SDL_SetColorKey(srf, SDL_TRUE, colorkey);
        }
        return srf;
    }

    SDL_Texture *InitTexture(SDL_Surface *surf)
    {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(sdl_render, surf);
        return tex;
    }

    void DrawText(TTF_Font *font, int x, int y, SDL_Color colour, const char *text)
    {
        SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, text, colour, 1280);
        SDL_SetSurfaceAlphaMod(surface, 255);
        SDL_Rect position = { x, y, surface->w, surface->h };
        SDL_BlitSurface(surface, NULL, sdl_surf, &position);
        SDL_FreeSurface(surface);
    }

    void DrawRect(int x, int y, int w, int h, SDL_Color colour)
    {
        SDL_Rect rect;
        rect.x = x; rect.y = y; rect.w = w; rect.h = h;
        SDL_SetRenderDrawColor(sdl_render, colour.r, colour.g, colour.b, colour.a);
        SDL_RenderFillRect(sdl_render, &rect);
    }

    void DrawBackXY(SDL_Surface *surf, SDL_Texture *tex, int x, int y)
    {
        SDL_Rect position;
        position.x = x;
        position.y = y;
        position.w = surf->w;
        position.h = surf->h;
        SDL_RenderCopy(sdl_render, tex, NULL, &position);
    }

    void DrawBack(SDL_Surface *surf, SDL_Texture *tex)
    {
        DrawBackXY(surf, tex, 0, 0);	
    }

    void Draw()
    {
        SDL_RenderClear(sdl_render);
        DrawBack(sdls_Back, sdlt_Back);
        DrawText(fntLarge, TitleX, TitleY, {0, 0, 0, 255}, "FreeShopNX - CDN title installer");
        int ox = Opt1X;
        int oy = Opt1Y;
        uint start = (idselected / 9) * 9;
        uint end = (start + 9 > idoptions.size()) ? idoptions.size() : start + 9;
        for(uint i = 0; i < options.size(); i++)
        {
            if(i == selected)
            {
                DrawText(fntMedium, ox, oy, {120, 120, 120, 255}, options[i].c_str());
                if(i == 0)
                {
                    DrawText(fntSmall, 450, 575, {0, 0, 0, 255}, "Move between titles using Up/Down or Left/Right.");
                    DrawText(fntSmall, 450, 600, {0, 0, 0, 255}, "Select a title by pressing A.");
                    int fx = 450;
                    int fy = 150;
                    for(uint j = start; j < end; j++)
                    {
                        if(j == idselected)
                        {
                            DrawText(fntMedium, fx, fy, {255, 0, 0, 255}, idoptions[j].c_str());
                        }
                        else DrawText(fntMedium, fx, fy, {0, 0, 0, 255}, idoptions[j].c_str());
                        fy += 45;
                    }
                }
                else if(i == 1)
                {
                    DrawText(fntLarge, 610, 330, {0, 0, 255, 255}, "eNXhop, enjoy installing titles!");
                }
            }
            else DrawText(fntMedium, ox, oy, {0, 0, 0, 255}, options[i].c_str());
            oy += 50;
        }
        DrawText(fntLarge, TitleX, 660, {0, 0, 0, 255}, FooterText.c_str());
        SDL_RenderPresent(sdl_render);
    }

    int Loop()
    {
        hidScanInput();
        int k = hidKeysDown(CONTROLLER_P1_AUTO);
        if(k & KEY_UP)
        {
            if (selected == 0)
            {
                if (idselected > 0)
                    idselected -= 1;
                else
                    idselected = idoptions.size() - 1;
                Draw();
            }
        }
        else if(k & KEY_DOWN)
        {
            if (selected == 0)
            {
                if (idselected < idoptions.size() - 1)
                    idselected += 1;
                else
                    idselected = 0;
                Draw();
            }
        }
        else if(k & KEY_LEFT)
        {
            if(selected == 0)
                if (selected == 0)
                {
                    if (idselected > 9)
                        idselected -= 9;
                    else
                        idselected = 0;
                    Draw();
                }
        }
        else if(k & KEY_RIGHT)
        {
            if (selected == 0)
            {
                if (idselected < idoptions.size() - 9)
                    idselected += 9;
                else
                    idselected = idoptions.size() - 1;
                Draw();
            }
        }
        else if(k & KEY_A)
        {
            // nsInitialize();
            u64 id = ids[idselected];
            // Result res = nsInstallTitle(id, 0, (FsStorageId)5);
            // if(R_SUCCEEDED(res)) FooterText = "Title ID " + idoptions[idselected] + " started downloading!";
            // else FooterText = "Error downloading title ID " + idoptions[idselected] + ".";
            // nsExit();
            char buf[256];
            sprintf(buf, "Selected Title ID: %016lX", id);
            FooterText = buf;
            Draw();
        } else if (k & KEY_MINUS)
        {
            if (selected == 0)
                selected = 1;
            else
                selected = 0;
            Draw();
        } else if (k & KEY_PLUS)
        {
            return 1;
        }
        return 0;
    }

    void Init()
    {
        romfsInit();
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_CreateWindowAndRenderer(1280, 720, 0, &sdl_wnd, &sdl_render);
        sdl_surf = SDL_GetWindowSurface(sdl_wnd);
        SDL_SetRenderDrawBlendMode(sdl_render, SDL_BLENDMODE_BLEND);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
        IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
        TTF_Init();
        SDL_SetRenderDrawColor(sdl_render, 255, 255, 255, 255);
        fntLarge = TTF_OpenFont("romfs:/Fonts/Roboto-Regular.ttf", 35);
        fntMedium = TTF_OpenFont("romfs:/Fonts/Roboto-Regular.ttf", 30);
        fntSmall = TTF_OpenFont("romfs:/Fonts/Roboto-Regular.ttf", 20);
        sdls_Back = InitSurface(Back);
        sdlt_Back = InitTexture(sdls_Back);
        options.push_back("Install title(s)");
        options.push_back("About eNXhop");
        FooterText = "Ready to download titles!";
        ifstream ifs("sdmc:/switch/eNXShop/eNXhop.txt");
        if(ifs.good())
        {
            string buf;
            while(getline(ifs, buf))
            {
                buf.pop_back();
                u64 id = strtoull(buf.c_str(), NULL, 16);
                ids.push_back(id);
                idoptions.push_back(buf);
            }
        }
        ifs.close();
        Draw();
    }

    void Exit()
    {
        TTF_Quit();
        IMG_Quit();
        SDL_DestroyRenderer(sdl_render);
        SDL_FreeSurface(sdl_surf);
        SDL_DestroyWindow(sdl_wnd);
        SDL_Quit();
        romfsExit();
    }
}