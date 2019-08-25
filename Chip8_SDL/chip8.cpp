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

//������ʾ�����弯����ǰ��ĩβ��
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
	pc = 0x200;        //���������ָ��0x200��ROM�������ص���λ�ã�
	opcode = 0;            //��ʼ����ǰopcode
	I = 0;            //��ʼ�������Ĵ���
	sp = 0;            //��ʼ��ջ��ָ��

	//�����Դ�
	for (int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	//����ջ
	for (int i = 0; i < 16; ++i)
		stack[i] = 0;

	//�����V0��VF�ļĴ����Ͱ�����Ϣ
	for (int i = 0; i < 16; ++i)
		key[i] = V[i] = 0;

	//�����ڴ�
	for (int i = 0; i < 4096; ++i)
		memory[i] = 0;

	//��ȡ���弯���ڴ��0x00-0x50��
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	//��ʼ����ʱ��
	delay_timer = 0;
	sound_timer = 0;

	//��ʼ���滭��־
	drawFlag = true;

	srand((unsigned int)time(NULL));//�������������
}

void chip8::emulateCycle()//ģ������
{
	//��ȡopcode���������ֽںϲ���һ��
	opcode = memory[pc] << 8 | memory[pc + 1];

	//����opcode�����Բο�http://en.wikipedia.org/wiki/CHIP-8��
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000: //0x00E0: �����Դ棨ȫ����Ϊ0��
			for (int i = 0; i < 2048; ++i)//һ��2048�����أ�ÿ������һ��byte
				gfx[i] = 0x0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: //0x00EE: ���Ӻ�������
			--sp;            //ջָ��������һ��
			pc = stack[sp];    //��PC��������������ָ���ԭ����ֵ                    
			pc += 2;        //������PC��Ҫ��ǰ��һ��
			break;

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
		break;

	case 0x1000: //0x1NNN: ����NNN�����ַ�����÷��أ�����jmp�����൱��jmp/goto
		pc = opcode & 0x0FFF;//NNN�����ַ�洢�ڵ�12λ��ͨ��&ȡ��
		break;

	case 0x2000: //0x2NNN: ����NNN�����ַ���Ӻ�����������Ҫ���أ�����call��
		stack[sp] = pc;//�ѵ�ǰPCѹջ
		++sp;//����ջ��ָ��
		pc = opcode & 0x0FFF;//��ת
		break;

	case 0x3000: //0x3XNN: ���VX == NN����������ȥ��ָ��
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: //0x4XNN: ���VX != NN����������ȥ��ָ��
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: //0x5XY0: ���VX == VY����������ȥ��ָ��
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

		case 0x0004: // 0x8XY4: VX +=VY. �������������VF����λ��־����Ϊ1������Ϊ0                    
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) //��VY > 255 - VX
				V[0xF] = 1; //��������������VF��Ϊ1
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];//VX += VY
			pc += 2;
			break;

		case 0x0005: // 0x8XY5: VX -= VY. ����н�λ��������С��0����VF��Ϊ0��������Ϊ1
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) //��VY > VX
				V[0xF] = 0; //�н�λ�������Ϊ0
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];//VX -= VY
			pc += 2;
			break;

		case 0x0006: // 0x8XY6: VX����һλ. VF��ΪVX����ǰ�����λ
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;//ȡ�����λ����VF
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007: // 0x8XY7: VX = VY - VX. ����н�λ��������С��0����VF��Ϊ0��������Ϊ1
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])//��VX > VY
				V[0xF] = 0; //�н�λ��������Ϊ0
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];//VX = VY - VX            
			pc += 2;
			break;

		case 0x000E: // 0x8XYE: VX����һλ. VF��ΪVX����ǰ�����λ
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;//ȡ�����λ
			V[(opcode & 0x0F00) >> 8] <<= 1;//����һλ
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
		}
		break;

	case 0x9000: //0x9XY0: ��������ȥ��ָ����VX != VY
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: // ANNN: I = NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: // BNNN: ��ת�� NNN + V0
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000: // CXNN: VX = random number & NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: // �ڵ�һƪ������ϸ����
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];//ȡ��x,y(��������)
		unsigned short height = opcode & 0x000F;//ȡ��ͼ���ĸ߶�
		unsigned short pixel;

		V[0xF] = 0;//��ʼ��VFΪ0
		for (int yline = 0; yline < height; yline++)//����ÿһ��
		{
			pixel = memory[I + yline];//ȡ���ڴ�I����ֵ��pixel�а�����һ�е�8������
			for (int xline = 0; xline < 8; xline++)//����1���е�8������
			{
				if ((pixel & (0x80 >> xline)) != 0)//��鵱ǰ�����Ƿ�Ϊ1
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)//�����ʾ����gfx[]�������ҲΪ1����������ײ(64��CHIP8����ʾ���)
					{
						V[0xF] = 1;//����VFΪ1                                
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;//gfx����1��byte����ʾ1�����أ���ֵΪ1��0����������൱��ȡ��
				}
			}
		}

		drawFlag = true;//�滭��־��Ϊ1        
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: // EX9E: ���VX�б���İ��������£�����������ָ��
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: ���VX�б���İ���û�б����£�����������ָ��
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
		case 0x0007: // FX07: VX = delay��ʱ��
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: // FX0A: ����а�����Ϣ������VX        
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;//����ֻ�������һ�����µİ�����VX
				}
			}

			//���û�а������£��򷵻�
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

		case 0x001E: // FX1E: VX += I,�������(I+VX>0xFFF)��VF��Ϊ1
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: // FX29: ��I��ΪVX���ַ���Ӧ�����弯����ʼλ��. �ַ�'0'-'F'ÿ������4x5�����ؾ������
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033: // FX33: ��VX��ʮ���Ƶı�ʾ����I/I+1/I+2������ַ������IΪ��λ��I+1Ϊʮλ��I+2Ϊ��λ��
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;//ȡ��ʮ���ư�λ
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;//ȡ��ʮ����ʮλ
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;//ȡ��ʮ���Ƹ�λ            
			pc += 2;
			break;

		case 0x0055: // FX55: ��V0-VX���δ����ڴ���I��ʼ�ĵط�                    
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			//��ԭ�������У������������ɵ�ʱ��, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065: // FX65: ��VX-VX������Ϊ�ڴ���I��ʼ�ĵط���ֵ(��FX55�����෴)                    
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			//��ԭ�������У������������ɵ�ʱ��, I = I + X + 1.
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

	//��ʱ��--
	if (delay_timer > 0)
		--delay_timer;

	//��ʱ��--
	if (sound_timer > 0)
	{
		//if(sound_timer == 1) printf("beep\n");
		--sound_timer;
	}
}

void chip8::debugRender()//����debug���ÿ���̨��ʽ����Դ��е�ֵ
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

bool chip8::loadApplication(const char * filename)//����ROM���൱�ڴ���Σ���0x200�ĵط�����memory map��
{
	init();
	printf("Loading: %s\n", filename);

	//��rom�ļ���rbģʽ�¶���ʲô����ʲô,�����ļ�ĩβ�Ż᷵��EOF��
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	//��ȡ�ļ���С
	fseek(pFile, 0, SEEK_END);//��pFile��ָ����ļ��ײ��Ƶ��ļ�β��
	long lSize = ftell(pFile);//�����ļ��Ĵ�С
	rewind(pFile);//��pFile������ָ��ͷ��
	printf("Filesize: %d\n", (int)lSize);

	//�½�ͬ����С�Ļ���
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	//��ROM�ļ����뻺��
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	//�ѻ��濽��Chip8���ڴ��ָ��λ��(0x200 = 512)
	if ((4096 - 512) > lSize)//�ж��Ƿ����㹻�ռ��ȡrom��4096�ֽ������ڴ棬512�Ǵ��ROM����ʼλ�ã�
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	//�ر��ļ����ͷŻ���
	fclose(pFile);
	free(buffer);

	return true;
}