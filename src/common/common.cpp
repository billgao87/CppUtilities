// common.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "common.h"


// ���ǵ���������һ��ʾ��
COMMON_API int ncommon=0;

// ���ǵ���������һ��ʾ����
COMMON_API int fncommon(void)
{
    return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� common.h
Ccommon::Ccommon()
{
    return;
}
