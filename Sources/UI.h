#include "Includes.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

namespace UI
{
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
    static vector<u64> titleIDs;
    static vector<u8> masterKeys;
    static vector<u64> titleKeys_high;
    static vector<u64> titleKeys_low;
    static string FooterText;

    static Service g_esSrv;
    static u64 g_esRefCnt;
    static Service g_nsAppManSrv, g_nsGetterSrv;
    static u64 g_nsRefCnt;

    Result esInitialize()
    {
        atomicIncrement64(&g_esRefCnt);
        Result rc = smGetService(&g_esSrv, "es");
        return rc;
    }

    void esExit()
    {
        if (atomicDecrement64(&g_esRefCnt) == 0)
        {
            serviceClose(&g_esSrv);
        }
    }

    Result esImportTicket(void const *tikBuf, size_t tikSize, void const *certBuf, size_t certSize)
    {
        IpcCommand c;
        ipcInitialize(&c);
        ipcAddSendBuffer(&c, tikBuf, tikSize, BufferType_Normal);
        ipcAddSendBuffer(&c, certBuf, certSize, BufferType_Normal);

        struct RAW
        {
            u64 magic;
            u64 cmd_id;
        } * raw;

        raw = (struct RAW *)ipcPrepareHeader(&c, sizeof(*raw));
        raw->magic = SFCI_MAGIC;
        raw->cmd_id = 1;

        Result rc = serviceIpcDispatch(&g_esSrv);

        if (R_SUCCEEDED(rc))
        {
            IpcParsedCommand r;
            ipcParse(&r);

            struct RESP
            {
                u64 magic;
                u64 result;
            } *resp = (struct RESP *)r.Raw;

            rc = resp->result;
        }

        return rc;
    }

    static Result _nsGetInterface(Service *srv_out, u64 cmd_id);

    Result nsextInitialize(void)
    {
        Result rc = 0;

        atomicIncrement64(&g_nsRefCnt);

        if (serviceIsActive(&g_nsGetterSrv) || serviceIsActive(&g_nsAppManSrv))
            return 0;

        if (!kernelAbove300())
            return smGetService(&g_nsAppManSrv, "ns:am");

        rc = smGetService(&g_nsGetterSrv, "ns:am2"); //TODO: Support the other services?(Only useful when ns:am2 isn't accessible)
        if (R_FAILED(rc))
            return rc;

        rc = _nsGetInterface(&g_nsAppManSrv, 7996);

        if (R_FAILED(rc))
            serviceClose(&g_nsGetterSrv);

        return rc;
    }

    void nsextExit(void)
    {
        if (atomicDecrement64(&g_nsRefCnt) == 0)
        {
            serviceClose(&g_nsAppManSrv);
            if (!kernelAbove300())
                return;

            serviceClose(&g_nsGetterSrv);
        }
    }

    static Result _nsGetInterface(Service *srv_out, u64 cmd_id)
    {
        IpcCommand c;
        ipcInitialize(&c);

        struct RAW
        {
            u64 magic;
            u64 cmd_id;
        } * raw;

        raw = (struct RAW *)ipcPrepareHeader(&c, sizeof(*raw));

        raw->magic = SFCI_MAGIC;
        raw->cmd_id = cmd_id;

        Result rc = serviceIpcDispatch(&g_nsGetterSrv);

        if (R_SUCCEEDED(rc))
        {
            IpcParsedCommand r;
            ipcParse(&r);

            struct RESP
            {
                u64 magic;
                u64 result;
            } *resp = (struct RESP *)r.Raw;

            rc = resp->result;

            if (R_SUCCEEDED(rc))
            {
                serviceCreate(srv_out, r.Handles[0]);
            }
        }

        return rc;
    }

    Result nsBeginInstallApplication(u64 tid, u32 unk, u8 storageId)
    {
        IpcCommand c;
        ipcInitialize(&c);

        struct RAW
        {
            u64 magic;
            u64 cmd_id;
            u32 storageId;
            u32 unk;
            u64 tid;
        } * raw;

        raw = (struct RAW *)ipcPrepareHeader(&c, sizeof(*raw));

        raw->magic = SFCI_MAGIC;
        raw->cmd_id = 26;
        raw->storageId = storageId;
        raw->unk = unk;
        raw->tid = tid;

        Result rc = serviceIpcDispatch(&g_nsAppManSrv);

        if (R_SUCCEEDED(rc))
        {
            IpcParsedCommand r;
            ipcParse(&r);

            struct RESP
            {
                u64 magic;
                u64 result;
            } *resp = (struct RESP *)r.Raw;

            rc = resp->result;
        }

        return rc;
    }

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
        uint start = (idselected / 10) * 10;
        uint end = (start + 10 > idoptions.size()) ? idoptions.size() : start + 10;
        for(uint i = 0; i < options.size(); i++)
        {
            if(i == selected)
            {
                DrawText(fntMedium, ox, oy, {120, 120, 120, 255}, options[i].c_str());
                if(i == 0)
                {
                    DrawText(fntSmall, 450, 575, {0, 0, 0, 255}, "Scroll: Up/Down  |  Jump 10: Left/Right  |  Jump 50: L/R");
                    DrawText(fntSmall, 450, 600, {0, 0, 0, 255}, "A: Select  |  -: About  |  +: Exit");
                    int fx = 450;
                    int fy = 105;
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
                    DrawText(fntLarge, 610, 330, {0, 0, 255, 255}, "Warning: You may be banned.\nCredits: AnalogMan, Adubbz,\nXorTroll, Reisyukaku,\nSimonMKWii");
                }
            }
            else DrawText(fntMedium, ox, oy, {0, 0, 0, 255}, options[i].c_str());
            oy += 50;
        }
        DrawText(fntMedium, TitleX, 660, {0, 0, 0, 255}, FooterText.c_str());
        SDL_RenderPresent(sdl_render);
    }

    unsigned long int byteswap(unsigned long int x)
    {
        x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
        x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
        x = (x & 0x00FF00FF00FF00FF) << 8 | (x & 0xFF00FF00FF00FF00) >> 8;
        return x;
    }

    int installTikCert(u64 tid, u8 mkey, u64 tkeyh, u64 tkeyl)
    {
        Result rc = 0;
        int tikBuf_size = 704;
        char* tikBuf = new char[tikBuf_size];
        int certBuf_size = 1792;
        char* certBuf = new char[certBuf_size];

        tid = byteswap(tid);
        tkeyh = byteswap(tkeyh);
        tkeyl = byteswap(tkeyl);
        

        ifstream tik("sdmc:/switch/FreeShopNX/Ticket.tik", ios::in | ios::binary);
        tik.read(tikBuf, tikBuf_size);
        tik.close();

        ifstream cert("sdmc:/switch/FreeShopNX/Certificate.cert", ios::in | ios::binary);
        cert.read(certBuf, certBuf_size);
        cert.close();

        // patch TIK with title data
        memcpy(tikBuf+0x180, &tkeyh, 8);
        memcpy(tikBuf+0x188, &tkeyl, 8);
        memcpy(tikBuf+0x286, &mkey, 1);
        memcpy(tikBuf+0x2A0, &tid, 8);
        memcpy(tikBuf+0x2AF, &mkey, 1);

        if (R_FAILED(rc = esImportTicket(tikBuf, tikBuf_size, certBuf, certBuf_size)))
        {
            return rc;
        }
            return rc;
    }

    int nsInstallTitle(u64 id)
    {
        Result res = nsBeginInstallApplication(id, 0, (FsStorageId)5);
        return res;
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
                    if (idselected > 10)
                        idselected -= 10;
                    else
                        idselected = 0;
                    Draw();
                }
        }
        else if(k & KEY_RIGHT)
        {
            if (selected == 0)
            {
                if (idselected < idoptions.size() - 10)
                    idselected += 10;
                else
                    idselected = idoptions.size() - 1;
                Draw();
            }
        }
        else if (k & KEY_L)
        {
            if (selected == 0)
                if (selected == 0)
                {
                    if (idselected > 50)
                        idselected -= 50;
                    else
                        idselected = 0;
                    Draw();
                }
        }
        else if (k & KEY_R)
        {
            if (selected == 0)
            {
                if (idselected < idoptions.size() - 50)
                    idselected += 50;
                else
                    idselected = idoptions.size() - 1;
                Draw();
            }
        }
        else if(k & KEY_A)
        {
            u64 tid = titleIDs[idselected];
            u8 mkey = masterKeys[idselected];
            u64 tkeyh = titleKeys_high[idselected];
            u64 tkeyl = titleKeys_low[idselected];

            char buf[256];
            if (installTikCert(tid, mkey, tkeyh, tkeyl))
            {
                FooterText =  "Cert & Tik install failed!";
            } else if (nsInstallTitle(tid))
            {
                FooterText = "Error downloading title ID " + idoptions[idselected] + ".";
            } else
            {
                sprintf(buf, "Download started for %016lx. Press + to exit or select another.", tid);
                FooterText = buf;
            }
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

    int Init()
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
        options.push_back("Install titles");
        options.push_back("About FreeShopNX");
        FooterText = "Ready to download titles!";
        Draw();
        ifstream ifs("sdmc:/switch/FreeShopNX/FreeShopNX.txt");
        if(ifs.good())
        {
            string buf;
            while(getline(ifs, buf))
            {
                if (buf.empty())
                    continue;
                stringstream ss(buf);
                string s_rightsID, s_titleKey, titleName;
                getline(ss, s_rightsID, ',');
                getline(ss, s_titleKey, ',');
                getline(ss, titleName, ',');

                u64 titleID = strtoull(s_rightsID.substr(0, 16).c_str(), NULL, 16);
                u8 masterKey = stoul(s_rightsID.substr(16, 32).c_str(), NULL, 16);
                u64 titleKey1 = strtoull(s_titleKey.substr(0, 16).c_str(), NULL, 16);
                u64 titleKey2 = strtoull(s_titleKey.substr(16, 32).c_str(), NULL, 16);

                titleIDs.push_back(titleID);
                masterKeys.push_back(masterKey);
                titleKeys_high.push_back(titleKey1);
                titleKeys_low.push_back(titleKey2);
                idoptions.push_back(titleName);
            }
        } else {
            return 1;
        }
        ifs.close();
        Draw();
        return 0;
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