#include "stdafx.h"
#include "DiffCache.h"
#include "Exceptions.h"
#include "Log.h"

namespace sf::lib::diff_cache
{
    DiffCache Create(const Data& data, IteratorList& iteratorList)
    {
        DiffCache cache;
        DiffCacheRef cacheRef = cache;
        const size_t dataSize = static_cast<uint32_t>(data.size());
        iteratorList.clear();

        for (size_t dataOffset = 0; dataOffset < dataSize; ++ dataOffset)
        {
            auto it = cache.emplace(Key(0, data.at(dataOffset)), Value(dataOffset));

            while (!it.second)
            {
                auto& parentKey = it.first->first;
                auto& parentValue = it.first->second;
                assert(dataOffset > parentValue.Offset);
                assert(data.at(parentValue.Offset + parentKey.DiffOffset) == parentKey.DiffByte);

                size_t childCmpOffset = dataOffset + parentKey.DiffOffset;
                size_t parentCmpOffset = parentValue.Offset + parentKey.DiffOffset;

                for (
                    ; childCmpOffset < dataSize && data.at(childCmpOffset) == data.at(parentCmpOffset)
                    ; ++childCmpOffset, ++parentCmpOffset);

                // these data ranges must be equal:
                //
                // child data:            [dataOffset] ... (childCmpOffset)
                // parrent data: [parrentValue.Offset] ... (parentCmpOffset)
                assert(data.substr(parentValue.Offset, parentCmpOffset - parentValue.Offset)
                    == data.substr(dataOffset, childCmpOffset - dataOffset));

                if (dataSize == childCmpOffset)
                {
                    // this range is sub range of parent range
                    // so we don't need to save it
                    break;
                }

                // try to create node in parrent diff tree
                // with key - value of first different current str byte from parrent str 
                if (!parentValue.DiffRanges)
                {
                    parentValue.DiffRanges = std::make_unique<DiffCache>();
                }

                it = parentValue.DiffRanges->emplace(Key(childCmpOffset - dataOffset, data.at(childCmpOffset))
                    , Value(dataOffset));
            }

            // save to iterator list parrent string
            iteratorList.push_back(it.first);
        }

        assert(data.size() == iteratorList.size());
        return cache;
    }
}