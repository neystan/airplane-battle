#include <iostream>
#include <easyx.h>
#include <vector>
#include <stdlib.h>
#include <conio.h>
#include "time.h"
#include "tool.hpp"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

using namespace std;

//界面大小
const int WIDTH = 1080;
const int HEIGHT = 1280;

//游戏界面
bool is_welcome = true;
bool is_play = false;
bool is_over = false;
//击杀数
unsigned long long kill = 0;

//英雄、敌机子弹发射频率
int fps_hero_bullet = 1;
int fps_enemy_bullet = 1;

//英雄子弹的种类 
int kind_bullet = 0;

//buff持续时间
unsigned long long time_buff1 = 0;
unsigned long long time_buff2 = 0;

//判断鼠标点击位置是否在矩形内
bool lclik_rect(int x,int y,RECT rect)
{
    return (x >= rect.left && x <= rect.right && y>= rect.top && y <= rect.bottom); 
}

void is_pause()
{
    // 非阻塞检查
    if (kbhit()) 
    {          
        char ch = getch();  // 获取按键
        if(ch == 0x20)
        {
            IMAGE resume;
            loadimage(&resume,_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\resume_pressed.png"),resume.getwidth(),resume.getheight());
            LPCTSTR pause1 = _T("空格键继续游戏");
            LPCTSTR pause2 = _T("R键重新游戏");
            LPCTSTR pause3 = _T("ESC键退出游戏");                
            settextstyle(40,0,_T("黑体"));
            settextcolor(BLACK);
            outtextxy(WIDTH / 2 - textwidth(pause1) / 2, HEIGHT - HEIGHT * 0.42, pause1);
            outtextxy(WIDTH / 2 - textwidth(pause2) / 2, HEIGHT  - HEIGHT * 0.36, pause2);
            outtextxy(WIDTH / 2 - textwidth(pause3) / 2, HEIGHT - HEIGHT * 0.30, pause3);
            transparentimage(WIDTH / 2 - resume.getwidth() / 2,HEIGHT / 2 - resume.getheight() / 2,resume);
            //刷新显示
            FlushBatchDraw();
            Sleep(100);
            while(1)
            { 
                if(kbhit())
                {
                    char ch = getch();  

                    if(ch == 0x20)    
                        break;
                    else if(ch == 0x52 || ch == 0x52 + 32)    
                    {
                        is_welcome = false;
                        is_play = false;
                        break;
                    }
                    else if(ch == 0x1B)    
                    {
                        is_play = false;
                        is_welcome = true;
                        break;
                    }
                }
            }
        }
    }   
}

//敌机生成重叠问题
bool enemys_check(const RECT &rect1,const RECT &rect2)
{   
    //敌机左上角、右上角、左下角、右下角
    bool crash1 = (rect2.left >= rect1.left && rect2.left <= rect1.right) 
    && (rect2.top >= rect1.top && rect2.top <= rect1.bottom);

    bool crash2 = (rect2.right >= rect1.left && rect2.right <= rect1.right) 
    && (rect2.top >= rect1.top && rect2.top <= rect1.bottom);

    bool crash3 = (rect2.left >= rect1.left && rect2.left <= rect1.right) 
    && (rect2.bottom >= rect1.top && rect2.bottom <= rect1.bottom);

    bool crash4 = (rect2.right >= rect1.left && rect2.right <= rect1.right) 
    && (rect2.bottom >= rect1.top && rect2.bottom <= rect1.bottom);
    return (crash1 || crash2 || crash3 || crash4);
}

//碰撞问题（这里默认1比2大）(条件宽松点)
bool crash_check(const RECT &rect1,const RECT &rect2)
{   
    int threshold = 6;
    //敌机左上角、右上角、左下角、右下角(条件宽松点)
    bool crash1 = (rect2.left - threshold >= rect1.left && rect2.left + threshold <= rect1.right) 
    && (rect2.top - threshold >= rect1.top && rect2.top + threshold <= rect1.bottom);

    bool crash2 = (rect2.right - threshold >= rect1.left && rect2.right + threshold <= rect1.right) 
    && (rect2.top - threshold >= rect1.top && rect2.top + threshold <= rect1.bottom);

    bool crash3 = (rect2.left - threshold >= rect1.left && rect2.left + threshold <= rect1.right) 
    && (rect2.bottom - threshold >= rect1.top && rect2.bottom + threshold <= rect1.bottom);

    bool crash4 = (rect2.right - threshold >= rect1.left && rect2.right + threshold <= rect1.right) 
    && (rect2.bottom - threshold >= rect1.top && rect2.bottom + threshold <= rect1.bottom);
    return (crash1 || crash2 || crash3 || crash4);
}

//背景、英雄、敌机、子弹、buff
class BG
{
private:
    IMAGE img;
    int y;
public:
    BG(IMAGE img) : img(img), y(-HEIGHT) {}

    void Show()
    {
        if(y == 0)  y = -HEIGHT;
        y += 2;
        putimage(0,y,&img);
    }
};

class HERO
{
private:
    IMAGE &img;
    RECT rect;
    int cnt_bullet_hero;
    int health_bar;
public:
    HERO(IMAGE &img) : img(img), cnt_bullet_hero(0), health_bar(3)
    {
        rect.left = WIDTH / 2 - img.getwidth() / 2;
        rect.right = WIDTH / 2 + img.getwidth() / 2;
        rect.top = HEIGHT * 0.8;
        rect.bottom = HEIGHT * 0.8 + img.getheight();
    }

    void Show()
    {
        Control();
        cnt_bullet_hero++;
        cnt_bullet_hero %= 801;
        transparentimage(rect.left,rect.top,img);
    }

    void Control()
    {
        ExMessage msg;
        if(peekmessage(&msg,EX_MOUSE,true))
        {
            rect.left = msg.x - img.getwidth() / 2;
            rect.right = msg.x + img.getwidth() / 2;
            rect.top = msg.y - img.getheight() / 2;
            rect.bottom = msg.y + img.getheight() / 2;
        }
    }

    int ReduceHealthBar()
    {
        return --health_bar;
    }

    void AddHealthBar()
    {
        if(health_bar < 3)
            health_bar++;
    }

    RECT GetRect() const
    {
        return rect;   
    }

    int GetBulletCnt() const
    {
        return cnt_bullet_hero;
    }

    int GetHealthBar() const
    {
        return health_bar;
    }
};

class ENEMY
{
protected:
    IMAGE img;
    RECT rect;
    int x;
    int cnt_bullet_enemy;
public:
    ENEMY(IMAGE img,int x) : img(img), x(x), cnt_bullet_enemy(0)
    {
        rect.left = x;
        rect.right = x + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }

    //单个敌机
    bool Show()
    {
        //敌机飞出屏幕
        if(rect.top >= HEIGHT)
            return false;
        transparentimage(rect.left,rect.top,img);
        rect.top += 6;
        rect.bottom = rect.top + img.getheight();
        cnt_bullet_enemy++;
        cnt_bullet_enemy %= 801;
        return true;
    }

    RECT GetRect() const
    {
        return rect;
    }

    int GetBulletCnt() const
    {
        return cnt_bullet_enemy;
    }
};

class ENEMYBOSS : public ENEMY
{
protected:
    int health_bar;
public:
    ENEMYBOSS(IMAGE img,int x) : ENEMY(img, x), health_bar(5)
    {
        rect.left = x;
        rect.right = x + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }

    //单个敌机
    bool Show()
    {
        //敌机飞出屏幕
        if(rect.top >= HEIGHT)
            return false;
        transparentimage(rect.left,rect.top,img);
        rect.top += 2;
        rect.bottom = rect.top + img.getheight();
        cnt_bullet_enemy++;
        cnt_bullet_enemy %= 151;
        return true;
    }

    int ReduceHealthBar()
    {
        return --health_bar;
    }

    int GetHealthBar() const
    {
        return health_bar;
    }
};

class ENEMYBIGBOSS : public ENEMYBOSS
{
public:
    ENEMYBIGBOSS(IMAGE img,int x) : ENEMYBOSS(img, x)
    {
        health_bar = 30;
        rect.left = x;
        rect.right = x + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }

    //单个敌机
    bool Show()
    {
        //敌机飞出屏幕
        if(rect.top >= HEIGHT)
            return false;
        transparentimage(rect.left,rect.top,img);
        rect.top += 1;
        rect.bottom = rect.top + img.getheight();
        cnt_bullet_enemy++;
        cnt_bullet_enemy %= 151;
        return true;
    }

    RECT GetRectLeft() 
    {
        rect.left -= 80;
        rect.right = rect.left + img.getwidth();
        return rect;
    }

    RECT GetRectRight()
    {
        rect.left += 160;
        rect.right = rect.left + img.getwidth();
        return rect;
    }
};

class BULLET
{
protected:
    IMAGE *img;
    RECT rect;
public:
    BULLET(IMAGE *img,RECT rect_hero) : img(img)
    {      
        rect.left = (rect_hero.left + rect_hero.right) / 2 - img->getwidth() / 2;
        rect.right = rect.left + img->getwidth();
        rect.top = rect_hero.top - img->getheight();
        rect.bottom = rect_hero.top;
    }

    bool Show()
    {
        if(rect.bottom <= 0)
            return false;
        transparentimage(rect.left,rect.top,*img);
        rect.top -= 6;
        rect.bottom = rect.top + img->getheight();
        return true;
    }

    RECT GetRect() const
    {
        return rect;
    }
}; 

class EBULLET : public BULLET
{
public:
    EBULLET(IMAGE *img,RECT rect_enemy) : BULLET(img,rect_enemy)
    {      
        rect.left = (rect_enemy.left + rect_enemy.right) / 2 - img->getwidth() / 2;
        rect.right = rect.left + img->getwidth();
        rect.top = rect_enemy.bottom;
        rect.bottom = rect_enemy.bottom + img->getheight();
    }

    bool Show()
    {
        if(rect.top >= HEIGHT)
            return false;
        transparentimage(rect.left,rect.top,*img);
        rect.top += 8;
        rect.bottom = rect.top + img->getheight();
        return true;
    }
};

class BUFF
{
private:
    IMAGE img;
    RECT rect;
    int x;
    int num_buff;
public:
    BUFF(IMAGE img,int x) : img(img), x(x), num_buff(0)
    {
        rect.left = x;
        rect.right = x + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }

    bool Show()
    {
        //BUFF飞出屏幕
        if(rect.top >= HEIGHT)
            return false;
        transparentimage(rect.left,rect.top,img);
        rect.top += 5;
        rect.bottom = rect.top + img.getheight();
        return true;
    }

    //确定是哪种buff
    void SetNumBuff(int num)
    {
        num_buff = num;
    }

    int GetNumbuff() const
    {
        return num_buff;
    }

    RECT GetRect() const
    {
        return rect;
    }
};

//英雄飞机死亡动画
void Hero_death_animation(HERO &hero,IMAGE heroimg[])
{
    transparentimage(hero.GetRect().left,hero.GetRect().top,heroimg[1]);
    FlushBatchDraw();
}

//敌机死亡动画
void Enemy_death_animation(ENEMY &enemy,IMAGE enemyimg[])
{
    transparentimage(enemy.GetRect().left,enemy.GetRect().top,enemyimg[1]);
    FlushBatchDraw();
}

//敌机boss死亡动画
void Enemyboss_death_animation(ENEMYBOSS &enemyboss,IMAGE enemybossimg[])
{
    transparentimage(enemyboss.GetRect().left,enemyboss.GetRect().top,enemybossimg[1]);
    FlushBatchDraw();
}

//敌机bigboss死亡动画
void Enemybigboss_death_animation(ENEMYBIGBOSS &enemybigboss,IMAGE enemybigbossimg[])
{
    transparentimage(enemybigboss.GetRect().left,enemybigboss.GetRect().top,enemybigbossimg[1]);
    FlushBatchDraw();
}

//生成互不重叠的敌机或者buff
void Add_enemyorbuff(vector<ENEMY> &enemys,vector<ENEMYBOSS> &enemyboss,vector<ENEMYBIGBOSS> &enemybigboss,vector<BUFF> &buffs,IMAGE img[],int flag_kind)
{
    while(1)
    {
        bool flag_crash = false;
        if(flag_kind == 0)          //敌机
        {
            auto e = ENEMY(img[0],rand() % (WIDTH - img[0].getwidth()));

            for(const auto &v : enemys)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemyboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemybigboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : buffs)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            if(!flag_crash)
            {
                enemys.push_back(e);
                break;
            }
        }
        else if(flag_kind == 1)     //boss
        {   
            auto e = ENEMYBOSS(img[0],rand() % (WIDTH - img[0].getwidth()));

            for(const auto &v : enemys)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemyboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemybigboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }            
            for(const auto &v : buffs)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }         
            if(!flag_crash)
            {
                enemyboss.push_back(e);
                break;
            }
        }
        else if(flag_kind == 2)         //bigboss
        {
            auto e = ENEMYBIGBOSS(img[0],rand() % (WIDTH - img[0].getwidth()));

            for(const auto &v : enemys)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemyboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemybigboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }            
            for(const auto &v : buffs)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            if(!flag_crash)
            {
                enemybigboss.push_back(e);
                break;
            }
        }
        else
        {
            int cnt_rand = rand() % 3;
            auto e = BUFF(img[cnt_rand],rand() % (WIDTH - img[cnt_rand].getwidth()));

            for(const auto &v : enemys)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemyboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : enemybigboss)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            for(const auto &v : buffs)
            {
                if(enemys_check(v.GetRect(),e.GetRect()))
                {
                    flag_crash = true;
                    break;
                }
            }
            if(!flag_crash)
            {
                e.SetNumBuff(cnt_rand);
                buffs.push_back(e);
                break;
            }
        }
    }
}

//生成敌机并检查是否碰撞
void Show_enemys(vector<ENEMY> &enemys,vector<ENEMYBOSS> &enemyboss,vector<ENEMYBIGBOSS> &enemybigboss,vector<BUFF> &buffs,HERO &hero,IMAGE heroimg[],IMAGE enemyimg[],IMAGE enemybossimg[],IMAGE enemybigbossimg[])
{
    //随机生成
    int num_rand1 = rand() % 30;
    if(num_rand1 == 6)
        Add_enemyorbuff(enemys,enemyboss,enemybigboss,buffs,enemyimg,0);
    
    int num_rand2 = rand() % 600;
    if(num_rand2 == 6)
        Add_enemyorbuff(enemys,enemyboss,enemybigboss,buffs,enemybossimg,1);
    
    //总击杀数50，召唤bigboss
    if(kill == 10 && enemybigboss.size() == 0)
        Add_enemyorbuff(enemys,enemyboss,enemybigboss,buffs,enemybigbossimg,2);

    for(int i = 0;i < enemys.size();i++)
    {
        //敌机出界
        if(!enemys[i].Show())   
        {
            enemys.erase(enemys.begin() + i--);
        }
        
        //英雄与敌机碰撞
        if(crash_check(hero.GetRect(),enemys[i].GetRect()))
        {
            if(hero.ReduceHealthBar() == 0)
            {
                Hero_death_animation(hero,heroimg);
                is_welcome = false;
                is_play = false;
                is_over = true;
            }
            Enemy_death_animation(enemys[i],enemyimg);    
            enemys.erase(enemys.begin() + i--);
        }
    }

    for(int i = 0;i < enemyboss.size();i++)
    {
        //敌机boos出界
        if(!enemyboss[i].Show())   
        {
            enemyboss.erase(enemyboss.begin() + i--);
        }
        
        //英雄与敌机boos碰撞
        if(crash_check(hero.GetRect(),enemyboss[i].GetRect()))
        {
            if(hero.ReduceHealthBar() == 0)
            {
                Hero_death_animation(hero,heroimg);
                is_welcome = false;
                is_play = false;
                is_over = true;
            }
            if(enemyboss[i].ReduceHealthBar() == 0)
            {
                Enemyboss_death_animation(enemyboss[i],enemybossimg);   
                enemyboss.erase(enemyboss.begin() + i--);
            }            
        }
    }

    for(int i = 0;i < enemybigboss.size();i++)
    {
        //敌机bigboos出界
        if(!enemybigboss[i].Show())   
        {
            enemybigboss.erase(enemybigboss.begin() + i--);
        }
        
        //英雄与敌机bigboos碰撞
        if(crash_check(hero.GetRect(),enemybigboss[i].GetRect()))
        {
            Hero_death_animation(hero,heroimg);
            is_welcome = false;
            is_play = false;
            is_over = true;
        }
    }
}

//处理子弹发射频率和子弹碰撞问题
void Show_bullets(vector<BULLET> &bullets,vector<EBULLET> &ebullets,vector<ENEMY> &enemys,vector<ENEMYBOSS> &enemyboss,vector<ENEMYBIGBOSS> &enemybigboss,HERO &hero,IMAGE heroimg[],IMAGE enemyimg[],IMAGE enemybossimg[],IMAGE enemybigbossimg[],IMAGE bulletimg[],IMAGE ebulletimg[])
{
    //英雄、敌机、敌机boos、bigboss每隔一段时间发射子弹
    if(hero.GetBulletCnt() % (16 / fps_hero_bullet) == 0)
        bullets.push_back(BULLET(&bulletimg[kind_bullet],hero.GetRect()));

    for(int i = 0;i < enemys.size();i++)
    {
        if(enemys[i].GetBulletCnt() % (160 / (fps_enemy_bullet + 1)) == 0)
            ebullets.push_back(EBULLET(&ebulletimg[0],enemys[i].GetRect()));
    }

    for(int i = 0;i < enemyboss.size();i++)
    {
        if(enemyboss[i].GetBulletCnt() % 30 == 0 && enemyboss[i].GetBulletCnt() <= 90)
            ebullets.push_back(EBULLET(&ebulletimg[1],enemyboss[i].GetRect()));
    }

    for(int i = 0;i < enemybigboss.size();i++)
    {
        if(enemybigboss[i].GetBulletCnt() % 30 == 0 && enemybigboss[i].GetBulletCnt() <= 90)
        {
            ebullets.push_back(EBULLET(&ebulletimg[1],enemybigboss[i].GetRectLeft()));
            ebullets.push_back(EBULLET(&ebulletimg[1],enemybigboss[i].GetRectRight()));   
            ebullets.push_back(EBULLET(&ebulletimg[1],enemybigboss[i].GetRectLeft()));
        }
    }

    //销毁英雄出界子弹
    for(int i = 0;i < bullets.size();i++)
    {
        if(!bullets[i].Show())
        {
            bullets.erase(bullets.begin() + i--);
        }
    }
    //销毁敌机、敌机boss、bigboss出界子弹
    for(int j = 0;j < ebullets.size();j++)
    {
        if(!ebullets[j].Show())
        {
            ebullets.erase(ebullets.begin() + j--);
        }
    }

    //敌机子弹与英雄碰撞检测
    for(int j = 0;j < ebullets.size();j++)
    {
        if(crash_check(hero.GetRect(),ebullets[j].GetRect()))
        {
            if(hero.ReduceHealthBar() == 0)
            {      
                Hero_death_animation(hero,heroimg);    
                is_welcome = false;
                is_play = false;
                is_over = true;
            }
            ebullets.erase(ebullets.begin() + j--);
        }
    }
    //英雄子弹与敌机碰撞检测
    for(int i = 0;i < enemys.size();i++)
    {
        for(int j = 0;j < bullets.size();j++)
        {
            if(crash_check(enemys[i].GetRect(),bullets[j].GetRect()))
            {
                kill++;
                Enemy_death_animation(enemys[i],enemyimg);    
                enemys.erase(enemys.begin() + i--);
                bullets.erase(bullets.begin() + j--);                
            }
        }
    }
    //英雄子弹与敌机boss碰撞检测    
    for(int i = 0;i < enemyboss.size();i++)
    {
        for(int j = 0;j < bullets.size();j++)
        {
            if(crash_check(enemyboss[i].GetRect(),bullets[j].GetRect()))
            {
                bullets.erase(bullets.begin() + j--);     
                if(enemyboss[i].ReduceHealthBar() == 0)
                {
                    kill++;
                    Enemyboss_death_animation(enemyboss[i],enemybossimg);   
                    enemyboss.erase(enemyboss.begin() + i--);
                }            
            }
        }
    }
    //英雄子弹与敌机bigboss碰撞检测 
    for(int i = 0;i < enemybigboss.size();i++)
    {
        for(int j = 0;j < bullets.size();j++)
        {
            if(crash_check(enemybigboss[i].GetRect(),bullets[j].GetRect()))
            {
                bullets.erase(bullets.begin() + j--);     
                if(enemybigboss[i].ReduceHealthBar() == 0)
                {
                    kill++;
                    Enemybigboss_death_animation(enemybigboss[i],enemybigbossimg);   
                    enemybigboss.erase(enemybigboss.begin() + i--);
                }            
            }
        }
    }}

//处理buff生成和获取问题
void Show_buff(vector<BUFF> &buffs,vector<ENEMY> &enemys,vector<ENEMYBOSS> &enemyboss,vector<ENEMYBIGBOSS> &enemybigboss,HERO &hero,IMAGE buffimg[])
{
    unsigned long long now_tick = GetTickCount();
    
    //随机生成
    int num_rand = rand() % 200;
    if(num_rand == 6)
        Add_enemyorbuff(enemys,enemyboss,enemybigboss,buffs,buffimg,3);

    for(int i = 0;i < buffs.size();i++)
    {
        //buff出界
        if(!buffs[i].Show())   
        {
            buffs.erase(buffs.begin() + i--);
        }
        //英雄获取buff
        if(crash_check(hero.GetRect(),buffs[i].GetRect()))
        {   
            //血包、子弹包、炸弹包
            if(buffs[i].GetNumbuff() == 0)
                hero.AddHealthBar();
            else if(buffs[i].GetNumbuff() == 1)
            {
                fps_hero_bullet = 3;
                time_buff1 = now_tick;
            }
            else if(buffs[i].GetNumbuff() == 2)
            {
                kind_bullet = 1;
                time_buff2 = now_tick;
            }
            
            buffs.erase(buffs.begin() + i--);
        }
    }
    if(time_buff1 != 0 && now_tick - time_buff1 >= 10000)
    {
        fps_hero_bullet = 1;
        time_buff1 = 0;
    }
    if(time_buff2 != 0 && now_tick - time_buff2 >= 10000)
    {
        kind_bullet = 0;
        time_buff2 = 0;
    }
}

//显示帧率与得分
void Show_fps_score_health(HERO &hero,vector<ENEMYBOSS> &enemyboss,vector<ENEMYBIGBOSS> &enemybigboss)
{
    static DWORD last_tick = GetTickCount();
    static int frames = 0; 
    static int fps = 0;

    DWORD now = GetTickCount();
    frames++;
    if(now - last_tick >= 1000)
    {
        fps = frames * 1000 / (int)(now - last_tick);
        frames = 0;
        last_tick = now;
    }

    TCHAR text_fps[32];
    TCHAR text_score[32];
    TCHAR text_healthbar[32];
    _stprintf(text_fps,_T("FPS:%d"),fps);
    _stprintf(text_score,_T("Score:%d"),kill);

    settextstyle(28,0,_T("Consolas"));
    settextcolor(BLACK);

    //透明背景
    setbkmode(TRANSPARENT);    

    outtextxy(10,10,text_fps);
    outtextxy(WIDTH - 140,10,text_score);
    
    //绘制英雄、敌机boss、bigboss血条
    setlinecolor(RED);
    setlinestyle(PS_SOLID,5);
    if(hero.GetHealthBar() != 0)
        line(hero.GetRect().left, hero.GetRect().bottom - 10, hero.GetRect().left + (hero.GetRect().right - hero.GetRect().left) * hero.GetHealthBar() / 3, hero.GetRect().bottom - 10);
    setlinecolor(RED);    
    for(int i = 0;i < enemyboss.size();i++)
    {
        if(enemyboss[i].GetHealthBar() != 0)
            line(enemyboss[i].GetRect().left, enemyboss[i].GetRect().top - 10, enemyboss[i].GetRect().left + (enemyboss[i].GetRect().right - enemyboss[i].GetRect().left) * enemyboss[i].GetHealthBar() / 5, enemyboss[i].GetRect().top - 10);
    }
    for(int i = 0;i < enemybigboss.size();i++)
    {
        if(enemybigboss[i].GetHealthBar() != 0)
            line(WIDTH - (enemybigboss[i].GetHealthBar() * WIDTH / 30) , 16, WIDTH, 16);
    }        

    if(time_buff1 != 0)
    {
        unsigned long long now_tick = GetTickCount();
        setlinecolor(BLUE);
        setlinestyle(PS_SOLID,8);
        line((now_tick - time_buff1) * WIDTH / 10000, HEIGHT - 6, WIDTH, HEIGHT - 6);
    }
    if(time_buff2 != 0)
    {
        unsigned long long now_tick = GetTickCount();
        setlinecolor(RED);
        setlinestyle(PS_SOLID,8);
        if(time_buff1 != 0)
            line((now_tick - time_buff2) * WIDTH / 10000, HEIGHT - 16, WIDTH, HEIGHT - 16);
        else
            line((now_tick - time_buff2) * WIDTH / 10000, HEIGHT - 6, WIDTH, HEIGHT - 6);
    }
}

//开始界面
void Welcome()
{
    LPCTSTR title = _T("飞机大战");
    LPCTSTR tstart = _T("开始游戏");
    LPCTSTR texit = _T("退出游戏");
    
    setbkcolor(WHITE);
    cleardevice();      // 用背景色清空屏幕
    settextstyle(80,0,_T("黑体"));
    settextcolor(BLACK);
    outtextxy(WIDTH / 2 - textwidth(title) / 2, HEIGHT - HEIGHT * 0.85, title);

    settextstyle(60,0,_T("微软雅黑"));
    RECT rect_play,rect_exit;
    rect_play.left = WIDTH / 2 - textwidth(title) / 2;
    rect_play.right = WIDTH / 2 + textwidth(title) / 2;
    rect_play.top = HEIGHT - HEIGHT * 0.55;
    rect_play.bottom = HEIGHT - HEIGHT * 0.55 + textheight(title);

    rect_exit.left = WIDTH / 2 - textwidth(title) / 2;
    rect_exit.right = WIDTH / 2 + textwidth(title) / 2;
    rect_exit.top = HEIGHT - HEIGHT * 0.45;
    rect_exit.bottom = HEIGHT - HEIGHT * 0.45 + textheight(title);

    outtextxy(rect_play.left, rect_play.top, tstart);
    outtextxy(rect_exit.left, rect_exit.top, texit);

    is_play = true;

    while(is_welcome)
    {
        ExMessage meg;
        getmessage(&meg,EX_MOUSE);
        if(meg.lbutton)
        {
            if(lclik_rect(meg.x,meg.y,rect_play))
                return;
            else if(lclik_rect(meg.x,meg.y,rect_exit))
                exit(0);
        }
    }
}

//游戏界面
void Play()
{
    setbkcolor(WHITE);
    cleardevice();      // 用背景色清空屏幕

    //图片素材加载
    IMAGE heroimg[2],enemyimg[2],enemybossimg[2],enemybigbossimg[2],bgimg,bulletimg[2],ebulletimg[2],buffimg[3]; 
    loadimage(&heroimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\me1.png"),heroimg[0].getwidth(),heroimg[0].getheight());
    loadimage(&heroimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\me_destroy_3.png"),heroimg[1].getwidth(),heroimg[1].getheight());
    loadimage(&enemyimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy1.png"),enemyimg[0].getwidth(),enemyimg[0].getheight());
    loadimage(&enemyimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy1_down3.png"),enemyimg[1].getwidth(),enemyimg[1].getheight());
    loadimage(&enemybossimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy2.png"),enemybossimg[0].getwidth(),enemybossimg[0].getheight());
    loadimage(&enemybossimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy2_down3.png"),enemybossimg[1].getwidth(),enemybossimg[1].getheight());
    loadimage(&enemybigbossimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy3_n1.png"),enemybigbossimg[0].getwidth(),enemybigbossimg[0].getheight());
    loadimage(&enemybigbossimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\enemy3_down5.png"),enemybigbossimg[1].getwidth(),enemybigbossimg[1].getheight());
    loadimage(&bgimg,_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bk.png"),WIDTH,HEIGHT * 2);
    loadimage(&bulletimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bullet2.png"),bulletimg[0].getwidth(),bulletimg[0].getheight());
    loadimage(&bulletimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bomb.png"),bulletimg[1].getwidth(),bulletimg[1].getheight());
    loadimage(&ebulletimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bullet1.png"),ebulletimg[0].getwidth(),ebulletimg[0].getheight());
    loadimage(&ebulletimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bomb1.png"),ebulletimg[1].getwidth(),ebulletimg[1].getheight());
    loadimage(&buffimg[0],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\hp_supply.png"),buffimg[0].getwidth(),buffimg[0].getheight());
    loadimage(&buffimg[1],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bullet_supply.png"),buffimg[1].getwidth(),buffimg[1].getheight());
    loadimage(&buffimg[2],_T("X:\\download\\c++learn\\c++project\\airplane_battle\\images\\bomb_supply.png"),buffimg[2].getwidth(),buffimg[2].getheight());

    srand(time(0));
    BG bg(bgimg);
    HERO hero(heroimg[0]);
    vector<ENEMY> enemys;
    vector<ENEMYBOSS> enemyboss;  
    vector<ENEMYBIGBOSS> enemybigboss;  
    vector<BULLET> bullets;
    vector<EBULLET> ebullets;  
    vector<BUFF> buffs;  

    while(is_play)
    {
        BeginBatchDraw();
        
        bg.Show();
        Sleep(1);
        flushmessage();
        Sleep(24);
        hero.Show();
        Show_enemys(enemys,enemyboss,enemybigboss,buffs,hero,heroimg,enemyimg,enemybossimg,enemybigbossimg);
        Show_buff(buffs,enemys,enemyboss,enemybigboss,hero,buffimg); 
        Show_bullets(bullets,ebullets,enemys,enemyboss,enemybigboss,hero,heroimg,enemyimg,enemybossimg,enemybigbossimg,bulletimg,ebulletimg);  
        Show_fps_score_health(hero,enemyboss,enemybigboss);
        is_pause();      
        
        EndBatchDraw();
    }
}

//结束界面
void Over(unsigned long long &kill)
{
    LPCTSTR over1 = _T("游戏结束，您的得分为:");
    LPCTSTR over2 = _T("R键重新游戏");
    LPCTSTR over3 = _T("ESC键退出游戏");                
    TCHAR s[6];
    _stprintf(s, _T("%llu"), kill);		
    kill = 0;
    time_buff1 = 1;
    time_buff2 = 1;

    settextstyle(50,0,_T("黑体"));
    settextcolor(BLACK);
    outtextxy(WIDTH / 2 - textwidth(over1) / 2 - 30, HEIGHT - HEIGHT * 0.55, over1);
    outtextxy(WIDTH / 2 + textwidth(over1) / 2 - 30, HEIGHT - HEIGHT * 0.55, s);
    settextstyle(40,0,_T("黑体"));
    outtextxy(WIDTH / 2 - textwidth(over2) / 2, HEIGHT - HEIGHT * 0.47, over2);
    outtextxy(WIDTH / 2 - textwidth(over3) / 2, HEIGHT - HEIGHT * 0.41, over3);
    //刷新显示
    while(is_over)
    {
        FlushBatchDraw();
        if(kbhit())
        {
            char ch = getch();  

            if(ch == 0x52 || ch == 0x52 + 32)    
            {
                is_welcome = false;
                is_play = false;
                is_over = false;
                break;
            }
            else if(ch == 0x1B)    
            {
                is_play = false;
                is_welcome = true;
                is_over = false;
                break;
            }
        }
    }
}

int main()
{
    TCHAR fullPath[] = _T("x:\\download\\c++learn\\c++project\\airplane_battle\\music\\game_music.mp3");
    TCHAR shortPath[MAX_PATH];
    GetShortPathName(fullPath, shortPath, MAX_PATH);

    TCHAR mciCmd[512];
    // 构造命令：open "XP~1.MP3" type mpegvideo alias bgm
    _stprintf(mciCmd, _T("open \"%s\" type mpegvideo alias bgm"), shortPath);

    mciSendString(_T("close bgm"), NULL, 0, NULL);
    mciSendString(mciCmd, NULL, 0, NULL);
    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);

    initgraph(WIDTH,HEIGHT);
    bool is_live = true;
    while(is_live)
    {
        Welcome();
        Play();
        Over(kill);
    }
    mciSendString(_T("close bgm"), NULL, 0, NULL);
    return 0;
}