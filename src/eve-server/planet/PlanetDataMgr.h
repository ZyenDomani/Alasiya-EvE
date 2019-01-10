
 /**
  * @name PlanetDataMgr.h
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  */


#ifndef EVEMU_PLANET_PLANETDATAMGR_H_
#define EVEMU_PLANET_PLANETDATAMGR_H_

#include <unordered_map>
#include "planet/PlanetDB.h"


// this class is a singleton object to have a common place for all (cached) planet data
class PlanetDataMgr
: public Singleton< PlanetDataMgr >
{
public:
    PlanetDataMgr();
    ~PlanetDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetPlanetData(uint32 planetID, std::vector<uint32> &typeIDs);

protected:
    void _Populate();

private:
    PlanetDB m_db;

    std::unordered_multimap<uint32, uint32> m_planetData;
};

#define sPlanetDataMgr \
( PlanetDataMgr::get() )


// this class is a singleton object to have a common place for all (cached) PI schematic data
class PIDataMgr
: public Singleton< PIDataMgr >
{
public:
    PIDataMgr();
    ~PIDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetSchematicData(uint16 schematicID, PI_Schematic& data);

protected:
    void _Populate();

private:
    PlanetDB m_db;

    std::map<uint8, PI_Schematic> m_schematicData;
};

#define sPIDataMgr \
( PIDataMgr::get() )


#endif  // EVEMU_PLANET_PLANETDATAMGR_H_