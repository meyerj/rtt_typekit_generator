/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_GENERATOR_H
#define RTT_TYPEKIT_GENERATOR_GENERATOR_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <map>
#include <stack>
#include <string>
#include <vector>

namespace rtt_typekit_generator {

/**
 * A PartGeneratorBase instance generates a fragment of a file specific to a
 * certain type.
 */
class PartGeneratorBase
{
public:
    typedef boost::shared_ptr<PartGeneratorBase> shared_ptr;
    typedef std::vector<std::string> Namespaces;

    struct StaticInitializer {
        StaticInitializer(const char *transport,
                          const shared_ptr &generator);
    };

    struct ContextData {
        std::ostream *stream;
        std::size_t indent;

        ContextData() : stream(0), indent(0) {}
        ContextData(const ContextData *other)
            : stream(other->stream)
            , indent(other->indent)
        {}
        virtual ~ContextData() {}

        virtual std::ostream &operator>>(std::ostream &os)
        {
            os << "PartGeneratorBase::ContextData { ";
            os << ".stream = " << stream << ", ";
            os << ".indent = " << indent << ", ";
            os << "}";
            return os;
        }
    };

    struct ContextBase;
    typedef std::stack<ContextBase *> ContextStack;
    struct ContextBase {
        ContextBase(ContextStack *stack) : stack_(stack) {}
        virtual ~ContextBase() {}
    protected:
        ContextStack *stack_;
    };

    template <typename ContextDataT = ContextData>
    struct Context_ : public ContextBase, public ContextDataT {
        Context_(ContextStack *stack)
            : ContextBase(stack)
            , ContextDataT(stack_->empty() ? ContextDataT() :
                                            dynamic_cast<const ContextData *>(stack_->top()))
        {
            stack_->push(this);
//            std::cerr << "Pushed context " << *this << "[" << stack_->size() << "]" << std::endl;
        }
        Context_(ContextStack *stack, std::ostream *_stream)
            : ContextBase(stack)
            , ContextDataT(stack_->empty() ? ContextDataT() :
                                            dynamic_cast<const ContextData *>(stack_->top()))
        {
            if (_stream) this->stream = _stream;
            stack_->push(this);
//            std::cerr << "Pushed context " << *this << "[" << stack_->size() << "]" << std::endl;
        }
        Context_(ContextStack *stack, const ContextDataT &data)
            : ContextBase(stack), ContextDataT(data)
        {
            stack_->push(this);
//            std::cerr << "Pushed context " << *this << "[" << stack_->size() << "]" << std::endl;
        }
        ~Context_()
        {
            assert(stack_->top() == this);
//            std::cerr << "Popd context " << *this << "[" << stack_->size() << "]" << std::endl;
            stack_->pop();
        }
    };
    typedef Context_<ContextData> Context;

    PartGeneratorBase(const std::string &type_name, const std::string &c_type_name);
    PartGeneratorBase(PartGeneratorBase *parent, const std::string &type_name, const std::string &c_type_name);
    virtual ~PartGeneratorBase();

    virtual std::string getTypeName() const { return type_name_; }
    virtual const Namespaces &getNamespaces() const { return namespaces_; }
    virtual std::string getCTypeName() const { return c_type_name_; }
    virtual std::string getCFullName() const;
    virtual std::string getCCorbaName() const;
    virtual std::string getPartName() const = 0;

    virtual void setNamespacePrefix(const std::string &prefix);

    virtual void generate(std::ostream *stream) = 0;

protected:
    ContextData &context() const;
    std::ostream &stream() const;
    std::string indent() const;
    std::string indent(int diff);

    PartGeneratorBase *parent() const;

    ContextStack *context_stack_;
    template <typename ContextDataT> friend class Context_;

    typedef std::map<std::string, std::string> AttributesMap;
    std::string stripAttributes(std::string name_and_attributes, AttributesMap &attributes);

protected:
    std::string type_name_;
    Namespaces namespaces_;
    std::string c_type_name_;
    boost::shared_ptr<std::string> namespace_prefix_;

    PartGeneratorBase *parent_;

    void parseNamespaces();
};

/**
 * A TransportGeneratorBase generates a transport plugin for a certain typekit.
 */
class TransportGeneratorBase
{
public:
    typedef boost::shared_ptr<TransportGeneratorBase> shared_ptr;
    typedef std::map<std::string, PartGeneratorBase::shared_ptr> PartGeneratorMap;
    typedef std::map<std::string, PartGeneratorMap> TypeGenerators;

    struct StaticInitializer
    {
        StaticInitializer(const char *transport,
                          const shared_ptr &generator);
    };

    virtual ~TransportGeneratorBase();

    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual void generate(const std::string &output_directory) = 0;

protected:
    friend class PartGeneratorBase::StaticInitializer;
    TypeGenerators part_generators_;
};

/**
 * A repository which contains all the registered transports.
 */
class TransportGeneratorRepository
        : public std::map<std::string,
                          boost::shared_ptr<TransportGeneratorBase> >
{
public:
    typedef boost::shared_ptr<TransportGeneratorRepository> shared_ptr;

    static shared_ptr Instance();

private:
    TransportGeneratorRepository();

    friend class TransportGeneratorBase::StaticInitializer;
    std::map<std::string, boost::shared_ptr<TransportGeneratorBase> > transports_;
};

std::ostream &operator<<(std::ostream &os, PartGeneratorBase::ContextData &context);

} // namespace rtt_typekit_generator

// Macro definition

#define _unique_identifier(_prefix, i) \
    BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(_prefix, __LINE__), _), i)

#define DECLARE_TRANSPORT_GENERATOR(_transport, _generator_class) \
    static ::rtt_typekit_generator::TransportGeneratorBase::StaticInitializer _unique_identifier(transport_generator_, 0)(_transport, boost::make_shared< _generator_class >());

#define DECLARE_PART_GENERATOR_TEMPLATE_TYPE(z, i, _data) \
    static ::rtt_typekit_generator::PartGeneratorBase::StaticInitializer _unique_identifier(part_generator_template_type_, i)(BOOST_PP_TUPLE_ELEM(2, 0, _data), boost::make_shared< BOOST_PP_TUPLE_ELEM(2, 1, _data)<TYPE(i)> >(TYPE_NAME(i), C_TYPE_NAME(i)));

#define DECLARE_PART_GENERATOR_TEMPLATE(_transport, _generator_class) \
    BOOST_PP_REPEAT(TYPE_CNT, DECLARE_PART_GENERATOR_TEMPLATE_TYPE, (_transport, _generator_class))

#endif // RTT_TYPEKIT_GENERATOR_GENERATOR_H
