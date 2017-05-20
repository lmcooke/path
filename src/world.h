#ifndef WORLD_H
#define WORLD_H

#include <G3D/G3DAll.h>

#include "dofCam.h"
#include "SkyCube.h"

#include "medium.h"

/** Represents a static scene with triangle mesh geometry, multiple lights, and
  * an initial camera specification
  */

class World
{
public:
    World();
    virtual ~World();

    /** Loads the geometry, lights and camera from a scene file.
      * Fails an assert if anything goes wrong.
      *
      * @param path The file to load (*.scn.any)
      */
    void load(const String &path);

    /** Clears the contents of this world object
      * Geometry and lights are cleared. The camera is not affected.
      */
    void unload();

    void setSkybox(String xPos, String xNeg,
                   String yPos, String yNeg,
                   String zPos, String zNeg, int image);

    /** Gets the scene's camera */
    shared_ptr<Camera> camera();

    /** Gets the scene's dof camera */
    shared_ptr<dofCam> dofCamera();

    shared_ptr<Medium> medium();

    /**
     * @brief returns scene's skycube
     * @return skycube
     */
    SkyCube skycube();



    /** Picks a point of light from the scene that emits light from the scene.
      * The point is picked uniformly at random.
      * @param random   A random number generator
      * @param point    Receives the point from which light is emitted
      * @param prob     Receives the probability of picking that point out of
      *                 all light-emitting points in the scene
      *
      */

    /**
     * @brief emissivePoint picks a point on an emitter from the scene.
     * the light's power is assumed to be emitted over a hemisphere (rather than
     * a double-sided light)
     * @param random    A random number generator
     * @param point     set to the point from which light is emitted
     * @param tri       triangle that contains the point
     * @param normal    face normal of the triangle
     * @param prob      probability of picking this point out of all light-emitting points in the scene
     * @param area      area of the triangle
     */
    void emissivePoint( Random &random, Vector3 &point, Tri &tri, Vector3 &normal, float &prob, float &area );

    /** Finds the first point a ray intersects with this scene
      *
      * @param ray  The ray to intersect
      * @param dist The distance from the ray origin to the point of
//      *             intersection
      * @param surf The surface at the point of intersection
      */
    void intersect(const Ray &ray, float &dist, shared_ptr<Surfel> &surf );



    /** Determines whether an object occludes the line of sight from beg to end
     *
      * @param beg  The starting point
      * @param end  The ending point
      * @return     True if there is no geometry in the scene between the
      *             starting point and the ending point
      */
    bool lineOfSight( const Vector3 &beg, const Vector3 &end );

    /** Returns true if there are any lights in the scene */
    bool lightsExist() { return m_emit.size() > 0; }

private:

    TriTree             m_tris;     // The scene's geometry in world space
    shared_ptr<Camera>  m_camera;   // The scene's camera
    shared_ptr<dofCam>  m_dofCam;   // The scene's camera
    shared_ptr<Medium>  m_medium;   // The scene's homogeneous participating medium
    SkyCube  m_skyCube;   // The scene's skybox
    Array<Tri>          m_emit;     // Triangles that emit light
    CPUVertexArray      m_verts;    // The scene's vertices

};

#endif
