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

#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//用于显示的字体集（见前文末尾）
unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

chip8::chip8()
{
	// empty
}

chip8::~chip8()
{
	// empty
}

void chip8::init()
{
	pc = 0x200;        //程序计数器指向0x200（ROM将被加载到的位置）
	opcode = 0;            //初始化当前opcode
	I = 0;            //初始化索引寄存器
	sp = 0;            //初始化栈顶指针

	//清理显存
	for (int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	//清理栈
	for (int i = 0; i < 16; ++i)
		stack[i] = 0;

	//清理从V0到VF的寄存器和按键信息
	for (int i = 0; i < 16; ++i)
		key[i] = V[i] = 0;

	//清理内存
	for (int i = 0; i < 4096; ++i)
		memory[i] = 0;

	//读取字体集到内存的0x00-0x50处
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	//初始化计时器
	delay_timer = 0;
	sound_timer = 0;

	//初始化绘画标志
	drawFlag = true;

	srand((unsigned int)time(NULL));//产生随机数种子
}

void chip8::emulateCycle()//模拟周期
{
	//获取opcode（把两个字节合并在一起）
	opcode = memory[pc] << 8 | memory[pc + 1];

	//处理opcode（可以参考http://en.wikipedia.org/wiki/CHIP-8）
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000: //0x00E0: 清理显存（全部置为0）
			for (int i = 0; i < 2048; ++i)//一共2048个像素，每个像素一个byte
				gfx[i] = 0x0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: //0x00EE: 从子函数返回
			--sp;            //栈指针往回走一格
			pc = stack[sp];    //把PC（程序计数器）恢复成原来的值                    
			pc += 2;        //别忘了PC还要往前跳一格
			break;

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
		break;

	case 0x1000: //0x1NNN: 跳到NNN这个地址（不用返回，类似jmp），相当于jmp/goto
		pc = opcode & 0x0FFF;//NNN这个地址存储在低12位，通过&取出
		break;

	case 0x2000: //0x2NNN: 调用NNN这个地址的子函数（将来需要返回，类似call）
		stack[sp] = pc;//把当前PC压栈
		++sp;//步进栈顶指针
		pc = opcode & 0x0FFF;//跳转
		break;

	case 0x3000: //0x3XNN: 如果VX == NN，跳过接下去的指令
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: //0x4XNN: 如果VX != NN，跳过接下去的指令
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: //0x5XY0: 如果VX == VY，跳过接下去的指令
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000: //0x6XNN: VX = NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000: // 0x7XNN: VX += NN
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x8XY0: VX = VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001: // 0x8XY1: VX = VX | VY
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002: // 0x8XY2: VX = VX & VY
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003: // 0x8XY3: VX = VX ^ VY
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004: // 0x8XY4: VX +=VY. 如果有溢出，则把VF（进位标志）设为1，否则为0                    
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) //即VY > 255 - VX
				V[0xF] = 1; //出现了溢出，则把VF置为1
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];//VX += VY
			pc += 2;
			break;

		case 0x0005: // 0x8XY5: VX -= VY. 如果有借位发生（差小于0），VF设为0，否则设为1
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) //即VY > VX
				V[0xF] = 0; //有借位情况，置为0
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];//VX -= VY
			pc += 2;
			break;

		case 0x0006: // 0x8XY6: VX右移一位. VF设为VX右移前的最低位
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;//取出最低位，给VF
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007: // 0x8XY7: VX = VY - VX. 如果有借位发生（差小于0），VF设为0，否则设为1
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])//即VX > VY
				V[0xF] = 0; //有借位发生，置为0
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];//VX = VY - VX            
			pc += 2;
			break;

		case 0x000E: // 0x8XYE: VX左移一位. VF设为VX左移前的最高位
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;//取出最高位
			V[(opcode & 0x0F00) >> 8] <<= 1;//左移一位
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
		}
		break;

	case 0x9000: //0x9XY0: 跳过接下去的指令，如果VX != VY
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: // ANNN: I = NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: // BNNN: 跳转到 NNN + V0
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000: // CXNN: VX = random number & NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: // 在第一篇中有详细分析
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];//取得x,y(横纵坐标)
		unsigned short height = opcode & 0x000F;//取得图案的高度
		unsigned short pixel;

		V[0xF] = 0;//初始化VF为0
		for (int yline = 0; yline < height; yline++)//对于每一行
		{
			pixel = memory[I + yline];//取得内存I处的值，pixel中包含了一行的8个像素
			for (int xline = 0; xline < 8; xline++)//对于1行中的8个像素
			{
				if ((pixel & (0x80 >> xline)) != 0)//检查当前像素是否为1
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)//如果显示缓存gfx[]里该像素也为1，则发生了碰撞(64是CHIP8的显示宽度)
					{
						V[0xF] = 1;//设置VF为1                                
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;//gfx中用1个byte来表示1个像素，其值为1或0。这里异或相当于取反
				}
			}
		}

		drawFlag = true;//绘画标志置为1        
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: // EX9E: 如果VX中保存的按键被按下，则跳过下条指令
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: 如果VX中保存的按键没有被按下，则跳过下条指令
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: VX = delay计时器
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: // FX0A: 如果有按键信息，存入VX        
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;//这里只存了最后一个按下的按键到VX
				}
			}

			//如果没有按键按下，则返回
			if (!keyPress)
				return;

			pc += 2;
		}
		break;

		case 0x0015: // FX15: delay_timer = VX
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: // FX18: sound_timer = VX
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: // FX1E: VX += I,如有溢出(I+VX>0xFFF)则VF置为1
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: // FX29: 把I设为VX中字符对应的字体集的起始位置. 字符'0'-'F'每个是由4x5的像素矩阵组成
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033: // FX33: 把VX的十进制的表示存于I/I+1/I+2三个地址，其中I为百位，I+1为十位，I+2为个位。
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;//取得十进制百位
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;//取得十进制十位
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;//取得十进制个位            
			pc += 2;
			break;

		case 0x0055: // FX55: 把V0-VX依次存入内存中I起始的地方                    
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			//在原解释器中，当这个操作完成的时候, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065: // FX65: 把VX-VX依次设为内存中I起始的地方的值(与FX55操作相反)                    
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			//在原解释器中，当这个操作完成的时候, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
	}

	//计时器--
	if (delay_timer > 0)
		--delay_timer;

	//计时器--
	if (sound_timer > 0)
	{
		//if(sound_timer == 1) printf("beep\n");
		--sound_timer;
	}
}

void chip8::debugRender()//用于debug，用控制台方式输出显存中的值
{
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfx[(y * 64) + x] == 0)
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}

bool chip8::loadApplication(const char * filename)//加载ROM（相当于代码段）到0x200的地方（见memory map）
{
	init();
	printf("Loading: %s\n", filename);

	//打开rom文件（rb模式下读到什么返回什么,读到文件末尾才会返回EOF）
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	//获取文件大小
	fseek(pFile, 0, SEEK_END);//把pFile的指向从文件首部移到文件尾部
	long lSize = ftell(pFile);//返回文件的大小
	rewind(pFile);//让pFile的重新指向头部
	printf("Filesize: %d\n", (int)lSize);

	//新建同样大小的缓存
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	//把ROM文件拷入缓存
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	//把缓存拷入Chip8的内存的指定位置(0x200 = 512)
	if ((4096 - 512) > lSize)//判断是否有足够空间读取rom（4096字节是总内存，512是存放ROM的起始位置）
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	//关闭文件，释放缓存
	fclose(pFile);
	free(buffer);

	return true;
}