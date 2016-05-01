
 /**
  * @name PlanetMgr.h
  *   Specific Class for managing planet resources
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_PLANETMGR_H_
#define EVEMU_PLANET_PLANETMGR_H_

#include "planet/PlanetDB.h"

class PlanetData
{
public:

protected:

private:

};

class PlanetMgr
{
public:
    PlanetMgr();
    ~PlanetMgr()    { /* do nothing here */ }

protected:

private:
    PlanetDB* m_db;
    
};


#endif  // EVEMU_PLANET_PLANETMGR_H_