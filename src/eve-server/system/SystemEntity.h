/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
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

#ifndef __SYSTEMENTITY_H_INCL__
#define __SYSTEMENTITY_H_INCL__

#include "inventory/InventoryItem.h"
#include "inventory/ItemRef.h"
#include "ship/TargetManager.h"

class PyDict;
class PyList;
class PyTuple;
class DoDestiny_AddBall;
class DoDestinyDamageState;
class DBSystemEntity;
class InventoryItem;
class SystemDB;
class GPoint;
class Damage;
class SystemBubble;
class SystemManager;
class Character;
class Client;
class NPC;

//  this is base class for ALL SystemEntities
class SystemEntity {
    friend class SystemBubble;    //only to update m_bubble
public:
    typedef enum {
        ecClient,
        ecNPC,
        ecCelestial,
        ecStation,
        ecSystemEntity,
        ecAsteroidEntity,
        ecShipEntity,
        ecDroneEntity,
        ecContainerEntity,
        ecStructureEntity,
        ecDeployableEntity,
        ecMissileEntity,
        ecWreckEntity,
        ecOther
    } EntityClass;

    SystemEntity();
    virtual ~SystemEntity() {}

    TargetManager TargMgr;

    virtual void Process();
    virtual void ProcessDestiny() = 0;

    //this is a bit crude, but I prefer this over RTTI.
    virtual Client *CastToClient() { return NULL; }
    virtual const Client *CastToClient() const { return NULL; }
    virtual NPC *CastToNPC() { return NULL; }
    virtual const NPC *CastToNPC() const { return NULL; }

    virtual EntityClass GetClass() const { return(ecSystemEntity); }

    virtual uint32 GetLocationID();

    inline SystemBubble *Bubble() const { return m_bubble; }    //may be NULL

    //may consume the arguments, or not.
    virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
    virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}

    //get the item ID of this entity
    virtual uint32 GetID() const = 0;
    //get the position of this entity in space.
    virtual const GPoint &GetPosition() const = 0;
    //get the velocity vector of this entity in space.
    virtual const GVector &GetVelocity() const = 0;
    //get other attributes of the entity:
    virtual const char *GetName() const = 0;
    virtual float GetRadius() const;

    //I am not sure if I want this here...
    virtual InventoryItemRef Item() const = 0;

    virtual SystemManager *System() const = 0;    //will yeild NULL when docked.

    //expand the vector as needed, and encode the destiny update into it.
    virtual void EncodeDestiny( Buffer& into ) const = 0;
    //return ownership of a new foo.SlimItem dict
    virtual PyDict *MakeSlimItem() const = 0;
    //fill in the supplied damage state object.
    virtual void MakeDamageState(DoDestinyDamageState &into) const = 0;
    //return ownership of a new damage state tuple (calls MakeDamageState)
    PyTuple *MakeDamageState() const;
    PyList *MakeDamageStateList() const;

    //Im not happy with these being here..
    virtual void TargetAdded(SystemEntity *who) = 0;
    virtual void TargetLost(SystemEntity *who) = 0;
    virtual void TargetedAdd(SystemEntity *who) = 0;
    virtual void TargetedLost(SystemEntity *who) = 0;
    virtual void TargetsCleared() = 0;

    //process incoming damage, returns true on death.
    virtual bool ApplyDamage(Damage &d) = 0;
    //handles death.
    virtual void Killed(Damage &fatal_blow);
    void AwardSecurityStatus(InventoryItemRef m_self, Character* pChar);

    //helpers:
    double DistanceTo2(const SystemEntity *other) const;

    //  class type helpers.  public for anyone to access.
    virtual bool IsStaticEntity() const         { return true; }
    virtual bool IsDynamicEntity() const        { return false; }
    virtual bool IsVisibleSystemWide() const    { return false; }
    virtual bool IsInanimate() const            { return false; }
    virtual bool IsInvul() const                { return false; }
    virtual bool IsLogin()                      { return false; }
    virtual bool IsClient() const               { return false; }
    virtual bool IsNPC() const                  { return false; }
    virtual bool IsCelestial() const            { return false; }
    virtual bool IsContainer() const            { return false; }
    virtual bool IsWreck() const                { return false; }
    virtual bool IsOutpost() const              { return false; }
    virtual bool IsAsteroid() const             { return false; }
    virtual bool IsPOS() const                  { return false; }
    virtual bool IsJumpBridge() const           { return false; }
    virtual bool IsTCU() const                  { return false; }
    virtual bool IsShip() const                 { return false; }
    virtual bool IsDrone() const                { return false; }
    virtual bool IsDeployable() const           { return false; }
    virtual bool IsMissile() const              { return false; }

protected:
    SystemBubble* m_bubble;    //we do not own this, may be NULL. Only changed by SystemBubble

    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
};

// this class is for
class ItemSystemEntity : public SystemEntity {
public:
    ItemSystemEntity(InventoryItemRef self = InventoryItemRef());
    virtual ~ItemSystemEntity();

    //Default implementations fall to m_self.
    virtual uint32 GetID() const;
    virtual InventoryItemRef Item() const { return m_self; }
    virtual const char* GetName() const;

    virtual void ProcessDestiny() { }
    virtual const GPoint &GetPosition() const;
    virtual const GVector &GetVelocity() const = 0;     //virtual here because of no DestinyManager
    //virtual float GetRadius() const;
    virtual PyDict *MakeSlimItem() const;
    virtual void MakeDamageState(DoDestinyDamageState &into) const;
    virtual bool IsStaticEntity() const { return false; }
    virtual bool IsVisibleSystemWide() const { return false; }

    //process incoming damage, returns true on death.
    virtual bool ApplyDamage(Damage &d);

protected:
    InventoryItemRef m_self;

    void _SendDamageStateChanged(SystemEntity* source);
    void _SetSelf(InventoryItemRef self);
};


class DestinyManager;

// this class is for items that are not static
class DynamicSystemEntity : public ItemSystemEntity {
public:
    DynamicSystemEntity(DestinyManager* mgr=nullptr /*ownership taken*/, InventoryItemRef self = InventoryItemRef());
    virtual ~DynamicSystemEntity();

    //partial implementation of SystemEntity interface:
    virtual void ProcessDestiny();
    virtual const GPoint &GetPosition() const;
    virtual const GVector &GetVelocity() const;
    virtual void EncodeDestiny( Buffer& into ) const;

    virtual double GetMass() const;
    virtual double GetMaxVelocity() const;
    virtual double GetAgility() const;

    virtual PyDict *MakeSlimItem() const;
    virtual void MakeDamageState(DoDestinyDamageState &into) const;
    virtual bool IsDynamicEntity() const        { return true; }

    //Added interface:
    //get the corporation of this entity
    virtual uint32 GetCorporationID() const = 0;
    virtual uint32 GetAllianceID() const = 0;

    inline DestinyManager* Destiny() const { return m_destiny; }

    virtual void Killed(Damage &fatal_blow);

protected:
    DestinyManager* m_destiny;    //we own this! NULL if we are not in a system
};

/*
 * This class is used for Targetable and Destructable Celestial Objects
 */
class CelestialDynamicSystemEntity : public DynamicSystemEntity {
public:
    CelestialDynamicSystemEntity(DestinyManager *mgr=NULL /*ownership taken*/, InventoryItemRef self = InventoryItemRef());
    virtual ~CelestialDynamicSystemEntity();

    //partial implementation of SystemEntity interface:
    virtual void EncodeDestiny( Buffer& into ) const;
    virtual PyDict *MakeSlimItem() const;

};

#endif
