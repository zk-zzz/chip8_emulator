///////////////////////////////////////////////////////////////////////////////
// Project description
// Name: myChip8
//
// Author: Laurence Muller
// Contact: laurence.muller@gmail.com
//
// License: GNU General Public License (GPL) v2 
// ( http://www.gnu.org/licenses/old-licenses/gpl-2.0.html )
//
// Copyright (C) 2011 Laurence Muller / www.multigesture.net
///////////////////////////////////////////////////////////////////////////////

class chip8
{
public:
	chip8();//构造函数是空的，初始化主要是由init()完成的
	~chip8();
	bool drawFlag;//用于记录是否需要绘画的标志位

	void emulateCycle();//模拟周期
	void debugRender();//用于debug，用控制台方式输出显存中的值
	bool loadApplication(const char * filename);//把游戏rom读入内存(包含init())        

	unsigned char  gfx[64 * 32];//Chip8的显示缓存，总共2048个像素
	unsigned char  key[16];//16个按键输入            

private:
	unsigned short pc;//程序计数器
	unsigned short opcode;//当前opcode
	unsigned short I;//当前索引寄存器
	unsigned short sp;//当前栈顶指针

	unsigned char  V[16];//寄存器(V0-VF)，前15个是通用寄存器，第16个是进位标志
	unsigned short stack[16];//栈(16级)
	unsigned char  memory[4096];//主内存(4k大小)        

	unsigned char  delay_timer;
	unsigned char  sound_timer;//两个计时器        

	void init();//主要的初始化工作
};