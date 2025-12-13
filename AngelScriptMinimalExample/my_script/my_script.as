class GameObjectWrapper {
    // '@' でオブジェクト参照を保持するハンドルを定義出来る (value_class で定義されたクラスは不可)
    GameObject@ obj = null;

    void isSame(const GameObject& in other) {
        if (@obj != null && obj.equalsTo(other)) {
            println("AS: GameObjectWrapper: obj.equalsTo(other): true");
        } else {
            println("AS: GameObjectWrapper: obj.equalsTo(other): false");
        }
    }
}

// C++ 側から呼び出されるエントリポイント
float as_main(FlagAndVector3 value, GameObject& obj) {
    const float result = value.manhattan();
    println("AS: value.manhattan(): " + result);

    if (value.flag) {
        println("AS: value.flag: true");
    } else {
        println("AS: value.flag: false");
    }

    const bool f = value // <-- FlagAndVector3 の operator bool() が呼ばれる
        && value.flag;
    println("AS: value && value.flag: " + f); // <-- 同じものを && しており、そりゃ true になる

    // -----------------------------------------------

    GameObjectWrapper wrapper;
    wrapper.isSame(obj); // <-- false

    // ハンドルの代入 (参照の付け替え)
    @wrapper.obj = @obj;
    wrapper.isSame(obj); // <-- true

    @wrapper.obj = @GameObject();
    wrapper.isSame(obj); // <-- false

    // -----------------------------------------------

    return value.manhattan();
}
