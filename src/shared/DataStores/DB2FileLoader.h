/*
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

#ifndef DB2_FILE_LOADER_H
#define DB2_FILE_LOADER_H

#include "Platform/Define.h"
#include "Utilities/ByteConverter.h"
#include "Common/Common.h"
#include <cassert>

/**
 * @brief
 *
 */
//enum FieldFormat
//{
//    DBC_FF_NA = 'x',                                        // ignore/ default, 4 byte size, in Source String means field is ignored, in Dest String means field is filled with default value
//    DBC_FF_NA_BYTE = 'X',                                   // ignore/ default, 1 byte size, see above
//    DBC_FF_NA_FLOAT = 'F',                                  // ignore/ default,  float size, see above
//    DBC_FF_NA_POINTER = 'p',                                // fill default value into dest, pointer size, Use this only with static data (otherwise mem-leak)
//    DBC_FF_STRING = 's',                                    // char*
//    DBC_FF_FLOAT = 'f',                                     // float
//    DBC_FF_INT = 'i',                                       // uint32
//    DBC_FF_BYTE = 'b',                                      // uint8
//    DBC_FF_SORT = 'd',                                      // sorted by this field, field is not included
//    DBC_FF_IND = 'n',                                       // the same,but parsed to data
//    DBC_FF_LOGIC = 'l'                                      // Logical (boolean)
//};

/**
 * @brief
 *
 */
class DB2FileLoader
{
    public:
        DB2FileLoader();
        ~DB2FileLoader();

    bool Load(const char *filename, const char *fmt);

    class Record
    {
    public:
        float getFloat(size_t field) const
        {
            assert(field < file.fieldCount);
            float val = *reinterpret_cast<float*>(offset+file.GetOffset(field));
            EndianConvert(val);
            return val;
        }
        uint32 getUInt(size_t field) const
        {
            assert(field < file.fieldCount);
            uint32 val = *reinterpret_cast<uint32*>(offset+file.GetOffset(field));
            EndianConvert(val);
            return val;
        }
        uint8 getUInt8(size_t field) const
        {
            assert(field < file.fieldCount);
            return *reinterpret_cast<uint8*>(offset+file.GetOffset(field));
        }

        const char *getString(size_t field) const
        {
            assert(field < file.fieldCount);
            size_t stringOffset = getUInt(field);
            assert(stringOffset < file.stringSize);
            return reinterpret_cast<char*>(file.stringTable + stringOffset);
        }

    private:
        Record(DB2FileLoader &file_, unsigned char *offset_): offset(offset_), file(file_) {}
        unsigned char *offset;
        DB2FileLoader &file;

        friend class DB2FileLoader;

    };

    // Get record by id
    Record getRecord(size_t id);
    /// Get begin iterator over records

    uint32 GetNumRows() const { return recordCount;}
    uint32 GetCols() const { return fieldCount; }
    uint32 GetOffset(size_t id) const { return (fieldsOffset != NULL && id < fieldCount) ? fieldsOffset[id] : 0; }
    bool IsLoaded() const { return (data != NULL); }
    char* AutoProduceData(const char* fmt, uint32& count, char**& indexTable);
    char* AutoProduceStringsArrayHolders(const char* fmt, char* dataTable);
    char* AutoProduceStrings(const char* fmt, char* dataTable, LocaleConstant loc);
    static uint32 GetFormatRecordSize(const char * format, int32 * index_pos = NULL);
    static uint32 GetFormatStringsFields(const char * format);
private:

    uint32 recordSize;
    uint32 recordCount;
    uint32 fieldCount;
    uint32 stringSize;
    uint32 *fieldsOffset;
    unsigned char *data;
    unsigned char *stringTable;

    // WDB2 / WCH2 fields
    uint32 tableHash;    // WDB2
    uint32 build;        // WDB2

    int unk1;            // WDB2 (Unix time in WCH2)
    int unk2;            // WDB2
    int unk3;            // WDB2 (index table)
    int locale;          // WDB2
    int unk5;            // WDB2
};

#endif
