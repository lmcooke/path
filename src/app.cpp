
#include "app.h"

#ifndef G3D_PATH
#define G3D_PATH "/contrib/projects/g3d10/G3D10"
#endif

RenderMethod App::m_currRenderMethod = PATH;
String App::m_scenePath = G3D_PATH "/data/scene";


// set this to 1 to debug a single render thread
#define THREADS 12

//#define xPos "cubemap/sponza/sponza-PX.png";
//#define xNeg "cubemap/sponza/sponza-NX.png";
//#define yPos "cubemap/sponza/sponza-PY.png";
//#define yNeg "cubemap/sponza/sponza-NY.png";
//#define zPos "cubemap/sponza/sponza-PZ.png";
//#define zNeg "cubemap/sponza/sponza-NZ.png";

App::App(const GApp::Settings &settings)
    : GApp(settings),
    pass(0),
    continueRender(true),
    m_renderer(new PathTracer)
{
    m_scenePath = dataDir + "/scene";

    num_passes= 8000;

    m_ptsettings.useDirectDiffuse=true;
    m_ptsettings.useDirectSpecular=true;
    m_ptsettings.useEmitted=true;
    m_ptsettings.useIndirect=true;

    m_ptsettings.dofEnabled=false;
    m_ptsettings.dofFocus=-4.8f;
    m_ptsettings.dofLens=0.2f;
    m_ptsettings.dofSamples=5;

    m_ptsettings.attenuation=true;
}

App::~App() { }

void App::setScenePath(const char* path)
{
    m_scenePath = path;
}


void App::threadCallback(int x, int y)
{
    if (!continueRender) return;
    // Set the pixel to green during calculation (makes it
    // easier to spot the 'scanline')
    Radiance3 last = m_canvas->get( x, y );
    m_canvas->set( x, y, Color3::green() );
    Radiance3 sample = m_renderer->sample(x,y,m_canvas->rect2DBounds());
    m_canvas->set( x, y, (float)pass/(float)(pass+1)*last + sample/(float)(pass+1) );
}

static void dispatcher(void *arg)
{
    App *self = (App*)arg;
    ThreadPool pool( self, THREADS );

    Stopwatch watch;
    float elapsed = 0.f;

    for ( int i = 0; self->continueRender && i < self->num_passes; ++i ) {
        watch.tick();
        printf("[%.3f s] Pass %d...\n", elapsed, i + 1); fflush( stdout );
        self->pass = i;
        pool.run();
        watch.tock();
        elapsed += watch.elapsedTime();
    }

    printf("Finished rendering.\n"); fflush( stdout );
}

void App::onInit()
{
    GApp::showRenderingStats = false;
    renderDevice->setSwapBuffersAutomatically(true);

    // Set up GUI
    createDeveloperHUD();
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    makeGUI();

    m_canvas = Image3::createEmpty(window()->width(),
                                   window()->height());
}

void App::onRender()
{
    // user clicks the render button
    if(m_dispatch == NULL || (m_dispatch != NULL && m_dispatch->completed()))
    {
        continueRender = true;
        String fullpath = m_scenePath + "/" + m_ddl->selectedValue().text();

        m_world.unload();
        m_world.load(fullpath);

//        bool useCubeMap = true;
        if (m_ptsettings.useImageBasedLighting) {

            String xPos;
            String xNeg;
            String yPos;
            String yNeg;
            String zPos;
            String zNeg;
            int skyImage;

            if (m_ptsettings.si == PTSettings::SPONZA) {
                xPos = "cubemap/sponza/sponza-PX.png";
                xNeg = "cubemap/sponza/sponza-NX.png";
                yPos = "cubemap/sponza/sponza-PY.png";
                yNeg = "cubemap/sponza/sponza-NY.png";
                zPos = "cubemap/sponza/sponza-PZ.png";
                zNeg = "cubemap/sponza/sponza-NZ.png";
                skyImage = 0;
            } else if (m_ptsettings.si == PTSettings::HIPSHOT) {
                xPos = "cubemap/hipshot_m9_sky/16_rt.png";
                xNeg = "cubemap/hipshot_m9_sky/16_lf.png";
                yPos = "cubemap/hipshot_m9_sky/16_up.png";
                yNeg = "cubemap/hipshot_m9_sky/16_dn.png";
                zPos = "cubemap/hipshot_m9_sky/16_ft.png";
                zNeg = "cubemap/hipshot_m9_sky/16_bk.png";
                skyImage = 1;
            }



            m_world.setSkybox(xPos, xNeg,
                                yPos, yNeg,
                                zPos, zNeg, skyImage);
        }

        m_renderer->setWorld(&m_world);
        m_renderer->setPTSettings(m_ptsettings);

        shared_ptr<Camera> cam = m_world.camera();
        cam->depthOfFieldSettings().setEnabled(true);
        cam->depthOfFieldSettings().setModel(DepthOfFieldModel::PHYSICAL);

        cam->depthOfFieldSettings().setLensRadius(m_ptsettings.dofLens);
        cam->depthOfFieldSettings().setFocusPlaneZ(m_ptsettings.dofFocus);

        m_canvas = Image3::createEmpty(window()->width(),
                                       window()->height());
        m_dispatch = Thread::create("dispatcher", dispatcher, this);
        m_dispatch->start();
    } else {
        continueRender=false;
    }
}

void App::onCleanup()
{
    m_world.unload();
}

void App::onGraphics(RenderDevice *rd,
                     Array<shared_ptr<Surface> >& posed3D,
                     Array<shared_ptr<Surface2D> >& posed2D)
{

    shared_ptr<Texture> tex = Texture::fromImage("Source", m_canvas);

    m_film->exposeAndRender(renderDevice, getFilmSettings(), tex, 0, 0);

    Surface2D::sortAndRender(rd, posed2D);

}

FilmSettings App::getFilmSettings()
{
    FilmSettings s;
    s.setAntialiasingEnabled(false);
    s.setBloomStrength(0);
    s.setGamma(2.060);
    s.setVignetteTopStrength(0);
    s.setVignetteBottomStrength(0);
    return s;
}

void App::changeDataDirectory()
{
    Array<String> sceneFiles;
    FileSystem::getFiles(m_dirName + "*.Any", sceneFiles);

    // if no files found return
    if (!sceneFiles.size())
    {
        m_warningLabel->setCaption("Sorry, no scene files found");
        return;
    }

    loadSceneDirectory(m_dirName);
}

void App::loadSceneDirectory(String directory)
{
    setScenePath(directory.c_str());

    updateScenePathLabel();
    m_warningLabel->setCaption("");
    m_ddl->clear();

    Array<String> sceneFiles;
    FileSystem::getFiles(directory + "/*.Any", sceneFiles);
    Array<String>::iterator scene;
    for (scene = sceneFiles.begin(); scene != sceneFiles.end(); ++scene)
    {
        m_ddl->append(*scene);
    }
}

void App::loadDefaultScene()
{
    App::loadSceneDirectory(dataDir + "/scene");
}

void App::loadCustomScene()
{
    String localDataDir = FileSystem::currentDirectory() + "/scene";
    App::loadSceneDirectory(localDataDir);
}

void App::loadCS244Scene()
{
    App::loadSceneDirectory("/contrib/projects/g3d/cs224/scenes");
}

void App::updateScenePathLabel()
{
    m_scenePathLabel->setCaption("..." + m_scenePath.substr(m_scenePath.length() - 30, m_scenePath.length()));
}

void App::saveCanvas()
{
    time_t rawtime;
    struct tm *info;
    char dayHourMinSec [7];
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(dayHourMinSec, 7, "%d%H%M%S",info);

    shared_ptr<Texture> colorBuffer = Texture::createEmpty("Color", renderDevice->width(), renderDevice->height());
    m_film->exposeAndRender(renderDevice, getFilmSettings(), Texture::fromImage("Source", m_canvas), 0, 0, colorBuffer);
    colorBuffer->toImage(ImageFormat::RGB8())->save(String("../images/scene-") +
                                                    "p" + String(std::to_string(pass).c_str()) +
                                                    "-" + dayHourMinSec + ".png");
}

void App::toggleWindowRendering()
{
    m_windowRendering->setVisible(!m_windowRendering->visible());
}

void App::toggleWindowScenes()
{
    m_windowScenes->setVisible(!m_windowScenes->visible());
}

void App::toggleWindowPath()
{
    m_windowPath->setVisible(!m_windowPath->visible());
}

void App::changeRenderMethod()
{
    continueRender=false;

    switch(m_renderdl->selectedIndex())
    {
    case 0:
        m_currRenderMethod = RenderMethod::RAY;
        break;
    case 1:
        m_currRenderMethod = RenderMethod::PATH;
        break;
    case 2:
        m_currRenderMethod = RenderMethod::PHOTON;
        break;
    }
}


void App::makeGUI()
{
    shared_ptr<GuiWindow> windowMain = GuiWindow::create("Main",
                                                     debugWindow->theme(),
                                                     Rect2D::xywh(0,0,50,50),
                                                     GuiTheme::MENU_WINDOW_STYLE);

    m_windowRendering = GuiWindow::create("Rendering",
                                                     debugWindow->theme(),
                                                     Rect2D::xywh(0,0,50,50),
                                                     GuiTheme::NORMAL_WINDOW_STYLE);

    m_windowScenes = GuiWindow::create("Scenes",
                                                 debugWindow->theme(),
                                                 Rect2D::xywh(50,0,50,50),
                                                 GuiTheme::NORMAL_WINDOW_STYLE);

    m_windowPath = GuiWindow::create("Path Tracer",
                                                 debugWindow->theme(),
                                                 Rect2D::xywh(50,0,50,50),
                                                 GuiTheme::NORMAL_WINDOW_STYLE);


    GuiPane* paneMain = windowMain->pane();
    GuiPane* paneScenes = m_windowScenes->pane();
    GuiPane* paneRendering = m_windowRendering->pane();
    GuiPane* panePath = m_windowPath->pane();

    //MAIN
    paneMain->addButton("Rendering", this, &App::toggleWindowRendering);
    paneMain->addButton("Scenes", this, &App::toggleWindowScenes);
    paneMain->addButton("Path Tracer", this, &App::toggleWindowPath);

    // SCENE
    m_ddl = paneScenes->addDropDownList("Scenes");

    paneScenes->addLabel("Scene Directory: ");
    m_scenePathLabel = paneScenes->addLabel("");
    paneScenes->addTextBox("Directory:", &m_dirName);
    paneScenes->addButton("Change Directory", this, &App::changeDataDirectory);

    paneScenes->addLabel("Scene Folders");
    paneScenes->addButton("My Scenes", this,  &App::loadCustomScene);
    paneScenes->addButton("G3D Scenes", this,  &App::loadDefaultScene);
    paneScenes->addButton("CS224 Scenes", this,  &App::loadCS244Scene);

    m_warningLabel = paneScenes->addLabel("");
    updateScenePathLabel();

    // RENDERING WINDOW
    GuiControl::Callback changeRender(this, &App::changeRenderMethod);
    m_renderdl = paneRendering->addDropDownList("Renderer", Array<GuiText>("Ray", "Path", "Photon"), (int*)(&m_currRenderMethod), changeRender);

    paneRendering->addButton("Save Image", this, &App::saveCanvas);
    GuiButton* renderButton = paneRendering->addButton("Render", this, &App::onRender);
    renderButton->setFocused(true);
    renderButton->moveBy(140.0f,0.0f);

    paneRendering->addLabel("--- Depth of Field ---");
    paneRendering->addCheckBox("Enable", &m_ptsettings.dofEnabled);

    paneRendering->addLabel("Lens Radius");
    paneRendering->addNumberBox(GuiText(""), &m_ptsettings.dofLens, GuiText(""), GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f, 0.05f);
    paneRendering->addLabel("Focus Plane");
    paneRendering->addNumberBox(GuiText(""), &m_ptsettings.dofFocus, GuiText(""), GuiTheme::LINEAR_SLIDER, 0.0f, 20.0f, 0.05f);
    paneRendering->addLabel("DOF Samples");
    paneRendering->addNumberBox(GuiText(""), &m_ptsettings.dofSamples, GuiText(""), GuiTheme::LINEAR_SLIDER, 1, 20, 1);

    paneRendering->addLabel("--- Stratified Sampling ---");
    paneRendering->addLabel("Subpixel Divisions");
    paneRendering->addNumberBox(GuiText(""), &m_ptsettings.superSamples, GuiText(""), GuiTheme::LINEAR_SLIDER, 1, 1, 1);

    paneRendering->addLabel("--- Image Based Lighting ---");
    paneRendering->addCheckBox("Enable", &m_ptsettings.useImageBasedLighting);
    paneRendering->addRadioButton("Sponza", PTSettings::SPONZA, &m_ptsettings.si);
    paneRendering->addRadioButton("Hipshot", PTSettings::HIPSHOT, &m_ptsettings.si);


    // PATH
    panePath->addNumberBox(GuiText("Passes"), &num_passes, GuiText(""), GuiTheme::NO_SLIDER, 1, 10000, 0);
    panePath->addCheckBox("Attenuation", &m_ptsettings.attenuation);
    panePath->addLabel("--- Radiance Components ---");
    panePath->addCheckBox("Emitted Light", &m_ptsettings.useEmitted);
    panePath->addCheckBox("Scattered Direct Light from Diffuse", &m_ptsettings.useDirectDiffuse);
    panePath->addCheckBox("Scattered Direct Light from Specular", &m_ptsettings.useDirectSpecular);
    panePath->addCheckBox("Scattered Indirect Light from Diffuse", &m_ptsettings.useIndirect);
    panePath->addCheckBox("Participating Media Attenuation", &m_ptsettings.useMedium);

    loadSceneDirectory(m_scenePath);

    m_windowPath->pack();
    m_windowPath->setVisible(false);

    m_windowScenes->pack();
    m_windowScenes->setVisible(false);

    m_windowRendering->pack();
    m_windowRendering->setVisible(false);

    windowMain->pack();
    windowMain->setVisible(true);

    addWidget(m_windowPath);
    addWidget(m_windowRendering);
    addWidget(m_windowScenes);
    addWidget(windowMain);
}
