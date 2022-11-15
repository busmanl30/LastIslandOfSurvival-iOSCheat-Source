//
//  ImGuiLoad.m
//  ImGuiTest
//
//  Created by yiming on 2021/6/2.
//

#import "ImGuiLoad.h"
#import "ImGuiDrawView.h"



@interface ImGuiLoad()
@property (nonatomic, strong) ImGuiDrawView *vna;
@end
UIWindow *mainWindow;
@implementation ImGuiLoad

+ (instancetype)share
{
    static ImGuiLoad *tool;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        tool = [[ImGuiLoad alloc] init];
    });
    return tool;
}



- (void)show
{
    if (!_vna) {
        ImGuiDrawView *vc = [[ImGuiDrawView alloc] init];
        _vna = vc;

        
    }

    
    

        [ImGuiDrawView showChange:true];
        [[[UIApplication sharedApplication] delegate].window addSubview:_vna.view];

        //static UITextField *Norecord = [[UITextField alloc]initWithFrame:[UIScreen mainScreen].bounds];
        //[Norecord setSecureTextEntry:YES];
        //Norecord.userInteractionEnabled = NO;
        //[[UIApplication sharedApplication].keyWindow addSubview:Norecord];
        //[[Norecord _textCanvasView] addSubview:self.view];

    
}

- (void)hide
{
    if (!_vna) {
        ImGuiDrawView *vc = [[ImGuiDrawView alloc] init];
        _vna = vc;

        
    }
    
    
    
    [ImGuiDrawView showChange:false];
    [[[UIApplication sharedApplication] delegate].window addSubview:_vna.view];
}

@end
