
/**
 * @name DataClasses.cpp
 *  data container classes that cannot be trivally constructed/destructed
 *
 * @author: allan
 * @date 4 January 2018
 */


#include "DataClasses.h"


ReactorData::ReactorData()
{
    Init();
}

ReactorData::~ReactorData()
{
    Clear();
}

void ReactorData::Clear()
{
    demands.clear();
    supplies.clear();
    connections.clear();
}

void ReactorData::Init()
{
    Clear();
    // not sure what 'default' init will be yet.
    active = false;
    itemID = 0;
    reaction = 0;
}

/*
PI_CCPin::PI_CCPin()
{

}

PI_CCPin::~PI_CCPin()
{

}

void PI_CCPin::Init()
{

}

PI_Pin::PI_Pin()
{

}

PI_Pin::~PI_Pin()
{

}

void PI_Pin::Init()
{

}
*/

