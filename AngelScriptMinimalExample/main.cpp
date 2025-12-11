#include <cassert>
#include <cstdio>

#include "angelscript.h"
#include "add_on/scriptbuilder/scriptbuilder.h"
#include "add_on/scriptstdstring/scriptstdstring.h"

// メッセージコールバック
void MessageCallback(const asSMessageInfo* msg, void* param)
{
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING) type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION) type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

// C++ から呼ばれる関数
void print(const std::string& msg)
{
    printf("%s", msg.c_str());
}

int main(int argc, char** argv)
{
    // エンジン生成
    asIScriptEngine* engine = asCreateScriptEngine();
    assert(engine != nullptr);

    // メッセージコールバック登録
    int r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    assert(r >= 0);

    // 標準文字列型を登録
    RegisterStdString(engine);

    // グローバル関数 print() を登録
    r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);
    assert(r >= 0);

    // スクリプトビルダーでモジュール作成
    CScriptBuilder builder;
    r = builder.StartNewModule(engine, "MyModule");
    if (r < 0)
    {
        printf("Failed to start module\n");
        return -1;
    }

    // スクリプトをセクションとして追加
    const char* script = R"(
        void as_main()
        {
            print("Hello world\n");
        }
)";

    r = builder.AddSectionFromMemory("my_script.as", script);
    if (r < 0)
    {
        printf("Failed to add script section\n");
        return -1;
    }

    r = builder.BuildModule();
    if (r < 0)
    {
        printf("Failed to build module\n");
        return -1;
    }

    // モジュール取得
    asIScriptModule* mod = engine->GetModule("MyModule");
    if (!mod)
    {
        printf("Module not found\n");
        return -1;
    }

    // 関数取得
    asIScriptFunction* func = mod->GetFunctionByDecl("void as_main()");
    if (!func)
    {
        printf("Function 'void as_main()' not found\n");
        return -1;
    }

    // コンテキスト作成・準備・実行
    asIScriptContext* ctx = engine->CreateContext();
    ctx->Prepare(func);
    r = ctx->Execute();
    if (r != asEXECUTION_FINISHED)
    {
        if (r == asEXECUTION_EXCEPTION)
        {
            printf("Exception: %s\n", ctx->GetExceptionString());
        }
    }

    // クリーンアップ
    ctx->Release();
    engine->ShutDownAndRelease();

    return 0;
}
