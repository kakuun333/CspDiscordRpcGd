---
name: "godot-cpp-guide"
description: "編寫 Godot C++（godot-cpp）程式碼時的規範，確保結構清楚、易維護並符合最佳實踐。"
---

## 使用時機

- 使用者提到 `godot-cpp`

---

## 規則

- 優先使用 Godot 提供的功能，避免使用 C++ STL
- 換行符使用 `LF`
- 檔案編碼使用 `UTF-8`

### 0. 執行結束檢查

檢查編譯錯誤時使用：

```shell
cmake --build cmake-build-codex --config Debug -j 26
```

必要時重新建置：

```shell
rm -rf cmake-build-codex
cmake -S . -B cmake-build-codex
cmake --build cmake-build-codex --config Debug -j 26
```

### 1. 類別結構

`.h` 中的類別方法與成員排列規則：

- 方法順序：`static` → `public` → `protected` → `private`
- 成員變數放在方法之後，順序為：`public` → `protected` → `private`
- 單行方法可直接在 `.h` 定義，其餘應在 `.cpp` 實作

初始化：

- 不要將 `godot::Ref<T>` 明確初始化為 `nullptr`
- 只有在預設值不安全時才初始化成員，其餘交由預設建構處理

覆寫：

- 方法需要 `override` 時，不要省略 `virtual`

屬性：

- 必要時加上 `[[nodiscard]]`，強調返回值的重要性
- 必要時加上 `[[maybe_unused]]`，避免未使用變數警告

關鍵字：

- 必要時加上 `_FORCE_INLINE_`
- 必要時加上 `_ALWAYS_INLINE_`

---

### 2. 命名規則

- 類別：PascalCase → `MyClass`
- 結構：PascalCase → `MyStruct`
- 方法：PascalCase → `MyMethod`
- 成員：PascalCase → `MyMember`
- 列舉值：UPPER_SNAKE_CASE → `MY_ENUMERATOR`
- 布林值：`b` + PascalCase → `bIsActive`
- 列舉：`E` + PascalCase → `EMyEnum`
- 縮寫在識別名稱中使用 PascalCase  
  例如：`Http`、`Ui`、`Npc`、`Xp`、`UserId`

---

### 3. 程式撰寫規則

- 避免重複宣告中的返回型別，改用大括號初始化
- 能使用 `range-based for loop` 時就使用

- `Pointer` 和 `Reference` 應放在型別後方，且不留空格：

```cpp
godot::Button* SomeFunc(const godot::String& SomeText);
```

- 使用 `callable_mp` 巨集代替 `godot::Callable`：

```cpp
godot::Callable(this, "some_method") // 不推薦
callable_mp(this, &MyClass::SomeMethod) // 推薦
```

- 如果 enum 要綁定到 Godot，例如用於 `VARIANT_ENUM_CAST`、`BIND_ENUM_CONSTANT`、`godot::ClassDB::bind_method`，使用 `enum EMyEnum`
- 其他情況一律使用 `enum class EMyEnum`
- 根據列舉值數量繼承 `uint8_t` 或 `uint16_t`

- 成員變數使用統一初始化：

```cpp
class MyClass
{
private:
    bool bMyBoolean{ false };
};
```

- `godot::String` 無法透過 `operator+` 連接 `const char*`：

```cpp
godot::String("MyString") + "C-StyleString" // 錯誤
```

- 使用 `callable_mp()` 代替 `call_deferred()`：

```cpp
callable_mp(this, &MyClass::MyFunc).call_deferred(); // 推薦
call_deferred("my_func"); // 不推薦
```

---

### 4. 記憶體規則

- `Resource` 使用 `godot::Ref<T>`
- 避免讓 raw pointer 擁有物件所有權
- 除非必要，不要直接使用 `new`

---

### 5. 命名空間規則

- 不要使用 `using namespace godot;`
- 使用 `godot::` 前綴明確指定命名空間

---

### 6. 方法綁定

- 使用 `godot::ClassDB::bind_method` 綁定方法
- `godot::D_METHOD` 的方法名稱與參數名稱使用 `snake_case`

```cpp
godot::ClassDB::bind_method(godot::D_METHOD("some_method", "some_param"), &MyClass::SomeMethod);
```

- `ADD_PROPERTY` 的第 2 和第 3 個參數使用 `snake_case`

```cpp
ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING_NAME, "Name", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_DEFAULT), "set_name", "get_name");
```

- `PropertyInfo` 的第 2 個參數使用 PascalCase

```cpp
godot::PropertyInfo(godot::Variant::STRING_NAME, "Name", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_DEFAULT)
```

---

### 7. 範例格式

所有繼承 Godot 類別的類別都必須使用以下格式：

```cpp
class MyClass : public godot::GodotClass
{
    GDCLASS(MyClass, GodotClass)

protected:
    static void _bind_methods();
}
```