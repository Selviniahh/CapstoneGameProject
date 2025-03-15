#pragma once
#include <string>
#include <iostream>
#include <boost/type_index.hpp>
#include <boost/mp11.hpp>
#include <boost/describe.hpp>

namespace ETG
{
    // //Remove everything up to and including the last ::
    static void RemoveNamespaceFromName(std::string& name)
    {
        const size_t LastColon = name.find_last_of("::");
        if (LastColon != std::string::npos)
        {
            name = name.substr(LastColon + 1);
        }
    }

    template <typename E, std::enable_if_t<std::is_enum_v<E>, int>  = 0>
    static const char* EnumToString(E e)
    {
        auto r = "unnamed";
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&](auto D)
        {
            if (e == D.value)
            {
                r = D.name;
            }
        });

        return r;
    }

    template <typename E>
    static std::string TypeNameToString()
    {
        auto actualTypeName = boost::typeindex::type_id<E>().pretty_name(); //: E
        RemoveNamespaceFromName(actualTypeName);
        return actualTypeName;
    }

    template <typename E>
    std::vector<E> ConstructEnumVector()
    {
        std::vector<E> enumValues;
        boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&enumValues](auto enumType)
        {
            enumValues.push_back(enumType.value);
        });

        return {enumValues};
    }
}
