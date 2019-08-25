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
	chip8();//���캯���ǿյģ���ʼ����Ҫ����init()��ɵ�
	~chip8();
	bool drawFlag;//���ڼ�¼�Ƿ���Ҫ�滭�ı�־λ

	void emulateCycle();//ģ������
	void debugRender();//����debug���ÿ���̨��ʽ����Դ��е�ֵ
	bool loadApplication(const char * filename);//����Ϸrom�����ڴ�(����init())        

	unsigned char  gfx[64 * 32];//Chip8����ʾ���棬�ܹ�2048������
	unsigned char  key[16];//16����������            

private:
	unsigned short pc;//���������
	unsigned short opcode;//��ǰopcode
	unsigned short I;//��ǰ�����Ĵ���
	unsigned short sp;//��ǰջ��ָ��

	unsigned char  V[16];//�Ĵ���(V0-VF)��ǰ15����ͨ�üĴ�������16���ǽ�λ��־
	unsigned short stack[16];//ջ(16��)
	unsigned char  memory[4096];//���ڴ�(4k��С)        

	unsigned char  delay_timer;
	unsigned char  sound_timer;//������ʱ��        

	void init();//��Ҫ�ĳ�ʼ������
};