/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_INTROSPECTION_H
#define RTT_TYPEKIT_GENERATOR_INTROSPECTION_H

#include <string>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace rtt_typekit_generator {
namespace introspection {

typedef const std::type_info *TypeId;

class ValueInterface
{
public:
    typedef void *pointer_type;
    typedef const void *const_pointer_type;

    ValueInterface(const_pointer_type p = 0) : p_(const_cast<pointer_type>(p)) {}
    ValueInterface(pointer_type p) : p_(p) {}

    template <typename T> T& as() {
        return *static_cast<T *>(get());
    }

    template <typename T> const T& as() const {
        return *static_cast<const T *>(get());
    }

    pointer_type get() { return p_; }
    const_pointer_type get() const { return p_; }

    operator const void *() const { return p_; }

private:
    pointer_type p_;
};

namespace details {
class AccessorInterface;
}  // namespace details


class TypeIntrospectionInterface;

struct Member {
    std::string name;
    boost::shared_ptr<TypeIntrospectionInterface> type;
    boost::shared_ptr<details::AccessorInterface> accessor;
};
typedef std::vector<Member> Members;
typedef std::vector<std::string> MemberNames;

class TypeIntrospectionInterface
{
public:
    typedef boost::shared_ptr<TypeIntrospectionInterface> shared_ptr;

    virtual ~TypeIntrospectionInterface() {}

    virtual const std::string& getTypeName() const = 0;
    virtual TypeId getTypeId() const = 0;
    virtual const char *getTypeIdName() const = 0;

    virtual ValueInterface getValue(void *instance) const = 0;
    virtual const ValueInterface getValue(const void *instance) const = 0;

    virtual bool isStruct() const = 0;

    virtual const Members *getMembers() const { return 0; }
    virtual const Member *getMember(const std::string &name) const { return 0; }
    virtual const TypeIntrospectionInterface *getMemberType(const std::string& name) const { return 0; }
    virtual MemberNames getMemberNames() const { return MemberNames(); }

    virtual ValueInterface getMemberValue(void *instance, const std::string& name) const { return ValueInterface(); }
    virtual const ValueInterface getMemberValue(const void *instance, const std::string& name) const { return ValueInterface(); }

    template <typename MemberT>
    MemberT &getMemberValueAs(void *instance, const std::string& name) const {
        assert(&typeid(MemberT) == getMemberType(name)->getTypeId());
        return getMemberValue(instance, name).as<MemberT>();
    }
    template <typename MemberT>
    const MemberT &getMemberValueAs(const void *instance, const std::string& name) const {
        assert(&typeid(MemberT) == getMemberType(name)->getTypeId());
        return getMemberValue(instance, name).as<MemberT>();
    }
};

}  // namespace introspection
}  // namespace rtt_typekit_generator

#endif  // RTT_TYPEKIT_GENERATOR_INTROSPECTION_H
