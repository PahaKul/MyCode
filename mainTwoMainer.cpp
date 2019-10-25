#define _WINSOCK_DEPRECATED_NO_WARNINGS

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
#error MinHook supports only x86 and x64 systems.
#endif

#include <wx/wx.h>
#include <wx/jsonreader.h>


#include <windows.h>

#include <stdio.h>

#include "../../GodPool/GodPoolMainerDll.h"

static PARENTINFO parentInfo = { 0 };

#define HASHCURRENT (0)
#define HASHAVERAGE (1)

#define WORKERCOUNT (0)

#define ASKCURRENTHASHANDSHARE (0)

/*
static wxArrayDouble xDateHash;
static wxArrayDouble yDateHash;
static wxArrayDouble xDateShare;
static wxArrayDouble yDateShare;
static long savetime;
*/


extern "C" __declspec (dllexport) void SetParentDataFunc(const char *sMainerType, const char *sPoolType, const char *sCash, const char *sPool, const char *sMainer, const char *sWallet, const char *sEmail, const char *sCards, long iSahreAccept, long iShareReject)
{
	memset((char *)&parentInfo, 0, sizeof(parentInfo));
	strcpy_s<sizeof(parentInfo.MainerType)>(parentInfo.MainerType, sMainerType);
	strcpy_s<sizeof(parentInfo.MainerName)>(parentInfo.MainerName, sMainer);
	strcpy_s<sizeof(parentInfo.PoolType)>(parentInfo.PoolType, sPoolType);
	strcpy_s<sizeof(parentInfo.PoolName)>(parentInfo.PoolName, sPool);
	strcpy_s<sizeof(parentInfo.CashName)>(parentInfo.CashName, sCash);
	strcpy_s<sizeof(parentInfo.Wallet)>(parentInfo.Wallet, sWallet);
	strcpy_s<sizeof(parentInfo.EMail)>(parentInfo.EMail, sEmail);
	strcpy_s<sizeof(parentInfo.Cards)>(parentInfo.Cards, sCards);


	return;
}


// количество графиков hashrate может быть не более 5
extern "C" __declspec (dllexport) int GetCountHashPokazatelFunc()
{
	return 2;
}

// количество графиков Share может быть не более 5

extern "C" __declspec (dllexport) int GetCountSharePokazatelFunc()
{
	return 1;
}

// количество значений показываемых на графике по оси (x)

extern "C" __declspec (dllexport) int GetShowLastCountFunc()
{
	return 36;
}


// названия показателей hashrate

extern "C" __declspec (dllexport) char *GetHashPokazatelFunc(int i)
{
	wxString s;
	static char buffer[128];
	memset((char *)&buffer, 0, sizeof(buffer));
	switch (i)
	{
	case HASHCURRENT:
		s = _("Максимальный Hashrate");
		break;
	case HASHAVERAGE:
		s = _("Средний Hashrate");
		break;
	default:
		s = wxT("");
		break;
	}
	strcat_s<sizeof(buffer)>(buffer, s.c_str());
	return (char *)&buffer;
}


// название показателей share

extern "C" __declspec (dllexport) char *GetSharePokazatelFunc(int i)
{
	wxString s;
	static char buffer[128];
	memset((char *)&buffer, 0, sizeof(buffer));
	switch (i)
	{
	case WORKERCOUNT:
		s = _("Количество Workers");
		break;
	default:
		s = wxT("");
		break;
	}
	strcat_s<sizeof(buffer)>(buffer, s.c_str());
	return (char *)&buffer;
}


// Подготовка адресов для запросов, формируется массив строк

extern "C" __declspec (dllexport) bool PrepairAskFunc(wxString sCash, wxString sWallet, wxArrayString &aAskAddr, wxArrayInt &aType, wxArrayString &sParam)
{
	wxString sAddr;
	wxString sWalletNormal;
	if (sWallet.Left(2) == wxT("0x"))
		sWalletNormal = sWallet;
	else
		sWalletNormal = wxT("0x") + sWallet;

	if (sCash == wxT("Ethereum (ETC)"))
		sAddr = wxT("https://etc.2miners.com/api/accounts/") + sWalletNormal;
	else
		sAddr = wxT("https://eth.2miners.com/api/accounts/") + sWalletNormal;
	aAskAddr.Add(sAddr);
	aType.Add(0);

	return true;
}



// Цвета графиков hashrate

extern "C" __declspec (dllexport) bool GetHashColorFunc(int i, int &r, int &g, int &b)
{
	switch (i)
	{
	case HASHCURRENT:
		r = 151; g = 187; b = 205;
		break;
	case HASHAVERAGE:
		r = 220; g = 220; b = 220;
		break;
	default:
		r = 255; g = 255; b = 255;
		break;
	}
	return true;
}


// Цвета графиков share

extern "C" __declspec (dllexport) bool GetShareColorFunc(int i, int &r, int &g, int &b)
{
	switch (i)
	{
	case WORKERCOUNT:
		r = 0; g = 255; b = 0;
		break;
	default:
		r = 255; g = 255; b = 255;
		break;
	}
	return true;
}

// проверка необходимости обновления графиков, здесь проверяем последнее значение даты на графике и JSON

extern "C" __declspec (dllexport) bool CheckUpdateNeedFunc(wxJSONValue m_root, long &m_PoolLastTime, int i)
{
	long dtime;
	bool NeedUpdate = false;
	int countElem;
	wxString sdata = wxT("minerCharts");


	if (m_root[sdata].IsNull())
		return NeedUpdate;
	countElem = m_root[sdata].AsArray()->Count();
	if (countElem > 0)
	{
		dtime = m_root[sdata][countElem - 1][_T("x")].AsLong();
		if (dtime != m_PoolLastTime)
		{
			m_PoolLastTime = dtime;
			NeedUpdate = true;
		}
	}

	return NeedUpdate;
}


double GetJSONData(wxJSONValue m_root, wxString sdata, int i, wxString s, double coef = 1)
{
	double retCode = .0;

	if (!m_root[sdata][i][s].IsNull())
		retCode = (m_root[sdata][i][s].IsLong() ? m_root[sdata][i][s].AsLong() : m_root[sdata][i][s].AsDouble()) / coef;
	return retCode;
}


double GetJSONDataString(wxJSONValue m_root, wxString sdata, int i, wxString s, double coef = 1)
{
	double retCode = .0;

	if (!m_root[sdata][i][s].IsNull())
		retCode = (m_root[sdata][i][s].IsString() ? atol(m_root[sdata][i][s].AsString()) : (m_root[sdata][i][s].IsLong() ? m_root[sdata][i][s].AsLong() : m_root[sdata][i][s].AsDouble())) / coef;
	return retCode;
}


double GetJSONData(wxJSONValue m_root, wxString sdata, double coef = 1)
{
	double retCode = .0;

	if (!m_root[sdata].IsNull())
		retCode = (m_root[sdata].IsLong() ? m_root[sdata].AsLong() : m_root[sdata].AsDouble()) / coef;
	return retCode;
}

int CompareFunction(const wxDouble &class1, const wxDouble &class2)
{
	return class2 - class1;
}


// Обновление данных для для hashrate (m_root: json ответ, askNum: Номер запроса к пулу, i: номер графика )

extern "C" __declspec (dllexport) bool GetUpdateValueHashFunc(wxJSONValue m_root, int askNum, int i, wxArrayDouble &x, wxArrayDouble &y)
{
	long dtime;
	int countElem, i1;
	wxString sdata = wxT("minerCharts");

	for (int j = 0; j < GetShowLastCountFunc(); j++)
	{
		x.Add(0);
		y.Add(0);
	}
	if ((countElem = m_root[sdata].AsArray()->Count()) > 0)
	{
		if (countElem > GetShowLastCountFunc())
			countElem = GetShowLastCountFunc();
		i1 = 0;
		for (int j = GetShowLastCountFunc() - 1; j >= 0; j--)
		{
			if (j < countElem)
			{
				dtime = m_root[sdata][j][_T("x")].AsLong();
				switch (i)
				{
				case HASHAVERAGE:
					x[i1] = dtime;
					y[i1] = GetJSONData(m_root, sdata, j, wxT("minerHash"), 1000000);
					break;
				case HASHCURRENT:
					x[i1] = dtime;
					y[i1] = GetJSONData(m_root, sdata, j, wxT("minerLargeHash"), 1000000);
					break;
				}
			}
			else
			{
				x[i1] = 0;
				y[i1] = 0;
			}
			i1++;
		}
		if (countElem < GetShowLastCountFunc())
		{
			dtime = x[countElem];
			for (int j = countElem - 1; j >= 0; j--)
			{
				dtime -= 1200;
				x[j] = dtime;
			}
		}
	}

	return true;
}



// Обновление данных для для share

extern "C" __declspec (dllexport) bool GetUpdateValueShareFunc(wxJSONValue m_root, int askNum, int i, wxArrayDouble &x, wxArrayDouble &y)
{
	long dtime;
	int countElem, i1;

	wxString sdata = wxT("minerCharts");

	for (int j = 0; j < GetShowLastCountFunc(); j++)
	{
		x.Add(0);
		y.Add(0);
	}
	if ((countElem = m_root[sdata].AsArray()->Count()) > 0)
	{
		if (countElem > GetShowLastCountFunc())
			countElem = GetShowLastCountFunc();
		i1 = 0;
		for (int j = GetShowLastCountFunc() - 1; j >= 0; j--)
		{
			if (j < countElem)
			{
				dtime = m_root[sdata][j][_T("x")].AsLong();
				switch (i)
				{
				case WORKERCOUNT:
					x[i1] = dtime;
					y[i1] = GetJSONDataString(m_root, sdata, j, wxT("workerOnline"));
					break;
				}
			}
			else
			{
				x[i1] = 0;
				y[i1] = 0;
			}
			i1++;
		}
		if (countElem < GetShowLastCountFunc())
		{
			dtime = x[countElem];
			for (int j = countElem - 1; j >= 0; j--)
			{
				dtime -= 1200;
				x[j] = dtime;
			}
		}
	}



	return true;
}


// Формирование параметра wallet, зависит от пула, с которым работает майнер.
extern "C" __declspec (dllexport) char *GetWalletParameterFunc(const char *sWallet, const char *sName, const char *sEmail, const char *sCurrency, const char *sPool, const char *sMainer)
{
	static char buffer[128];
	memset((char *)&buffer, 0, sizeof(buffer));
	strcat_s<sizeof(buffer)>(buffer, sWallet);
	if (sName != NULL)
	{
		strcat_s<sizeof(buffer)>(buffer, ".");
		strcat_s<sizeof(buffer)>(buffer, sName);
	}
	if ( strcmp(parentInfo.CashName, "Pirl (PIRL)") == 0 && strcmp(parentInfo.MainerType, "Claymore") == 0 )
	{
		strcat_s<sizeof(buffer)>(buffer, " -allcoins 1");
	}
	

	return (char *)&buffer;
}


// Формирование титула на графике для шар
extern "C" __declspec (dllexport) char *GetShareTitleFunc()
{
	return "Статистика шар (шт.)";
}


// Формирование титула на графике для хешей
extern "C" __declspec (dllexport) char *GetHashTitleFunc()
{
	return "Статистика HashRate (Mh/s)";
}



int __stdcall DllMain(HINSTANCE instance, unsigned long int reason, void *reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		break;
	}

	return true;
}