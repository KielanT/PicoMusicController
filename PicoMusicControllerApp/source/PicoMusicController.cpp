#include "App.h"
#include <iostream>


#ifdef _RELEASE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    std::string port = "COM10";

    std::string cmdLineStr{ cmdLine };
    if (!cmdLineStr.empty())
        port = cmdLineStr;

    

    App app; // Create app object
    app.Init(port); // Initialize app

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
