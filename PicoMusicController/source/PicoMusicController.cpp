#include "App.h"
#include <iostream>


#ifdef _RELEASE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    App app; // Create app object
    app.Init(); // Initialize app

    app.Run(); // Run the app


    return 0;
}

#endif // _RELEASE

#ifdef _DEBUG
int main()
{
    App app; // Create app object
    app.Init(); // Initialize app

    app.Run(); // Run the app


    return 0;
}
#endif // _DEBUG
