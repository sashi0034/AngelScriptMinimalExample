#include <iostream>

#include "add_on/scriptarray/scriptarray.h"
#include "add_on/scriptstdstring/scriptstdstring.h"
#include "add_on/datetime/datetime.h"
#include "add_on/scriptbuilder/scriptbuilder.h"
#include "add_on/scriptfile/scriptfile.h"
#include "asbind20/bind.hpp"
#include "asbind20/invoke.hpp"

namespace
{
    void MessageCallback(const asSMessageInfo* msg, void* param)
    {
        auto type = "[error]";
        if (msg->type == asMSGTYPE_WARNING)
            type = "[warn] ";
        else if (msg->type == asMSGTYPE_INFORMATION)
            type = "[info] ";

        const auto message = std::format("{} {}({},{}) {}", type, msg->section, msg->row, msg->col, msg->message);
        if (msg->type == asMSGTYPE_INFORMATION)
        {
            std::cout << message << std::endl;
        }
        else
        {
            std::cerr << message << std::endl;
        }
    }

    void script_print(const std::string& message)
    {
        printf("%s", message.c_str());
    }

    void script_println(const std::string& message)
    {
        printf("%s\n", message.c_str());
    }

    struct Vector3
    {
        float x, y, z;

        bool operator==(const Vector3&) const = default;
    };

    struct FlagAndVector3
    {
        bool flag;
        Vector3 vec;

        FlagAndVector3() = default;

        FlagAndVector3(bool flag) : flag(flag), vec{0.0f, 0.0f, 0.0f}
        {
        }

        bool operator==(const FlagAndVector3&) const = default;

        operator bool() const
        {
            return flag;
        }

        float manhattan() const
        {
            return vec.x + vec.y + vec.z;
        }
    };

    // C++ の型や関数を AngelScript に登録する
    void registerEngine(const asbind20::script_engine& engine)
    {
        RegisterStdString(engine);
        RegisterScriptArray(engine, true);

        // -----------------------------------------------
        // asbind20 を使用して型や関数を登録する

        asbind20::global(engine)
            .message_callback(&MessageCallback)
            .function("void print(const string& in message)", &script_print) // グローバル関数登録
            .function("void println(const string& in message)", &script_println);

        // 値型の登録
        asbind20::value_class<Vector3>(engine, "Vector3", asOBJ_APP_CLASS_ALLFLOATS)
            .behaviours_by_traits() // デフォルトの CTOR, DTOR などを自動登録
            .property("float x", &Vector3::x) // メンバ x の登録
            .property("float y", &Vector3::y)
            .property("float z", &Vector3::z)
            .opEquals(); // operator== の登録

        asbind20::value_class<FlagAndVector3>(engine, "FlagAndVector3")
            .behaviours_by_traits() // デフォルトの CTOR, DTOR などを自動登録
            .constructor<bool>("bool flag")
            .opEquals() // operator== の登録
            .opConv<bool>() // operator bool の登録
            .opImplConv<bool>() // operator bool の登録 (暗黙の型変換用)
            .property("bool flag", &FlagAndVector3::flag)
            .property("Vector3 vec", &FlagAndVector3::vec)
            .method("float manhattan() const", &FlagAndVector3::manhattan);
    }
}

int main()
{
    const auto engine = asbind20::make_script_engine();

    registerEngine(engine);

    CScriptBuilder builder{};

    std::string moduleName{"my_script/my_script.as"};

    int r;
    r = builder.StartNewModule(engine, moduleName.c_str()) < 0;
    assert(r >= 0);

    r = builder.AddSectionFromFile(moduleName.c_str()) < 0;
    assert(r >=0);

    r = builder.BuildModule();
    assert(r >= 0);

    asIScriptModule* module = engine->GetModule(moduleName.c_str());
    assert(module != nullptr);

    const asbind20::request_context ctx{engine};

    FlagAndVector3 arg0;
    arg0.flag = true;
    arg0.vec = {1.0f, 2.0f, 3.0f};

    // my_script.as で定義された as_main を取得する
    asIScriptFunction* func = module->GetFunctionByDecl("float as_main(FlagAndVector3 value)");

    // 引数を渡して実行する
    const auto result = asbind20::script_invoke<float>(ctx, func, arg0);
    if (result.has_value())
    {
        std::cout << "C++: result: " << result.value() << std::endl;
    }
    else
    {
        std::cerr << "Failed to execute the script: " << result.error() << std::endl;
    }

    module->Discard();
}
