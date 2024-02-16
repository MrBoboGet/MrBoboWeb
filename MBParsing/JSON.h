#pragma once
#include "MBParsing.h"
namespace MBParsing
{
    inline JSONObject ToJSON(double Value)
    {
        return JSONObject(Value);
    }
    inline JSONObject ToJSON(float Value)
    {
        return JSONObject(Value);
    }
    inline JSONObject ToJSON(intmax_t Value)
    {
        return JSONObject(Value);
    }
    inline JSONObject ToJSON(int Value)
    {
        return JSONObject(Value);
    }
    inline JSONObject ToJSON(bool Value)
    {
        return JSONObject(Value);
    }
    inline JSONObject ToJSON(std::string Value)
    {
        return JSONObject(std::move(Value));
    }
    template<typename T>
    JSONObject ToJSON(std::vector<T> const& VectorToConvert)
    {
        std::vector<MBParsing::JSONObject> Result;
        Result.reserve(VectorToConvert.size());
        for(auto const& Value : VectorToConvert)
        {
            Result.emplace_back(ToJSON(Value));   
        }
        return JSONObject(std::move(Result));
    }
    template<typename T>
    JSONObject ToJSON(std::unordered_map<std::string,T> const& MapToConvert)
    {
        std::map<std::string,MBParsing::JSONObject> Result;
        for(auto const& Pair : MapToConvert)
        {
            Result[Pair.first] = ToJSON(Pair.second);
        }
        return JSONObject(std::move(Result));
    }



    template<typename T>
    void FromJSON(std::vector<T>& Result,JSONObject const& ObjectToParse)
    {
        if(ObjectToParse.GetType() != JSONObjectType::Array)
        {
            throw std::runtime_error("Object not of array type");
        }
        auto const& Data = ObjectToParse.GetArrayData();
        size_t Index = 0;
        Result.resize(Data.size());
        for(auto const& Value : Data)
        {
            FromJSON(Result[Index],Value);
            Index++;
        }
    }
    template<typename T>
    void FromJSON(std::unordered_map<std::string,T>& Result,JSONObject const& ObjectToParse)
    {
        if(ObjectToParse.GetType() != JSONObjectType::Aggregate)
        {
            throw std::runtime_error("Object not of aggregate type");
        }
        auto const& Data = ObjectToParse.GetMapData();
        Result.reserve(Data.size());
        for(auto const& Pair : Data)
        {
            FromJSON(Result[Pair.first],Pair.second);
        }
    } 
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    void FromJSON(T& Result,JSONObject const& ObjectToParse)
    {
        if(ObjectToParse.GetType() != JSONObjectType::Integer)
        {
            throw std::runtime_error("Object not of aggregate type");
        }
        Result = ObjectToParse.GetIntegerData();
    }
    inline void FromJSON(std::string& Result,JSONObject const& ObjectToParse)
    {
        if(ObjectToParse.GetType() != JSONObjectType::String)
        {
            throw std::runtime_error("Object not of aggregate type");
        }
        Result = ObjectToParse.GetStringData();
    }

}
