// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� ALGORITHM_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// ALGORITHM_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef ALGORITHM_EXPORTS
#define ALGORITHM_API __declspec(dllexport)
#else
#define ALGORITHM_API __declspec(dllimport)
#endif

// �����Ǵ� algorithm.dll ������
class ALGORITHM_API Calgorithm {
public:
	Calgorithm(void);
	// TODO:  �ڴ��������ķ�����
};

extern ALGORITHM_API int nalgorithm;

ALGORITHM_API int fnalgorithm(void);