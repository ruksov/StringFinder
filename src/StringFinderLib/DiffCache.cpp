#include "stdafx.h"
#include "DiffCache.h"
#include "Exceptions.h"
#include "Log.h"

namespace sf::lib::diff_cache
{
    DiffCachePtr Create(const Data& data, IteratorList& iteratorList)
    {
        THROW_IF(data.size() > std::numeric_limits<uint32_t>::max()
            , "Failed to create DiffCache. Input data is too big, max size is "
            << std::numeric_limits<uint32_t>::max() << " bytes.");

        iteratorList.clear();
        DiffCachePtr cache = std::make_unique<DiffCache>();

        // index in for each data loop
        uint32_t dataIndex = 0;

        // index in current str with last offset value of known byte equal to parent byte
        uint32_t currIndex = 0;

        // index in parent str with last offset value of known byte equal to current byte 
        uint32_t parentIndex = 0;

        const uint32_t dataSize = static_cast<uint32_t>(data.size());
        
        for (auto currByte : data)
        {
            auto it = cache->emplace(Key(0, currByte), Value(dataIndex));

            while (!it.second)
            {
                auto& parentKey = it.first->first;
                auto& parentValue = it.first->second;
                assert(data.at(static_cast<size_t>(parentValue.Offset) + parentKey.Info.Offset) == parentKey.Info.Byte);

                currIndex = dataIndex + parentKey.Info.Offset;
                parentIndex = parentValue.Offset + parentKey.Info.Offset;
                assert(currIndex > parentIndex);

                for (
                    ; currIndex < dataSize && data.at(parentIndex) == data.at(currIndex)
                    ; ++parentIndex, ++currIndex);

                // current str: [dataIndex] ... (currIndex)
                // parrent str: [val.Offset] ... (parrentIndex)
                // before start comaring these string ranges must be equal 
                assert(data.at(parentValue.Offset) == data.at(dataIndex));
                assert(currIndex == dataSize || data.at(parentIndex) != data.at(currIndex));
                assert(data.substr(parentValue.Offset, parentIndex - parentValue.Offset)
                    == data.substr(dataIndex, currIndex - dataIndex));

                if (dataSize == currIndex)
                {
                    // save current str like full equal sub string in parrent node
                    if (!parentValue.SubStrings)
                    {
                        parentValue.SubStrings = std::make_unique<OffsetList>();
                    }
                    parentValue.SubStrings->push_back(dataIndex);
                    assert(std::is_sorted(parentValue.SubStrings->begin(), parentValue.SubStrings->end()));
                    break;
                }

                // try to create node in parrent diff tree
                // with key - value of first different current str byte from parrent str 
                if (!parentValue.DiffStrings)
                {
                    parentValue.DiffStrings = std::make_unique<DiffCache>();
                }

                it = parentValue.DiffStrings->emplace(Key(currIndex - dataIndex, data.at(currIndex))
                    , Value(dataIndex));
            }

            // save to iterator list parrent string
            iteratorList.push_back(it.first);

            ++dataIndex;
        }

        assert(cache->size() == iteratorList.size());
        return cache;
    }
}