
 /**
  * @name test.cpp
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */

#include "eve-server.h"

#include "Client.h"
#include "system/Container.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    // create and add markers for found data to visualize positions
    /* ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
     *           const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
     */
    FactionData data = FactionData();
    ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagNone, "Found Position Test", NULL_ORIGIN, "Position Test");
    CargoContainerRef iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("tuple 1");
        iRef->SetPosition(GPoint(506169425920,2437608374272,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }

    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("tuple 2");
        iRef->SetPosition(GPoint(-1018362724352,948782891008,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }

    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("tuple 3");
        iRef->SetPosition(GPoint(2007360077824,948782891008,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("tuple 4");
        iRef->SetPosition(GPoint(506169425920,948782891008,154119258112));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("tuple 5");
        iRef->SetPosition(GPoint(506169425920,948782891008,3236538089472));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    // create list positions
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("List 1");
        iRef->SetPosition(GPoint(506169425920,-555071897600,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("List 2");
        iRef->SetPosition(GPoint(-1018362724352,948782891008,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("List 3");
        iRef->SetPosition(GPoint(506169425920,948782891008,154119258112));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("data");
        iRef->SetPosition(GPoint(9814029035.87747,-280669480683.521,-171426724332.669));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    // create probe positions
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 1 (470)");
        iRef->SetPosition(GPoint(506169425920,2437608374272,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 2 (440)");
        iRef->SetPosition(GPoint(506169425920,-555071897600,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 3 (473)");
        iRef->SetPosition(GPoint(2007360077824,948782891008,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 4 (446)");
        iRef->SetPosition(GPoint(-1018362724352,948782891008,1636387389440));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 5 (453)");
        iRef->SetPosition(GPoint(506169425920,948782891008,154119258112));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }
    iRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (iRef.get() != nullptr) {
        iRef->Rename("probe 6 (428)");
        iRef->SetPosition(GPoint(506169425920,948782891008,3236538089472));
        // create new container
        ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
        if (cSE == nullptr)
            return;
        iRef->SetMySE(cSE);
        mySE->SystemMgr()->AddMarker(cSE, false, true);
    }

    sLog.Warning("\ttesting","Test competed");
}
/*
                        [PyIntegerVar 9000000000000020470]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 506169425920]
                            [PyFloat 2437608374272]
                            [PyFloat 1636387389440]
                        [PyIntegerVar 9000000000000020440]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 506169425920]
                            [PyFloat -555071897600]
                            [PyFloat 1636387389440]
                        [PyIntegerVar 9000000000000020473]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 2007360077824]
                            [PyFloat 948782891008]
                            [PyFloat 1636387389440]
                        [PyIntegerVar 9000000000000020446]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat -1018362724352]
                            [PyFloat 948782891008]
                            [PyFloat 1636387389440]
                        [PyIntegerVar 9000000000000020453]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 506169425920]
                            [PyFloat 948782891008]
                            [PyFloat 154119258112]
                        [PyIntegerVar 9000000000000020428]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 506169425920]
                            [PyFloat 948782891008]
                            [PyFloat 3236538089472]
            */
 /*
  *    [PyString "pos"]
  *    [PyList 6 items]
  *      [PyObjectEx Type2]
  *        [PyTuple 2 items]
  *          [PyTuple 1 items]
  *            [PyToken foo.Vector3]
  *          [PyTuple 3 items]
  *            [PyFloat 506169425920]
  *            [PyFloat 2437608374272]
  *            [PyFloat 1636387389440]
  *      [PyObjectEx Type2]
  *        [PyTuple 2 items]
  *          [PyTuple 1 items]
  *            [PyToken foo.Vector3]
  *          [PyTuple 3 items]
  *            [PyFloat -1018362724352]
  *            [PyFloat 948782891008]
  *            [PyFloat 1636387389440]
  *      [PyObjectEx Type2]
  *        [PyTuple 2 items]
  *          [PyTuple 1 items]
  *            [PyToken foo.Vector3]
  *          [PyTuple 3 items]
  *            [PyFloat 2007360077824]
  *            [PyFloat 948782891008]
  *            [PyFloat 1636387389440]
  *      [PyObjectEx Type2]
  *        [PyTuple 2 items]
  *          [PyTuple 1 items]
  *            [PyToken foo.Vector3]
  *          [PyTuple 3 items]
  *            [PyFloat 506169425920]
  *            [PyFloat 948782891008]
  *            [PyFloat 154119258112]
  *      [PyList 3 items]
  *        [PyObjectEx Type2]
  *          [PyTuple 2 items]
  *            [PyTuple 1 items]
  *              [PyToken foo.Vector3]
  *            [PyTuple 3 items]
  *              [PyFloat 506169425920]
  *              [PyFloat -555071897600]
  *              [PyFloat 1636387389440]
  *        [PyObjectEx Type2]
  *          [PyTuple 2 items]
  *            [PyTuple 1 items]
  *              [PyToken foo.Vector3]
  *            [PyTuple 3 items]
  *              [PyFloat -1018362724352]
  *              [PyFloat 948782891008]
  *              [PyFloat 1636387389440]
  *        [PyObjectEx Type2]
  *          [PyTuple 2 items]
  *            [PyTuple 1 items]
  *              [PyToken foo.Vector3]
  *            [PyTuple 3 items]
  *              [PyFloat 506169425920]
  *              [PyFloat 948782891008]
  *              [PyFloat 154119258112]
  *      [PyObjectEx Type2]
  *        [PyTuple 2 items]
  *          [PyTuple 1 items]
  *            [PyToken foo.Vector3]
  *          [PyTuple 3 items]
  *            [PyFloat 506169425920]
  *            [PyFloat 948782891008]
  *            [PyFloat 3236538089472]
  */