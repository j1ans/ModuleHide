// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
using namespace std;
typedef struct _UNICODE_STRING;
typedef struct _PEB_LDR_DATA;
typedef struct _LDR_DATA_TABLE_ENTRY;
bool MODIFY_LIST_ENTRY(_PEB_LDR_DATA* pNow, HMODULE hMODULE, int nType);


typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
};

typedef struct _PEB_LDR_DATA
{
	DWORD Length;
	bool Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList; // 指向了 InLoadOrderModuleList 链表的第一项
	LIST_ENTRY InMemoryOrderModuleList; // BLINK指针指向前一个元素,FLINK指针指向下一个元素
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY          InLoadOrderModuleList;
	LIST_ENTRY          InMemoryOrderModuleList;
	LIST_ENTRY          InInitializationOrderModuleList;
	void* BaseAddress;
	void* EntryPoint;
	ULONG               SizeOfImage;
	_UNICODE_STRING		FullDllName;
	_UNICODE_STRING      BaseDllName;
	ULONG               Flags;
	SHORT               LoadCount;
	SHORT               TlsIndex;
	HANDLE              SectionHandle;
	ULONG               CheckSum;
	ULONG               TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

int main()
{
	cout << "按任意键开始隐藏" << endl;
	getchar();
	_PEB_LDR_DATA* ldr;
	//先通过汇编定位到PEB_LDR_DATA
	_asm {
		mov eax, fs: [0x30] // PEB 
		mov ecx, [eax + 0x0c] // LDR
		mov ldr, ecx
	}
	HMODULE hModule = GetModuleHandle(TEXT("kernel32.dll"));
	MODIFY_LIST_ENTRY(ldr, hModule, 1);
	MODIFY_LIST_ENTRY(ldr, hModule, 2);
	MODIFY_LIST_ENTRY(ldr, hModule, 3);
	cout << "隐藏成功" << endl;
	getchar();
	return 0;
}

bool MODIFY_LIST_ENTRY(_PEB_LDR_DATA* pNow, HMODULE hMODULE, int nType) {
	PLIST_ENTRY pHead = NULL;
	switch (nType) {
	case 1:pHead = &(pNow->InLoadOrderModuleList); // 定义头双向链表的指针
		break;
	case 2:pHead = &(pNow->InMemoryOrderModuleList); // 定义头双向链表的指针
		break;
	case 3:pHead = &(pNow->InInitializationOrderModuleList); // 定义头双向链表的指针
		break;
	}
	PLIST_ENTRY pCur; // 初始化当前双向链表指针
	PLDR_DATA_TABLE_ENTRY pCurData =NULL;
	pCur = pHead;
	do {
		switch (nType) {
		case 1:pCurData = CONTAINING_RECORD(pCur, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
			break;
		case 2:pCurData = CONTAINING_RECORD(pCur, LDR_DATA_TABLE_ENTRY, InMemoryOrderModuleList);
			break;
		case 3:pCurData = CONTAINING_RECORD(pCur, LDR_DATA_TABLE_ENTRY, InInitializationOrderModuleList);
			break;
		}
		if (pCurData->BaseAddress == hMODULE) {
			pCur->Blink->Flink = pCur->Flink;
			pCur->Flink->Blink = pCur->Blink;
			break;
		}
		pCur = pCur->Flink;

	} while (pCur != pHead);


	return true;
}

