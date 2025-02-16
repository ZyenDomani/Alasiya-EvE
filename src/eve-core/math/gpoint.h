/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
*/

#ifndef G_POINT_H
#define G_POINT_H

#include "GaTypes.h"
#include "utils/misc.h"


class GPoint : public Ga::GaVec3 {
public:
     GPoint():Ga::GaVec3(){}
     GPoint(Ga::GaFloat v):Ga::GaVec3(v){}
     GPoint(const Ga::GaFloat *v):Ga::GaVec3(v){}
     GPoint(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Ga::GaVec3(X, Y, Z){}
     GPoint(const GPoint& oth):Ga::GaVec3(oth){}
     GPoint(const Ga::GaVec3& oth):Ga::GaVec3(oth){}

     // Public functions for manipulating 3D coordinates in space:
     // Take existing (x,y,z) point and use that as the center of a sphere of 'radius' and
     // modify it to be a new (x,y,z) point randomly placed on that sphere about the original
     // center coordinate: (x,y,z)
     void MakeRandomPointOnSphere(double radius)
     {
        double theta = MakeRandomFloat( 0.0, (2*M_PI) );
        double phi = MakeRandomFloat( 0.0, (2*M_PI) );
        x += radius * sin(theta) * cos(phi);
        z += radius * sin(theta) * sin(phi);
        y += radius * cos(theta);
     }

     // Take existing (x,y,z) point and use that as the center of two spheres of 'radiusInner', the
     // smaller radius sphere, and 'radiusOuter', the larger radius sphere, and modify
     // the original coordinate to be a new (x,y,z) point randomly placed somewhere inside the volume
     // enclosed between the smaller sphere and the large sphere
    void MakeRandomPointOnSphereLayer(double radiusInner, double radiusOuter)
    {
        double theta = MakeRandomFloat( 0.0, (2*M_PI) );
        double phi = MakeRandomFloat( 0.0, (2*M_PI) );
        double intermediateRadius = MakeRandomFloat( radiusInner, radiusOuter );
        x += intermediateRadius * sin(theta) * cos(phi);
        z += intermediateRadius * sin(theta) * sin(phi);
        y += intermediateRadius * cos(theta);
    }
};

class GVector : public Ga::GaVec3 {
public:
    GVector():Ga::GaVec3(){}
    GVector(Ga::GaFloat v):Ga::GaVec3(v){}
    GVector(const Ga::GaFloat *v):Ga::GaVec3(v){}
    GVector(Ga::GaFloat X,Ga::GaFloat Y,Ga::GaFloat Z):Ga::GaVec3(X, Y, Z){}
    GVector(const GPoint& oth):Ga::GaVec3(oth){}
    GVector(const Ga::GaVec3& oth):Ga::GaVec3(oth){}
    GVector(const GPoint& from, const GPoint& to)
        : Ga::GaVec3( (to.x-from.x), (to.y-from.y), (to.z-from.z) ) {}
};

class GVector4 : public GVector {
public:
    GVector4();
    GVector4(const GPoint &them);
    GVector4(double x, double y, double z, double w = 1.0f);

    inline void operator()(double nx, double ny, double nz, double nw) { pt[0] = nx; pt[1] = ny; pt[2] = nz; W = nw; }
    double dot4(const GVector4 &them) const;
    double dot4(const GPoint &them) const;

    inline const double w() const { return W; }

    double W;
    double pt[3];
};

#endif
