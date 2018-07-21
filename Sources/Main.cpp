#include "UI.h"

int main()
{
    if (R_FAILED(ncmInitialize()))
        return 1;

    if (R_FAILED(nsInitialize()))
        return 2;

    if (R_FAILED(UI::nsextInitialize()))
        return 3;

    if (R_FAILED(UI::esInitialize()))
        return 4;

    if (R_FAILED(UI::Init()))
        return 5;
        
    while(appletMainLoop())
    {
        if (UI::Loop())
            break;
    }
    ncmExit();
    nsExit();
    UI::nsextExit();
    UI::esExit();
    UI::Exit();
    return 0;
}