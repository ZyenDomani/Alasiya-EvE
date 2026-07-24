/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Updates:    Allan
*/

#include "eve-xmlpktgen.h"

#include "EncodeGenerator.h"


ClassEncodeGenerator::ClassEncodeGenerator( FILE* outputFile )
: Generator( outputFile ),
  mItemNumber( 0 ),
  mName( nullptr )
{
    RegisterProcessors();
}

bool ClassEncodeGenerator::ProcessElementDef( const TiXmlElement* field )
{
    //  new switch to (dis)allow encoding.    -xmlp bloat wip
    bool encode = true;
    const char* encode_str = field->Attribute("encode");
    if (encode_str != nullptr)
        encode = str2<bool>(encode_str);
    if (!encode)
        return true;

    mName = field->Attribute( "name" );
    if (mName == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator:: <element> at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    const TiXmlElement* main = field->FirstChildElement();
    if (main->NextSiblingElement() != nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator:: <element> at line " << field->Row() << " contains more than one root element, skipping.";
        return false;
    }

    const char* encode_type = GetEncodeType( main );
    fprintf( mOutputFile,
        "%s* %s::Encode() const {\n"
        "    %s* res = nullptr;\n\n",
        encode_type, mName,
            encode_type
    );

    mItemNumber = 0;
    clear();

    push( "res" );
    if (!ParseElement( main ) )
        return false;

    fprintf( mOutputFile,
        "    return res;\n"
        "}\n\n"
    );

    return true;
}

bool ClassEncodeGenerator::ProcessElement( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessElement field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Fetch the target destination string or layout mask from the stack
    std::string currentTarget = top();
    pop();

    // 2. Pre-format the C++ output expression for encoding the sub-object
    char valueBuffer[64];
    snprintf(valueBuffer, sizeof(valueBuffer), "%s.Encode()", name);

    // 3. Merge the sub-object expression safely based on target type
    char finalizedLine[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat destination variable (like obj0_args or ss_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 4. Output the completed statement
    fprintf( mOutputFile, "   %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessElementPtr( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessElementPtr field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Fetch the parent layout mask from your string stack
    std::string currentMask = top();
    pop();

    // 2. Format separate buffers for the valid encoding block and the null fallback block
    char validBuffer[64];
    snprintf(validBuffer, sizeof(validBuffer), "%s->Encode()", name);
    char validLine[256];
    snprintf(validLine, sizeof(validLine), currentMask.c_str(), validBuffer);

    char nullLine[256];
    snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "PyStatic.NewNone()");

    // 3. Write out the conditional logic to the output file
    fprintf( mOutputFile, "    if (%s != nullptr) {\n", name );
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    } else {\n" );
    fprintf( mOutputFile, "        sLog.Error(\"EncodeObject()\",\"%s.%s is null. hacking some weird shit here, so be aware.\");\n", mName, name );
    fprintf( mOutputFile, "        EvE::traceStack();\n" );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    }\n\n" );

    return true;
}
bool ClassEncodeGenerator::ProcessRaw( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessRaw() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // Grab the layout mask string directly from your native stack
    std::string currentMask = top();
    pop();

    char noneBuffer[256];
    char validBuffer[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(noneBuffer, sizeof(noneBuffer), currentMask.c_str(), "PyStatic.NewNone()");
        snprintf(validBuffer, sizeof(validBuffer), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict3_2)
    else {
        snprintf(noneBuffer, sizeof(noneBuffer), "%s = PyStatic.NewNone()", currentMask.c_str());
        snprintf(validBuffer, sizeof(validBuffer), "%s = %s", currentMask.c_str(), name);
    }

    // Generate code that safely increments the borrowed reference if it is valid
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        %s;\n", noneBuffer );
    fprintf( mOutputFile, "    } else {\n" );

    // Safely borrow ownership before the sink claims it
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validBuffer );
    fprintf( mOutputFile, "    }\n" );

    return true;
}

bool ClassEncodeGenerator::ProcessInt( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessInt() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Create the clean object allocation string
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyInt( %s )", name);

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessLong( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessLong() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyLong( %s )", name);

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessReal( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessReal() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyFloat( %s )", name);

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessBool( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessBool() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyBool( %s )", name);

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessNone( const TiXmlElement* field )
{
    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "PyStatic.NewNone()");

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessBuffer( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessBuffer() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    std::string currentMask = top();
    pop();

    char nullLine[256];
    char validLine[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "new PyBuffer(0)");
        snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict1_0)
    else {
        snprintf(nullLine, sizeof(nullLine), "%s = new PyToken(\"\")", currentMask.c_str());
        snprintf(validLine, sizeof(validLine), "%s = %s", currentMask.c_str(), name);
    }

    // Output the structural code block cleanly
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        sLog.Error(\"EncodeObject()\",\"%s.%s is null. hacking some weird shit here, so be aware.\");\n", mName, name );
    fprintf( mOutputFile, "        EvE::traceStack();\n" );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    } else {\n" );

    // Borrowing protection (Only run this if it's a valid pointer we are capturing)
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    }\n\n" );

    return true;
}

bool ClassEncodeGenerator::ProcessString( const TiXmlElement* field )
{
    const char* value = field->Attribute("value");
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessString() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char valueBuffer[256];
    memset(valueBuffer, 0, sizeof(valueBuffer));

    // CASE 1: It is a hardcoded string literal constant (<stringInline value="xxx" />)
    if (value != nullptr) {
        // Enclose it in explicit quotes so it compiles as a raw C++ string literal
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( \"%s\" )", value);
    }
    // CASE 2: It is a standard class member variable (<string name="xxx" />)
    else if (name != nullptr) {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( %s )", name);
    }
    // FALLBACK: Security safety check if both attributes are totally missing
    else {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( \"\" )");
    }

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessStringInline( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    const char* value = field->Attribute("value");
    if (value == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessStringInline() field at line " << field->Row() << " is missing the value attribute, skipping.";
        return false;
    }

    char valueBuffer[256];
    memset(valueBuffer, 0, sizeof(valueBuffer));

    // CASE 1: It is a hardcoded string literal constant (<string value="xxx" />)
    if (value != nullptr) {
        // Enclose it in explicit quotes so it compiles as a raw C++ string literal
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( \"%s\" )", value);
    }
    // CASE 2: It is a standard class member variable (<string name="xxx" />)
    else if (name != nullptr) {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( %s )", name);
    }
    // FALLBACK: Security safety check if both attributes are totally missing
    else {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyString( \"\" )");
    }

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessWString( const TiXmlElement* field )
{
    const char* value = field->Attribute("value");
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessWString() field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    memset(valueBuffer, 0, sizeof(valueBuffer));

    // CASE 1: It is a hardcoded string literal constant (<wstring value="xxx" />)
    if (value != nullptr) {
        // Enclose it in explicit quotes so it compiles as a raw C++ string literal
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( \"%s\" )", value);
    }
    // CASE 2: It is a standard class member variable (<string name="xxx" />)
    else if (name != nullptr) {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( %s )", name);
    }
    // FALLBACK: Security safety check if both attributes are totally missing
    else {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( \"\" )");
    }

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessWStringInline( const TiXmlElement* field )
{
    const char* value = field->Attribute("value");
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessWStringInline() field at line " << field->Row() << " is missing the name attribute, skipping.";
    }

    // 1. Create the clean C++ initialization text fragment
    char valueBuffer[256];
    memset(valueBuffer, 0, sizeof(valueBuffer));

    // CASE 1: It is a hardcoded string literal constant (<wstring value="xxx" />)
    if (value != nullptr) {
        // Enclose it in explicit quotes so it compiles as a raw C++ string literal
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( \"%s\" )", value);
    }
    // CASE 2: It is a standard class member variable (<string name="xxx" />)
    else if (name != nullptr) {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( %s )", name);
    }
    // FALLBACK: Security safety check if both attributes are totally missing
    else {
        snprintf(valueBuffer, sizeof(valueBuffer), "new PyWString( \"\" )");
    }

    // 2. Fetch the target destination string from your stack
    std::string currentTarget = top();
    pop();

    char finalizedLine[512];
    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    }
    // CASE B: The parent is a flat temporary variable (like ss_2 or dict1_0)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 3. Output the perfectly formed C++ statement
    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}
bool ClassEncodeGenerator::ProcessToken( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessToken field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    std::string currentMask = top();
    pop();

    char nullLine[256];
    char validLine[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "new PyToken(\"\")");
        snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict1_0)
    else {
        snprintf(nullLine, sizeof(nullLine), "%s = new PyToken(\"\")", currentMask.c_str());
        snprintf(validLine, sizeof(validLine), "%s = %s", currentMask.c_str(), name);
    }

    // Output the structural code block cleanly
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        sLog.Error(\"EncodeObject()\",\"%s.%s is null. hacking some weird shit here, so be aware.\");\n", mName, name );
    fprintf( mOutputFile, "        EvE::traceStack();\n" );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    } else {\n" );

    // Borrowing protection (Only run this if it's a valid pointer we are capturing)
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    }\n\n" );

    return true;
}
bool ClassEncodeGenerator::ProcessTokenInline( const TiXmlElement* field ) {
    const char* value = field->Attribute( "value" );
    if (value == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator:: Token element at line " << field->Row() << " has no value attribute, skipping.";
        return false;
    }

    // 1. FIXED: Enclose the value in explicit double quotes so it outputs as a proper C++ string literal
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyToken( \"%s\" )", value);

    // 2. Fetch the parent layout target string safely from your stack
    std::string currentTarget = top();
    pop();

    // 3. FIXED: Apply the target block switch check to handle both masks and flat variables
    char finalizedLine[256];
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), valueBuffer);
    } else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), valueBuffer);
    }

    // 4. Print the pristine, completed C++ statement
    fprintf(mOutputFile, "  %s;\n", finalizedLine);
    return true;
}


bool ClassEncodeGenerator::ProcessObject( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessObject field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    std::string currentMask = top();
    pop();

    char nullLine[256];
    char validLine[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "new PyObject(\"unknown\", PyStatic.NewNone())");
        snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict1_0)
    else {
        snprintf(nullLine, sizeof(nullLine), "%s = new PyObject(\"\")", currentMask.c_str());
        snprintf(validLine, sizeof(validLine), "%s = %s", currentMask.c_str(), name);
    }

    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        sLog.Error(\"EncodeObject()\",\"%s.%s is null. hacking some weird shit here, so be aware.\");\n", mName, name );
    fprintf( mOutputFile, "        EvE::traceStack();\n" );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    } else {\n" );
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name ); // Borrowing protection
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    }\n");

    return true;
}

bool ClassEncodeGenerator::ProcessObjectInline( const TiXmlElement* field ) {
    const uint32 num = mItemNumber++;
    char tname[32];
    snprintf( tname, sizeof( tname ), "obj%u_type", num );
    char aname[32];
    snprintf( aname, sizeof( aname ), "obj%u_args", num );

    fprintf( mOutputFile, "    PyString* %s = nullptr;\n"
    "    PyRep* %s = nullptr;\n\n", tname, aname );

    push( aname );
    push( tname );

    if (!ParseElementChildren( field, 2 ) )
        return false;

    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PyObject(%s, %s)", tname, aname);

    std::string currentMask = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety
    // CASE A: The tuple is nested inside another array layer layout (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentMask.c_str(), valueBuffer);
    }
    // CASE B: The tuple is being saved straight into a flat sub-stream variable (like ss_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentMask.c_str(), valueBuffer);
    }

    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessObjectEx( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessObjectEx field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* type = field->Attribute( "type" );
    if (type == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator::ProcessObjectEx field at line " << field->Row() << " is missing the type attribute, skipping.";
        return false;
    }

    std::string currentMask = top();
    pop();

    char finalizedLine[256];
    snprintf(finalizedLine, sizeof(finalizedLine), currentMask.c_str(), name);

    fprintf( mOutputFile, "    PyIncRef( %s );\n", name ); // Borrowing protection
    fprintf( mOutputFile, "    %s;\n", finalizedLine );

    return true;
}

bool ClassEncodeGenerator::ProcessTuple( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessTuple field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    std::string currentMask = top();
    pop();

    char nullLine[256];
    char validLine[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "PyStatic.mtTuple()");
        snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict1_0)
    else {
        snprintf(nullLine, sizeof(nullLine), "%s = new PyTuple(\"\")", currentMask.c_str());
        snprintf(validLine, sizeof(validLine), "%s = %s", currentMask.c_str(), name);
    }

    // Output the structural code block cleanly
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        sLog.Error(\"EncodeObject()\",\"%s.%s is null. hacking some weird shit here, so be aware.\");\n", mName, name );
    fprintf( mOutputFile, "        EvE::traceStack();\n" );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    } else {\n" );

    // Borrowing protection (Only run this if it's a valid pointer we are capturing)
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    }\n\n" );

    return true;
}

bool ClassEncodeGenerator::ProcessTupleInline( const TiXmlElement* field ) {
    const TiXmlNode* i = nullptr;
    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) ) {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT ) ++count;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "tuple%u", mItemNumber++ );

    // Output standard initialization
    fprintf( mOutputFile, "    PyTuple* %s = new PyTuple( %u );\n", iname, count );

    // Queue up the destination strings onto your native const char* stack
    char varname[64];
    while( count-- > 0 ) {
        // Enforce Rule 1: SetItem claims ownership of the slot item cleanly
        snprintf( varname, sizeof( varname ), "%s->SetItem( %u, %%s )", iname, count );
        push( varname ); // Compiles perfectly with your native void push(const char *v)
    }

    if (!ParseElementChildren( field ) )
        return false;

    // --- UPDATED EXCLUSIVE TARGET CHECK ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety
    // CASE A: The tuple is nested inside another array layer layout (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), iname);
    }
    // CASE B: The tuple is being saved straight into a flat sub-stream variable (like ss_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), iname);
    }

    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessList( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessList field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // 1. Fetch the parent layout mask string safely from your string stack
    std::string currentMask = top();
    pop();

    // 2. Pre-format the C++ output strings for BOTH the null and valid branches
    char nullLine[256];
    snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "PyStatic.mtList()");

    char validLine[256];
    snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(nullLine, sizeof(nullLine), currentMask.c_str(), "new PyToken(\"\")");
        snprintf(validLine, sizeof(validLine), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict1_0)
    else {
        snprintf(nullLine, sizeof(nullLine), "%s = new PyToken(\"\")", currentMask.c_str());
        snprintf(validLine, sizeof(validLine), "%s = %s", currentMask.c_str(), name);
    }

    // 3. Write out the conditional logic to the output file
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        %s;\n", nullLine );
    fprintf( mOutputFile, "    } else {\n" );

    // CRITICAL MEMORY COMPLIANCE: Because this represents a long-lived shared
    // dictionary, increment it before the parent layout sink claims ownership!
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validLine );
    fprintf( mOutputFile, "    }\n\n" );

    return true;
}

bool ClassEncodeGenerator::ProcessListInline( const TiXmlElement* field )
{
    const TiXmlNode* i = nullptr;
    uint32 count = 0;
    while( ( i = field->IterateChildren( i ) ) ) {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT ) ++count;
    }

    char iname[16];
    snprintf( iname, sizeof( iname ), "list%u", mItemNumber++ );

    // Output standard initialization
    fprintf( mOutputFile, "    PyList* %s = new PyList( %u );\n", iname, count );

    // Queue up the destination strings onto your native const char* stack
    char varname[64];
    while( count-- > 0 ) {
        // Enforce Rule 1: SetItem claims ownership of the slot item cleanly
        snprintf( varname, sizeof( varname ), "%s->SetItem( %u, %%s )", iname, count );
        push( varname ); // Compiles perfectly with your native void push(const char *v)
    }

    if (!ParseElementChildren( field ) )
        return false;

    // --- UPDATED EXCLUSIVE TARGET CHECK ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The tuple is nested inside another array layer layout (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), iname);
    }
    // CASE B: The tuple is being saved straight into a flat sub-stream variable (like ss_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), iname);
    }

    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessListInt( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator:: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    // 1. Output the initialization and population loops
    fprintf( mOutputFile, "    PyList* %s = new PyList();\n"
    "    for (auto &cur : %s)\n"
    "        %s->AddItemInt(cur);\n", rname, name, rname );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), rname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), rname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessListLong( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator:: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( mOutputFile, "    PyList* %s = new PyList();\n"
    "    for (auto &cur : %s)\n"
    "        %s->AddItemLong(cur);\n", rname, name, rname );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), rname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), rname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessListStr( const TiXmlElement* field ) {
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl << "ClassEncodeGenerator:: field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char rname[16];
    snprintf( rname, sizeof( rname ), "list%u", mItemNumber++ );

    fprintf( mOutputFile, "    PyList* %s = new PyList();\n"
    "    for (auto &cur : %s)\n"
    "        %s->AddItemString(cur.c_str());\n", rname, name, rname );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), rname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), rname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessDict( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDict field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    // Grab the layout mask string directly from your native stack
    std::string currentMask = top();
    pop();

    char noneBuffer[256];
    char validBuffer[256];

    // CASE A: The parent is an inline collection layout mask (contains "%s")
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(noneBuffer, sizeof(noneBuffer), currentMask.c_str(), "PyStatic.mtDict()");
        snprintf(validBuffer, sizeof(validBuffer), currentMask.c_str(), name);
    }
    // CASE B: The parent is a flat temporary destination variable (e.g. dict3_2)
    else {
        snprintf(noneBuffer, sizeof(noneBuffer), "%s = PyStatic.mtDict()", currentMask.c_str());
        snprintf(validBuffer, sizeof(validBuffer), "%s = %s", currentMask.c_str(), name);
    }

    // Generate code that safely increments the borrowed reference if it is valid
    fprintf( mOutputFile, "    if (%s == nullptr) {\n", name );
    fprintf( mOutputFile, "        %s;\n", noneBuffer );
    fprintf( mOutputFile, "    } else {\n" );

    // Safely borrow ownership before the sink claims it
    fprintf( mOutputFile, "        PyIncRef( %s );\n", name );
    fprintf( mOutputFile, "        %s;\n", validBuffer );
    fprintf( mOutputFile, "    }\n" );

    return true;
}

bool ClassEncodeGenerator::ProcessDictInline( const TiXmlElement* field )
{
    //first, create the dict container
    char iname[16];
    snprintf( iname, sizeof( iname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile,
             "      PyDict* %s = new PyDict();\n",
        iname
    );

    //now we process each element, putting it into the dict:
    const TiXmlNode* i = nullptr;

    uint32 count = 0;
    bool keyTypeInt = false, keyTypeLong = false;
    while ((i = field->IterateChildren(i))) {
        if (i->Type() == TiXmlNode::TINYXML_ELEMENT) {
            const TiXmlElement* ele = i->ToElement();

            //we only handle dictInlineEntry elements
            if (strcmp( ele->Value(), "dictInlineEntry" ) != 0 )            {
                std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictInline non-dictInlineEntry in <dictInline> at line " << field->Row() << ", ignoring.";
                continue;
            }
            const char* key = ele->Attribute( "key" );
            if (key == nullptr) {
                std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictInline <dictInlineEntry> at line " << field->Row() << " is missing the key attribute, skipping.";
                return false;
            }

            const char* keyType = ele->Attribute( "key_type" );
            if (keyType != nullptr) {
                keyTypeInt = ( strcmp( keyType, "int" ) == 0 );
                keyTypeLong = ( strcmp( keyType, "long" ) == 0 );
            }

            char vname[32];
            snprintf( vname, sizeof( vname ), "%s_%u", iname, count++ );

            fprintf( mOutputFile,
                "      PyRep* %s = nullptr;\n",
                vname
            );
            push( vname );

            //now process the data part, putting the value into `varname`
            if (!ParseElementChildren( ele, 1 ) )
                return false;

            //now store the result in the dict:
            //taking the keyType into account
            if (keyTypeInt ) {
                fprintf( mOutputFile,
                    "      %s->SetItem(new PyInt( %s ), %s);\n",
                    iname, key, vname
                );
            } else if (keyTypeLong ) {
                fprintf( mOutputFile,
                    "      %s->SetItem(new PyLong( %s ), %s);\n",
                    iname, key, vname
                );
            } else {
                fprintf( mOutputFile,
                    "      %s->SetItemString(\"%s\", %s);\n",
                    iname, key, vname
                );
            }
        }
    }

    // --- UPDATED EXCLUSIVE TARGET CHECK ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The tuple is nested inside another array layer layout (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), iname);
    }
    // CASE B: The tuple is being saved straight into a flat sub-stream variable (like ss_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), iname);
    }

    fprintf(mOutputFile, "    %s;\n", finalizedLine);
    return true;
}

bool ClassEncodeGenerator::ProcessDictRaw( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictRaw field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }
    const char* key = field->Attribute( "key" );
    if (key == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictRaw field at line " << field->Row() << " is missing the key attribute, skipping.";
        return false;
    }
    const char* pykey = field->Attribute( "pykey" );
    if (pykey == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictRaw field at line " << field->Row() << " is missing the pykey attribute, skipping.";
        return false;
    }
    const char* value = field->Attribute( "value" );
    if (value == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictRaw field at line " << field->Row() << " is missing the value attribute, skipping.";
        return false;
    }
    const char* pyvalue = field->Attribute( "pyvalue" );
    if (pyvalue == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictRaw field at line " << field->Row() << " is missing the pyvalue attribute, skipping.";
        return false;
    }

    char dname[16];
    snprintf( dname, sizeof( dname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile, "    PyDict* %s = new PyDict();\n", dname );
    fprintf( mOutputFile, "    for (auto &cur : %s) {\n", name );
    fprintf( mOutputFile, "        %s->SetItem( new Py%s( cur.first ), new Py%s( cur.second ) );\n", dname, pykey, pyvalue );
    fprintf( mOutputFile, "    }\n" );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), dname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), dname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessDictInt( const TiXmlElement* field ) {
    const char* name = field->Attribute("name");
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictInt field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char dname[16];
    snprintf( dname, sizeof( dname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile, "  PyDict* %s = new PyDict();\n", dname );
    fprintf( mOutputFile, "  for (auto &cur : %s) {\n", name );
    fprintf( mOutputFile, "    PyIncRef( cur.second );\n" );
    fprintf( mOutputFile, "    %s->SetItem( new PyInt( cur.first ), cur.second );\n", dname );
    fprintf( mOutputFile, "  }\n" );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), dname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), dname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}


bool ClassEncodeGenerator::ProcessDictStr( const TiXmlElement* field )
{
    const char* name = field->Attribute( "name" );
    if (name == nullptr) {
        std::cout << std::endl <<  "ClassEncodeGenerator::ProcessDictInt field at line " << field->Row() << " is missing the name attribute, skipping.";
        return false;
    }

    char dname[16];
    snprintf( dname, sizeof( dname ), "dict%u", mItemNumber++ );

    fprintf( mOutputFile, "    PyDict* %s = new PyDict();\n", dname );
    fprintf( mOutputFile, "    for (auto &cur : %s) {\n", name );

    // Protect the persistent internal collection from being stolen and deleted
    fprintf( mOutputFile, "        PyIncRef( cur.second );\n" );
    fprintf( mOutputFile, "        %s->SetItemString( cur.first.c_str(), cur.second );\n", dname );
    fprintf( mOutputFile, "    }\n" );

    // --- THE FIXED LOGIC CHECK (MATCHING INLINE) ---
    std::string currentTarget = top();
    pop();

    char finalizedLine[256]; // Uniform size bump to 256 for safety

    // CASE A: The target is a string format mask (contains "%s")
    if (currentTarget.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentTarget.c_str(), dname);
    }
    // CASE B: The target is a flat temporary tracking variable (like dict0_2)
    else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentTarget.c_str(), dname);
    }

    fprintf( mOutputFile, "  %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessSubStreamInline( const TiXmlElement* field ) {
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%u", mItemNumber++ );

    // Initialize temporary pointer
    fprintf( mOutputFile, "    PyRep* %s = nullptr;\n", varname );

    // Push the raw variable target to the stack for children to assign themselves directly to it!
    push( varname );

    // Process the children (this will now output: ss_2 = tuple3;)
    if (!ParseElementChildren( field, 1 ) ) return false;

    // Capture the parent format mask (e.g. tuple0->SetItem(3, %s))
    std::string currentMask = top();
    pop();

    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PySubStream( %s )", varname);

    char finalizedLine[512];
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentMask.c_str(), valueBuffer);
    } else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentMask.c_str(), valueBuffer);
    }

    fprintf( mOutputFile, "    %s;\n", finalizedLine );
    return true;
}

bool ClassEncodeGenerator::ProcessSubStructInline( const TiXmlElement* field ) {
    char varname[16];
    snprintf( varname, sizeof( varname ), "ss_%u", mItemNumber++ );

    // 1. Initialize the temporary pointer container
    fprintf( mOutputFile, "    PyRep* %s = nullptr;\n", varname );

    // 2. Queue up the temporary variable name for child processing
    push( varname );

    if (!ParseElementChildren( field, 1 ) ) return false;

    // 3. Construct the clean heap initialization fragment
    char valueBuffer[256];
    snprintf(valueBuffer, sizeof(valueBuffer), "new PySubStruct( %s )", varname);

    // 4. Safely extract the parent layout mask from your stack
    std::string currentMask = top();
    pop();

    char finalizedLine[512];
    if (currentMask.find("%s") != std::string::npos) {
        snprintf(finalizedLine, sizeof(finalizedLine), currentMask.c_str(), valueBuffer);
    } else {
        snprintf(finalizedLine, sizeof(finalizedLine), "%s = %s", currentMask.c_str(), valueBuffer);
    }

    // 6. Output the completed statement
    fprintf( mOutputFile, "    %s;\n", finalizedLine );

    return true;
}