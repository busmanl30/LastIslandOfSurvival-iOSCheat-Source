#include <mach-o/dyld.h>
#include <string>
#include <map>

/*
This struct can hold a native C# array. Credits to caoyin.

Think of it like a wrapper for a C array. For example, if you had Player[] players in a dump,
the resulting monoArray definition would be monoArray<void **> *players;

To get the C array, call getPointer.
To get the length, call getLength.
*/
template <typename T>
struct monoArray
{
    void* klass;
    void* monitor;
    void* bounds;
    int   max_length;
    T vector [0];
    int getLength()
    {
        return max_length;
    }
    T *getPointer()
    {
        return vector;
    }

    template<typename V = T>
    std::vector<V> toCPPlist() {
        std::vector<V> ret;
        for (int i = 0; i < max_length; i++)
            ret.push_back(vector[i]);
        return std::move(ret);
    }
};

/*
This struct represents a C# string. Credits to caoyin.

It is pretty straight forward. If you have this in a dump,

public class Player {
	public string username; // 0xC8
}

getting that string would look like this: monoString *username = *(monoString **)((uint64_t)player + 0xc8);

C# strings are UTF-16. This means each character is two bytes instead of one.

To get the length of a monoString, call getLength.
To get a NSString from a monoString, call toNSString.
To get a std::string from a monoString, call toCPPString.
To get a C string from a monoString, call toCString.
*/
typedef struct _monoString
{
    void* klass;
    void* monitor;
    int length;    
    char chars[1];   
    int getLength()
    {
      return length;
    }
    char* getChars()
    {
        return chars;
    }
    NSString* toNSString()
    {
      return [[NSString alloc] initWithBytes:(const void *)(chars)
                     length:(NSUInteger)(length * 2)
                     encoding:(NSStringEncoding)NSUTF16LittleEndianStringEncoding];
    }

    char* toCString()
    {
      NSString* v1 = toNSString();
      return (char*)([v1 UTF8String]);  
    }
    std::string toCPPString()
    {
      return std::string(toCString());
    }
}monoString;

template <typename T>
struct monoList {
	void *unk0;
	void *unk1;
	monoArray<T> *items;
	int size;
	int version;
	
	T *getItems(){
		return items->getPointer();
	}
	
	int getSize(){
		return size;
	}
	
	int getVersion(){
		return version;
	}
};

/*
This struct represents a Dictionary. In the dump, a Dictionary is defined as Dictionary`1.

You could think of this as a Map in Java or C++. Keys correspond with values. This wraps the C arrays for keys and values.

If you had this in a dump,

public class GameManager {
	public Dictionary`1<int, Player> players; // 0xB0
	public Dictionary`1<Weapon, Player> playerWeapons; // 0xB8
	public Dictionary`1<Player, string> playerNames; // 0xBC
}

to get players, it would look like this: monoDictionary<int *, void **> *players = *(monoDictionary<int *, void **> **)((uint64_t)player + 0xb0);
to get playerWeapons, it would look like this: monoDictionary<void **, void **> *playerWeapons = *(monoDictionary<void **, void **> **)((uint64_t)player + 0xb8);
to get playerNames, it would look like this: monoDictionary<void **, monoString **> *playerNames = *(monoDictionary<void **, monoString **> **)((uint64_t)player + 0xbc);

To get the C array of keys, call getKeys.
To get the C array of values, call getValues.
To get the number of keys, call getNumKeys.
To get the number of values, call getNumValues.
*/




template <typename K, typename V>
struct monoDictionary {
	void *unk0;
	void *unk1;
	monoArray<int **> *table;
	monoArray<void **> *linkSlots;
	monoArray<K> *keys;
	monoArray<V> *values;
	int touchedSlots;
	int emptySlot;
	int size;
	
	K getKeys(){
		return keys->getPointer();
	}
	
	V getValues(){
		return values->getPointer();
	}
	
	int getNumKeys(){
		return keys->getLength();
	}
	
	int getNumValues(){
		return values->getLength();
	}
	
	int getSize(){
		return size;
	}
};

template<typename TKey, typename TValue>
    struct monoDictionary2 {
        struct Entry {
            int hashCode, next;
            TKey key;
            TValue value;
        };
        void *klass;
        void *monitor;
        monoArray<int> *buckets;
        monoArray<Entry> *entries;
        int count;
        int version;
        int freeList;
        int freeCount;
        void *comparer;
        monoArray<TKey> *keys;
        monoArray<TValue> *values;
        void *syncRoot;

        std::map<TKey, TValue> toMap() {
            std::map<TKey, TValue> ret;
            auto lst = entries->template toCPPlist();
            for (auto enter : lst)
                ret.insert(std::make_pair(enter.key, enter.value));
            return std::move(ret);
        }

        std::vector<TKey> getKeys() {
            std::vector<TKey> ret;
            auto lst = entries->template toCPPlist();
            for (auto enter : lst)
                ret.push_back(enter.key);
            return std::move(ret);
        }

        std::vector<TValue> getValues() {
            std::vector<TValue> ret;
            auto lst = entries->template toCPPlist();
            for (auto enter : lst)
                ret.push_back(enter.value);
            return std::move(ret);
        }

        int getSize() {
            return count;
        }

        int getVersion() {
            return version;
        }

        bool TryGet(TKey key, TValue &value);
        void Add(TKey key, TValue value);
        void Insert(TKey key, TValue value);
        bool Remove(TKey key);
        bool ContainsKey(TKey key);
        bool ContainsValue(TValue value);

        TValue Get(TKey key) {
            TValue ret;
            if (TryGet(key, ret))
                return ret;
            return {};
        }

        TValue operator [](TKey key) {
            return Get(key);
        }
    };



/*
Turn a C string into a C# string!

This function is included in Unity's string class. There are two versions of this function in the dump, you want the one the comes FIRST.
The method signature is:

private string CreateString(PTR value);

Again, you want the FIRST one, not the second.
*/



/*

Create a native C# array with a starting length.

This one is kind of tricky to complete. I'm currently looking for an easier way to implement this.

The offsets you need are both found at String::Split(char separator[], int count). Go to that function in IDA and scroll down until you find something like this:

	TBNZ            W20, #0x1F, loc_101153944
	CMP             W20, #1
	B.EQ            loc_1011538B0
	CBNZ            W20, loc_101153924
	ADRP            X8, #qword_1026E0F20@PAGE
	NOP
	LDR             X19, [X8,#qword_1026E0F20@PAGEOFF]
	MOV             X0, X19
	BL              sub_101DD8AF8
	MOV             X0, X19
	MOV             W1, #0
	LDP             X29, X30, [SP,#0x30+var_10]
	LDP             X20, X19, [SP,#0x30+var_20]
	LDP             X22, X21, [SP+0x30+var_30],#0x30
	B               sub_101DD74E8

I filled in the locations here:

	TBNZ            W20, #0x1F, loc_101153944
	CMP             W20, #1
	B.EQ            loc_1011538B0
	CBNZ            W20, loc_101153924
	ADRP            X8, #qword_1026E0F20@PAGE
	NOP
	LDR             X19, [X8,#qword_1026E0F20@PAGEOFF] <-------- Whatever 1026E0F20 is in your game is your second location
	MOV             X0, X19
	BL              sub_101DD8AF8
	MOV             X0, X19
	MOV             W1, #0
	LDP             X29, X30, [SP,#0x30+var_10]
	LDP             X20, X19, [SP,#0x30+var_20]
	LDP             X22, X21, [SP+0x30+var_30],#0x30
	B               sub_101DD74E8						<-------- Whatever 101DD74E8 is in your game is your first location
	
For example, if you wanted an array of 10 ints, you would do this: monoArray<int *> *integers = CreateNativeCSharpArray<int *>(10);

You can use any type with this!
*/

#if 0
template<typename T>
monoArray<T> *CreateNativeCSharpArray(int startingLength){
	monoArray<T> *(*IL2CPPArray_Create)(void *klass, int startingLength) = (monoArray<T> *(*)(void *, int))getRealOffset(/*FIRST LOCATION HERE*/);
	
	void *unkptr0 = *(void **)(ASLR_BIAS + /*SECOND LOCATION HERE*/);
	void *klass = *(void **)((uint64_t)unkptr0);
	
	monoArray<T> *arr = IL2CPPArray_Create(klass, startingLength);
	
	return arr;
}
#endif

/*
Here are some functions to safely get/set values for types from Anti Cheat Toolkit (https://assetstore.unity.com/packages/tools/utilities/anti-cheat-toolkit-10395)

I will add more to this as I go along.
*/

union intfloat {
	int i;
	float f;
};

/*
Get the real value of an ObscuredInt.

Parameters:
	- location: the location of the ObscuredInt
*/
int GetObscuredIntValue(uint64_t location){
	int cryptoKey = *(int *)location;
	int obfuscatedValue = *(int *)(location + 0x4);
	
	return obfuscatedValue ^ cryptoKey;
}

/*
Set the real value of an ObscuredInt.

Parameters:
	- location: the location of the ObscuredInt
	- value: the value we're setting the ObscuredInt to
*/
void SetObscuredIntValue(uint64_t location, int value){
	int cryptoKey = *(int *)location;
	
	*(int *)(location + 0x4) = value ^ cryptoKey;
}

/*
Get the real value of an ObscuredFloat.

Parameters:
	- location: the location of the ObscuredFloat
*/
float GetObscuredFloatValue(uint64_t location){
	int cryptoKey = *(int *)location;
	int obfuscatedValue = *(int *)(location + 0x4);
	
	/* use this intfloat to set the integer representation of our parameter value, which will also set the float value */
	intfloat IF;
	IF.i = obfuscatedValue ^ cryptoKey;
	
	return IF.f;
}

/*
Set the real value of an ObscuredFloat.

Parameters:
	- location: the location of the ObscuredFloat
	- value: the value we're setting the ObscuredFloat to
*/
void SetObscuredFloatValue(uint64_t location, float value){
	int cryptoKey = *(int *)location;

	/* use this intfloat to get the integer representation of our parameter value */
	intfloat IF;
	IF.f = value;
	
	/* use this intfloat to generate our hacked ObscuredFloat */
	intfloat IF2;
	IF2.i = IF.i ^ cryptoKey;
	
	*(float *)(location + 0x4) = IF2.f;
}

/*
Get the real value of an ObscuredVector3.

Parameters:
	- location: the location of the ObscuredVector3
*/
Vector3 GetObscuredVector3Value(uint64_t location){
	int cryptoKey = *(int *)location;

	Vector3 ret;

	intfloat IF;

	IF.i = *(int *)(location + 0x4) ^ cryptoKey;

	ret.x = IF.f;

	IF.i = *(int *)(location + 0x8) ^ cryptoKey;

	ret.y = IF.f;

	IF.i = *(int *)(location + 0xc) ^ cryptoKey;

	ret.z = IF.f;

	return ret;
}

/*
Set the real value of an ObscuredVector3.

Parameters:
	- location: the location of the ObscuredVector3
	- value: the value we're setting the ObscuredVector3 to
*/
void SetObscuredVector3Value(uint64_t location, Vector3 value){
	int cryptoKey = *(int *)location;
	
	intfloat IF;
	IF.f = value.x;

	intfloat IF2;
	IF2.i = IF.i ^ cryptoKey;

	*(float *)(location + 0x4) = IF2.f;

	IF.f = value.y;
	IF2.i = IF.i ^ cryptoKey;

	*(float *)(location + 0x8) = IF2.f;

	IF.f = value.z;
	IF2.i = IF.i ^ cryptoKey;

	*(float *)(location + 0xc) = IF2.f;
}

struct Color {
    union {
        struct {
            float r, b, g, a;
        };
        float data[4];
    };
    Color() {
        SetColor(0, 0, 0, 255);
    }
    Color(float r, float g, float b) {
        SetColor(r, g, b, 255);
    }
    Color(float r, float g, float b, float a) {
        SetColor(r, g, b, a);
    }
    void SetColor(float r1, float g1, float b1, float a1 = 255) {
        r = r1;
        g = g1;
        b = b1;
        a = a1;
    }
    static Color Black(float a = 255) { return Color(0, 0, 0, a); }
    static Color Red(float a = 255) { return Color(255, 0, 0, a); }
    static Color Green(float a = 255) { return Color(0, 255, 0, a); }
    static Color Blue(float a = 255) { return Color(0, 0, 255, a); }
    static Color White(float a = 255) { return Color(255, 255, 255, a); }
    static Color Orange(float a = 255) { return Color(255, 153, 0, a); }
    static Color Yellow(float a = 255) { return Color(255, 255, 0, a); }
    static Color Cyan(float a = 255) { return Color(0, 255, 255, a); }
    static Color Magenta(float a = 255) { return Color(255, 0, 255, a); }
    static Color MonoBlack(float a = 1){ return Color(0, 0, 0, a); }
    static Color MonoRed(float a = 1){ return Color(1, 0, 0, a); }
    static Color MonoGreen(float a = 1){ return Color(0, 1, 0, a); }
    static Color MonoBlue(float a = 1){ return Color(0, 0, 1, a); }
    static Color MonoWhite(float a = 1){ return Color(1, 1, 1, a); }
    static Color MonoOrange(float a = 1){ return Color(1, 0.55, 0, a); }
    static Color MonoYellow(float a = 1){ return Color(1, 1, 0, a); }
    static Color MonoCyan(float a = 1){ return Color(0, 1, 1, a); }
    static Color MonoMagenta(float a = 1){ return Color(1, 0, 1, a); }
    static Color random(float a = 255) {
        float r = static_cast <float> (rand()) / static_cast <float> (255);
        float g = static_cast <float> (rand()) / static_cast <float> (255);
        float b = static_cast <float> (rand()) / static_cast <float> (255);
        return Color(r, g, b, a);
    }
    Color invert(int a = 255){
        return Color(255 - r, 255 - g, 255 - b, a);
    }
};