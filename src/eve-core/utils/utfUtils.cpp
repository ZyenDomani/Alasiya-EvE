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
    Author:     eve-moo
    Update:     Allan
 */

#include <string>
#include <cstdint>

#include "utfUtils.h"


/**
 * Convert a 32 bit character code to a UTF8 string
 * Enforced standard RFC 3629 max 4-byte tracking constraints
 */
std::string codeToUTF8(char32_t code)
{
    std::string out;
    if (code < 0x80) {
        out += static_cast<char>(code);
        return out;
    }
    if (code < 0x800) {
        out += static_cast<char>(0xC0 + ((code >> 6) & 0x1F));
        out += static_cast<char>(0x80 + (code & 0x3F));
        return out;
    }
    if (code < 0x10000) {
        out += static_cast<char>(0xE0 + ((code >> 12) & 0x0F));
        out += static_cast<char>(0x80 + ((code >> 6) & 0x3F));
        out += static_cast<char>(0x80 + (code & 0x3F));
        return out;
    }
    if (code < 0x110000) {
        out += static_cast<char>(0xF0 + ((code >> 18) & 0x07));
        out += static_cast<char>(0x80 + ((code >> 12) & 0x3F));
        out += static_cast<char>(0x80 + ((code >> 6) & 0x3F));
        out += static_cast<char>(0x80 + (code & 0x3F));
        return out;
    }
    return ""; // Invalid Unicode code point past RFC limits
}

std::u16string codeToUTF16(char32_t code)
{
    std::u16string out;
    if (code < 0x010000) {
        out += static_cast<char16_t>(code);
        return out;
    }
    if (code > 0x10FFFF) {
        return u"";
    }
    char32_t ch = code - 0x010000;
    char32_t cl = ch & 0x03FF;
    ch >>= 10;
    out += static_cast<char16_t>(0xD800 + ch);
    out += static_cast<char16_t>(0xDC00 + cl);
    return out;
}

class UTFCodeParser
{
public:
    UTFCodeParser() = default;
    ~UTFCodeParser() = default;

    UTFCodeParser(const UTFCodeParser& orig) = delete;
    UTFCodeParser& operator=(const UTFCodeParser& orig) = delete;

    int addChar(char c)
    {
        uint8_t uc = static_cast<uint8_t>(c);
        if (byte == 0) {
            if ((uc & 0x80) == 0) {
                code = uc;
                return byte;
            }

            // FIX: Track leading bits securely to prevent infinite shift boundaries
            uint8_t mask = 0x80;
            while ((uc & mask) != 0) {
                byte++;
                mask >>= 1;
                if (byte > 4) { // Secure ceiling protection
                    code = 0;
                    byte = 0;
                    return -1;
                }
            }

            if (byte < 2) {
                code = 0;
                byte = 0;
                return -1;
            }

            code = uc & (mask - 1);
            byte--;
            return byte;
        }

        if ((uc & 0xC0) != 0x80) {
            code = 0;
            byte = 0;
            return -1;
        }

        byte--;
        code <<= 6;
        code += (uc & 0x3F);
        return byte;
    }

    int addChar(char16_t c)
    {
        if (byte == 0) {
            if (c < 0xD800 || c >= 0xE000) {
                code = c;
                return byte;
            }
            if (c >= 0xDC00) {
                code = 0;
                return -1;
            }
            byte = 1;
            code = c & 0x3FF;
            return byte;
        }
        if (c < 0xDC00 || c >= 0xE000) {
            if (c < 0xD800) {
                code = c;
                byte = 0;
                return 0;
            }
            byte = 1;
            code = c & 0x3FF;
            return byte;
        }
        byte--;
        code <<= 10;
        code += (c & 0x3FF);
        code += 0x010000;
        return byte;
    }

    int addChar(char32_t c)
    {
        code = c;
        byte = 0;
        return byte;
    }

    std::string getUTF8()
    {
        if (byte == 0) {
            return codeToUTF8(code);
        }
        return "";
    }

    std::u16string getUTF16()
    {
        if (byte == 0) {
            return codeToUTF16(code);
        }
        return u"";
    }

    std::u32string getUTF32()
    {
        if (byte == 0) {
            std::u32string out;
            out += code;
            return out;
        }
        return U"";
    }

    void reset()
    {
        code = 0;
        byte = 0;
    }

    char32_t getCode() const { return code; }
    bool valid() const { return (byte == 0); }

private:
    char32_t code = 0;
    int byte = 0;
};

// FIX: Corrected iteration parameters to align exact string element widths
std::u16string utf8to16(const std::string &str)
{
    std::u16string out;
    UTFCodeParser code_parser;
    for (char c : str) { // Changed char16_t back to correct char layout
        int res = code_parser.addChar(c);
        if (res == -1) {
            return u"";
        }
        if (res == 0) {
            out += code_parser.getUTF16();
        }
    }
    return out;
}

std::string utf16to8(const std::u16string &str)
{
    std::string out;
    UTFCodeParser code_parser;
    for (char16_t c : str) { // Changed char back to correct char16_t layout
        int res = code_parser.addChar(c);
        if (res == -1) {
            return "";
        }
        if (res == 0) {
            out += code_parser.getUTF8();
        }
    }
    return out;
}
