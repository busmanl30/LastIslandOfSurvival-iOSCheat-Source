#import <Foundation/Foundation.h>
#import <TargetConditionals.h>
#import <mach/mach_traps.h>
#import <mach-o/dyld.h>
#import <mach/mach.h>
#import <substrate.h>
#import <dlfcn.h>
#import <UIKit/UIKit.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#import <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <pthread.h>
#import <CommonCrypto/CommonDigest.h>
#import <AVFoundation/AVFoundation.h>
#include <CoreFoundation/CoreFoundation.h>


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


monoString *U3DStr(const char *str){
	monoString *(*String_CreateString)(void *_this, const char *str) = (monoString *(*)(void *, const char *))getRealOffset(0x305952C);
	
	return String_CreateString(NULL, str);
}


bool isFirst = true;
bool isON = true;
pthread_t thread;
const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL 
};

/*
struct U3DRect {
	float x;
	float y;
	float width;
	float height;
	
	
};*/

extern const double uZero;  

class Vector3
{
public:
  float x;
  float y;
  float z;
  Vector3() :x(0), y(0), z(0){}
  Vector3(float x1, float y1, float z1) :x(x1), y(y1), z(z1){}
  Vector3(const Vector3 &v);
  ~Vector3();
  void operator=(const Vector3 &v);
  Vector3 operator+(const Vector3 &v);
  Vector3 operator-(const Vector3 &v);
  Vector3 operator/(const Vector3 &v);
  Vector3 operator*(const Vector3 &v);
  Vector3 operator+(float f);
  Vector3 operator-(float f);
  Vector3 operator/(float f);
  Vector3 operator*(float f);
  Vector3 static Normalize(Vector3 vec);
  Vector3 static Cross(Vector3 a, Vector3 b);
  float dot(const Vector3 &v);
  float length();
  void clear();
  void normalized();
  Vector3 static Lerp(Vector3 start, Vector3 end, float percent);
  Vector3 cross(const Vector3 &v);
  static float distance(const Vector3 &a, const Vector3 &b);
  bool equals(Vector3 &v);

  inline void Rotate2D(const float &f);
};

bool Vector3::equals(Vector3 &v){
if(x == v.x && y == v.y && z == v.z){
return true;
}

return false;
}

const double uZero = 1e-6;

Vector3 Vector3::Lerp(Vector3 start, Vector3 end, float percent)
{
     return (start + (end - start) * percent);
}

Vector3::Vector3(const Vector3 &v) :x(v.x), y(v.y), z(v.z)
{
  
}



Vector3::~Vector3()
{
}

void Vector3::operator=(const Vector3 &v)
{
  x = v.x;
  y = v.y;
  z = v.z;
}

Vector3 Vector3::operator+(const Vector3 &v)
{
  return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator-(const Vector3 &v)
{
  return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator/(const Vector3 &v)
{
  if (fabsf(v.x) <= uZero || fabsf(v.y) <= uZero || fabsf(v.z) <= uZero)
  {
    return *this;
  }
  return Vector3(x / v.x, y / v.y, z / v.z);
}

Vector3 Vector3::operator*(const Vector3 &v)
{
  return Vector3(x*v.x, y*v.y, z*v.z);
}

Vector3 Vector3::operator+(float f)
{
  return Vector3(x + f, y + f, z + f);
}

Vector3 Vector3::operator-(float f)
{
  return Vector3(x - f, y - f, z - f);
}

void Vector3::clear()
{
  x = y = z = 0;
}

Vector3 Vector3::operator/(float f)
{
  if (fabsf(f) < uZero)
  {
    return *this;
  }
  return Vector3(x / f, y / f, z / f);
}

Vector3 Vector3::operator*(float f)
{
  return Vector3(x*f, y*f, z*f);
}

float Vector3::dot(const Vector3 &v)
{
  return x*v.x + y*v.y + z*v.z;
}

float Vector3::length()
{
  return sqrtf(dot(*this));
}

void Vector3::normalized()
{
  float len = length();
  if (len < uZero) len = 1;
  len = 1 / len;

  x *= len;
  y *= len;
  z *= len;
}
Vector3 Vector3::Normalize(Vector3 vec)
{
  vec.normalized();
  return vec;
}


float Vector3::distance(const Vector3 &point1, const Vector3 &point2)
{
  return sqrt(pow((point1.x - point2.x), 2.0f) + pow((point1.y - point2.y), 2) + pow((point1.z - point2.z), 2)); 
}

Vector3 Vector3::cross(const Vector3 &v)
{
  return Vector3(y * v.z - z * v.y,
    z * v.x - x * v.z,
    x * v.y - y * v.x);
}

Vector3 Vector3::Cross(Vector3 a, Vector3 b)
{
  return a.cross(b);
}

float Deg2Rad(float deg) 
{
    return deg * M_PI / 180.0f;
}

inline void SinCos(const float &r, float &s, float &c)
{
	s = sin(r);
	c = cos(r);
}


inline void Vector3::Rotate2D(const float &f)
{
	float _x, _y;
 
	float s, c;
 
	SinCos(Deg2Rad(f), s, c);
 
	_x = x;
	_y = y;
 
	x = (_x * c) - (_y * s);
	y = (_x * s) + (_y * c);
}


struct Quaternion 
{
public:
  // System.Single UnityEngine.Quaternion::x
  float x;
  // System.Single UnityEngine.Quaternion::y
  float y;
  // System.Single UnityEngine.Quaternion::z
  float z;
  // System.Single UnityEngine.Quaternion::w
  float w;
  static 
Quaternion LookRotation(Vector3 forward, Vector3 up)
{
  forward.normalized();

  Vector3 vector = Vector3::Normalize(forward);
  Vector3 vector2 = Vector3::Normalize(Vector3::Cross(up, vector));
  Vector3 vector3 = Vector3::Cross(vector, vector2);
  auto m00 = vector2.x;
  auto m01 = vector2.y;
  auto m02 = vector2.z;
  auto m10 = vector3.x;
  auto m11 = vector3.y;
  auto m12 = vector3.z;
  auto m20 = vector.x;
  auto m21 = vector.y;
  auto m22 = vector.z;


  float num8 = (m00 + m11) + m22;
  Quaternion quaternion;
  if (num8 > 0)
  {
    auto num = (float)sqrt(num8 + 1.0f);
    quaternion.w = num * 0.5f;
    num = 0.5f / num;
    quaternion.x = (m12 - m21) * num;
    quaternion.y = (m20 - m02) * num;
    quaternion.z = (m01 - m10) * num;
    return quaternion;
  }
  if ((m00 >= m11) && (m00 >= m22))
  {
    auto num7 = (float)sqrt(((1.0f + m00) - m11) - m22);
    auto num4 = 0.5f / num7;
    quaternion.x = 0.5f * num7;
    quaternion.y = (m01 + m10) * num4;
    quaternion.z = (m02 + m20) * num4;
    quaternion.w = (m12 - m21) * num4;
    return quaternion;
  }
  if (m11 > m22)
  {
    auto num6 = (float)sqrt(((1.0f + m11) - m00) - m22);
    auto num3 = 0.5f / num6;
    quaternion.x = (m10 + m01) * num3;
    quaternion.y = 0.5f * num6;
    quaternion.z = (m21 + m12) * num3;
    quaternion.w = (m20 - m02) * num3;
    return quaternion;
  }
  auto num5 = (float)sqrt(((1.0f + m22) - m00) - m11);
  auto num2 = 0.5f / num5;
  quaternion.x = (m20 + m02) * num2;
  quaternion.y = (m21 + m12) * num2;
  quaternion.z = 0.5f * num5;
  quaternion.w = (m01 - m10) * num2;
  return quaternion;
}
  Quaternion(float _x,float _y,float _z,float _w)
  {
     x = _x;
     y = _y;
     z = _z;
     w = _w;
  }
  Quaternion()
  {
     x = 0;
     y = 0;
     z = 0;
     w = 0;
  }
  
  Quaternion(const Quaternion &v) :x(v.x), y(v.y), z(v.z) , w(v.w)
  {
  
  }



 
};



float NormalizeAngle (float angle){
	while (angle>360)
		angle -= 360;
	while (angle<0)
		angle += 360;
	return angle;
}

Vector3 NormalizeAngles (Vector3 angles){
	angles.x = NormalizeAngle (angles.x);
	angles.y = NormalizeAngle (angles.y);
	angles.z = NormalizeAngle (angles.z);
	return angles;
}

Vector3 ToEulerRad(Quaternion q1){
	float Rad2Deg = 360.0f / (M_PI * 2.0f);

	float sqw = q1.w * q1.w;
	float sqx = q1.x * q1.x;
	float sqy = q1.y * q1.y;
	float sqz = q1.z * q1.z;
	float unit = sqx + sqy + sqz + sqw;
	float test = q1.x * q1.w - q1.y * q1.z;
	Vector3 v;

	if (test>0.4995f*unit) {
		v.y = 2.0f * atan2f (q1.y, q1.x);
		v.x = M_PI / 2.0f;
		v.z = 0;
		return NormalizeAngles(v * Rad2Deg);
	}
	if (test<-0.4995f*unit) {
		v.y = -2.0f * atan2f (q1.y, q1.x);
		v.x = -M_PI / 2.0f;
		v.z = 0;
		return NormalizeAngles (v * Rad2Deg);
	}
	Quaternion q(q1.w, q1.z, q1.x, q1.y);
	v.y = atan2f (2.0f * q.x * q.w + 2.0f * q.y * q.z, 1 - 2.0f * (q.z * q.z + q.w * q.w)); // yaw
	v.x = asinf (2.0f * (q.x * q.z - q.w * q.y)); // pitch
	v.z = atan2f (2.0f * q.x * q.y + 2.0f * q.z * q.w, 1 - 2.0f * (q.y * q.y + q.z * q.z)); // roll
	return NormalizeAngles (v * Rad2Deg);
}


unsigned int crc32( const unsigned char *buf, uint32_t size)
{
     uint32_t i, crc;
     crc = 0xFFFFFFFF;
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
     return crc^0xFFFFFFFF;
}


unsigned long getFileSize(const char *strFileName) 
{  
    FILE * fp = fopen(strFileName, "rb");  
    if(fp == NULL) return 0;
    fseek(fp, 0L, SEEK_END);  
    int size = ftell(fp);  
    fclose(fp);  
    return size;   
}




void showLog(void* byte_ptr,int size)
{
   for(int i = 0 ; i < size ; i ++)
   {
     NSLog(@"---byte%d,[0x%X]",i,*(unsigned char*)((long)byte_ptr + (long)i));
   }
}

NSString* md5(NSString* str)
{
  const char *cStr = [str UTF8String];
  unsigned char result[16];
  CC_MD5( cStr, strlen(cStr), result );
  return [NSString stringWithFormat:
  @"%X%X%X%X%X%X%X%X%X%X",
  result[0],result[1],result[2],result[3],result[4],result[5],result[6],result[8],result[9],result[10]
  ]; 
}



char* N2C(NSString* a)
{ 
const char* b = [a UTF8String];
char* output = (char*)calloc(1,strlen(b)+1);
memcpy(output,b,strlen(b));
return output;
}

NSString* C2N(char* a)
{
return [[NSString alloc] initWithUTF8String:a];
}


using namespace std;


/*
 CNCString quickly converts a const char* to NSString *
 Usage: CNCString("c string");
 */

#define CNCString(cStr) [NSString stringWithCString:cStr encoding:NSUTF8StringEncoding]



/*
 Functions to show an UIAlertView at load time
 */

/*
 CNConstructor is a hidden constructor
 */

#define CNConstructor(name) __attribute__((constructor)) void name(struct Programfloats *pinfo)

/*
Allows you to check the Version of a Game
Usage: CNVersion("1.0.0") { }
*/

#define CNVersion(version) if ((strcmp((version), [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"] UTF8String]) == 0))

/*
 Allows you to check the BundleID of a Game
 Usage: CNFilter("com.appdev.appname") { }
 */

#define CNFilter(bundleid) if ((strcmp(([[[NSBundle mainBundle] bundleIdentifier] UTF8String]), bundleid) == 0))

/*
 CNApplyHook applies the hook
 Usage: CNApplyHook(func)
 Example: CNApplyHook(Player$$getMoney);
 */

#define CNApplyHook(func) MSHookFunction((void*)func,(void*)($ ## func),(void**)&_ ## func)

/*
 Sets the Function to the Address.
 Example: CNStub(0x12345, Player$$getMoney);
 Example: CNStub(MSFindSymbol(NULL, "symbol"),Player$$getMoney);
 */

#define CNStub(addr, func) func = (typeof(func))(addr);

/*
 Allows you to hook symbols
 Usage: CNHook(type,function, args);
 Example: CNHook(int, Player$$getMoney, arguments);
 */

#define CNHook(type, func, args...) \
static type (*func)(args); \
static type (*_ ## func)(args); \
static type $ ## func(args)

/*
 CNAddr handles ASLR
 Usage: CNAddr(0x12345)
 */

#define CNAddr(addr) ((CNSlide) + addr)

/*
 CNWrite simply calls writeData
 Usage: CNWrite(0xADDR,0xCODE,SIZE)
 */

#define CNWrite(addr,data) writeData(addr,data)

/*
 CNRead simply calls readData
 Usage: CNRead(0xADDR,0xCODE,SIZE)
 */

#define CNRead(addr) readData(addr)

/*
 Gets the ASLR Slide
 Usage: CNSlide
 */

#define CNSlide (uintptr_t)_dyld_get_image_vmaddr_slide(0)

/*
 Checks for the ARCH in the mach_header
 Usage: CNArch("arch") { }
 */

#define CNArch(arch) \
const struct mach_header *mach = _dyld_get_image_header(0); \
uint32_t arch_num = mach->cpusubtype; \
const char *arch_type = ""; \
if (arch_num == 6) { arch_type = "armv6"; } else if (arch_num == 9) { arch_type = "armv7"; } else if (arch_num == 11) { arch_type = "armv7s"; } \
if ((strcmp((arch), arch_type) == 0))

/*
 CNDeclare declares floatiables used for writing, to hide the offset
 Usage: CNDeclare(name, offset);
 */


/*
 CNAlert shows an UIAlertView for Credits
 Usage: CNAlert("title","message");
 */

#define CNAlert(theTitle,msg) setAlertInfo(theTitle,msg); \
CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), NULL, &didFinishLaunching2,(CFStringRef)UIApplicationDidFinishLaunchingNotification, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

/*
 Get dictionary
 Usage: CNDict(id); / CNDict("com.xxx.yyy");
 */

#define CNDict(id) NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:[NSString stringWithFormat:@"/User/Library/Preferences/%s.plist",id]]

/*
 CNBool gets the BOOL value from the dict returned by CNDict
 Usage: CNBool("kKey") { //do code here, apply a hack }
 */

#define CNBool(key) if ([[dict objectForKey:CNCString(key)] boolValue])

/*
 CNInt gets the int value from the dict returned by CNDict
 Usage: CNInt("kKey") { //do code here, apply a hack }
 */

#define CNInt(key) [[dict objectForKey:CNCString(key)] intValue]

/*
 CNFloat gets the float value from the dict returned by CNDict
 Usage: CNFloat("kKey") { //do code here, apply a hack }
 */

#define CNFloat(key) [[dict objectForKey:CNCString(key)] floatValue]

/*
 CNCString quickly converts a const char* to NSString *
 Usage: CNCString("c string");
 */

#define CNCString(cStr) [NSString stringWithCString:cStr encoding:NSUTF8StringEncoding]

/*
 CNGetClassName(id);
 returns the class name of the id type.
 */
#define CNGetClassName(id) getClassNameFromId(id)




bool isFileExist(char* str)
{
  FILE* file = fopen(str,"r");
  if(file)
  {
   // fclose(file);
    return true;
  }
  return false;
}


char * replaceChar(char * src,char oldChar,char newChar){
char * head=src;
while(*src!='\0'){
if(*src==oldChar) *src=newChar;
src++;
}
return head;
}
FILE* file = NULL;



void createFileHandle(const char* name)
{
  file = fopen(name,"wb+");
}
void writeToFile(const void* buffer,int size)
{
  fwrite(buffer,size,1,file);
}
void closeFileHandle()
{
  fclose(file);
}




