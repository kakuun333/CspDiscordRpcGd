---
name: "godot-cpp-guide"
description: "編寫 Godot C++ (godot-cpp) 程式碼時的規範，確保結構化、可維護且符合最佳實踐。"
---

## When to use

- User mentions "godot-cpp"

---

## Rules (VERY IMPORTANT)

### 0. 執行結束時 (MANDATORY)
- 不要編譯程式碼

### 1. Class Structure (MANDATORY)

在 `.h` 中的類別方法與成員排列規則：
- 方法順序：`static` → `public` → `protected` → `private`
- 成員變數放在方法之後，順序同樣為：`public` → `protected` → `private`
- 單行方法可直接在 `.h` 定義，其餘應於 `.cpp` 實現

初始化：
- 不要將 `godot::Ref<T>` 明確初始化為 `nullptr`，除非需要非空值，否則使用預設建構
- 僅在「不安全的預設值」情況下初始化成員，其餘交由預設建構處理

覆寫：
- 當方法需要`override`時不要省略`virtual`關鍵字

屬性：
- 如果有必要就加上`[[nodiscard]]`，以強調返回值的重要性
- 如果有必要就加上`[[maybe_unused]]`，以避免未使用變數的警告

關鍵字：
- 如果有必要就加上`_FORCE_INLINE_`，以強調方法應該被內聯
- 如果有必要就加上`_ALWAYS_INLINE_`，以強調方法應該被始終內聯

---

### 2. Naming Convention

- Class: PascalCase → `MyClass`
- Methods: PascalCase → `MyClassMethod`
- Members: PascalCase → `MyClassMember`
- bool: `b` + PascalCase → `bIsActive`
- Enum: `E` + PascalCase → `EMyEnum`
- Abbreviations: UpperCase → `HTTP`, `XML`

### 3. Programming Rules
- 使用`braced initializer list`來當作函式的返回值，而不是整個建構函數，這樣可以避免不必要的複製和提高效率。
```cpp
godot::Ref<MyResource> SomeFunc()
{
    return {}
}
```

- 能使用`range-based for loop`時就使用

- `Pointer`和`Reference`應該在型別的後面並且不要空格
```cpp
godot::Button* SomeFunc(const godot::String& SomeText);
```


### 4. Memory Rules

- Use godot::Ref<T> for Resources
- Avoid raw pointer ownership
- Do NOT use `new` directly unless necessary

---

### 5. Namespace Rule

- 不要使用`using namespace godot;`，請使用`godot::`前綴來明確指定命名空間。

---

### 6. Method Binding (MANDATORY)
- 使用 `godot::ClassDB::bind_method` 來綁定方法，並且都使用`snake_case`來命名方法和參數，也就是`godot::D_METHOD`的第1和第2個參數都應該是小寫字母和底線的組合。
例如：
```cpp
godot::ClassDB::bind_method(godot::D_METHOD("some_method", "some_param"), &MyClass::SomeMethod);
```

- `ADD_PROPERTY`的第2和第3個參數也應該是小寫字母和底線的組合
例如：
```cpp
ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING_NAME, "Name", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_DEFAULT), "set_name", "get_name");
```

- `PropertyInfo`的第2個參數應該是PascalCase
例如：
```cpp
godot::PropertyInfo(godot::Variant::STRING_NAME, "Name", godot::PROPERTY_HINT_NONE, "", godot::PROPERTY_USAGE_DEFAULT)
```

---

### 7. Example Pattern (STRICT)

所有繼承 Godot Class 的 Class 都必須按照以下程式碼的格式：
```cpp
class MyClass : public godot::GodotClass
{
    GDCLASS(MyClass, GodotClass)

protected:
    static void _bind_methods();
}
```