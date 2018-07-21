#include "UI.h"

int main()
{
    if (UI::Init())
    {
        UI::Exit();
        return 1;
    }
    while(appletMainLoop())
    {
        if (UI::Loop())
            break;
    }
    UI::Exit();
    return 0;
}