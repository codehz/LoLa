#include "lolacore.hpp"

#include "driver.hpp"

#include "compiler.hpp"
#include "runtime.hpp"

#include <sstream>

#define STR(x) #x
#define SSTR(x) STR(x)

using LoLa::Runtime::Function;
using LoLa::Runtime::Value;

struct GenericSyncFunction : LoLa::Runtime::Function
{
    std::function<Value(Value const * args, size_t cnt)> fn;

    explicit GenericSyncFunction(decltype(fn) const & f) :
        fn(f)
    {
    }

    CallOrImmediate call(const LoLa::Runtime::Value *args, size_t argc) const override
    {
        return fn(args, argc);
    }
};

struct CounterObject : LoLa::Object
{
    double counter = 0;

    GenericSyncFunction getValue, increment, decrement;

    CounterObject() :
        getValue([this](Value const *, size_t) -> Value { return counter; }),
        increment([this](Value const *, size_t) -> Value { return ++counter; }),
        decrement([this](Value const *, size_t) -> Value { return --counter; })
    {

    }

    std::optional<LoLa::Runtime::Function const *> getFunction(std::string const & name) const override
    {
        if(name == "GetValue") {
            return &getValue;
        }
        else if(name == "Increment") {
            return &increment;
        }
        else if(name == "Decrement") {
            return &decrement;
        }
        return std::nullopt;
    }
};


struct StackObject : LoLa::Object
{
    std::vector<LoLa::Runtime::Value> contents;

    GenericSyncFunction getSize, push, pop;

    StackObject() :
        getSize([this](Value const *, size_t) -> Value { return double(contents.size()); }),
        push([this](Value const * a, size_t) -> Value { contents.push_back(a[0]); return LoLa::Runtime::Void { }; }),
        pop([this](Value const *, size_t) -> Value {
            auto val = contents.back();
            contents.pop_back();
            return val;
        })
    {

    }

    std::optional<LoLa::Runtime::Function const *> getFunction(std::string const & name) const override
    {
        if(name == "GetSize") {
            return &getSize;
        }
        else if(name == "Push") {
            return &push;
        }
        else if(name == "Pop") {
            return &pop;
        }
        return std::nullopt;
    }
};

bool LoLa::verify(std::string_view code)
{
    std::stringstream str;
    str.write(code.data(), code.size());
    str.seekg(0);

    LoLa::LoLaDriver driver;

    driver.parse(str);

    Compiler::Compiler compiler;

    auto compile_unit = compiler.compile(driver.program);

    Compiler::Disassembler disasm;
    disasm.disassemble(*compile_unit, std::cout);

    Runtime::Environment env(compile_unit);
    env.functions["Print"] = new GenericSyncFunction([](Value const * argv, size_t argc) -> Value
    {
        for(size_t i = 0; i < argc; i++)
        {
            if(i > 0)
                std::cout << " ";
            std::cout << argv[i];
        }
        std::cout << std::endl;
        return LoLa::Runtime::Void { };
    });
    env.functions["CreateCounter"] = new GenericSyncFunction([](Value const * argv, size_t argc) -> Value
    {
        return ObjectRef(new CounterObject);
    });
    env.functions["CreateStack"] = new GenericSyncFunction([](Value const * argv, size_t argc) -> Value
    {
        return ObjectRef(new StackObject);
    });

    env.known_globals["RealGlobal"] = Value { LoLa::Runtime::Void { } };
    env.known_globals["ReadOnlyGlobal"] = std::make_pair(
        []() -> LoLa::Runtime::Value {
            return 42.0;
        },
        LoLa::Runtime::Environment::Setter()
    );

    Runtime::VirtualMachine machine { env };
    machine.enable_trace = true;

//    try
    {
        while(machine.exec() != LoLa::Runtime::ExecutionResult::Done)
        {

        }
    }
//    catch (LoLa::Error err)
//    {
//        std::cerr << to_string(err) << std::endl;
//        return false;
//    }

    return true;
}
