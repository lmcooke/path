#ifndef APP_H
#define APP_H

#include "world.h"
#include "threadpool.h"
#include <ctime>
#include "pathtracer.h"

enum RenderMethod { RAY, PATH, PHOTON };

/** The entry point and main window manager */
class App : public GApp
{
public:
    App(const GApp::Settings &settings = GApp::Settings());
    virtual ~App();



    /** Called once per pixel for raytracing */
    void threadCallback(int x, int y);

    /** Called once at application startup */
    virtual void onInit();

    /** Called once at application shutdown */
    virtual void onCleanup();

    /** Called once per frame to render the scene */
    virtual void onGraphics(RenderDevice *dev,
                            Array<shared_ptr<Surface> >& posed3D,
                            Array<shared_ptr<Surface2D> >& posed2D);
    /** Called from onInit() */
    void makeGUI();

    void loadSceneDirectory(String directory = m_scenePath);
    void changeDataDirectory();

    void onRender();
    void setScenePath(const char *path);
    void loadDefaultScene();
    void loadCustomScene();
    void loadCS244Scene();
    void saveCanvas();
    FilmSettings getFilmSettings();
    void toggleWindowRendering();
    void toggleWindowScenes();

    static RenderMethod m_currRenderMethod;
    static bool m_kill;
    void changeRenderMethod();
    void toggleWindowPath();

    int             pass; // how many passes we have taken for a given pixel
    int             num_passes;
    bool            continueRender;

private:

    // path flags
    PTSettings          m_ptsettings;
    shared_ptr<PathTracer> m_renderer;

    shared_ptr<GuiWindow> m_windowRendering;
    shared_ptr<GuiWindow> m_windowScenes;
    shared_ptr<GuiWindow> m_windowPath;

    static String         m_scenePath; // path to scene folder

    World               m_world;    // The scene being rendered
    shared_ptr<Image3>  m_canvas;   // Output buffer for raytrace()
    shared_ptr<Thread>  m_dispatch; // Spawns rendering threads


#if 0
    bool                m_pointLights; // true if these are turned on
    bool                m_areaLights;
    bool				m_direct;      // show direct lighting of first intersection point?
    bool				m_direct_s;      // show SPECULAR reflections of direct lighting of first intersection point?
    bool				m_indirect;    // show indirect lighting at first intersection point?
    bool				m_emit;        // show emitted light

    bool                m_fresnelEnabled;
    bool                m_attenuation;
#endif

    float                m_scaleFactor; // how much to scale down images by.

    // GUI stuff
    GuiDropDownList*    m_ddl;
    GuiDropDownList*    m_renderdl;
    GuiLabel*           m_warningLabel;
    GuiLabel*           m_scenePathLabel;
    String              m_dirName;
    void updateScenePathLabel();
};

#endif
