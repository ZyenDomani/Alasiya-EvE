/*
 * Asteroid.cpp
 *
 * asteroid item and entity classes for EVEmu
 *
 * Original file/code by Zhur
 * Rewrite:     Allan
 */


#include "../eve-server.h"
#include "system/Asteroid.h"

#include "EVEServerConfig.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/SystemBubble.h"


AsteroidItem::AsteroidItem(const ItemType& type, const ItemData& idata, const AsteroidData& adata)
: InventoryItem(adata.itemID, type, idata),
m_data(adata)
{
    _log(ITEM__TRACE, "Created AsteroidItem for %s(%u).", adata.itemName.c_str(), adata.itemID);
}

AsteroidItemRef AsteroidItem::Load( uint32 asteroidID) {
    return InventoryItem::Load<AsteroidItem>(asteroidID );
}

AsteroidItemRef AsteroidItem::Spawn(ItemData& idata, AsteroidData& adata) {
    const ItemType *type = sItemFactory.GetType(adata.typeID);
    if (type == nullptr)
        return AsteroidItemRef(nullptr);

    idata.name = type->name();
    adata.itemName = type->name();

    ManagerDB::CreateRoidItemID(idata, adata);
    if (adata.itemID == 0)
        return AsteroidItemRef(nullptr);

    AsteroidItemRef roidRef = AsteroidItemRef(new AsteroidItem(*type, idata, adata));
    roidRef->SetAttribute(AttrRadius,    adata.radius);
    roidRef->SetAttribute(AttrQuantity,  adata.quantity);
    roidRef->SetAttribute(AttrVolume,    type->volume());
    roidRef->SetAttribute(AttrMass,      type->mass() * adata.quantity);

    return roidRef;
}

AsteroidItemRef AsteroidItem::SpawnTemp(ItemData& idata, AsteroidData& adata) {
    const ItemType *type = sItemFactory.GetType(adata.typeID);
    if (type == nullptr)
        return AsteroidItemRef(nullptr);

    idata.name = type->name();
    adata.itemName = type->name();
    adata.itemID = sItemFactory.GetNextTempID();

    AsteroidItemRef roidRef = AsteroidItemRef(new AsteroidItem(*type, idata, adata));
    roidRef->SetAttribute(AttrRadius,    adata.radius);
    roidRef->SetAttribute(AttrQuantity,  adata.quantity);
    roidRef->SetAttribute(AttrVolume,    type->volume());
    roidRef->SetAttribute(AttrMass,      type->mass() * adata.quantity);

    return roidRef;
}

//iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), iData);

AsteroidSE::AsteroidSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system)
: ObjectSystemEntity(self, services, system),
m_beltMgr(nullptr),
m_growTimer(0),
//m_growTimer(sConfig.cosmic.BeltGrowTime * EvE::Time::Hour),  // hours->ms
m_beltID(0)
{
}

void AsteroidSE::Process() {
    /* called by EntityMgr::Process on every loop */
    /*   Base call to Process Movement  */
    SystemEntity::Process();

    if (m_growTimer.Check())
        if (!m_system->GetBeltMgr()->IsActive(m_bubble->GetID()))
            Grow();
}

PyDict* AsteroidSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for ASE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
    slim->SetItemString("name",             new PyString(m_self->itemName()));
    slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
    slim->SetItemString("typeID",           new PyInt(GetTypeID()));
    slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
    slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
    slim->SetItemString("quantity",         new PyInt(m_self->GetAttribute(AttrQuantity).get_int()));
    slim->SetItemString("radius",           new PyFloat(m_self->GetAttribute(AttrRadius).get_float()));
    return slim;
}

void AsteroidSE::EncodeDestiny(Buffer& into) {
    using namespace Destiny;

    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = Ball::Mode::RIGID;
        head.radius = m_self->GetAttribute(AttrRadius).get_float();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = 0;
    into.Append(head);
    RIGID_Struct main;
        main.formationID = -1;
    into.Append(main);

    _log(SE__DESTINY, "AsteroidSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void AsteroidSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1.0;
    into.recharge = 30000;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0;
    into.structure = 1.0;
}

void AsteroidSE::SendDamageStateChanged() {  //working 24Apr15
    DoDestinyDamageState dmgState;
    MakeDamageState(dmgState);
    OnDamageStateChange dmgChange;
    dmgChange.entityID = m_self->itemID();
    dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    if (m_targMgr != nullptr)
        m_targMgr->QueueUpdate(&up);
    PySafeDecRef(up);
}

void AsteroidSE::Killed(Damage& damage) {
    // determine active miner(s) and call Depleted()
    m_targMgr->Depleted(damage.weaponRef);
    Delete();
}

void AsteroidSE::Grow() {
    // this will increase asteroid size (radius, quantity and mass)
    double radius = m_self->GetAttribute(AttrRadius).get_double();
    // grow 10%?  based on system activity (or lack thereof)?  maybe something about secstatus too?
    radius *= sConfig.rates.BeltGrowPct;
    double quantity = log(radius / 89.675) * (1.0 / 4e-05);

    // per client, roid quantity will never be above 130000m3
    if (quantity > 130000) {
        // if it is, cap and disable timer
        quantity = 130000;
        m_growTimer.Disable();
    }

    m_self->SetAttribute(AttrQuantity,  quantity);   // quantity in m^3
}

void AsteroidSE::Delete() {
    _log(SPAWN__DEPOP, "AsteroidSE::Delete() - Removing asteroid %s(%u) from beltID %u", \
            m_self->name(), m_self->itemID(), m_beltID);
    m_beltMgr->RemoveAsteroid(m_beltID, this);
    SystemEntity::Delete();
}

/*
 * def ComputeRadiusFromQuantity(categoryID, groupID, typeID, quantity):
 *    if quantity < 0:
 *        quantity = 1
 *    if categoryID == const.categoryAsteroid:
 *        qty = quantity
 *        if qty > 130000:
 *            qty = 130000
 *        return 89.675 * math.exp(4e-05 * qty)
 *    if groupID == const.groupHarvestableCloud:
 *        return quantity * cfg.invtypes.Get(typeID).radius / 10.0
 *    return quantity * cfg.invtypes.Get(typeID).radius
 *
 *
 * def ComputeQuantityFromRadius(categoryID, groupID, typeID, radius):
 *    if categoryID == const.categoryAsteroid:
 *        quantity = math.log(radius / 89.675) * (1.0 / 4e-05)
 *        return quantity
 *    if groupID == const.groupHarvestableCloud:
 *        quantity = radius * 10.0 / cfg.invtypes.Get(typeID).radius
 *        return quantity
 *    return radius / cfg.invtypes.Get(typeID).radius
 */