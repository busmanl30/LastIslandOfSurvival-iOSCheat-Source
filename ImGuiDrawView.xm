#import "x2nios.h"

#import "Esp/ImGuiDrawView.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <Foundation/Foundation.h>
#include "KittyMemory/imgui.h"
#include "KittyMemory/imgui_internal.h"
#include "KittyMemory/imgui_impl_metal.h"
#import <Foundation/Foundation.h>
#import <os/log.h>
#include "Vector3.h"
#include "Quaternion.h"
#include "UnityStuff.h"
#include "Theme.h"
#include "CheatState.hpp"
#include <vector>
#import <dlfcn.h>
#include <map>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <unistd.h>
#include <string.h>
#include <pthread.h>


#define kWidth  [UIScreen mainScreen].bounds.size.width
#define kHeight [UIScreen mainScreen].bounds.size.height
#define kScale [UIScreen mainScreen].scale





@interface ImGuiDrawView () <MTKViewDelegate>
//@property (nonatomic, strong) IBOutlet MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;

@end
@implementation ImGuiDrawView

static bool MenDeal = true;

#define BundlePath @"/Library/PreferenceBundles/Tweak.bundle"
- (instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    if (!self.device) abort();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    NSString *FontPath = @"/System/Library/Fonts/AppFonts/Charter.ttc";
    io.Fonts->AddFontFromFileTTF(FontPath.UTF8String, 40.f,NULL,io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    
    ImGui_ImplMetal_Init(_device);

    return self;
}

+ (void)showChange:(BOOL)open
{
    MenDeal = open;
}

static UITextField *Norecord;


- (MTKView *)mtkView
{
    return (MTKView *)self.view;
}



- (void)loadView
{
    CGFloat w = [UIApplication sharedApplication].windows[0].rootViewController.view.frame.size.width;
    CGFloat h = [UIApplication sharedApplication].windows[0].rootViewController.view.frame.size.height;
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, w, h)];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.mtkView.device = self.device;
    self.mtkView.delegate = self;
    self.mtkView.clearColor = MTLClearColorMake(0, 0, 0, 0);
    self.mtkView.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0 alpha:0];
    self.mtkView.clipsToBounds = YES;
    
}


#pragma mark - Interaction



- (void)updateIOWithTouchEvent:(UIEvent *)event
{
    UITouch *anyTouch = event.allTouches.anyObject;
    CGPoint touchLocation = [anyTouch locationInView:self.view];
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(touchLocation.x, touchLocation.y);

    BOOL hasActiveTouch = NO;
    for (UITouch *touch in event.allTouches)
    {
        if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled)
        {
            hasActiveTouch = YES;
            break;
        }
    }
    io.MouseDown[0] = hasActiveTouch;

   

    
}

 

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self updateIOWithTouchEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self updateIOWithTouchEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self updateIOWithTouchEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [self updateIOWithTouchEvent:event];
}

struct Bounds
{
    Vector3 center;
    Vector3 extents;

    Vector3 min()
    {
        return center - extents;
    }

    Vector3 max()
    {
        return center + extents;
    }
};


void (*get_bounds_Injected)(void *collider, Bounds *out) = (void (*)(void *, Bounds *))getRealOffset(0x2F723C0);

Bounds get_bounds(void *collider)
{
    Bounds boundz;
    get_bounds_Injected(collider, &boundz);
    return boundz;
}


struct me_t
{
    void *third;
    void *player;
    void *trans;
    Vector3 position;
    Vector3 w2sposition;

    void *head;
    Vector3 headposition;
    Vector3 w2sheadposition;

    void *camera;
    void *cameratrans;
    Vector3 camposition;
};
me_t *me;

struct enemy_t
{
    void *object;

    void *trans;
    void *renderer;
    bool isVisible;

    void* roleID;

    Vector3 position;
    Vector3 w2sposition;

    float yaw;

    void *head;
    Vector3 headposition;
    Vector3 w2sheadposition;

    Bounds boundz;
    
};
enemy_t *enemy;
enemy_t *mobs;

void *(*Camera_get_main)()= (void *(*)())getRealOffset(0x2E47BE4);
void (*WorldToViewPoint_Injected)(void *Camera, Vector3 position, int eye, Vector3 *ret) = (void(*)(void *, Vector3, int, Vector3 *))getRealOffset(0x2E476CC);

bool (WorldToScreen)(void *camera, Vector3 position, Vector3 &screen)
{
    WorldToViewPoint_Injected(camera, position, 2, &screen);

    screen.x = screen.x * [[UIScreen mainScreen]bounds].size.width ;
    screen.y = [[UIScreen mainScreen]bounds].size.height - screen.y * [[UIScreen mainScreen]bounds].size.height;

    if(screen.z < 0.01)
        return false;

    return true; 
}

void *(*Component_GetTransform)(void *component) = (void *(*)(void *))getRealOffset(0x2E49B84);

void (*Transform_INTERNAL_get_position)(void *transform, Vector3 *out) = (void (*)(void *, Vector3 *))getRealOffset(0x2EC47E0);


Vector3 GetPlayerLocation(void *player)
{
  Vector3 location;
  Transform_INTERNAL_get_position(Component_GetTransform(player), &location);

  return location;
}


Vector3 GetTransformLocation(void *transform)
{
  Vector3 location;
  Transform_INTERNAL_get_position(transform, &location);

  return location;
}

#pragma mark - MTKViewDelegate
float colors[3] = {255,255,255};
float colors2[3] = {255,255,255};

void RenderLine(const ImVec2& from, const ImVec2& to, ImColor Color, float thickness)
{
ImDrawList* draw_list = ImGui::GetForegroundDrawList();

	draw_list->AddLine(from, to, Color, thickness);
}

void DrawBoneToBone(void *camera, void *bone1, void *bone2, ImColor Color, float thickness)
{
    

    if(bone1 != nullptr && bone2 != nullptr)
    {
        Vector3 boneposition = GetTransformLocation(bone1);
        Vector3 boneposition2 = GetTransformLocation(bone2);

        Vector3 w2sboneposition;
        Vector3 w2sboneposition2;


        if(WorldToScreen(camera, boneposition, w2sboneposition) && WorldToScreen(camera, boneposition2, w2sboneposition2))
        {
            RenderLine(ImVec2(w2sboneposition.x, w2sboneposition.y), ImVec2(w2sboneposition2.x, w2sboneposition2.y), Color, thickness);
        }
    }

    
}


void DrawBox(float X, float Y, float W, float H, ImColor Color, float curve, float thickness)
{
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddRect(ImVec2(X + 1, Y + 1), ImVec2(((X + W) - 1), ((Y + H) - 1)), Color);
	draw_list->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), Color, curve, thickness);
	
}

void DrawText2(float fontSize, ImVec2 position, ImColor Color, const char *text)
{
    
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    draw_list->AddText(NULL, fontSize, position, Color, text);

}

void DrawCircle(float X, float Y, float radius, bool filled, ImColor Color)
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    
    if(filled)
    {
        draw_list->AddCircleFilled(ImVec2(X, Y), radius, Color);
    } 
    else
    {
        draw_list->AddCircle(ImVec2(X, Y), radius, Color);
    }
    
}

Vector3 GetMidPoint(Vector3 V1, Vector3 V2)
{
	Vector3 Mid;
	Mid.x = (V1.x + V2.x) / 2;
	Mid.y = (V1.y + V2.y) / 2;
	Mid.z = (V1.z + V2.z) / 2;
	return Mid;
}




void Draw3DBox2(void* Camera, Vector3 min, Vector3 max, ImColor Color, float thickness) 
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    Vector3 corners[8] =
	{
		Vector3(min.x,min.y,min.z),
		Vector3(min.x,max.y,min.z),
		Vector3(max.x,max.y,min.z),
		Vector3(max.x,min.y,min.z),
		Vector3(min.x,min.y,max.z),
		Vector3(min.x,max.y,max.z),
		Vector3(max.x,max.y,max.z),
		Vector3(max.x,min.y,max.z)
	};


    Vector3 _corners[8];

    for (int i = 0; i <= 7; i++)
    {
        WorldToScreen(Camera, corners[i], _corners[i]);
    }

    for (int i = 1; i <= 4; i++)
	{
        if(_corners[i].x > 0.01 && _corners[i].x < kWidth)
        {
            RenderLine(ImVec2(_corners[i - 1].x, _corners[i - 1].y), ImVec2(_corners[i % 4].x, _corners[i % 4].y), Color, thickness);
            RenderLine(ImVec2(_corners[i - 1].x, _corners[i - 1].y), ImVec2(_corners[i + 3].x, _corners[i + 3].y), Color, thickness);
            RenderLine(ImVec2(_corners[i + 3].x, _corners[i + 3].y), ImVec2(_corners[i % 4 + 4].x, _corners[i % 4 + 4].y), Color, thickness);
        }
        
    }
}

- (void)drawInMTKView:(MTKView*)view
{


 


    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;


    CGFloat framebufferScale = view.window.screen.nativeScale ?: UIScreen.mainScreen.nativeScale;
    io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);
    io.DeltaTime = 1 / float(view.preferredFramesPerSecond ?: 60);
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    
    
   
        static bool show_line = false; 
        if (MenDeal == true) 
        {
            [self.view setUserInteractionEnabled:YES];
        } 
        else if 
        (MenDeal == false) 
        {
           
            [self.view setUserInteractionEnabled:NO];

        }
        
     
            
            
            
    
        setTheme();

        MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
        if (renderPassDescriptor != nil)
        {
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"ImGui Jane"];

            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui::NewFrame();
            
            ImFont* font = ImGui::GetFont();
            font->Scale = 16.f / font->FontSize;
            
            CGFloat x = (([UIApplication sharedApplication].windows[0].rootViewController.view.frame.size.width) - 360) / 2;
            CGFloat y = (([UIApplication sharedApplication].windows[0].rootViewController.view.frame.size.height) - 320) / 2;
            
            ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, 210), ImGuiCond_FirstUseEver);

            
            

            
            if (MenDeal == true)
            {

                
                ImGui::Begin("EzWin-iOS.com Last Island of Survival 2.9.0", &MenDeal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

                ImGui::Checkbox("ESP Lines", &CheatState::EnemyLines);
                ImGui::SameLine();
                ImGui::Checkbox("ESP Skeleton", &CheatState::EnemySkeleton);
                ImGui::SameLine();
                ImGui::Checkbox("ESP Boxes", &CheatState::EnemyBox);
                ImGui::SameLine();
                ImGui::Checkbox("3D ESP Boxes", &CheatState::Enemy3DBox);
                ImGui::SameLine();
                ImGui::ColorEdit3("Color", &*(float*)colors, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Mob ESP Lines", &CheatState::MobLines);
                ImGui::SameLine();
                ImGui::Checkbox("Mob 3D ESP ", &CheatState::Mob3DBox);
                ImGui::SameLine();
                ImGui::ColorEdit3("Color ", &*(float*)colors2, ImGuiColorEditFlags_NoInputs);

                ImGui::End();

    
                
            }

            void *Battle_TypeInfo = *(void **)getRealOffset(0x45958B8);
            if(Battle_TypeInfo != nullptr)
            {
                void *Battle_Static = *(void **)((uint64_t)Battle_TypeInfo + 0xB8);
                
                if(Battle_Static != nullptr)
                {
                    void *Battle_Class = *(void **)((uint64_t)Battle_Static + 0x88);
                    
                    if(Battle_Class != nullptr)
                    {
                        
                        
                        me->player = *(void **)((uint64_t)Battle_Class + 0x20);
                        if(me->player != nullptr)
                        {
                            me->position = GetPlayerLocation(me->player);
                            me->trans = Component_GetTransform(me->player);           
                           
                            me->third = *(void **)((uint64_t)Battle_Class + 0x30);
                            if(me->third != nullptr)
                            {
                                me->camera = *(void **)((uint64_t)me->third + 0x58);
                                me->camposition = GetPlayerLocation(me->third);            
                            }

                            
                        }

                        monoDictionary2<long*, void **> *OtherPlayers = *(monoDictionary2<long*, void **> **)((uint64_t)Battle_Class + 0x68);

                            
                       

                        if(OtherPlayers != nullptr)
                        {
                            for(int i = 0; i < OtherPlayers->getSize(); i++)
                            {
                                enemy->object = OtherPlayers->getValues()[i];

                                if(enemy->object != nullptr)
                                {

                                    monoArray<void **> *Parts = *(monoArray<void **> **)((uint64_t)enemy->object + 0x60);
                                    enemy->trans = Component_GetTransform(enemy->object);
                                    enemy->position = GetPlayerLocation(enemy->object);

                                    enemy->head = *(void **)((uint64_t)enemy->object + 0x80);
                                    if(enemy->head != nullptr)
                                    {
                                        enemy->headposition = GetTransformLocation(enemy->head);
                                        enemy->headposition.y += 0.20; 


                                    } 
                                    
                                    if(me->camera != nullptr)
                                    {

                                        float height =  enemy->w2sposition.y - enemy->w2sheadposition.y;
                                        float width = height / 2.4;

                                        
                                        if(WorldToScreen(me->camera, enemy->position, enemy->w2sposition) && WorldToScreen(me->camera, enemy->headposition, enemy->w2sheadposition))
                                        {
                                            if(CheatState::EnemyBox)
                                            {
                                                DrawBox(enemy->w2sposition.x - (width * 0.5),  enemy->w2sheadposition.y, width, height, ImColor(colors[0], colors[1], colors[2]),0.9, 0.9); 
                                                
                                            }

                                            if(CheatState::EnemyLines)
                                            {
                                                RenderLine(ImVec2(kWidth / 2, kHeight * 0), ImVec2(enemy->w2sposition.x, enemy->w2sheadposition.y), ImColor(colors[0], colors[1], colors[2]), 0.9);
                                            }

                                            if(CheatState::Enemy3DBox)
                                            {
                                                void *Collider = *(void **)((uint64_t)enemy->object + 0x58);
                                                if(Collider != nullptr)
                                                {
                                                    enemy->boundz = get_bounds(Collider);
                                                   // float yaww = *(float *)((uint64_t)enemy->object + 0x288);
                                                    Draw3DBox2(me->camera, enemy->boundz.min(), enemy->boundz.max(), ImColor(colors[0], colors[1], colors[2]), 0.9);
                                                }    
                                            }

                                            if(CheatState::EnemySkeleton)
                                            {
                                                if(Parts != nullptr)
                                                {
                                                    
                                                    DrawBoneToBone(me->camera, Parts->getPointer()[15], Parts->getPointer()[2],  ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[2], Parts->getPointer()[0], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[0], Parts->getPointer()[16], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[14], Parts->getPointer()[3], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[3], Parts->getPointer()[1], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[1], Parts->getPointer()[16], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[16], Parts->getPointer()[8], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[8], Parts->getPointer()[11], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[11], Parts->getPointer()[4], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[4], Parts->getPointer()[6], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[6], Parts->getPointer()[12], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[11], Parts->getPointer()[5], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[5], Parts->getPointer()[7], ImColor(colors[0], colors[1], colors[2]),0.9);

                                                    DrawBoneToBone(me->camera, Parts->getPointer()[7], Parts->getPointer()[13], ImColor(colors[0], colors[1], colors[2]),0.9);
                                                }
                                        
                                    
                                                
                                            }

                                            
                                                 
                                        }

                                        
                                        
                                    }
                                    
                                }
                            }    
                        }   
                    }
                }
            }

            void *MonsterMgr_TypeInfo = *(void **)getRealOffset(0x45CDDE0);

            if(MonsterMgr_TypeInfo != nullptr)
            {
                void *MonsterMgr_Static = *(void **)((uint64_t)MonsterMgr_TypeInfo + 0xB8);
                if(MonsterMgr_Static != nullptr)
                {
                    void *MonsterMgr_Class = *(void **)((uint64_t)MonsterMgr_Static + 0x38);
                    if(MonsterMgr_Class != nullptr)
                    {
                        monoDictionary2<long*, void **> *MonsterDic = *(monoDictionary2<long*, void **> **)((uint64_t)MonsterMgr_Class + 0x18);
                        if(MonsterDic != nullptr)
                        {
                            for(int i = 0; i < MonsterDic->getSize(); i++)
                            {
                                mobs->object = MonsterDic->getValues()[i];

                                if(mobs->object != nullptr)
                                {
                                    mobs->position = GetPlayerLocation(mobs->object);
                                    
                                    if(me->camera != nullptr)
                                    {
                                        Vector3 w2spos; 
                                        if(WorldToScreen(me->camera, mobs->position, w2spos))
                                        {
                                            if(CheatState::MobLines)
                                            {
                                                RenderLine(ImVec2(kWidth / 2, kHeight * 0), ImVec2(w2spos.x, w2spos.y), ImColor(colors2[0], colors2[1], colors2[2]), 0.9);
                                            }

                                            if(CheatState::Mob3DBox)
                                            {
                                                void *Collider = *(void **)((uint64_t)mobs->object + 0x128);
                                                if(Collider != nullptr)
                                                {
                                                    Bounds boundz = get_bounds(Collider);
                                                    Draw3DBox2(me->camera, boundz.min(), boundz.max(), ImColor(colors2[0], colors2[1], colors2[2]), 0.9);

                                                }  
                                            } 
                                        }
                                        
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ImDrawList* draw_list = ImGui::GetForegroundDrawList();

            
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:view.currentDrawable];
            
        }
        

        [commandBuffer commit];
          
}






- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    
}

@end






%ctor 
{
    me = new me_t();
    enemy = new enemy_t();
    mobs = new enemy_t();
   

    patchOffset2(0x210C8, 0xC0035FD6);

    patchOffset2(0x20B88, 0xC0035FD6);

    patchOffset2(0x37DD8, 0xC0035FD6);
    
}
