#ifndef PATHTRACER_H
#define PATHTRACER_H

#include <G3D/G3DAll.h>
#include "world.h"



// you can extend this if you want
class PTSettings
{
public:

    enum SkyImage {SPONZA, HIPSHOT};

    // all light contributions assumed to be area lights
    bool useDirectDiffuse;
    bool useDirectSpecular;
    bool useIndirect;
    bool useEmitted;

    int superSamples; // for say, stratified sampling
    bool attenuation; // refracted path absorption through non-vacuum spaces
    bool useMedium; // enable volumetric mediums

    bool dofEnabled;
    float dofFocus;
    float dofLens;
    int dofSamples;

    bool useImageBasedLighting;
    SkyImage si = SPONZA;

};

class PathTracer
{
public:
    PathTracer();

    /**
     * Generates a single path tracing sample. Samples are averaged in App::threadCallback()
     * You may optionally want to edit this function for supersampling.
     */
    Radiance3 sample(int x, int y, Rect2D viewport);

    void setWorld(World* world);
    void setPTSettings(PTSettings settings);



protected:
    World* m_world;
    PTSettings m_settings;

    /** TODO Your recursive raytracing function. This is the only function you
      * will need to modify for this assignment, but you can add additional
      * methods to this class, such as separate functions for indirect/direct light
      * contributions.
      *
      * Given the ray to trace and the current recursion depth,
      * trace the ray into m_world and return the radiance along that ray.
      *
      * World::intersect() and World::lineOfSight() will be useful,
      * as will a handful of G3D helper object types (Surfel, Light, Ray,
      * Camera, CoordinateFrame (a.k.a. CFrame), Vector3, Vector4).
      *
      * More info and links to G3D documentation are provided in the handout
      * for this assignment. Read the handout!
      */
    Radiance3 trace( const Ray &ray,
                     bool isEyeRay,
                     float *distance = NULL );

    Radiance3 estimateL(const Ray &ray, int bounceNum);

    Radiance3 calculateEmittedLight(shared_ptr<Surfel> surf, const Ray &ray);

    Radiance3 calculateDirectLighting(shared_ptr<Surfel> surf, const Ray &ray, int bounceNum);

    Radiance3 calculateAreaLighting(shared_ptr<Surfel> surf, const Ray &ray, int bounceNum);

    Radiance3 calculateSpecular(shared_ptr<Surfel> surf, const Ray &ray);

};

#endif // PATHTRACER_H
