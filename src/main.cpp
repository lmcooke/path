#include <G3D/G3DAll.h>
#include "app.h"

#ifndef G3D_PATH
#define G3D_PATH "/contrib/projects/g3d10/G3D10"
#endif

G3D_START_AT_MAIN();

int main(int argc, const char *argv[])
{
    GApp::Settings s;
    s.window.caption = "CS224 Project 4: Path";
    s.dataDir = G3D_PATH "/../data10/common";

    // oldstuff is badstuff lolololol rip [ :( :( :( ] AHAHHAH G3D9 good times
    // environment G3D9DATA variable overrides hard coded path

    if (const char* envDataPath = getenv("G3D9DATA"))
    {
        s.dataDir = envDataPath;
    }

    if (!G3D::FileSystem::exists(s.dataDir))
    {
        std::cerr << "Invalid G3D Path: " <<  s.dataDir << std::endl;
        return 1;
    }

//    s.window.height = 188;
//    s.window.width = 300;

    App app(s);


    // Parse Arguments
    if (argc > 1) {
        const char *scenePath = argv[1];
        app.setScenePath(scenePath);
    }

    return app.run();
}
