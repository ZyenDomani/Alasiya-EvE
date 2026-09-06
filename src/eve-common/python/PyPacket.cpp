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
    Author:     Zhur
    Rewrite:    Allan
*/


#include "eve-common.h"

#include "python/PyPacket.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"

const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT] =
{
    "AUTHENTICATION_REQ",
    "AUTHENTICATION_RSP",
    "IDENTIFICATION_REQ",
    "IDENTIFICATION_RSP",
    "DISCONNECT_NOTIFICATION", //*
    "HEARTBEAT",        //*
    "CALL_REQ",
    "CALL_RSP",
    "TRANSPORTCLOSED",
    "FORWARD_REQ",  //*
    "RESOLVE_REQ",
    "RESOLVE_RSP",
    "NOTIFICATION",
    "MULTICAST_NOTIFICATION",  // pack the python object data exactly once, compress it once, attach an array of target SessionIDs to the header, and throw it at the network card.
    "REJECT_NOTIFICATION",  //*
    "ERRORRESPONSE",
    "SESSIONCHANGENOTIFICATION",
    "SESSIONINITIALSTATEREQ",   // send me the local grid layout.
    "SESSIONINITIALSTATENOTIFICATION",
    "HEARTBEAT_ACK",  //*
    "PING_REQ",
    "PING_RSP"
};

PyPacket::PyPacket()
: type_string("none"),
type(DISCONNECT_NOTIFICATION),
userid(0),
payload(nullptr),
named_payload(nullptr)
{
    // nothing to do here
}

PyPacket::~PyPacket()
{
    PySafeDecRef(payload);
    PySafeDecRef(named_payload);
}

PyPacket *PyPacket::Clone() const
{
    PyPacket *res = new PyPacket();
        res->type_string = type_string;
        res->type = type;
        res->source = source;
        res->dest = dest;
        res->userid = userid;
        res->payload = payload->Clone();
    if (payload == nullptr) {
        res->payload = nullptr;
    } else {
        res->payload = payload->Clone();
    }
    if (named_payload == nullptr) {
        res->named_payload = nullptr;
    } else {
        res->named_payload = named_payload->Clone();
    }
    return res;
}

void PyPacket::Dump(LogType ltype, PyVisitor& dumper)
{
    _log(ltype, "Packet:");
    _log(ltype, "  Type: %s", type_string.c_str());
    _log(ltype, "  Command: %s (%d)",
         (type >= 0 && type < MACHONETMSG_TYPE_COUNT) ? MACHONETMSG_TYPE_NAMES[type] : "UNKNOWN_TYPE",
         type);
    _log(ltype, "  Source:");
    source.Dump(ltype, "    ");
    _log(ltype, "  Dest:");
    dest.Dump(ltype, "    ");
    _log(ltype, "  User ID: %u", userid);
    _log(ltype, "  Payload:");
    payload->visit( dumper );
    if (named_payload == nullptr) {
        _log(ltype, "  Named Payload: None (null)");
    } else {
        _log(ltype, "  Named Payload:");
        named_payload->visit( dumper );
    }
}

bool PyPacket::Decode(PyRep **in_packet)
{
    PyRep *pRep = *in_packet;    //assign
    *in_packet = nullptr;       //consume

    PySafeDecRef(payload);
    PySafeDecRef(named_payload);
    payload = nullptr;
    named_payload = nullptr;

    if ( pRep == nullptr) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet is null.");
        return false;
    }

    if ( pRep->IsChecksumedStream()) {
        //TODO: check cs->checksum
        pRep = pRep->AsChecksumedStream()->stream();
    }
    //Dragon nuance... it gets wrapped again
    if ( pRep->IsSubStream()) {
        PySubStream* ss = pRep->AsSubStream();
        ss->DecodeData();
        if (ss->decoded() == nullptr) {
            codelog(NET__PACKET_ERROR, "PyPacket::Decode() - unable to decode initial packet substream.");
            PyDecRef( pRep );
            return false;
        }

        pRep = ss->decoded();
    }

    if (!pRep->IsObject()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet body is not PyObject: %s", pRep->TypeString());
        PyDecRef( pRep );
        return false;
    }

    type_string = pRep->AsObject()->type()->content();
    if (!pRep->AsObject()->arguments()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet body does not contain a tuple");
        return false;
    }

    PyTuple *tuple = pRep->AsObject()->arguments()->AsTuple();
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - tuple is null.");
        return false;
    }

    if (tuple->items.size() != 7) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet body does not contain a tuple of length 7 (is %zu)", tuple->items.size());
        PyDecRef( pRep );
        return false;
    }

    if (!tuple->items[0]->IsInt()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - First main tuple element is not an integer");
        PyDecRef( pRep );
        return false;
    }

    switch(PyRep::IntegerValue(tuple->items[0])) {
        case AUTHENTICATION_REQ:
        case AUTHENTICATION_RSP:
        case IDENTIFICATION_REQ:
        case IDENTIFICATION_RSP:
        case CALL_REQ:
        case CALL_RSP:
        case TRANSPORTCLOSED:
        case RESOLVE_REQ:
        case RESOLVE_RSP:
        case NOTIFICATION:
        case ERRORRESPONSE:
        case SESSIONCHANGENOTIFICATION:
        case SESSIONINITIALSTATEREQ:
        case SESSIONINITIALSTATENOTIFICATION:
        case PING_REQ:
        case PING_RSP: {
            type = (MACHONETMSG_TYPE) PyRep::IntegerValue(tuple->items[0]);
        } break;
        default: {
            codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Unknown message type %li", PyRep::IntegerValue(tuple->items[0]));
            PyDecRef( pRep );
            return false;
        } break;
    }

    //source address
    if (!source.Decode(tuple->items[1]))  {
        //error printed in decoder
        PyDecRef( pRep );
        return false;
    }
    //dest address
    if (!dest.Decode(tuple->items[2])) {
        //error printed in decoder
        PyDecRef( pRep );
        return false;
    }

    userid = PyRep::IntegerValue(tuple->items[3]);

    //payload
    if (!tuple->items[4]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Fifth main tuple element is not a tuple");
        PyDecRef( pRep );
        return false;
    }
    payload = tuple->items[4]->AsTuple();
    PyIncRef(payload);

    //options dict
    if (tuple->items[5]->IsNone()) {
        named_payload = nullptr;
    } else if (tuple->items[5]->IsDict()) {
        named_payload = tuple->items[5]->AsDict();
        PyIncRef(named_payload);
    } else {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Sixth main tuple element is neither dict or none.");
        PyDecRef( pRep );
        return false;
    }

    receivedTime = GetFileTimeNow();

    PyDecRef( pRep );
    return true;
}

PyRep *PyPacket::Encode() {
    PyTuple* arg_tuple = new PyTuple(7);
    //command
    arg_tuple->SetItem(0, new PyInt(type));
    //source
    arg_tuple->SetItem(1, source.Encode());
    //dest
    arg_tuple->SetItem(2, dest.Encode());
    //userid
    arg_tuple->SetItem(3, (userid == 0 ? PyStatic.NewZero() : new PyInt(userid)));
    //payload
    arg_tuple->SetItem(4, payload);     // dont clone here.  set actual rep in item, and it will be cleaned up by d'tor later
    //named arguments (OID+ or sn)
    // dont clone here.  set actual rep in item, and it will be cleaned up by d'tor later
    arg_tuple->SetItem(5, (named_payload == nullptr ? PyStatic.NewNone() : named_payload));
    //TODO: Not sure what this is, On packets so far they always have as PyNone
    arg_tuple->SetItem(6, PyStatic.NewNone());
    return new PyObject(type_string.c_str(), arg_tuple);
}

PyAddress::PyAddress()
: type(Invalid),
objectID(0),
callID(0),
service("")
{
    // nothing to do here
}

void PyAddress::Dump(FILE *into, const char *pfx) const {
    switch(type) {
        case Any:
            fprintf(into, "%sAny: service='%s' callID=%li", pfx, service.c_str(), callID);
        break;
        case Node:
            fprintf(into, "%sNode: nodeID=%li service='%s' callID=%li", pfx, objectID, service.c_str(), callID);
        break;
        case Client:
            fprintf(into, "%sClient: clientID=%li service='%s' callID=%li", pfx, objectID, service.c_str(), callID);
        break;
        case Broadcast:
            fprintf(into, "%sBroadcast: broadcastID='%s' narrowcast=(not implemented) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
        break;
        case Invalid:
        break;
    //no default on purpose
    }
}

void PyAddress::Dump(LogType ltype, const char *pfx) const {
    switch(type) {
        case Any:
            _log(ltype, "%sAny: service='%s' callID=%li", pfx, service.c_str(), callID);
        break;
        case Node:
            _log(ltype, "%sNode: nodeID=%li service='%s' callID=%li", pfx, objectID, service.c_str(), callID);
        break;
        case Client:
            _log(ltype, "%sClient: clientID=%li callID=%li service='%s'", pfx, objectID, callID, service.c_str());
        break;
        case Broadcast:
            _log(ltype, "%sBroadcast: broadcastID='%s' narrowcast=(not implemented) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
        break;
        case Invalid:
        break;
    //no default on purpose
    }
}

PyAddress& PyAddress::operator=(const PyAddress &right) {
    type = right.type;
    objectID = right.objectID;
    callID = right.callID;
    service = right.service;
    bcast_idtype = right.bcast_idtype;
    return *this;
}

bool PyAddress::Decode(PyRep *&in_object) {
    PyRep* pRep = in_object;   //assign
    in_object = nullptr;

    if ( pRep == nullptr) {
        codelog(NET__PACKET_ERROR, "PyAddress::Decode() - base is null.");
        return false;
    }

    if (!pRep->IsObject()) {
        codelog(NET__PACKET_ERROR, "Invalid element type, expected object but got %s", pRep->TypeString());
        PyDecRef( pRep );
        return false;
    }

    PyTuple *tuple = pRep->AsObject()->arguments()->AsTuple();
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "PyAddress::Decode() - tuple is null.");
        return false;
    }

    if (tuple->items.size() < 3) {
        codelog(NET__PACKET_ERROR, "Not enough elements in address tuple: %zu", tuple->items.size());
        tuple->Dump(NET__PACKET_ERROR, "  ");
        return false;
    }

    //decode the address type.
    if (!tuple->items[0]->IsInt()) {
        codelog(NET__PACKET_ERROR, "Wrong type on address element (0)");
        tuple->items[0]->Dump(NET__PACKET_ERROR, "  ");
        return false;
    }

    switch(PyRep::IntegerValue(tuple->items[0])) {
        case Any: {
            if (tuple->items.size() != 3) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Any address tuple: %zu", tuple->items.size());
                return false;
            }
            type = Any;
            service = PyRep::StringContent(tuple->items[1]);
            callID = PyRep::IntegerValue(tuple->items[2]);
        }  break;
        case Node: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Node address tuple: %zu", tuple->items.size());
                return false;
            }
            type = Node;
            objectID = PyRep::IntegerValue(tuple->items[1]);
            service = PyRep::StringContent(tuple->items[2]);
            callID = PyRep::IntegerValue(tuple->items[3]);
        }  break;
        case Client: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Client address tuple: %zu", tuple->items.size());
                return false;
            }
            type = Client;
            objectID = PyRep::IntegerValue(tuple->items[1]);
            callID = PyRep::IntegerValue(tuple->items[2]);
            service = PyRep::StringContent(tuple->items[3]);
        }  break;
        case Broadcast: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Broadcast address tuple: %zu", tuple->items.size());
                return false;
            }
            type = Broadcast;

            if (!tuple->items[1]->IsString()) {
                codelog(NET__PACKET_ERROR, "Invalid type %s for brodcastID", tuple->items[1]->TypeString());
                return false;
            }
            if (!tuple->items[3]->IsString()) {
                codelog(NET__PACKET_ERROR, "Invalid type %s for idtype", tuple->items[3]->TypeString());
                return false;
            }

            service = PyRep::StringContent(tuple->items[1]);       //assign op
            bcast_idtype = PyRep::StringContent(tuple->items[3]);  //assign op

            //items[2] is either a list or a tuple.
            /*
             *            //PyList *nclist = (PyList *) tuple->items[2];
             *            if (!nclist->items.empty()) {
             *                printf("Not decoding narrowcast list:");
             *                nclist->Dump(NET__PACKET_ERROR, "     ");
        }*/
        }   break;
        default: {
            codelog(NET__PACKET_ERROR, "Unknown address type: %li", PyRep::IntegerValue(tuple->items[0]));
            return false;
        }
    }

    return true;
}

PyRep* PyAddress::Encode() {
    PyTuple* rsp = nullptr;
    switch(type) {
        case Any: {             //8
            rsp = new PyTuple(3);
            rsp->SetItemInt(0, type);
            rsp->SetItem(1, (service.empty() ? PyStatic.NewNone() : new PyString(service.c_str())));
            rsp->SetItem(2, (objectID == 0 ? PyStatic.NewNone() : new PyLong(objectID)));
        } break;
        case Node: {    //1
            rsp = new PyTuple(4);
            rsp->SetItemInt(0, type);
            rsp->SetItem(1, new PyLong(objectID));
            rsp->SetItem(2, (service.empty() ? PyStatic.NewNone() : new PyString(service.c_str())));
            rsp->SetItem(3, (callID == 0 ? PyStatic.NewNone() : new PyLong(callID)));
        } break;
        case Client: {  //2
            rsp = new PyTuple(4);
            rsp->SetItemInt(0, type);
            rsp->SetItem(1, new PyLong(objectID));
            rsp->SetItem(2, (callID == 0 ? PyStatic.NewNone() : new PyLong(callID)));
            rsp->SetItem(3, (service.empty() ? PyStatic.NewNone() : new PyString(service.c_str())));
        } break;
        case Broadcast: {       //4
            rsp = new PyTuple(4);
            rsp->SetItemInt(0, type);
            //broadcastID
            rsp->SetItem(1, (service.empty() ? PyStatic.NewNone() : new PyString(service.c_str())));
            //narrowcast
            rsp->SetItem(2, PyStatic.mtList()); // LSC uses tuple here, others None() or empty List()
            //typeID
            rsp->SetItemString(3, bcast_idtype.c_str());
        } break;
        case Invalid:
        default: {
            //this still needs to be something which will not crash us.
            rsp = PyStatic.mtTuple();
        } break;
    }

    return new PyObject("macho.MachoAddress", rsp);
}


PyCallStream::PyCallStream()
: remoteObject(0),
  method(""),
  arg_tuple(nullptr),
  arg_dict(nullptr)
{
}

PyCallStream::~PyCallStream() {
    // verify these
    //PySafeDecRef(arg_tuple);
    //PySafeDecRef(arg_dict);
}

PyCallStream *PyCallStream::Clone() const {
    PyCallStream *res = new PyCallStream();
    res->remoteObject = remoteObject;
    res->remoteObjectStr = remoteObjectStr;
    res->method = method;
    res->arg_tuple = arg_tuple->Clone();
    if (arg_dict == nullptr) {
        res->arg_dict = nullptr;
    } else {
        res->arg_dict = arg_dict->Clone();
    }

    return res;
}

void PyCallStream::Dump(LogType type, PyVisitor& dumper)
{
    _log(type, "Call Stream:");
    if (remoteObject == 0) {
        _log(type, "  Remote Object: '%s'", remoteObjectStr.c_str());
    } else {
        _log(type, "  Remote Object: %u", remoteObject);
    }
    _log(type, "  Method: %s", method.c_str());
    _log(type, "  Arguments:");
    arg_tuple->visit( dumper );
    if (arg_dict == nullptr) {
        _log(type, "  Named Arguments: None");
    } else {
        _log(type, "  Named Arguments:");
        arg_dict->visit( dumper );
    }
}

bool PyCallStream::Decode(const std::string &type, PyTuple *&in_payload) {
    PyTuple *payload = in_payload;   //copy
    in_payload = nullptr;            //consume

    PySafeDecRef(arg_tuple);
    PySafeDecRef(arg_dict);
    arg_tuple = nullptr;
    arg_dict = nullptr;

    if (payload == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - payload is null.");
        return false;
    }

    if (type != "macho.CallReq") {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - packet payload has unknown string type '%s'", type.c_str());
        return false;
    }

    if (payload->items.size() != 1) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - invalid tuple length %zu", payload->items.size());
        return false;
    }
    if (!payload->items[0]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - non tuple payload[0]");
        return false;
    }

    PyTuple *payload2(payload->items[0]->AsTuple());
    if (payload2 == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - payload2 is null.");
        return false;
    }

    if (payload2->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - invalid tuple2 length %zu", payload2->items.size());
        return false;
    }

    //decode inner payload tuple
    // payload2->items[0] represents the Macho Service Routing Mode flag:
    // 0 = Specific Instantiated GPC Object - bound memory address instance
    // 1 = Named Cluster Service - calling a manager or broker service by name
    if (!payload2->items[1]->IsSubStream()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - non-substream type");
        return false;
    }

    PySubStream *ss(payload2->items[1]->AsSubStream());
    if (ss == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - ss is null.");
        return false;
    }

    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - Unable to decode call stream");
        return false;
    }

    if (!ss->decoded()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - packet body does not contain a tuple");
        return false;
    }

    PyTuple *maint(ss->decoded()->AsTuple());
    if (maint == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint is null.");
        return false;
    }
    if (maint->items.size() != 4) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - packet body has %zu elements, expected %d", maint->items.size(), 4);
        return false;
    }

    //parse first tuple element, Target Object Identifier / Destination Service Key
    /* PyString: holds the explicit name of the cluster service being executed (e.g., "LSC", "market", "docking").
     * This maps right to remoteObjectStr.
     * PyInt / PyLong: It holds a dynamic memory object runtime tracking handle or node reference target
     * (like an active fleet object ID or specific structural inventory instance handle).
     * This maps straight to remoteObject.
     */
    if (maint->items[0]->IsInt()) {
        remoteObject = PyRep::IntegerValue(maint->items[0]);
        remoteObjectStr = "";
    } else if (maint->items[0]->IsString()) {
        remoteObject = 0;
        remoteObjectStr = PyRep::StringContent(maint->items[0]);
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint->items[0] has invalid type %s", maint->items[0]->TypeString());
        codelog(NET__PACKET_ERROR, " in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        return false;
    }

    //parse tuple[1]: method name
    if (maint->items[1]->IsString()) {
        method = PyRep::StringContent(maint->items[1]);
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint->items[1] has non-string type");
        maint->items[1]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, " in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        return false;
    }

    //grab argument list.
    if (!maint->items[2]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - argument list has non-tuple type");
        maint->items[2]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, "in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        return false;
    }
    arg_tuple = maint->items[2]->AsTuple();

    //options dict
    if (maint->items[3]->IsNone()) {
        arg_dict = nullptr;
    } else if (maint->items[3]->IsDict()) {
        arg_dict = maint->items[3]->AsDict();
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - tuple[3] has non-dict type");
        maint->items[3]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, "in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        return false;
    }

    return true;
}

PyTuple *PyCallStream::Encode() {
    PyTuple *res_tuple = new PyTuple(4);

    //remoteObject
    if (remoteObject == 0) {
        res_tuple->items[0] = new PyString(remoteObjectStr.c_str());
    } else {
        res_tuple->items[0] = new PyInt(remoteObject);
    }

    //method name
    res_tuple->items[1] = new PyString(method.c_str());

    //args
    // set actual rep in item, and it will be cleaned up by d'tor later
    res_tuple->items[2] = arg_tuple;

    //options
    if (arg_dict == nullptr) {
        res_tuple->items[3] = PyStatic.NewNone();
    } else {
        // set actual rep in item, and it will be cleaned up by d'tor later
        res_tuple->items[3] = arg_dict;
    }

    //now that we have the main arg tuple, build the other stuff around it...
    PyTuple *it2 = new PyTuple(2);
        //If remoteObject == 0 (it's a string, like "dynamicIDManager" or "broker"), this flag tells the cluster router that this call targets a named cluster-wide Service, requiring the proxy to resolve it to an active service node.
        //If remoteObject != 0, it means it targets a specific Global Project Object (GPC) / bound object instance (like an active session object, a solar system manager, or a specific ship entity), bypassing named-service lookup.
        it2->items[0] = new PyInt(remoteObject==0?1:0); /* isService or isGPCObject flag */
        it2->items[1] = new PySubStream(res_tuple);
    PyTuple *it1 = new PyTuple(2);
        it1->items[0] = it2;
        //this is the "channel" dict if populated.  obviously incomplete  **see xtra notes on this**
        it1->items[1] = PyStatic.NewNone();
    return it1;
}

EVENotificationStream::EVENotificationStream()
: notifyType("NO TYPE SET"),
  remoteObject(0),
  args(nullptr)
{
}

EVENotificationStream::~EVENotificationStream() {
    PySafeDecRef(args);
}

EVENotificationStream *EVENotificationStream::Clone() const {
    EVENotificationStream* res = new EVENotificationStream();
    if (args == nullptr) {
        res->args = nullptr;
    } else {
        res->args = args->Clone();
    }

    return res;
}

void EVENotificationStream::Dump(LogType type, PyVisitor& dumper)
{
    _log(type, "Notification: %s", notifyType.c_str());
    if (remoteObject == 0) {
        _log(type, "  Remote Object: %s", remoteObjectStr.c_str());
    } else {
        _log(type, "  Remote Object: %u", remoteObject);
    }

    _log(type, "  Arguments:");
    args->visit( dumper );
}

bool EVENotificationStream::Decode(const std::string &pkt_type, const std::string &notify_type, PyTuple *&in_payload) {
    PyTuple *payload = in_payload;      //copy
    in_payload = nullptr;               //consume

    PySafeDecRef(args);
    args = nullptr;

    if (payload == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - payload is null.");
        return false;
    }

    if (pkt_type != "macho.Notification") {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - notification payload has unknown string type %s", pkt_type.c_str());
        return false;
    }

    //decode payload tuple
    if (payload->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - invalid tuple length %zu", payload->items.size());
        return false;
    }
    if (!payload->items[0]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - non-tuple payload[0]");
        return false;
    }
    PyTuple *payload2(payload->items[0]->AsTuple());
    if (payload2 == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - payload2 is null.");
        return false;
    }

    if (payload2->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - invalid tuple2 length %zu", payload2->items.size());
        return false;
    }

    //decode inner payload tuple
    // payload2->items[0] represents the Macho Service Routing Mode flag:
    // 0 = Specific Instantiated GPC Object - bound memory address instance
    // 1 = Named Cluster Service - calling a manager or broker service by name
    if (!payload2->items[1]->IsSubStream()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - non-substream type");
        return false;
    }

    PySubStream *ss(payload2->items[1]->AsSubStream());
    if (ss == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - ss is null.");
        return false;
    }
    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - Unable to decode call stream");
        return false;
    }

    if (!ss->decoded()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body does not contain a tuple");
        return false;
    }

    PyTuple *robjt(ss->decoded()->AsTuple());
    if (robjt == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - robjt is null.");
        return false;
    }
    if (robjt->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body has %zu elements, expected %d", robjt->items.size(), 2);
        return false;
    }

    //parse first tuple element, remote object
    if (robjt->items[0]->IsInt()) {
        remoteObject = PyRep::IntegerValueU32(robjt->items[0]);
        remoteObjectStr = "";
    } else if (robjt->items[0]->IsString()) {
        remoteObject = 0;
        remoteObjectStr = PyRep::StringContent(robjt->items[0]);
    } else {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - main tuple[0] has invalid type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " in:");
        payload->Dump( NET__PACKET_ERROR, "" );
       return false;
    }

    if (!robjt->items[1]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - main tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " it is:");
        payload->Dump( NET__PACKET_ERROR, "" );
        return false;
    }

    PyTuple *subt(robjt->items[1]->AsTuple());
    if (subt == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - subt is null.");
        return false;
    }
    if (subt->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body has %zu elements, expected %d", subt->items.size(), 2);
        return false;
    }

    //parse first tuple element, remote object
    if (subt->items[0]->IsInt()) {
        ; //MachoCommandType identifier (1 = OBJECT_METHOD_CALL)
        //PyInt *tuple0 = (PyInt *) maint->items[0];
    } else {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - sub tuple[0] has invalid type %s", subt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " in:");
        payload->Dump( NET__PACKET_ERROR, "" );
        return false;
    }

    if (!subt->items[1]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - subt tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " it is:");
        payload->Dump( NET__PACKET_ERROR, "" );
        return false;
    }

    args = subt->items[1]->AsTuple();
    notifyType = notify_type;

    return true;
}

PyTuple *EVENotificationStream::Encode() {
    PyTuple *t4 = new PyTuple(2);
        t4->SetItem(0, PyStatic.NewOne());      // MachoCommandType  (1 = OBJECT_METHOD_CALL, 2 (Macho Exception))
        t4->SetItem(1, args);       // set actual rep in item, and it will be cleaned up by d'tor later
    PyTuple *t3 = new PyTuple(2);
        t3->SetItem(0, PyStatic.NewZero());     // Data Stream Compression Flag  (StreamType)
        t3->SetItem(1, t4);
    PyTuple *t2 = new PyTuple(2);
        t2->SetItem(0, PyStatic.NewZero());     // Transport Protocol Version  (MachoProtocolVersion)
        t2->SetItem(1, new PySubStream(t3));
    PyTuple *t1 = new PyTuple(1);
        t1->SetItem(0, t2);
    return t1;
}
