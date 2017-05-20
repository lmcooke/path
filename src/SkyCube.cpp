#include "SkyCube.h"


#define DIST 3.f

SkyCube::SkyCube(){

}

SkyCube::SkyCube(String xPos, String xNeg,
                 String yPos, String yNeg,
                 String zPos, String zNeg, int image)
{
    m_xPos = Image::fromFile(xPos);
    m_xNeg = Image::fromFile(xNeg);

    m_yPos = Image::fromFile(yPos);
    m_yNeg = Image::fromFile(yNeg);

    m_zPos = Image::fromFile(zPos);
    m_zNeg = Image::fromFile(zNeg);

    if (image == 0) {
        m_skyImage = SPONZA;
    } else if (image == 1) {
        m_skyImage = HIPSHOT;
    }
}

SkyCube::~SkyCube()
{

}


Color4 SkyCube::getIntersectedColor(const Ray &ray)
{
    Point3 rayOrigin = ray.origin();
    Vector3 rayDirection = ray.direction();

    float t;
    float xIntersectionPt;
    float yIntersectionPt;
    Point3 intersectionPt;
    Point2 intersection2D;

    if (ray.direction().x > 0) {
        // solve for intersection with x = DIST

        t = (DIST - rayOrigin.x) / rayDirection.x;
        xIntersectionPt = rayOrigin.z + (t * rayDirection.z);
        yIntersectionPt = rayOrigin.y + (t * rayDirection.y);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(0, intersection2D);
        }

    }
    if (ray.direction().x <= 0) {
        // solve for intersection with x = -DIST

        t = (-DIST - rayOrigin.x) / rayDirection.x;
        xIntersectionPt = (rayOrigin.z + (t * rayDirection.z)) * -1.f;
        yIntersectionPt = rayOrigin.y + (t * rayDirection.y);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, -DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(1, intersection2D);
        }
    }


    if (ray.direction().y > 0) {
        // solve for intersection with y = DIST

        t = (DIST - rayOrigin.y) / rayDirection.y;
        xIntersectionPt = rayOrigin.x + (t * rayDirection.x);
        yIntersectionPt = rayOrigin.z + (t * rayDirection.z);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(2, intersection2D);
        }
    }

    if (ray.direction().y <= 0) {
        // solve for intersection with y = -DIST

        t = (-DIST - rayOrigin.y) / rayDirection.y;
        xIntersectionPt = rayOrigin.x + (t * rayDirection.x);
        yIntersectionPt = rayOrigin.z + (t * rayDirection.z);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, -DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(3, intersection2D);
        }
    }

    if (ray.direction().z > 0) {
        // solve for intersection with z = DIST
        t = (DIST - rayOrigin.z) / rayDirection.z;
        xIntersectionPt = (rayOrigin.x + (t * rayDirection.x)) * -1.f;
        yIntersectionPt = rayOrigin.y + (t * rayDirection.y);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(4, intersection2D);
        }
    }

    // solve for intersection with z = -DIST:
    if (ray.direction().z <= 0) {
        t = (-DIST - rayOrigin.z) / rayDirection.z;
        xIntersectionPt = rayOrigin.x + (t * rayDirection.x);
        yIntersectionPt = rayOrigin.y + (t * rayDirection.y);
        intersectionPt = Point3(xIntersectionPt, yIntersectionPt, -DIST);

        intersection2D = Point2(intersectionPt.x, intersectionPt.y);

        if (intersectionInBounds(intersection2D)) {
            return sampleSkyCube(5, intersection2D);
        }
    }

    return Color4(1.f, 1.f, 1.f, 1.f);

}

Color4 SkyCube::sampleSkyCube(int faceNum, Point2 intersectionP)
{

    float imageHeight = static_cast<float>(m_xPos->height());
    float imageWidth = static_cast<float>(m_xPos->width());

    // scale world space 2D intersection point within image bounds
    float translatedXpt = intersectionP.x + DIST;
    float translatedYpt = (2.f * DIST) - (intersectionP.y + DIST);

    Color4 toReturn = Color4(1.f, 1.f, 1.f, 1.f);
    float scaledXcoord = (translatedXpt * imageWidth)/(DIST * 2.f);
    float scaledYcoord = (translatedYpt * imageHeight)/(DIST * 2.f);

    int scaledXcoordInt = floor(scaledXcoord);
    int scaledYcoordInt = floor(scaledYcoord);

    scaledXcoordInt = G3D::clamp(scaledXcoordInt, 1, m_xPos->height() - 1);
    scaledYcoordInt = G3D::clamp(scaledYcoordInt, 1, m_xPos->height() - 1);

    Point2int32 sampleCoord = Point2int32(scaledXcoordInt, scaledYcoordInt);

    if (faceNum == 0) {
        // xPos
        m_xPos->get(sampleCoord, toReturn);
    } else if (faceNum == 1) {
        // xNeg
        m_xNeg->get(sampleCoord, toReturn);
    } else if (faceNum == 2) {
        m_yPos->get(sampleCoord, toReturn);
    } else if (faceNum == 3) {
        // y Pos
        m_yNeg->get(sampleCoord, toReturn);
    } else if (faceNum == 4) {
        m_zPos->get(sampleCoord, toReturn);
    } else { // faceNum == 5
        // zNeg

        m_zNeg->get(sampleCoord, toReturn);
    }

    return toReturn;
}

bool SkyCube::intersectionInBounds(Point2 intrsct2D)
{
    return (intrsct2D.x <= DIST &&
            intrsct2D.x >= -DIST &&
            intrsct2D.y <= DIST &&
            intrsct2D.y >= -DIST);
}


