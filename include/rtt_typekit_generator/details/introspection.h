/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_DETAILS_INTROSPECTION_H
#define RTT_TYPEKIT_GENERATOR_DETAILS_INTROSPECTION_H

#include "../introspection.h"

#include <algorithm>
#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/type_traits/has_left_shift.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>

namespace boost {
namespace serialization {

// default implementation - do nothing
template<class Archive, class T>
inline bool introspect(Archive& ar, const T&, const BOOST_PFTO unsigned int file_version) {
    return false;
}

template <class Archive, class T>
bool introspect_adl(Archive& ar, const T& t, const BOOST_PFTO unsigned int file_version) {
    // note usage of function overloading to delay final resolution
    // until the point of instantiation.  This works around the two-phase
    // lookup "feature" which inhibits redefintion of a default function
    // template implementation. Due to Robert Ramey
    //
    // Note that this trick generates problems for compiles which don't support
    // PFTO, suppress it here.  As far as we know, there are no compilers
    // which fail to support PFTO while supporting two-phase lookup.
    #if ! defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
        const version_type v(file_version);
        return introspect(ar, t, v);
    #else
        return introspect(ar, t, file_version);
    #endif
}

}  // namespace serialization
}  // namespace boost


namespace rtt_typekit_generator {
namespace introspection {
namespace details {

template <typename T>
class TypeIntrospectionBase : public TypeIntrospectionInterface {
public:
    typedef T type;
    typedef boost::has_left_shift<std::ostream, T> has_ostream;

    TypeIntrospectionBase(const std::string& name)
        : name_(name), type_id_(&typeid(T)) {
        if (!instance_) {
            instance_.reset(this);
        }
    }
    virtual ~TypeIntrospectionBase() {}

    virtual const std::string& getTypeName() const {
        return name_;
    }

    virtual TypeId getTypeId() const {
        return type_id_;
    }

    virtual const char *getTypeIdName() const {
        return type_id_->name();
    }

    virtual ValueInterface getValue(void *instance) const {
        return ValueInterface(instance);
    }

    virtual const ValueInterface getValue(const void *instance) const {
        return ValueInterface(instance);
    }

    virtual void *create() const {
        return new T();
    }

    virtual void *clone(const void *instance) {
        return new T(*static_cast<const T *>(instance));
    }

protected:
    std::string name_;
    TypeId type_id_;
    static const T prototype_;
    static boost::shared_ptr<TypeIntrospectionBase<T> > instance_;
};
template <typename T> const T TypeIntrospectionBase<T>::prototype_ = T();
template <typename T> boost::shared_ptr< TypeIntrospectionBase<T> > TypeIntrospectionBase<T>::instance_;

/**
 * Implements the Boost serialization Loading Archive Concept
 */
template <class IntrospectorT>
class IntrospectionArchive {
public:
    typedef typename IntrospectorT::type type;

    IntrospectionArchive(IntrospectorT *introspector, const type &prototype)
        : introspector_(introspector)
        , prototype_(prototype)
    {}

    typedef boost::mpl::bool_<true> is_loading;
    typedef boost::mpl::bool_<false> is_saving;
    unsigned int get_library_version() { return 0; }
    void reset_object_address(const void *new_address, const void *old_address) {}
    void delete_created_pointers() {}
    template <class T> void register_type(T* = 0) { return 0; }

    template <class T> IntrospectionArchive &operator>>(T &t) {
        introspect(t);
        return *this;
    }

    template <class T> IntrospectionArchive &operator&(T &t) {
        introspect(t);
        return *this;
    }

    template <class T>
    Members::iterator introspect(T &t) {
        return introspector_->introspect(0, t, prototype_);
    }

    template <class T>
    Members::iterator introspect(T (type::*t)) {
        return introspector_->introspect(0, t, prototype_);
    }

    template <class T>
    Members::iterator introspect(const char *name, T &t) {
        return introspector_->introspect(name, t, prototype_);
    }

    template <class T>
    Members::iterator introspect(const char *name, T (type::*t)) {
        return introspector_->introspect(name, t, prototype_);
    }

    template <typename T>
    Members::iterator introspect(const boost::serialization::nvp<T> &t) {
        return introspector_->introspect(t.name(), t.value(), prototype_);
    }

private:
    IntrospectorT *introspector_;
    const type &prototype_;
};

template <typename T, bool is_class = boost::is_class<T>::value>
class TypeIntrospection;

template <typename T>
class TypeIntrospection<T, /* is_class = */ false>
    : public TypeIntrospectionBase<T> {
public:
    typedef TypeIntrospection<T, false> this_type;
    typedef IntrospectionArchive<this_type> archive_type;

    TypeIntrospection(const std::string& name)
        : TypeIntrospectionBase<T>(name) {
        introspect();
    }

    virtual bool isStruct() const { return false; }

    template <typename MemberT>
    Members::iterator introspect(const char *, MemberT &t, const T &instance) {
        assert(!nested_);
        nested_ = TypeIntrospectionBase<MemberT>::Instance();
        return Members::iterator();
    }

    template <typename MemberT>
    Members::iterator introspect(const char *, const MemberT *t, const T &instance) {
        return Members::iterator();
    }

protected:
    void introspect() {
        archive_type ar(this, this->prototype_);
        boost::serialization::serialize_adl(ar, const_cast<T&>(this->prototype_), 0);
    }

    TypeIntrospectionInterface::shared_ptr nested_;
};

class IsMemberPredicate {
public:
    IsMemberPredicate(const std::string& name) : name_(name) {}
    bool operator()(const Member &other) const { return other.name == name_; }
private:
    std::string name_;
};

class AccessorInterface {
public:
    virtual ~AccessorInterface() {}
    virtual ValueInterface get(void *instance) const = 0;
    virtual const ValueInterface get(const void *instance) const = 0;
};

template <typename T>
class Accessor : public AccessorInterface {
public:
    virtual ValueInterface get(void *instance) const { return ValueInterface(static_cast<T *>(instance)); }
    virtual const ValueInterface get(const void *instance) const { return ValueInterface(static_cast<const T *>(instance)); }
};

template <typename T, typename MemberT>
class MemberAccessor : public AccessorInterface {
public:
    typedef MemberT (T::*pointer_type);
    MemberAccessor(pointer_type p)
        : p_(p) {}

    virtual ValueInterface get(void *instance) const { return ValueInterface(&(static_cast<T *>(instance)->*p_)); }
    virtual const ValueInterface get(const void *instance) const { return ValueInterface(&(static_cast<const T *>(instance)->*p_)); }

private:
    pointer_type p_;
};

template <typename T>
class TypeIntrospection<T, /* is_class = */ true>
    : public TypeIntrospectionBase<T>
{
public:
    typedef TypeIntrospection<T, true> this_type;
    typedef IntrospectionArchive<this_type> archive_type;

    TypeIntrospection(const std::string &name)
        : TypeIntrospectionBase<T>(name) {
        introspect();
    }

    virtual bool isStruct() const { return true; }

    virtual const Members *getMembers() const {
        return &members_;
    }

    virtual const Member *getMember(const std::string &name) const {
        Members::const_iterator it = std::find_if(members_.begin(), members_.end(), details::IsMemberPredicate(name));
        if (it == members_.end()) return 0;
        return &(*it);
    }

    virtual const TypeIntrospectionInterface *getMemberType(const std::string& name) const {
        const Member *member = getMember(name);
        if (!member) return 0;
        return member->type.get();
    }

    virtual std::vector<std::string> getMemberNames() const {
        MemberNames names;
        names.reserve(members_.size());
        for(Members::const_iterator it = members_.begin(); it != members_.end(); ++it) {
            names.push_back(it->name);
        }
        return names;
    }

    virtual ValueInterface getMemberValue(void *instance, const std::string& name) const {
        const Member *member = getMember(name);
        if (!member || !member->accessor) return ValueInterface();
        return member->accessor->get(instance);
    }

    virtual const ValueInterface getMemberValue(const void *instance, const std::string& name) const {
        const Member *member = getMember(name);
        if (!member || !member->accessor) return ValueInterface();
        return member->accessor->get(instance);
    }

    // Arbitrary reference
    template <typename MemberT>
    Members::iterator introspect(const char *name, MemberT &t, const T &instance) {
        Members::iterator member = members_.insert(members_.end(), Member());
        if (name) member->name = name;
        // member->accessor.reset(new Accessor<T>());
        return member;
    }

    // Pointers cannot be introspected and will be ignored.
    template <typename MemberT>
    Members::iterator introspect(const char *name, const MemberT *t, const T &) {
        return Members::iterator();
    }

    // Pointer to member variable
    template <typename MemberT>
    Members::iterator introspect(const char *name, MemberT (T::*t), const T &) {
        Members::iterator member = members_.insert(members_.end(), Member());
        if (name) member->name = name;
        member->accessor.reset(new MemberAccessor<T, MemberT>(t));
        return member;
    }

protected:
    std::vector<Member> members_;

private:
    void introspect() {
        archive_type ar(this, this->prototype_);
        if (!boost::serialization::introspect_adl(ar, this->prototype_, 0)) {
            boost::serialization::serialize_adl(ar, const_cast<T&>(this->prototype_), 0);
        }
    }
};

}  // namespace details
}  // namespace introspection
}  // namespace rtt_typekit_generator

#endif  // RTT_TYPEKIT_GENERATOR_DETAILS_INTROSPECTION_H
