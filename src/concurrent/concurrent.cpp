// concurrent.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "concurrent.h"


// ���ǵ���������һ��ʾ��
CONCURRENT_API int nconcurrent=0;

// ���ǵ���������һ��ʾ����
CONCURRENT_API int fnconcurrent(void)
{
    return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� concurrent.h
Cconcurrent::Cconcurrent()
{
    return;
}
