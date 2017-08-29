
 /**
  * @name Scan.h
  *   Scanning methods for Alasiya EvE
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  *
  */


#ifndef EVEMU_SCANING_SCAN_H_
#define EVEMU_SCANING_SCAN_H_

#include "packets/Scan.h"
#include "system/cosmicMgrs/ManagerDB.h"

class PyRep;
class Client;

class Scan {
public:
    Scan(Client* pClient);
    //Scan(Scan& scan);

    ~Scan();

    PyRep* ConeScan(Call_ConeScan args);
    void RequestScans(PyDict* dict);
    void ScanResult();


protected:
    void ScanStart();
    void SurveyScan();

private:
    ManagerDB* m_db;
    Client* m_client;
};

#endif // EVEMU_SCANING_SCAN_H_
