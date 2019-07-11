
 /**
  * @name ModuleItem.cpp
  *   module item class
  * @Author:         Allan
  * @date:   20 June 2019
  */


#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "ship/modules/ModuleItem.h"

ModuleItem::ModuleItem(uint32 _modID, const ItemType& _type, const ItemData& _data)
: InventoryItem(_modID, _type, _data)
{
    _log(ITEM__TRACE, "Created ModuleItem for %s(%u).", itemName().c_str(), itemID());
}

ModuleItemRef ModuleItem::Load( uint32 modID)
{
    return InventoryItem::Load<ModuleItem>(modID);
}

ModuleItemRef ModuleItem::Spawn( ItemData &data)
{
    uint32 modID = InventoryItem::CreateItemID(data);
    if ( modID == 0 )
        return ModuleItemRef(nullptr);

    ModuleItemRef modRef = ModuleItem::Load(modID);
    if (modRef.get() == nullptr) {
        // make error msg here for failure to load module?
        return ModuleItemRef(nullptr);
    }

    modRef->SaveItem();
    return modRef;
}

bool ModuleItem::_Load()
{
    Client* pClient = sItemFactory.GetUsingClient();
    // test for character creation (which throws errors here and isnt really needed)
    if ((pClient != nullptr) and pClient->IsCharCreation())
        return true;
    // load attributes
    if (!InventoryItem::_Load())
        return false;

    // modules need the Online attribute set.  easier to do here than add to item attributes
    if (!HasAttribute(AttrOnline))
        SetAttribute(AttrOnline, EvilZero, false);

    return true;
}

void ModuleItem::SetOnline(bool online/*false*/, bool isRig/*false*/) {
    _log(SHIP__MODULE_DEBUG, "ModuleItem::SetOnline() - set module %s(%u) to %s", \
                    m_itemName.c_str(), m_itemID, (online ? "Online" : "Offline"));

    m_modifiers.clear();
    if (!isRig)   // rigs DO NOT get isOnline attrib set.
        SetAttribute(AttrOnline, (online?1:0));

    Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
    if (pClient == nullptr) {
        _log(SHIP__MODULE_WARNING, "ModuleItem::SetOnline() - No client object found using m_ownerID (%u) for module %s(%u)", \
                            m_ownerID, m_itemName.c_str(), m_itemID);
        return;
    }

    GodmaEnvironment ge;
        ge.selfID = m_itemID;
        ge.charID = m_ownerID;
        ge.shipID = pClient->GetShipID();
        ge.targetID = 0;
        ge.other = PyStatic.NewNone();
        ge.area = new PyList();
        ge.effectID = 16;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = online;
        shipEff.active = online;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
    if (HasAttribute(AttrDuration)) {
        shipEff.duration = (online ? GetAttribute(AttrDuration).get_float() : 0.0);
    } else if (HasAttribute(AttrSpeed)) {
        shipEff.duration = (online ? GetAttribute(AttrSpeed).get_float() : 0.0);
    } else {
        shipEff.duration = 0.0;
    }
        shipEff.repeat = (online ? 1 : 0);
        shipEff.error = PyStatic.NewNone();
    PyList* events = new PyList();
        events->AddItem(shipEff.Encode());
    Notify_OnMultiEvent multi;
        multi.events = events;
    PyTuple* tmp = multi.Encode();
    pClient->SendNotification("OnMultiEvent", "clientID", &tmp);
}
