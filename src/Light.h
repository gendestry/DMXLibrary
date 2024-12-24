#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <string.h>

const std::string colorGreen = "\x1B[32m";
const std::string colorYellow = "\x1B[33m";
const std::string colorBlue = "\x1B[34m";
const std::string colorReset = "\x1B[0m";
const std::string colorDim = "\x1B[2m";
const std::string colorItalic = "\x1B[3m";

namespace DMX
{

    enum LightPatchUnit
    {
        R,
        G,
        B,
        Dimmer,
        Strobo,

        R16 = 100,
        G16,
        B16,
        Dimmer16,
        Strobo16
    };

#define LIGHT_PATCH_UNIT_SIZE(x) (x >= R16 ? 2 : 1)
#define LIGHT_PATCH_UNIT_TO_STRING(x) (x == R ? "R" : x == G      ? "G"        \
                                                  : x == B        ? "B"        \
                                                  : x == Dimmer   ? "Dimmer"   \
                                                  : x == Strobo   ? "Strobo"   \
                                                  : x == R16      ? "R16"      \
                                                  : x == G16      ? "G16"      \
                                                  : x == B16      ? "B16"      \
                                                  : x == Dimmer16 ? "Dimmer16" \
                                                  : x == Strobo16 ? "Strobo16" \
                                                                  : "Unknown")

    struct EffectParams
    {
        int globalSize = -1, offsetGlobal = 0, localSize = -1, localId = -1, tick;
        void *other;
        EffectParams(int tick = 0, void *other = nullptr) : tick(tick), other(other) {}
    };

    typedef void (*EffectFn)(EffectParams &params, std::vector<std::vector<int>> &values);

    struct Light
    {
        struct Group
        {
            std::string name;
            std::vector<LightPatchUnit> units;
            std::vector<unsigned int> offsets;
            uint8_t **memory;
            int offset;
            unsigned int size = 0;

            // copy and move constructors
            Group() = default;
            Group(const Group &other) = default;
            Group(Group &&other) = default;

            Group(std::string name, std::vector<LightPatchUnit> units);

            std::vector<int> getValues() const;

            bool setValues(std::vector<int> values);

            inline unsigned int getGroupFootprint() const { return size; }

            inline unsigned int getNumUnits() const { return units.size(); }

            std::string describe() const;

            std::string toString() const;

            friend std::ostream &operator<<(std::ostream &os, const Group &group)
            {
                os << group.toString();
                return os;
            }
        };

        // the name of the light
        std::string m_Name;
        unsigned int m_ID;
        static std::unordered_map<std::string, unsigned int> m_currentIndex;

        bool m_inverted = false;
        bool m_standalone = false;

        // the byte values of the light
        uint8_t *m_bytes = nullptr;
        unsigned int m_size;

        // fragment position
        unsigned int start = 0;

        // the description of the light patches
        std::vector<LightPatchUnit> m_patchID;

        // map patch unit to indexes
        std::unordered_map<LightPatchUnit, std::vector<unsigned int>> m_patchMap;

        // map group name to group
        std::unordered_map<std::string, std::vector<Group>> m_groupMap;

        Light() = default;
        // create a light with name using a sequence of patch units
        Light(std::string name, std::vector<LightPatchUnit> patchUnits, bool inverted = false, bool standalone = false);

        // create a light with name using a group and amount of said groups
        Light(std::string name, Group group, unsigned int numGroups, bool inverted = false, bool standalone = false);

        ~Light();

        // copy constructor so groups get refreshed
        Light(const Light &other);

        static unsigned int incrementID(std::string name);

        // functions for getting raw bytes and size
        inline const unsigned int getSize() const { return m_size; }
        inline uint8_t *getBytes() const { return m_bytes; }

        // sets all bytes to 0
        inline void clearBytes() { memset(m_bytes, 0, m_size); }

        // sets all unit patches of the light to a value
        void set(LightPatchUnit unit, int value);

        // adds a custom indexing group
        bool addGroup(Group group);

        // used to refresh groups' memory pointers
        void refreshGroups();

        // sets group values at index
        bool setGroup(std::string name, unsigned int index, std::vector<int> values);

        // sets group values at indexes
        bool setGroup(std::string name, std::vector<unsigned int> indexes, std::vector<int> values);

        // sets all groups to values
        bool setAllGroups(std::string name, std::vector<std::vector<int>> values);

        // get reference to a group at index
        Group &getGroup(std::string name, unsigned int index);

        // get all groups at name
        std::vector<Group> &operator[](std::string name);

        // return values of all groups
        std::vector<std::vector<int>> getGroups(std::string name);

        // apply a lambda function to a group at index
        bool applyFunctionToGroup(std::string name, int index, EffectParams params, EffectFn func);

        // apply a lambda function to a group at indexes
        bool applyFunctionToGroup(std::string name, std::vector<unsigned int> indexes, EffectParams params, EffectFn func);

        // apply a lambda function to all groups
        bool applyFunctionToAllGroups(std::string name, EffectParams params, EffectFn func);

        // // PRINTING FUNCTIONS
        std::string toString() const noexcept;
        void printGroups() const noexcept;
        void printBytesPatched() const noexcept;
        void print() const noexcept;
    };

};
