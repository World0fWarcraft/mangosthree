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

#include "WorldModel.h"
#include "VMapDefinitions.h"
#include "MapTree.h"

using G3D::Vector3;
using G3D::Ray;

enum ModelFlags
{
    MOD_M2 = 1
};

template<> struct BoundsTrait<VMAP::GroupModel>
{
    static void getBounds(const VMAP::GroupModel& obj, G3D::AABox& out) { out = obj.GetBound(); }
};

namespace VMAP
{
    /**
     * @brief Checks if a ray intersects with a triangle.
     *
     * @param tri The triangle to check.
     * @param points Iterator to the vertices of the triangle.
     * @param ray The ray to check.
     * @param distance The distance to the intersection.
     * @return bool True if the ray intersects, false otherwise.
     */
    static bool IntersectTriangle(const MeshTriangle& tri, std::vector<Vector3>::const_iterator points, const G3D::Ray& ray, float& distance)
    {
        static const float EPS = 1e-5f;

        // See RTR2 ch. 13.7 for the algorithm.

        const Vector3 e1 = points[tri.idx1] - points[tri.idx0];
        const Vector3 e2 = points[tri.idx2] - points[tri.idx0];
        const Vector3 p(ray.direction().cross(e2));
        const float a = e1.dot(p);

        if (fabs(a) < EPS)
        {
            // Determinant is ill-conditioned; abort early
            return false;
        }

        const float f = 1.0f / a;
        const Vector3 s(ray.origin() - points[tri.idx0]);
        const float u = f * s.dot(p);

        if ((u < 0.0f) || (u > 1.0f))
        {
            // We hit the plane of the m_geometry, but outside the m_geometry
            return false;
        }

        const Vector3 q(s.cross(e1));
        const float v = f * ray.direction().dot(q);

        if ((v < 0.0f) || ((u + v) > 1.0f))
        {
            // We hit the plane of the triangle, but outside the triangle
            return false;
        }

        const float t = f * e2.dot(q);

        if ((t > 0.0f) && (t < distance))
        {
            // This is a new hit, closer than the previous one
            distance = t;

            /* baryCoord[0] = 1.0 - u - v;
            baryCoord[1] = u;
            baryCoord[2] = v; */

            return true;
        }
        // This hit is after the previous hit, so ignore it
        return false;
    }

    /**
     * @brief Functor to calculate the bounding box of a triangle.
     */
    class TriBoundFunc
    {
    public:
        /**
         * @brief Constructor for TriBoundFunc.
         *
         * @param vert Vector of vertices.
         */
        TriBoundFunc(std::vector<Vector3>& vert) : vertices(vert.begin()) {}
        /**
         * @brief Calculates the bounding box of a triangle.
         *
         * @param tri The triangle to calculate the bounding box for.
         * @param out The calculated bounding box.
         */
        void operator()(const MeshTriangle& tri, G3D::AABox& out) const
        {
            G3D::Vector3 lo = vertices[tri.idx0];
            G3D::Vector3 hi = lo;

            lo = (lo.min(vertices[tri.idx1])).min(vertices[tri.idx2]);
            hi = (hi.max(vertices[tri.idx1])).max(vertices[tri.idx2]);

            out = G3D::AABox(lo, hi);
        }
    protected:
        const std::vector<Vector3>::const_iterator vertices;
    };

    // ===================== WmoLiquid ==================================

    /**
     * @brief Constructor for WmoLiquid.
     *
     * @param width Width of the liquid area.
     * @param height Height of the liquid area.
     * @param corner The lower corner of the liquid area.
     * @param type The type of the liquid.
     */
    WmoLiquid::WmoLiquid(uint32 width, uint32 height, const Vector3& corner, uint32 type) :
        iTilesX(width), iTilesY(height), iCorner(corner), iType(type)
    {
        iHeight = new float[(width + 1) * (height + 1)];
        iFlags = new uint8[width * height];
    }

    /**
     * @brief Copy constructor for WmoLiquid.
     *
     * @param other The WmoLiquid to copy from.
     */
    WmoLiquid::WmoLiquid(const WmoLiquid& other) : iHeight(NULL), iFlags(NULL)
    {
        *this = other;                                      // use assignment operator defined below
    }

    /**
     * @brief Destructor for WmoLiquid.
     */
    WmoLiquid::~WmoLiquid()
    {
        delete[] iHeight;
        delete[] iFlags;
    }

    /**
     * @brief Assignment operator for WmoLiquid.
     *
     * @param other The WmoLiquid to assign from.
     * @return WmoLiquid& Reference to the assigned WmoLiquid.
     */
    WmoLiquid& WmoLiquid::operator=(const WmoLiquid& other)
    {
        if (this == &other)
        {
            return *this;
        }

        iTilesX = other.iTilesX;
        iTilesY = other.iTilesY;
        iCorner = other.iCorner;
        iType = other.iType;
        delete[] iHeight;
        delete[] iFlags;

        if (other.iHeight)
        {
            iHeight = new float[(iTilesX + 1) * (iTilesY + 1)];
            memcpy(iHeight, other.iHeight, (iTilesX + 1) * (iTilesY + 1) * sizeof(float));
        }
        else
        {
            iHeight = NULL;
        }
        if (other.iFlags)
        {
            iFlags = new uint8[iTilesX * iTilesY];
            memcpy(iFlags, other.iFlags, iTilesX * iTilesY * sizeof(uint8));
        }
        else
        {
            iFlags = NULL;
        }

        return *this;
    }

    /**
     * @brief Gets the liquid height at a specific position.
     *
     * @param pos The position to check.
     * @param liqHeight The liquid height at the position.
     * @return bool True if the liquid height was retrieved, false otherwise.
     */
    bool WmoLiquid::GetLiquidHeight(const Vector3& pos, float& liqHeight) const
    {
        float tx_f = (pos.x - iCorner.x) / LIQUID_TILE_SIZE;
        uint32 tx = uint32(tx_f);
        if (tx_f < 0.0f || tx >= iTilesX)
        {
            return false;
        }
        float ty_f = (pos.y - iCorner.y) / LIQUID_TILE_SIZE;
        uint32 ty = uint32(ty_f);
        if (ty_f < 0.0f || ty >= iTilesY)
        {
            return false;
        }

        // check if tile shall be used for liquid level
        // checking for 0x08 *might* be enough, but disabled tiles always are 0x?F:
        if ((iFlags[tx + ty * iTilesX] & 0x0F) == 0x0F)
        {
            return false;
        }

        // (dx, dy) coordinates inside tile, in [0,1]^2
        float dx = tx_f - (float)tx;
        float dy = ty_f - (float)ty;

        /* Tesselate tile to two triangles (not sure if client does it exactly like this)

            ^ dy
            |
          1 x---------x (1,1)
            | (b)   / |
            |     /   |
            |   /     |
            | /   (a) |
            x---------x---> dx
          0           1
        */
        const uint32 rowOffset = iTilesX + 1;
        if (dx > dy) // case (a)
        {
            float sx = iHeight[tx + 1 + ty * rowOffset] - iHeight[tx + ty * rowOffset];
            float sy = iHeight[tx + 1 + (ty + 1) * rowOffset] - iHeight[tx + 1 + ty * rowOffset];
            liqHeight = iHeight[tx + ty * rowOffset] + dx * sx + dy * sy;
        }
        else // case (b)
        {
            float sx = iHeight[tx + 1 + (ty + 1) * rowOffset] - iHeight[tx + (ty + 1) * rowOffset];
            float sy = iHeight[tx + (ty + 1) * rowOffset] - iHeight[tx + ty * rowOffset];
            liqHeight = iHeight[tx + ty * rowOffset] + dx * sx + dy * sy;
        }
        return true;
    }

    /**
     * @brief Gets the file size of the liquid data.
     *
     * @return uint32 The file size of the liquid data.
     */
    uint32 WmoLiquid::GetFileSize() const
    {
        return 2 * sizeof(uint32) +
            sizeof(Vector3) +
            (iTilesX + 1) * (iTilesY + 1) * sizeof(float) +
            iTilesX * iTilesY;
    }

    /**
     * @brief Writes the liquid data to a file.
     *
     * @param wf The file to write to.
     * @return bool True if the write was successful, false otherwise.
     */
    bool WmoLiquid::WriteToFile(FILE* wf) const
    {
        bool result = true;
        if (result && fwrite(&iTilesX, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&iTilesY, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&iCorner, sizeof(Vector3), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&iType, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        uint32 size = (iTilesX + 1) * (iTilesY + 1);
        if (result && fwrite(iHeight, sizeof(float), size, wf) != size)
        {
            result = false;
        }
        size = iTilesX * iTilesY;
        if (result && fwrite(iFlags, sizeof(uint8), size, wf) != size)
        {
            result = false;
        }
        return result;
    }

    /**
     * @brief Reads the liquid data from a file.
     *
     * @param rf The file to read from.
     * @param out The WmoLiquid to read into.
     * @return bool True if the read was successful, false otherwise.
     */
    bool WmoLiquid::ReadFromFile(FILE* rf, WmoLiquid*& out)
    {
        bool result = true;
        WmoLiquid* liquid = new WmoLiquid();
        if (result && fread(&liquid->iTilesX, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&liquid->iTilesY, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&liquid->iCorner, sizeof(Vector3), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&liquid->iType, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        uint32 size = (liquid->iTilesX + 1) * (liquid->iTilesY + 1);
        liquid->iHeight = new float[size];
        if (result && fread(liquid->iHeight, sizeof(float), size, rf) != size)
        {
            result = false;
        }
        size = liquid->iTilesX * liquid->iTilesY;
        liquid->iFlags = new uint8[size];
        if (result && fread(liquid->iFlags, sizeof(uint8), size, rf) != size)
        {
            result = false;
        }
        if (!result)
        {
            delete liquid;
        }
        else
        {
            out = liquid;
        }
        return result;
    }

    /**
     * @brief Copy constructor for GroupModel.
     *
     * @param other The GroupModel to copy from.
     */
    GroupModel::GroupModel(const GroupModel& other) :
        iBound(other.iBound), iMogpFlags(other.iMogpFlags), iGroupWMOID(other.iGroupWMOID),
        vertices(other.vertices), triangles(other.triangles), meshTree(other.meshTree), iLiquid(0)
    {
        if (other.iLiquid)
        {
            iLiquid = new WmoLiquid(*other.iLiquid);
        }
    }

    /**
     * @brief Passes mesh data to the object and creates the BIH.
     *
     * @param vert Vector of vertices.
     * @param tri Vector of triangles.
     */
    void GroupModel::SetMeshData(std::vector<Vector3>& vert, std::vector<MeshTriangle>& tri)
    {
        vertices.swap(vert);
        triangles.swap(tri);
        TriBoundFunc bFunc(vertices);
        meshTree.build(triangles, bFunc);
    }

    /**
     * @brief Writes the group model data to a file.
     *
     * @param wf The file to write to.
     * @return bool True if the write was successful, false otherwise.
     */
    bool GroupModel::WriteToFile(FILE* wf)
    {
        bool result = true;
        uint32 chunkSize, count;

        if (result && fwrite(&iBound, sizeof(G3D::AABox), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&iMogpFlags, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&iGroupWMOID, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }

        // write vertices
        if (result && fwrite("VERT", 1, 4, wf) != 4)
        {
            result = false;
        }
        count = vertices.size();
        chunkSize = sizeof(uint32) + sizeof(Vector3) * count;
        if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&count, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (!count) // models without (collision) geometry end here, unsure if they are useful
        {
            return result;
        }
        if (result && fwrite(&vertices[0], sizeof(Vector3), count, wf) != count)
        {
            result = false;
        }

        // write triangle mesh
        if (result && fwrite("TRIM", 1, 4, wf) != 4)
        {
            result = false;
        }
        count = triangles.size();
        chunkSize = sizeof(uint32) + sizeof(MeshTriangle) * count;
        if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&count, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (count)
            if (result && fwrite(&triangles[0], sizeof(MeshTriangle), count, wf) != count)
            {
                result = false;
            }

        // write mesh BIH
        if (result && fwrite("MBIH", 1, 4, wf) != 4)
        {
            result = false;
        }
        if (result)
        {
            result = meshTree.WriteToFile(wf);
        }

        // write liquid data
        if (result && fwrite("LIQU", 1, 4, wf) != 4)
        {
            result = false;
        }
        chunkSize = iLiquid ? iLiquid->GetFileSize() : 0;
        if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (chunkSize)
            if (result)
            {
                result = iLiquid->WriteToFile(wf);
            }

        return result;
    }

    /**
     * @brief Reads the group model data from a file.
     *
     * @param rf The file to read from.
     * @return bool True if the read was successful, false otherwise.
     */
    bool GroupModel::ReadFromFile(FILE* rf)
    {
        char chunk[8];
        bool result = true;
        uint32 chunkSize = 0;
        uint32 count = 0;
        triangles.clear();
        vertices.clear();
        delete iLiquid;
        iLiquid = 0;

        // Read bounding box
        if (result && fread(&iBound, sizeof(G3D::AABox), 1, rf) != 1)
        {
            result = false;
        }
        // Read model flags
        if (result && fread(&iMogpFlags, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        // Read group WMO ID
        if (result && fread(&iGroupWMOID, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }

        // Read vertices
        if (result && !readChunk(rf, chunk, "VERT", 4))
        {
            result = false;
        }
        if (result && fread(&chunkSize, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&count, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (!count) // models without (collision) geometry end here, unsure if they are useful
        {
            return result;
        }
        if (result)
        {
            vertices.resize(count);
        }
        if (result && fread(&vertices[0], sizeof(Vector3), count, rf) != count)
        {
            result = false;
        }

        // Read triangle mesh
        if (result && !readChunk(rf, chunk, "TRIM", 4))
        {
            result = false;
        }
        if (result && fread(&chunkSize, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&count, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (count)
        {
            if (result)
            {
                triangles.resize(count);
            }
            if (result && fread(&triangles[0], sizeof(MeshTriangle), count, rf) != count)
            {
                result = false;
            }
        }

        // Read mesh BIH
        if (result && !readChunk(rf, chunk, "MBIH", 4))
        {
            result = false;
        }
        if (result)
        {
            result = meshTree.ReadFromFile(rf);
        }

        // Read liquid data
        if (result && !readChunk(rf, chunk, "LIQU", 4))
        {
            result = false;
        }
        if (result && fread(&chunkSize, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && chunkSize > 0)
        {
            result = WmoLiquid::ReadFromFile(rf, iLiquid);
        }
        return result;
    }

    /**
     * @brief Callback structure for ray intersection with group model.
     */
    struct GModelRayCallback
    {
        GModelRayCallback(const std::vector<MeshTriangle>& tris, const std::vector<Vector3>& vert) :
        vertices(vert.begin()), triangles(tris.begin()), hit(false) {}
        bool operator()(const G3D::Ray& ray, uint32 entry, float& distance, bool /*pStopAtFirstHit*/)
        {
            bool result = IntersectTriangle(triangles[entry], vertices, ray, distance);
            if (result)
            {
                hit = true;
            }
            return hit;
        }
        std::vector<Vector3>::const_iterator vertices;
        std::vector<MeshTriangle>::const_iterator triangles;
        bool hit;
    };

    /**
     * @brief Checks if a ray intersects with the group model.
     *
     * @param ray The ray to check.
     * @param distance The distance to the intersection.
     * @param stopAtFirstHit Whether to stop at the first hit.
     * @return bool True if the ray intersects, false otherwise.
     */
    bool GroupModel::IntersectRay(const G3D::Ray& ray, float& distance, bool stopAtFirstHit) const
    {
        if (triangles.empty())
        {
            return false;
        }
        GModelRayCallback callback(triangles, vertices);
        meshTree.IntersectRay(ray, callback, distance, stopAtFirstHit);
        return callback.hit;
    }

    /**
     * @brief Checks if a position is inside the object.
     *
     * @param pos The position to check.
     * @param down The direction vector.
     * @param z_dist The distance to the intersection.
     * @return bool True if the position is inside the object, false otherwise.
     */
    bool GroupModel::IsInsideObject(const Vector3& pos, const Vector3& down, float& z_dist) const
    {
        if (triangles.empty() || !iBound.contains(pos))
        {
            return false;
        }
        GModelRayCallback callback(triangles, vertices);
        Vector3 rPos = pos - 0.1f * down;
        float dist = G3D::inf();
        G3D::Ray ray(rPos, down);
        bool hit = IntersectRay(ray, dist, false);
        if (hit)
        {
            z_dist = dist - 0.1f;
        }
        return hit;
    }

    /**
     * @brief Gets the liquid level at a specific position.
     *
     * @param pos The position to check.
     * @param liqHeight The liquid height at the position.
     * @return bool True if the liquid level was retrieved, false otherwise.
     */
    bool GroupModel::GetLiquidLevel(const Vector3& pos, float& liqHeight) const
    {
        if (iLiquid)
        {
            return iLiquid->GetLiquidHeight(pos, liqHeight);
        }
        return false;
    }

    /**
     * @brief Gets the type of the liquid.
     *
     * @return uint32 The type of the liquid.
     */
    uint32 GroupModel::GetLiquidType() const
    {
        if (iLiquid)
        {
            return iLiquid->GetType();
        }
        return 0;
    }

    /**
     * @brief Passes group models to WorldModel and creates the BIH.
     *
     * @param models Vector of group models.
     */
    void WorldModel::SetGroupModels(std::vector<GroupModel>& models)
    {
        groupModels.swap(models);
        groupTree.build(groupModels, BoundsTrait<GroupModel>::getBounds, 1);
    }

    /**
     * @brief Callback structure for ray intersection with world model.
     */
    struct WModelRayCallBack
    {
        WModelRayCallBack(const std::vector<GroupModel>& mod) : models(mod.begin()), hit(false) {}
        bool operator()(const G3D::Ray& ray, uint32 entry, float& distance, bool pStopAtFirstHit)
        {
            bool result = models[entry].IntersectRay(ray, distance, pStopAtFirstHit);
            if (result)
            {
                hit = true;
            }
            return hit;
        }
        std::vector<GroupModel>::const_iterator models;
        bool hit;
    };

    /**
     * @brief Checks if a ray intersects with the world model.
     *
     * @param ray The ray to check.
     * @param distance The distance to the intersection.
     * @param stopAtFirstHit Whether to stop at the first hit.
     * @return bool True if the ray intersects, false otherwise.
     */
    bool WorldModel::IntersectRay(const G3D::Ray& ray, float& distance, bool stopAtFirstHit) const
    {
        // M2 models are not taken into account for LoS calculation
        if (Flags & MOD_M2)
        {
            return false;
        }
        // small M2 workaround, maybe better make separate class with virtual intersection funcs
        // in any case, there's no need to use a bound tree if we only have one submodel
        if (groupModels.size() == 1)
        {
            return groupModels[0].IntersectRay(ray, distance, stopAtFirstHit);
        }

        WModelRayCallBack isc(groupModels);
        groupTree.IntersectRay(ray, isc, distance, stopAtFirstHit);
        return isc.hit;
    }

    /**
     * @brief Callback structure for area information retrieval.
     */
    class WModelAreaCallback
    {
    public:
        WModelAreaCallback(const std::vector<GroupModel>& vals, const Vector3& down) :
        prims(vals.begin()), hit(vals.end()), minVol(G3D::inf()), zDist(G3D::inf()), zVec(down) {}
        std::vector<GroupModel>::const_iterator prims;
        std::vector<GroupModel>::const_iterator hit;
        float minVol;
        float zDist;
        Vector3 zVec;
        void operator()(const Vector3& point, uint32 entry)
        {
            float group_Z;
            // float pVol = prims[entry].GetBound().volume();
            // if (pVol < minVol)
            //{
            /* if (prims[entry].iBound.contains(point)) */
            if (prims[entry].IsInsideObject(point, zVec, group_Z))
            {
                // minVol = pVol;
                // hit = prims + entry;
                if (group_Z < zDist)
                {
                    zDist = group_Z;
                    hit = prims + entry;
                }
#ifdef VMAP_DEBUG
                const GroupModel& gm = prims[entry];
                printf("%10u %8X %7.3f,%7.3f,%7.3f | %7.3f,%7.3f,%7.3f | z=%f, p_z=%f\n", gm.GetWmoID(), gm.GetMogpFlags(),
                       gm.GetBound().low().x, gm.GetBound().low().y, gm.GetBound().low().z,
                       gm.GetBound().high().x, gm.GetBound().high().y, gm.GetBound().high().z, group_Z, point.z);
#endif
            }
        }
    };

    /**
     * @brief Gets area information at a specific position.
     *
     * @param p The position to check.
     * @param down The direction vector.
     * @param dist The distance to the intersection.
     * @param info The area information.
     * @return bool True if area information was retrieved, false otherwise.
     */
    bool WorldModel::GetAreaInfo(const G3D::Vector3& p, const G3D::Vector3& down, float& dist, AreaInfo& info) const
    {
        if (groupModels.empty())
        {
            return false;
        }
        WModelAreaCallback callback(groupModels, down);
        groupTree.intersectPoint(p, callback);
        if (callback.hit != groupModels.end())
        {
            info.rootId = RootWMOID;
            info.groupId = callback.hit->GetWmoID();
            info.flags = callback.hit->GetMogpFlags();
            info.result = true;
            dist = callback.zDist;
            return true;
        }
        return false;
    }

    /**
     * @brief Gets location information at a specific position.
     *
     * @param p The position to check.
     * @param down The direction vector.
     * @param dist The distance to the intersection.
     * @param info The location information.
     * @return bool True if location information was retrieved, false otherwise.
     */
    bool WorldModel::GetLocationInfo(const G3D::Vector3& p, const G3D::Vector3& down, float& dist, LocationInfo& info) const
    {
        if (groupModels.empty())
        {
            return false;
        }
        WModelAreaCallback callback(groupModels, down);
        groupTree.intersectPoint(p, callback);
        if (callback.hit != groupModels.end())
        {
            info.hitModel = &(*callback.hit);
            dist = callback.zDist;
            return true;
        }
        return false;
    }

    /**
     * @brief Writes the world model data to a file.
     *
     * @param filename The file to write to.
     * @return bool True if the write was successful, false otherwise.
     */
    bool WorldModel::writeFile(const std::string& filename)
    {
        FILE* wf = fopen(filename.c_str(), "wb");
        if (!wf)
        {
            return false;
        }

        uint32 chunkSize, count;
        bool result = fwrite(VMAP_MAGIC, 1, 8, wf) == 8;
        if (result && fwrite("WMOD", 1, 4, wf) != 4)
        {
            result = false;
        }
        chunkSize = sizeof(uint32) + sizeof(uint32);
        if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }
        if (result && fwrite(&RootWMOID, sizeof(uint32), 1, wf) != 1)
        {
            result = false;
        }

        // write group models
        count = groupModels.size();
        if (count)
        {
            if (result && fwrite("GMOD", 1, 4, wf) != 4)
            {
                result = false;
            }
            // chunkSize = sizeof(uint32)+ sizeof(GroupModel)*count;
            // if (result && fwrite(&chunkSize, sizeof(uint32), 1, wf) != 1) result = false;
            if (result && fwrite(&count, sizeof(uint32), 1, wf) != 1)
            {
                result = false;
            }
            for (uint32 i = 0; i < groupModels.size() && result; ++i)
            {
                result = groupModels[i].WriteToFile(wf);
            }

            // write group BIH
            if (result && fwrite("GBIH", 1, 4, wf) != 4)
            {
                result = false;
            }
            if (result)
            {
                result = groupTree.WriteToFile(wf);
            }
        }

        fclose(wf);
        return result;
    }

    bool WorldModel::readFile(const std::string& filename)
    {
        FILE* rf = fopen(filename.c_str(), "rb");
        if (!rf)
        {
            return false;
        }

        bool result = true;
        uint32 chunkSize = 0;
        uint32 count = 0;
        char chunk[8];                          // Ignore the added magic header
        if (!readChunk(rf, chunk, VMAP_MAGIC, 8))
        {
            result = false;
        }

        if (result && !readChunk(rf, chunk, "WMOD", 4))
        {
            result = false;
        }
        if (result && fread(&chunkSize, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }
        if (result && fread(&RootWMOID, sizeof(uint32), 1, rf) != 1)
        {
            result = false;
        }

        // read group models
        if (result && readChunk(rf, chunk, "GMOD", 4))
        {
            // if (fread(&chunkSize, sizeof(uint32), 1, rf) != 1) result = false;

            if (result && fread(&count, sizeof(uint32), 1, rf) != 1)
            {
                result = false;
            }
            if (result)
            {
                groupModels.resize(count);
            }
            // if (result && fread(&groupModels[0], sizeof(GroupModel), count, rf) != count) result = false;
            for (uint32 i = 0; i < count && result; ++i)
            {
                result = groupModels[i].ReadFromFile(rf);
            }

            // read group BIH
            if (result && !readChunk(rf, chunk, "GBIH", 4))
            {
                result = false;
            }
            if (result)
            {
                result = groupTree.ReadFromFile(rf);
            }
        }

        fclose(rf);
        return result;
    }
}
