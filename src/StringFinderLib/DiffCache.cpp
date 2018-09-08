#include "stdafx.h"
#include "DiffCache.h"

namespace sf::lib::diff_cache
{
    DiffCache Create(const std::string& data)
    {
        DiffCache cache;

        // index in for each data loop
        size_t dataIndex = 0;

        // index in current str with last offset value of known byte equal to parent byte
        size_t currIndex = 0;

        // index in parent str with last offset value of known byte equal to current byte 
        size_t parentIndex = 0;

        // iterator in diff tree, which contain node bound with prev byte in data
        Iterator prevIt = cache.end();

        const size_t dataSize = data.size();
        
        for (auto currChar : data)
        {
            auto it = cache.emplace(Key(currChar, 0), Value(dataIndex));

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
                    parentValue.SubStrings.push_back(dataIndex);
                    break;
                }

                // try to create node in parrent diff tree
                // with key - value of first different current str byte from parrent str 
                it = parentValue.DiffStrings.emplace(Key(data.at(currIndex), currIndex - dataIndex)
                    , Value(dataIndex));
            }

            if (dataIndex != 0 && prevIt->second.Offset + 1 == it.first->second.Offset)
            {
                prevIt->second.NextDataByte = it.first;
            }

            prevIt = it.first;

            ++dataIndex;
        }

        return cache;
    }
}