#include<graphics.h>
#include<conio.h>
#include<iostream>
using namespace std;

void transparentimage(int x,int y,IMAGE img);

void transparentimage(int x,int y,IMAGE img)
{
	IMAGE img1;
	DWORD *d1;
	img1=img;
	d1=GetImageBuffer(&img1);
	float h,s,l;
	for(int i=0;i<img1.getheight()*img1.getwidth();i++){
		RGBtoHSL(BGR(d1[i]),&h,&s,&l);
		if(l<0.008){
			d1[i]=BGR(WHITE);
		}
		if(d1[i]!=BGR(WHITE)){
			d1[i]=0;
		}
	}
	putimage(x,y,&img1,SRCAND);
	putimage(x,y,&img,SRCPAINT);
}
