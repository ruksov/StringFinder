#include "stdafx.h"
#include "DiffCache.h"
#include "Exceptions.h"

namespace sf::lib::diff_cache
{
    DiffCachePtr Create(const std::string& data)
    {
        THROW_IF(data.size() > std::numeric_limits<uint32_t>::max()
            , "Failed to create DiffCache. Input data is too big, max size is "
            << std::numeric_limits<uint32_t>::max() << " bytes.");

        DiffCachePtr cache = std::make_unique<DiffCache>();

        // index in for each data loop
        uint32_t dataIndex = 0;

        // index in current str with last offset value of known byte equal to parent byte
        uint32_t currIndex = 0;

        // index in parent str with last offset value of known byte equal to current byte 
        uint32_t parentIndex = 0;

        // iterator in diff tree, which contain node bound with prev byte in data
        Iterator prevIt = cache->end();

        const uint32_t dataSize = static_cast<uint32_t>(data.size());
        
        for (auto currChar : data)
        {
            auto it = cache->emplace(Key(currChar, 0), Value(dataIndex));

            while (!it.second)
            {
                auto& parentKey = it.first->first;
                auto& parentValue = it.first->second;
                assert(data.at(parentValue.Offset + parentKey.DiffOffset) == parentKey.DiffChar);

                currIndex = dataIndex + parentKey.DiffOffset;
                parentIndex = parentValue.Offset + parentKey.DiffOffset;

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
                    break;
                }

                // try to create node in parrent diff tree
                // with key - value of first different current str byte from parrent str 
                if (!parentValue.DiffStrings)
                {
                    parentValue.DiffStrings = std::make_unique<DiffCache>();
                }

                it = parentValue.DiffStrings->emplace(Key(data.at(currIndex), currIndex - dataIndex)
                    , Value(dataIndex));
            }

            if (dataIndex != 0 && prevIt->second.Offset + 1 == it.first->second.Offset)
            {
                prevIt->second.NextDataByte = std::make_unique<Iterator>(it.first);
            }

            prevIt = it.first;

            ++dataIndex;
        }

        return cache;
    }
}