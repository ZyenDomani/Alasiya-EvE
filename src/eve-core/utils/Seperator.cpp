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
    Author:        Zhur, Bloody.Rabbit
    Update:     Allan
*/

#include <cstring>

#include "Seperator.h"

Seperator::Seperator( const char* str, const char* divs, const char* quotes )
{
    if (str == nullptr || *str == '\0') {
        return;
    }

    bool inQuote = false;
    std::string currentArg;
    bool hasToken = false;

    // Linear single-pass scan through the string until the null-terminator
    for (; *str != '\0'; ++str) {
        const char c = *str;

        // Fast direct lookup replacements for strchr
        const bool isDiv = (std::strchr(divs, c) != nullptr);
        const bool isQuote = (std::strchr(quotes, c) != nullptr);

        if (inQuote) {
            if (isQuote) {
                // Closing quote found, close the active token state
                inQuote = false;
            } else {
                currentArg.push_back(c);
            }
        } else {
            if (isQuote) {
                inQuote = true;
                hasToken = true; // Quotes implicitly start a token boundary
            } else if (isDiv) {
                if (hasToken) {
                    // Delimiter hit outside of quotes ends the current argument block
                    mArgs.push_back(std::move(currentArg)); // Leverage C++11 move semantics!
                    currentArg.clear();
                    hasToken = false;
                }
            } else {
                currentArg.push_back(c);
                hasToken = true;
            }
        }
    }

    // Flush any trailing token left sitting in the buffer after the loop terminates
    if (hasToken) {
        mArgs.push_back(std::move(currentArg));
    }
}
