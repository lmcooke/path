
#include "world.h"

World::World() :
    m_skyCube()
{ }

World::~World() { }

void World::load(const String &path )
{

    printf("Loading scene %s...\n", path.c_str());

    Any scene;
    scene.load(path);

    // Read the model table
    debugAssert( scene.containsKey("models") );
    const Table<String, Any> models = scene["models"].table();

    // Dump it to stdout
    printf("%d model(s)\n", (int)models.size());
    for (int i = 0; i < (int)models.size(); ++i)
        printf("    %s\n", models.getKeys()[i].c_str());

    // Read the entity table
    debugAssert(scene.containsKey("entities"));
    const Table<String, Any> &entities = scene["entities"].table();

    // Parse entities
    printf("%d entities(s)\n", (int)entities.size());
    Array<shared_ptr<Surface>> geometry;
    for (int i = 0; i < (int)entities.size(); ++i)
    {
        String key = entities.getKeys()[i];
        Any e = entities[key];
        String type = e.name();

        printf("    %s (%s) ... ", key.c_str(), type.c_str());

        if (type == "Camera")
        {
            AnyTableReader props(e);

            m_camera = dynamic_pointer_cast<Camera>(Camera::create(type, NULL, props));
            m_dofCam = dynamic_pointer_cast<dofCam>(dofCam::create(type, NULL, props));

            printf("done\n");
        }
        else if (type == "Light")
        {
            printf("ignored (only emitters are used as lights in path)\n");
        }
        else if (type == "Medium")
        {
            m_medium = Medium::create(e);
            printf("done\n");
        }
        else if (type == "VisibleEntity")
        {
            const Table<String, Any> &props = e.table();

            // Read the model from disk
            shared_ptr<ArticulatedModel> model =
                ArticulatedModel::create(models[props["model"]]);

            // Pose it in world space
            Vector3 pos = Vector3::zero();
            if (props.containsKey("position"))
                pos = Vector3(props["position"]);

            Array<shared_ptr<Surface>> posed;
            model->pose(posed, CFrame(pos));

            // Add it to the scene
            for (int i = 0; i < posed.size(); ++i)
                geometry.append(posed[i]);

            printf("done\n");
        }
        else {
            printf("ignored (unknown entity type)\n");
        }
    }

    if ( !m_medium ) m_medium = shared_ptr<Medium>( new HomogeneousMedium );

    // Build bounding interval hierarchy for scene geometry
    Array<Tri> triArray;

    Surface::getTris( geometry, m_verts, triArray );
    for (int i = 0; i < triArray.size(); ++i)
    {
        triArray[i].material()->setStorage(COPY_TO_CPU);

        // Check if this triangle emits light
        shared_ptr<Material> m = triArray[i].material();
        if (m)
        {
            shared_ptr<UniversalMaterial> mtl =
                dynamic_pointer_cast<UniversalMaterial>(m);

            if ( mtl->emissive().notBlack() ) {
                m_emit.append(triArray[i]);
            }
        }
    }

    m_tris.setContents(triArray, m_verts);

    printf( "%d light-emitting triangle(s) in scene.\n", (int) m_emit.size() );
    fflush( stdout );

}

void World::setSkybox(String xPos, String xNeg,
                      String yPos, String yNeg,
                      String zPos, String zNeg, int image)
{
    m_skyCube = SkyCube(xPos, xNeg, yPos, yNeg, zPos, zNeg, image);
    printf("loaded SkyCube\n");
}

void World::unload()
{
    m_tris.clear();
    m_emit.clear();
}

shared_ptr<Camera> World::camera()
{
    return m_camera;
}

shared_ptr<dofCam> World::dofCamera()
{
    return m_dofCam;
}

shared_ptr<Medium> World::medium()
{
    return m_medium;
}

SkyCube World::skycube()
{
    return m_skyCube;
}

void World::emissivePoint( Random &random,
                           Vector3 &point,
                           Tri &tri,
                           Vector3 &normal,
                           float &prob,
                           float &area )
{
    // Pick an emissive triangle uniformly at random
    int i = random.integer(0, m_emit.size() - 1);
    tri = m_emit[i];
    normal = tri.normal(m_verts);

    // Pick a point in that triangle uniformly at random
    // http://books.google.com/books?id=fvA7zLEFWZgC&pg=PA24#v=onepage&q&f=false
    float s = random.uniform(),
          t = random.uniform(),
          sqrtT = sqrt(t),
          a = (1.f - sqrtT),
          b = (1.f - s) * sqrtT,
          c = s * sqrtT;

    point = tri.position(m_verts, 0) * a
          + tri.position(m_verts, 1) * b
          + tri.position(m_verts, 2) * c;

    // assumes all light emitting triangles are the same area
    prob = 1.f / m_emit.size() / tri.area();
    area = tri.area();
}

void World::intersect(const Ray &ray, float &dist, shared_ptr<Surfel> &surf)
{
    TriTree::Hit hit;
    if (m_tris.intersectRay(ray, hit)) {
        dist = hit.distance;
        m_tris.sample(hit, surf);
    }
}

bool World::lineOfSight(const Vector3 &beg, const Vector3 &end)
{
    Vector3 d = end - beg;
    float dist = d.length();
    if (dist < 1e-4)
        return false;

    // The two rightmost pieces of nonsense are necessary
    // to prevent occlusion from the opposite side of geometry
    // ~vn6
    Ray ray = Ray::fromOriginAndDirection(beg, d / dist, 1e-4, dist - 1e-4);

    TriTreeBase::Hit hit;

    return !m_tris.intersectRay(ray, hit, TriTree::DO_NOT_CULL_BACKFACES | TriTree::OCCLUSION_TEST_ONLY);
}

