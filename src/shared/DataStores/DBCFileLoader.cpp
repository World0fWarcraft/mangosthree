/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2025 MaNGOS <https://www.getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DBCFileLoader.h"

DBCFileLoader::DBCFileLoader()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DBCFileLoader::Load(const char* filename, const char* fmt)
{
    uint32 header;
    delete[] data;

    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        return false;
    }

    if (fread(&header, 4, 1, f) != 1)                       // Number of records
    {
        fclose(f);
        return false;
    }

    EndianConvert(header);
    if (header != 0x43424457)                               //'WDBC'
    {
        fclose(f);
        return false;
    }

    if (fread(&recordCount, 4, 1, f) != 1)                  // Number of records
    {
        fclose(f);
        return false;
    }

    EndianConvert(recordCount);

    if (fread(&fieldCount, 4, 1, f) != 1)                   // Number of fields
    {
        fclose(f);
        return false;
    }

    EndianConvert(fieldCount);

    if (fread(&recordSize, 4, 1, f) != 1)                   // Size of a record
    {
        fclose(f);
        return false;
    }

    EndianConvert(recordSize);

    if (fread(&stringSize, 4, 1, f) != 1)                   // String size
    {
        fclose(f);
        return false;
    }

    EndianConvert(stringSize);

    fieldsOffset = new uint32[fieldCount];
    fieldsOffset[0] = 0;
    for (uint32 i = 1; i < fieldCount; ++i)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X')         // byte fields
        {
            fieldsOffset[i] += 1;
        }
        else                                                // 4 byte fields (int32/float/strings)
        {
            fieldsOffset[i] += 4;
        }
    }

    data = new unsigned char[recordSize * recordCount + stringSize];
    stringTable = data + recordSize * recordCount;

    if (fread(data, recordSize * recordCount + stringSize, 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

DBCFileLoader::~DBCFileLoader()
{
    delete[] data;
    delete[] fieldsOffset;
}

DBCFileLoader::Record DBCFileLoader::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id * recordSize);
}

uint32 DBCFileLoader::GetFormatRecordSize(const char* format, int32* index_pos)
{
    uint32 recordsize = 0;
    int32 i = -1;
    for (uint32 x = 0; format[x]; ++ x)
    {
        switch (format[x])
        {
            case DBC_FF_FLOAT:
                recordsize += sizeof(float);
                break;
            case DBC_FF_INT:
                recordsize += sizeof(uint32);
                break;
            case DBC_FF_STRING:
                recordsize += sizeof(char*);
                break;
            case DBC_FF_SORT:
                i = x;
                break;
            case DBC_FF_IND:
                i = x;
                recordsize += sizeof(uint32);
                break;
            case DBC_FF_BYTE:
                recordsize += sizeof(uint8);
                break;
            case DBC_FF_LOGIC:
                assert(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBCfmt.h or your DBC files.");
                break;
            case DBC_FF_NA:
            case DBC_FF_NA_BYTE:
                break;
            default:
                assert(false && "Unknown field format character in DBCfmt.h");
                break;
        }
    }

    if (index_pos)
    {
        *index_pos = i;
    }

    return recordsize;
}

uint32 DBCFileLoader::GetFormatStringsFields(const char * format)
{
    uint32 stringfields = 0;
    for(uint32 x=0; format[x];++x)
        if (format[x] == DBC_FF_STRING)
        {
            ++stringfields;
        }

    return stringfields;
}

char* DBCFileLoader::AutoProduceData(const char* format, uint32& records, char**& indexTable)
{
    /*
    format STRING, NA, FLOAT,NA,INT <=>
    struct{
    char* field0,
    float field1,
    int field2
    }entry;

    this func will generate  entry[rows] data;
    */

    typedef char* ptr;
    if (strlen(format) != fieldCount)
    {
        return NULL;
    }

    // get struct size and index pos
    int32 i;
    uint32 recordsize = GetFormatRecordSize(format, &i);

    if (i >= 0)
    {
        uint32 maxi = 0;
        // find max index
        for (uint32 y = 0; y < recordCount; ++y)
        {
            uint32 ind = getRecord(y).getUInt(i);
            if (ind > maxi)
            {
                maxi = ind;
            }
        }

        ++maxi;
        records = maxi;
        indexTable = new ptr[maxi];
        memset(indexTable, 0, maxi * sizeof(ptr));
    }
    else
    {
        records = recordCount;
        indexTable = new ptr[recordCount];
    }

    char* dataTable = new char[recordCount * recordsize];

    uint32 offset = 0;

    for (uint32 y = 0; y < recordCount; ++y)
    {
        if (i >= 0)
        {
            indexTable[getRecord(y).getUInt(i)] = &dataTable[offset];
        }
        else
        {
            indexTable[y] = &dataTable[offset];
        }

        for (uint32 x = 0; x < fieldCount; ++x)
        {
            switch (format[x])
            {
                case DBC_FF_FLOAT:
                    *((float*)(&dataTable[offset])) = getRecord(y).getFloat(x);
                    offset += sizeof(float);
                    break;
                case DBC_FF_IND:
                case DBC_FF_INT:
                    *((uint32*)(&dataTable[offset])) = getRecord(y).getUInt(x);
                    offset += sizeof(uint32);
                    break;
                case DBC_FF_BYTE:
                    *((uint8*)(&dataTable[offset])) = getRecord(y).getUInt8(x);
                    offset += sizeof(uint8);
                    break;
                case DBC_FF_STRING:
                    *((char**)(&dataTable[offset])) = NULL; // will replace non-empty or "" strings in AutoProduceStrings
                    offset += sizeof(char*);
                    break;
                case DBC_FF_LOGIC:
                    assert(false && "Attempted to load DBC files that do not have field types that match what is in the core. Check DBCfmt.h or your DBC files.");
                    break;
                case DBC_FF_NA:
                case DBC_FF_NA_BYTE:
                case DBC_FF_SORT:
                    break;
                default:
                    assert(false && "Unknown field format character in DBCfmt.h");
                    break;
            }
        }
    }

    return dataTable;
}

static char const* const nullStr = "";

char* DBCFileLoader::AutoProduceStringsArrayHolders(const char* format, char* dataTable)
{
    if (strlen(format)!=fieldCount)
    {
        return NULL;
    }

    // we store flat holders pool as single memory block
    size_t stringFields = GetFormatStringsFields(format);
    // each string field at load have array of string for each locale
    size_t stringHolderSize = sizeof(char*) * MAX_LOCALE;
    size_t stringHoldersRecordPoolSize = stringFields * stringHolderSize;
    size_t stringHoldersPoolSize = stringHoldersRecordPoolSize * recordCount;

    char* stringHoldersPool= new char[stringHoldersPoolSize];

    // dbc strings expected to have at least empty string
    for(size_t i = 0; i < stringHoldersPoolSize / sizeof(char*); ++i)
    {
        ((char const**)stringHoldersPool)[i] = nullStr;
    }

    uint32 offset=0;

    // assign string holders to string field slots
    for(uint32 y =0;y<recordCount;y++)
    {
        uint32 stringFieldNum = 0;

        for(uint32 x=0;x<fieldCount;x++)
            switch(format[x])
        {
            case DBC_FF_FLOAT:
            case DBC_FF_IND:
            case DBC_FF_INT:
                offset+=4;
                break;
            case DBC_FF_BYTE:
                offset+=1;
                break;
            case DBC_FF_STRING:
            {
                // init dbc string field slots by pointers to string holders
                char const*** slot = (char const***)(&dataTable[offset]);
                *slot = (char const**)(&stringHoldersPool[stringHoldersRecordPoolSize * y + stringHolderSize*stringFieldNum]);

                ++stringFieldNum;
                offset+=sizeof(char*);
                break;
            }
            case DBC_FF_NA:
            case DBC_FF_NA_BYTE:
            case DBC_FF_SORT:
                break;
            default:
                assert(false && "unknown format character");
        }
    }

    //send as char* for store in char* pool list for free at unload
    return stringHoldersPool;
}

char* DBCFileLoader::AutoProduceStrings(const char* format, char* dataTable, LocaleConstant loc)
{
    if (strlen(format) != fieldCount)
    {
        return NULL;
    }

    // each string field at load have array of string for each locale
    size_t stringHolderSize = sizeof(char*) * MAX_LOCALE;

    char* stringPool = new char[stringSize];
    memcpy(stringPool, stringTable, stringSize);

    uint32 offset = 0;

    for (uint32 y = 0; y < recordCount; ++y)
    {
        for (uint32 x = 0; x < fieldCount; ++x)
        {
            switch (format[x])
            {
                case DBC_FF_FLOAT:
                    offset += sizeof(float);
                    break;
                case DBC_FF_IND:
                case DBC_FF_INT:
                    offset += sizeof(uint32);
                    break;
                case DBC_FF_BYTE:
                    offset += sizeof(uint8);
                    break;
                case DBC_FF_STRING:
                {
                    // fill only not filled entries
                    char** slot = (char**)(&dataTable[offset]);
                    if (!*slot || !** slot)
                    {
                        const char* st = getRecord(y).getString(x);
                        *slot = stringPool + (st - (const char*)stringTable);
                    }
                    offset += sizeof(char*);
                    break;
                }
                case DBC_FF_LOGIC:
                    assert(false && "Attempted to load DBC files that does not have field types that match what is in the core. Check DBCfmt.h or your DBC files.");
                    break;
                case DBC_FF_NA:
                case DBC_FF_NA_BYTE:
                case DBC_FF_SORT:
                    break;
                default:
                    assert(false && "Unknown field format character in DBCfmt.h");
                    break;
            }
        }
    }

    return stringPool;
}
