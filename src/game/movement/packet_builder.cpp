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

#include "packet_builder.h"
#include "MoveSpline.h"
#include "Util.h"
#include "WorldPacket.h"
#include "../Object/Creature.h"

namespace Movement
{
    /**
     * @brief Overloads the << operator to write a Vector3 to a ByteBuffer.
     * @param b The ByteBuffer to write to.
     * @param v The Vector3 to write.
     */
    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    /**
     * @brief Overloads the >> operator to read a Vector3 from a ByteBuffer.
     * @param b The ByteBuffer to read from.
     * @param v The Vector3 to read.
     */
    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    /**
     * @brief Writes the common part of a monster move packet.
     * @param move_spline The MoveSpline object containing movement data.
     * @param data The WorldPacket to write the data to.
     */
    void PacketBuilder::WriteCommonMonsterMovePart(const MoveSpline& move_spline, WorldPacket& data)
    {
        MoveSplineFlag splineflags = move_spline.splineflags;

        data << uint8(0);
        data << move_spline.spline.getPoint(move_spline.spline.first());
        data << move_spline.GetId();

        switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            default:
                data << uint8(MonsterMoveNormal);
                break;
            case MoveSplineFlag::Final_Target:
                data << uint8(MonsterMoveFacingTarget);
                data << move_spline.facing.target;
                break;
            case MoveSplineFlag::Final_Angle:
                data << uint8(MonsterMoveFacingAngle);
                data << NormalizeOrientation(move_spline.facing.angle);
                break;
            case MoveSplineFlag::Final_Point:
                data << uint8(MonsterMoveFacingSpot);
                data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
                break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        splineflags.enter_cycle = move_spline.isCyclic();
        // add fake Runmode flag - client has strange issues without that flag
        data << uint32(splineflags & ~MoveSplineFlag::Mask_No_Monster_Move);

        if (splineflags.animation)
        {
            data << splineflags.getAnimationId();
            data << move_spline.effect_start_time;
        }

        data << move_spline.Duration();

        if (splineflags.parabolic)
        {
            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;
        }
    }

    /**
     * @brief Writes a linear path to a ByteBuffer.
     * @param spline The spline containing the path points.
     * @param data The ByteBuffer to write the data to.
     */
    void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.getPointCount() - 3;
        const Vector3* real_path = &spline.getPoint(1);

        data << last_idx;
        data << real_path[last_idx];   // destination
        if (last_idx > 1)
        {
            Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
            Vector3 offset;
            // first and last points already appended
            for (uint32 i = 1; i < last_idx; ++i)
            {
                offset = middle - real_path[i];
                data.appendPackXYZ(offset.x, offset.y, offset.z);
            }
        }
    }

    /**
     * @brief Writes a Catmull-Rom path to a ByteBuffer.
     * @param spline The spline containing the path points.
     * @param data The ByteBuffer to write the data to.
     */
    void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << count;
        data.append<Vector3>(&spline.getPoint(2), count);
    }

    /**
     * @brief Writes a cyclic Catmull-Rom path to a ByteBuffer.
     * @param spline The spline containing the path points.
     * @param data The ByteBuffer to write the data to.
     */
    void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << uint32(count + 1);
        data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.getPoint(1), count);
    }

    /**
     * @brief Writes a monster move packet.
     * @param move_spline The MoveSpline object containing movement data.
     * @param data The WorldPacket to write the data to.
     */
    void PacketBuilder::WriteMonsterMove(const MoveSpline& move_spline, WorldPacket& data)
    {
        WriteCommonMonsterMovePart(move_spline, data);

        const Spline<int32>& spline = move_spline.spline;
        MoveSplineFlag splineflags = move_spline.splineflags;
        if (splineflags & MoveSplineFlag::UncompressedPath)
        {
            if (splineflags.cyclic)
            {
                WriteCatmullRomCyclicPath(spline, data);
            }
            else
            {
                WriteCatmullRomPath(spline, data);
            }
        }
        else
        {
            WriteLinearPath(spline, data);
        }
    }

    void PacketBuilder::WriteCreateBits(const MoveSpline& move_spline, ByteBuffer& data)
    {
        if (!data.WriteBit(!move_spline.Finalized()))
        {
            return;
        }

        MoveSplineFlag splineFlags = move_spline.splineflags;
        uint32 nodes = move_spline.getPath().size();
        bool hasSplineStartTime = move_spline.splineflags & (MoveSplineFlag::Trajectory | MoveSplineFlag::Animation);
        bool hasSplineVerticalAcceleration = (move_spline.splineflags & MoveSplineFlag::Trajectory) && move_spline.effect_start_time < move_spline.Duration();

        data.WriteBits(uint8(move_spline.spline.mode()), 2);
        data.WriteBit(hasSplineStartTime);
        data.WriteBits(nodes, 22);

        switch (move_spline.splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            case MoveSplineFlag::Final_Target:
            {
                data.WriteBits(2, 2);

                data.WriteGuidMask<4, 3, 7, 2, 6, 1, 0, 5>(ObjectGuid(move_spline.facing.target));
                break;
            }
            case MoveSplineFlag::Final_Angle:
                data.WriteBits(0, 2);
                break;
            case MoveSplineFlag::Final_Point:
                data.WriteBits(1, 2);
                break;
            default:
                data.WriteBits(3, 2);
                break;
        }

        data.WriteBit(hasSplineVerticalAcceleration);
        data.WriteBits(move_spline.splineflags.raw(), 25);
    }

    void PacketBuilder::WriteCreateBytes(const MoveSpline& move_spline, ByteBuffer& data)
    {
        if (!move_spline.Finalized())
        {
            MoveSplineFlag splineFlags = move_spline.splineflags;
            uint32 nodes = move_spline.getPath().size();
            bool hasSplineStartTime = move_spline.splineflags & (MoveSplineFlag::Trajectory | MoveSplineFlag::Animation);
            bool hasSplineVerticalAcceleration = (move_spline.splineflags & MoveSplineFlag::Trajectory) && move_spline.effect_start_time < move_spline.Duration();

            if (hasSplineVerticalAcceleration)
            {
                data << float(move_spline.vertical_acceleration);   // added in 3.1
            }

            data << int32(move_spline.timePassed());

            if (move_spline.splineflags & MoveSplineFlag::Final_Angle)
            {
                data << float(NormalizeOrientation(move_spline.facing.angle));
            }
            else if (move_spline.splineflags & MoveSplineFlag::Final_Target)
            {
                data.WriteGuidBytes<5, 3, 7, 1, 6, 4, 2, 0>(ObjectGuid(move_spline.facing.target));
            }

            for (uint32 i = 0; i < nodes; ++i)
            {
                data << float(move_spline.getPath()[i].z);
                data << float(move_spline.getPath()[i].x);
                data << float(move_spline.getPath()[i].y);
            }

            if (move_spline.splineflags & MoveSplineFlag::Final_Point)
            {
                data << float(move_spline.facing.f.x) << float(move_spline.facing.f.z) << float(move_spline.facing.f.y);
            }

            data << float(1.f);
            data << int32(move_spline.Duration());
            if (hasSplineStartTime)
            {
                data << int32(move_spline.effect_start_time);   // added in 3.1
            }

            data << float(1.f);
        }

        if (!move_spline.isCyclic())
        {
            Vector3 dest = move_spline.FinalDestination();
            data << float(dest.z);
            data << float(dest.x);
            data << float(dest.y);
        }
        else
        {
            data << Vector3::zero();
        }

        data << uint32(move_spline.GetId());
    }
}
