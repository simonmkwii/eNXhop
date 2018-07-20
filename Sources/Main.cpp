#include "UI.h"

int main()
{
    UI::Init();
    while(appletMainLoop())
    {
        if (UI::Loop())
            break;
    }
    UI::Exit();
    return 0;
}