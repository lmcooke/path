#include "pathtracer.h"

#include <cmath>

#define BUMP 1e-4
#define PI 3.1415
#define funBackGround false // enable to give a fun background color ("clay")

static Random &rng = Random::common();


PathTracer::PathTracer() {}

Radiance3 PathTracer::sample(int x, int y, Rect2D viewport)
{
    Radiance3 s = Radiance3::zero();

    if (m_settings.dofEnabled) { // if depth of field is enabled, super sampling is disabled

        float apertureRad = m_settings.dofLens / 10.f;

        float sampleNum = static_cast<float>(m_settings.dofSamples);

        // sample points within aperture radius (this sampling favors points towards the center of the lens,
        // in order to lessen noise
        for (int i = 0; i < m_settings.dofSamples; i++) {

            float randomRad = rng.uniform() * (apertureRad/2.f);
            float randomAngle = rng.uniform() * (2.f * PI);
            float dx = randomRad * cos(randomAngle);
            float dy = randomRad * sin(randomAngle);

            float dx2 = rng.uniform() * 1.0f;
            float dy2 = rng.uniform() * 1.0f;

            Ray testRay = m_world->dofCamera()->worldRay(x + dx2, y + dy2, dx, dy,
                                                         viewport, m_settings.dofFocus);
            s += trace(testRay, true);
        }
        s = s / sampleNum;


    } else {
        if (m_settings.superSamples == 1) {
            double dx = rng.uniform(), dy = rng.uniform();

            Ray ray = m_world->camera()->worldRay(x + dx, y + dy, viewport);
            s=trace(ray,true);
        } else {
            float superSampleFl = static_cast<float>(m_settings.superSamples);

            float incr = 1.f / superSampleFl;


            for (int i = 0; i < m_settings.superSamples; i++) {
                for (int j = 0; j < m_settings.superSamples; j++) {
                    float dx = static_cast<float>(i) * incr;
                    float dy = static_cast<float>(j) * incr;
                    Ray ray = m_world->camera()->worldRay(x + dx, y + dy, viewport);
                    s += trace(ray,true);
                }
            }

            s = s / (superSampleFl * superSampleFl);
        }
    }

    return s;
}


Radiance3 PathTracer::trace( const Ray &ray,
                      bool isEyeRay,
                      float *distance )
{

    Radiance3 final = Radiance3::blue();
    final=Radiance3::blue();

//    if (!m_world->lightsExist()) return final;

    Radiance3 preClamped = estimateL(ray, 0);
    float finalR = G3D::clamp(preClamped.r, 0.f, 10.f);
    float finalG = G3D::clamp(preClamped.g, 0.f, 10.f);
    float finalB = G3D::clamp(preClamped.b, 0.f, 10.f);

    return Radiance3(finalR, finalG, finalB);
}

Radiance3 PathTracer::estimateL(const Ray &ray, int bounceNum)
{

    // set initial results
    float rVal = 0.f;
    float gVal = 0.f;
    float bVal = 0.f;


    // cast ray
    float dist = 0.0;
    shared_ptr<Surfel> surf;
    m_world->intersect(ray, dist, surf);

    if (surf) {

        if (m_settings.useEmitted && bounceNum == 0) {
            // get emitted light coming from surf to eyepoint
            Radiance3 eLight = calculateEmittedLight(surf, ray);
            rVal += eLight.r;
            gVal += eLight.g;
            bVal += eLight.b;
        }

        // calculate direct lighting contribution
        Radiance3 dirLight = calculateDirectLighting(surf, ray, bounceNum);
        rVal += dirLight.r;
        gVal += dirLight.g;
        bVal += dirLight.b;

        // ray from intersection point towards eye point
        const Vector3& w_o = -1.0 * ray.direction();

//         get reflectivity probability
        float p = rng.uniform();

        Color3 weight;

        // ray coming into intersection point before having been scattered to eye (in reverse dir)
        Vector3 w_i;

        surf->scatter(PathDirection::EYE_TO_SOURCE, w_o, false, rng,
                      weight, w_i);

        w_i = normalize(w_i);

        float r = (weight.r + weight.g + weight.b)/3.f;
        if (p < r) {

            Ray outgoingRay = Ray(surf->position + (.001 * w_i), w_i);

            Radiance3 returnedEst = estimateL(outgoingRay, bounceNum+1);
            Radiance3 integrand = returnedEst * weight;

            integrand = integrand / r;

            rVal += integrand.r;
            gVal += integrand.g;
            bVal += integrand.b;
        }
    } else {

        if (m_settings.useImageBasedLighting) {
            Color4 intersectedColor = m_world->skycube().getIntersectedColor(ray);

            return Radiance3(intersectedColor.r, intersectedColor.g, intersectedColor.b);

        } else {
            if (funBackGround) {
                return Radiance3(0.407f, 0.085f, 0.0f);
            } else {
                return Radiance3::black();
            }
        }
    }



    Radiance3 final = Radiance3(rVal, gVal, bVal);



    return final;

}

// calculates the light coming from surf in direction -1 * ray
Radiance3 PathTracer::calculateEmittedLight(shared_ptr<Surfel> surf, const Ray &ray)
{
    Radiance3 emittedLight = surf->emittedRadiance(ray.direction() * -1.0);
    return emittedLight;
}

Radiance3 PathTracer::calculateDirectLighting(shared_ptr<Surfel> surf, const Ray &ray, int bounceNum)
{
    Radiance3 toReturn;
    if (m_settings.useImageBasedLighting) {

        // choose random ray to sample lighting from, using scatter
        Vector3 wi;

        float pdfValue; // 1/PI
        const Vector3& norm = surf->shadingNormal;
        Vector3::hemiRandom(norm, rng, wi, pdfValue);

        Ray skySampler = Ray(ray.origin() + (BUMP * wi.direction()), wi.direction());

        // check for obstructing objects along wi
        float dist = 0.0;
        shared_ptr<Surfel> obstructingObj;
        m_world->intersect(skySampler, dist, obstructingObj);

        Radiance3 skyLight;

        if (obstructingObj) {

            // there is shadow caster in direction of skycube sampling ray
            skyLight = Radiance3::black();

        } else {
            Color4 skySample = m_world->skycube().getIntersectedColor(skySampler);
            Radiance3 returnedLight = Radiance3(skySample.r, skySample.g, skySample.b);

            Vector3 lightDir = -1.f * wi;
            float dotProd1 = lightDir.dot(surf->shadingNormal);
            dotProd1 = G3D::clamp(dotProd1, 0.0f, 1.0f);

            skyLight = returnedLight * pdfValue * dotProd1;
        }


        if (m_world->lightsExist()) {


            float r = rng.uniform(0.f, 1.f);
            if (r < 0.5f) {
                Radiance3 areaLighting = calculateAreaLighting(surf, ray, bounceNum);
                toReturn = 0.5f * areaLighting;
            } else {
                toReturn = 0.5f * skyLight;
            }


        } else {
            toReturn = skyLight;
        }


    } else {
        toReturn = calculateAreaLighting(surf, ray, bounceNum);
    }
        return toReturn;
}

Radiance3 PathTracer::calculateAreaLighting(shared_ptr<Surfel> surf, const Ray &ray, int bounceNum)
{   
    Point3 loc = surf->position;

    Radiance3 toReturn = Radiance3::black();

    // get random emissive point from scene
    Vector3 lightPt = Vector3();
    Tri tri = Tri();
    Vector3 light_norm = Vector3();
    float prob = 0.0f;
    float area = 0.0f;

    m_world->emissivePoint(rng, lightPt, tri, light_norm, prob, area);


    // get vector from surfel geom to emissive pt
    Vector3 lightDir = lightPt - loc;
    float distToLight = lightDir.length();
    lightDir = normalize(lightDir);

    // check for obstructing geometry between surfel and emissive pt.
    shared_ptr<Surfel> lightSurf;
    Ray rayToLight = Ray(surf->position, lightDir);
    m_world->intersect(rayToLight, distToLight, lightSurf);

    if (lightSurf) {

        // light from geo intersection point to eye
        Vector3 wo = -1.0 * ray.direction();


        float dotProd = lightDir.dot(surf->shadingNormal);

        dotProd = G3D::clamp(dotProd, 0.0f, 1.0f);

        float dotProd2 = light_norm.dot(-lightDir);
        dotProd2 = G3D::clamp(dotProd2, 0.f, 1.f);


        // light from emissive point to geo intersection

        Radiance3 emittedRad;
        if (lightSurf->emittedRadiance(-1.f * lightDir) != Radiance3::black()) {

            shared_ptr<UniversalMaterial> material = dynamic_pointer_cast<UniversalMaterial>(tri.material());
            emittedRad = material->emissive().mean();

            // account for conversion between radiance and power
            float otherVal = 1.f/(PI * area * distToLight * distToLight);
            emittedRad = emittedRad * otherVal;

        } else {
            emittedRad = Radiance3::black();
        }

        Radiance3 fs = surf->finiteScatteringDensity(lightDir, wo);


        if (bounceNum == 0) {
            // direct diffuse
            if (m_settings.useDirectDiffuse) {
                toReturn += emittedRad * dotProd * dotProd2 * fs / prob;
            }

        } else {
            // indirect diffuse
            if (m_settings.useIndirect) {
                toReturn += emittedRad * dotProd * dotProd2 * fs / prob;
            }
        }

        // add specular components
        if (m_settings.useDirectSpecular) {
            if (emittedRad != Radiance3::black()) {
                Radiance3 specAddition = calculateSpecular(surf, ray);
                toReturn += specAddition;
            }
        }
    }
    return toReturn;
}

Radiance3 PathTracer::calculateSpecular(shared_ptr<Surfel> surf, const Ray &ray)
{
    Radiance3 specAddition = Radiance3::black();
    Point3 loc = surf->position;

    Surfel::ImpulseArray impArray;
    surf->getImpulses(PathDirection::EYE_TO_SOURCE, -1.0 * ray.direction(), impArray);

    if (impArray.size() > 0) {

        for (int i = 0; i < impArray.size(); i++) {

            Vector3 impDir = impArray[i].direction;

            Ray impRay = Ray(loc,impDir);
            float dist = 0.0;
            shared_ptr<Surfel> specSurf;
            m_world->intersect(impRay, dist, specSurf);

            if (specSurf) {
                Radiance3 specEmitted = specSurf->emittedRadiance(-1.0 * impDir);
                if (specEmitted != Radiance3::black()) {
                    specAddition += specEmitted;
                }
            }
        }
    }
    return specAddition;
}


void PathTracer::setWorld(World *world)
{
    m_world=world;
}

void PathTracer::setPTSettings(PTSettings settings)
{
    m_settings=settings;
}
