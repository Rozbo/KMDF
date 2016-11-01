#include <ntddk.h>
#include <windef.h>
#include <stdlib.h> 



//��1������������ӣ�һ����˵�޸�Ϊ���������ּ���
#define DEVICE_NAME   L"\\Device\\KrnlHW64"
#define LINK_NAME   L"\\DosDevices\\KrnlHW64"
#define LINK_GLOBAL_NAME  L"\\DosDevices\\Global\\KrnlHW64" 

//��2�������������ܺź����֣��ṩ�ӿڸ�Ӧ�ó������
#define IOCTL_IO_TEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SAY_HELLO  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS) 

//��3������ж�صĴ�������
VOID DriverUnload(PDRIVER_OBJECT pDriverObj) {
	UNICODE_STRING strLink;
	DbgPrint(L"[KrnlHW64]DriverUnload\n");
	RtlInitUnicodeString(&strLink, LINK_NAME);
	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(pDriverObj->DeviceObject);
	//��������
}
//��4��IRP_MJ_CREATE��Ӧ�Ĵ������̣�һ�㲻�ù���
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//DbgPrint(L"[KrnlHW64]DispatchCreate\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//��5��IRP_MJ_CLOSE��Ӧ�Ĵ������̣�һ�㲻�ù���
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//DbgPrint(L"[KrnlHW64]DispatchClose\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//��6��IRP_MJ_DEVICE_CONTROL��Ӧ�Ĵ������̣���������Ҫ�ĺ���֮һ��һ��������;���� ���������ܵĳ��򣬶��ᾭ���������
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;  PIO_STACK_LOCATION pIrpStack;  ULONG uIoControlCode;  PVOID pIoBuffer;  ULONG uInSize;  ULONG uOutSize;  DbgPrint("[KrnlHW64]DispatchIoctl\n");  pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//������
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	//�������������
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	//���������С
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	//��������С 
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	switch (uIoControlCode)
	{
	case IOCTL_IO_TEST:
	{
		DWORD dw = 0;
		//������������
		memcpy(&dw, pIoBuffer, sizeof(DWORD));
		//ʹ�����������
		dw++;
		//�������Ľ�� 
		memcpy(pIoBuffer, &dw, sizeof(DWORD));
		//����ɹ������ط�STATUS_SUCCESS����DeviveIoControl����ʧ�� 
		status = STATUS_SUCCESS;
		break;
	}
	case IOCTL_SAY_HELLO:
	{
		DbgPrint("[KrnlHW64]IOCTL_SAY_HELLO\n");
		status = STATUS_SUCCESS;
		break;
	}
	default:
		status = STATUS_SUCCESS;
		break;
	}
	if (status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;
	else
		pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

//��7���������صĴ������̣���������������ĳ�ʼ������
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ustrLinkName;
	UNICODE_STRING ustrDevName;
	PDEVICE_OBJECT pDevObj;
	//��ʼ����������
	pDriverObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	pDriverObj->DriverUnload = DriverUnload;
	//���������豸
	RtlInitUnicodeString(&ustrDevName, DEVICE_NAME);
	status = IoCreateDevice(pDriverObj, 0, &ustrDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
		return status;
	if (IoIsWdmVersionAvailable(1, 0x10))
		RtlInitUnicodeString(&ustrLinkName, LINK_GLOBAL_NAME);
	else
		RtlInitUnicodeString(&ustrLinkName, LINK_NAME);
	//������������
	status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	//�ߵ���������ʵ�����Ѿ���ʼ����ɣ�������ӵ��ǹ��ܳ�ʼ���Ĵ���
	DbgPrint("[KrnlHW64]DriverEntry\n");
	//-------��ʼ����Ŀ
	return STATUS_SUCCESS;
}