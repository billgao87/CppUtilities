// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CONCURRENT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CONCURRENT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef CONCURRENT_EXPORTS
#define CONCURRENT_API __declspec(dllexport)
#else
#define CONCURRENT_API __declspec(dllimport)
#endif

// �����Ǵ� concurrent.dll ������
class CONCURRENT_API Cconcurrent {
public:
	Cconcurrent(void);
	// TODO:  �ڴ�������ķ�����
};

extern CONCURRENT_API int nconcurrent;

CONCURRENT_API int fnconcurrent(void);
