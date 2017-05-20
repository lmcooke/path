#ifndef SKYCUBE_H
#define SKYCUBE_H

#include <G3D/G3DAll.h>


class SkyCube
{
public:

    enum SkyImage {SPONZA, HIPSHOT};

    SkyCube();

    /**
     * @brief SkyCube: makes sky cube based on paths to six images to be used
     * @param xPos
     * @param xNeg
     * @param yPos
     * @param yNeg
     * @param zPos
     * @param zNeg
     */
    SkyCube(String xPos, String xNeg,
            String yPos, String yNeg,
            String zPos, String zNeg, int image);
    ~SkyCube();

    /**
     * @brief getIntersectedColor: given a ray, shoots it into the skycube, finds
     * the intersection point, samples the appropiate image at the correct spot,
     * and returns that color.
     * @param ray: the ray being cast into the skycube.
     * @return: the color at intersected point.
     */
    Color4 getIntersectedColor(const Ray &ray);

private:

    /**
     * @brief sampleSkyCube: given a 2D point in world space and an int signifying
     * which image to sample, returns the appropriate color. Private helper function
     * for 'getIntersectedColor'
     * @param faceNum - int signaling which face to sample.
     * @param intersectionP - 2D intersection point.
     * @return: color of skycube at point.
     */
    Color4 sampleSkyCube(int faceNum, Point2 intersectionP);

    bool intersectionInBounds(Point2 intrsct2D);

    shared_ptr<Image> m_xPos;
    shared_ptr<Image> m_xNeg;
    shared_ptr<Image> m_yPos;
    shared_ptr<Image> m_yNeg;
    shared_ptr<Image> m_zPos;
    shared_ptr<Image> m_zNeg;

    SkyImage m_skyImage;


};

#endif // SKYCUBE_H
