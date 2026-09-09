#include <SDL3/SDL_main.h>
#include "SimpleTriangle.h"

int main(int argc, char* argv[])
{
    SimpleTriangle app(1280u, 720u);

    if (app.Initialize())
    {
        app.Run();
    }
    app.Shutdown();
    return 0;
}