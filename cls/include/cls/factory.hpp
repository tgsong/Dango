///////////////////////////////////////////////////////////////////////////////
// The MIT License(MIT)
//
// Copyright (c) 2014 Tiangang Song
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

#ifndef CLS_FACTORY_HPP
#define CLS_FACTORY_HPP

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "algorithm.hpp"
#include "cls_defs.h"

CLS_BEGIN
namespace detail {
template<typename T, template<typename...> class Ptr = std::unique_ptr>
struct ObjCreator {
    template<typename... Args>
    Ptr<T> operator()(Args&&... args)
    {
        return Ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename... Args>
    Ptr<T> create(Args&&... args)
    {
        return (*this)(std::forward<Args>(args)...);
    }
};

template<typename IDType>
class ObjFactoryBase {
public:
    virtual bool removeType(const IDType& ID) = 0;
};

template<typename BaseType, typename IDType, template<typename...> class Ptr, typename... CtorArgs>
class ObjFactory : public ObjFactoryBase<IDType> {
public:
    using BasePtr = Ptr<BaseType>;
    using Creator = std::function<BasePtr(CtorArgs...)>;

    bool addType(const IDType& ID, const Creator& creator)
    {
        if (creators_map.find(ID) == creators_map.end()) {
            creators_map.insert({ID, creator});
            return true;
        }

        return false;
    }

    template<typename Derived>
    bool addType(const IDType& ID)
    {
        return addType(ID, ObjCreator<Derived, Ptr>());
    }

    virtual bool removeType(const IDType& ID) override
    {
        if (creators_map.find(ID) != creators_map.end()) {
            creators_map.erase(ID);
            return true;
        }

        return false;
    }

    BasePtr create(const IDType& ID, CtorArgs&&... args) const
    {
        auto iter = creators_map.find(ID);
        if (iter != creators_map.end()) {
            return (iter->second)(std::forward<CtorArgs>(args)...);
        }

        return nullptr;
    }

    static ObjFactory& instance()
    {
        static ObjFactory obj_factory;
        return obj_factory;
    }

private:
    ObjFactory() = default;

    std::map<IDType, Creator> creators_map;
};
} // End namespace detail

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Base, typename IDType = std::string, template<typename...> class Ptr = std::unique_ptr>
class Factory {
    using ObjFactoryBasePtr = detail::ObjFactoryBase<IDType>*;

    template<typename... CtorArgs>
    using ObjFactory = detail::ObjFactory<Base, IDType, Ptr, CtorArgs...>;

    template<typename... CtorArgs>
    using Creator = typename ObjFactory<CtorArgs...>::Creator;

public:
    template<typename Derived, typename... CtorArgs>
    static bool addType(const IDType& id)
    {
        auto& obj_factory = ObjFactory<CtorArgs...>::instance();
        auto success =  obj_factory.template addType<Derived>(id);
        if (success) {
            instance().obj_factory_vec[id].emplace_back(&obj_factory);
        }

        return success;
    }

    template<typename Derived, typename... CtorArgs>
    static bool addType(const IDType& id, const Creator<CtorArgs...>& creator)
    {
        auto& obj_factory = ObjFactory<CtorArgs...>::instance();
        auto success =  obj_factory.template addType(id, creator);
        if (success) {
            instance().obj_factory_vec[id].emplace_back(&obj_factory);
        }

        return success;
    }

    template<typename... CtorArgs>
    static Ptr<Base> create(const IDType& id, CtorArgs&&... args)
    {
        auto& obj_factory = ObjFactory<CtorArgs...>::instance();
        return obj_factory.create(id, std::forward<CtorArgs>(args)...);
    }

    static bool removeType(const IDType& ID)
    {
        bool success = true;
        auto& type_factories = instance().obj_factory_vec[ID];
        for_each(type_factories, [&ID, &success](ObjFactoryBasePtr iter) {
            success &= iter->removeType(ID);
        });

        return success;
    }

    static Factory& instance()
    {
        static Factory factory;
        return factory;
    }

private:
    std::map<IDType, std::vector<ObjFactoryBasePtr>> obj_factory_vec;
};

CLS_END

///////////////////////////////////////////////////////////////////////////////////////////////////
// Use these macros in *.cpp file
#define REGISTER_TO_FACTORY(BaseType, DerivedType, ...) \
namespace { \
    const bool ADD_##DerivedType = cls::Factory<BaseType>::addType<DerivedType, ##__VA_ARGS__>(#DerivedType); \
}

#endif // CLS_FACTORY_HPP
