/////////////////////////////////////////////////////////////////////////////
// Name:        samples/sockbase/client.cpp
// Purpose:     Sockets sample for wxBase
// Author:      Pavel Kulakov
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/app.h"
#include <wx/socket.h>
#include <wx/list.h>
#include <wx/ffile.h>
#include <wx/datetime.h>
#include <wx/timer.h>
#include <wx/thread.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>
#include <wx/wfstream.h>
#include <wx/sckstrm.h>
#include <wx/dir.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/imagpng.h>
#include <wx/dialog.h>


#include <../src/zlib/zlib.h>
#include "../CommonLib/Common.h"
#include "../CommonLib/FileSendThread.h"
#include "../CommonLib/FileReceiveThread.h"
#include "../CommonLib/images.h"
#include "../CommonLib/version.h"


class ThreadWorker;


class GodPoolMainFrame : public wxDialog
{
public:
	GodPoolMainFrame(wxWindow *parent, wxWindowID id= wxID_ANY, const wxString &title = "", wxPoint position=wxDefaultPosition, wxSize size=wxDefaultSize)
		: wxDialog(parent, id, title, position, size, 0) 
	{
		m_icon = new wxBitmap(GetBitmapFromData("godPoolLogoSquare"));

		m_Message = "";
		m_maxStr = 9;
	};
	~GodPoolMainFrame() 
	{
	};
	void AddMessage(wxString s);
protected:
	void SetMessage(wxString s) { m_Message = s; Refresh(); ::SendMessage(this->GetHWND(), WM_PAINT, (WPARAM)0, (LPARAM)0); };
	void OnPaint(wxPaintEvent& aEvent);
	void OnSize(wxSizeEvent& aEvent);
	void OnActivate(wxActivateEvent& aEvent);
	wxBitmap *m_icon;
	wxImage *m_iconImage;
	wxString m_Message;
	wxArrayString m_ArrayMessage;
	int m_maxStr;
	DECLARE_EVENT_TABLE()
};


void GodPoolMainFrame::AddMessage(wxString s) 
{ 
	int i;

	if(m_ArrayMessage.GetCount() >= m_maxStr )
		m_ArrayMessage.RemoveAt(0);
	m_ArrayMessage.Add(s);

	for (i = 0; i < m_ArrayMessage.GetCount(); i++)
	{
		if (i == 0)
		{
			m_Message = m_ArrayMessage[i];
		}
		else
		{
			m_Message += wxT("\n") + m_ArrayMessage[i];
		}
	}
	SetMessage(m_Message);
};


void GodPoolMainFrame::OnSize(wxSizeEvent& aEvent)
{
	aEvent.Skip();
}

void GodPoolMainFrame::OnActivate(wxActivateEvent& aEvent)
{

	aEvent.Skip();
}


void GodPoolMainFrame::OnPaint(wxPaintEvent& aEvent)
{
	wxPaintDC old_dc(this);
	int w, h;
	int x, y;
	wxString s;
	wxSize sz;

	GetClientSize(&w, &h);
	if (m_icon->IsOk())
	{
		wxMemoryDC dc;
		dc.SelectObject(*m_icon);
		old_dc.Blit(10, 5, w-10, h-5, &dc, 0, 0);
	}

//	old_dc.DrawBitmap(*m_icon, 10, 5);

	s = wxT(MyAppPublisher);
	sz=old_dc.GetTextExtent(s);
	x = ((w - (m_icon->GetWidth() + 20)) / 2 - sz.GetWidth() / 2)+ m_icon->GetWidth() + 20;
	y = 10;
	old_dc.DrawText(s, wxPoint(x, y));
	y += sz.GetHeight();

	s = wxT(MyAppCopyRight);
	sz = old_dc.GetTextExtent(s);
	y += 3;
	x = ((w - (m_icon->GetWidth() + 20)) / 2 - sz.GetWidth() / 2) + m_icon->GetWidth() + 20;
	old_dc.DrawText(s, wxPoint(x, y));
	y += sz.GetHeight();

	s = wxT(MyAppAllRightReserved);
	sz = old_dc.GetTextExtent(s);
	y += 3;
	x = ((w - (m_icon->GetWidth() + 20)) / 2 - sz.GetWidth() / 2) + m_icon->GetWidth() + 20;
	old_dc.DrawText(s, wxPoint(x, y));
	y += sz.GetHeight();

	old_dc.DrawText(m_Message, wxPoint(10, m_icon->GetHeight()+15));
	aEvent.Skip();
}


BEGIN_EVENT_TABLE(GodPoolMainFrame, wxDialog)
EVT_PAINT(GodPoolMainFrame::OnPaint)
EVT_SIZE(GodPoolMainFrame::OnSize)
EVT_ACTIVATE(GodPoolMainFrame::OnActivate)
END_EVENT_TABLE()



class Client : public wxApp
{
private:

	wxString m_message;
//	wxString m_host;
	wxString m_path;
	wxString m_Programpath;
	wxString m_name;
	wxString m_ext;
	wxString m_sep;
	int m_version;
	long long m_crc;


	virtual bool OnInit() wxOVERRIDE;
	virtual int OnRun() wxOVERRIDE;
	virtual int OnExit() wxOVERRIDE;
	unsigned long long GetLastVersion(wxSocketClient* clientSocket, int &version);
	bool CreateBackup(wxString pCompressedFile, wxString sPath, wxString sName, wxString sSep);

	GodPoolMainFrame *m_frame;
};

IMPLEMENT_APP(Client)

class ThreadWorker : public wxThread
{
public:
	ThreadWorker(const wxString& p_host, int cmd, int p_size, char *fName);
	virtual ExitCode Entry() wxOVERRIDE;
private:
	wxString m_host;
	char *m_Filename;
	wxString m_workerIdent;
	SERVERPROTOCOL m_spo;
	SERVERPROTOCOL m_spi;
};



BOOL(WINAPI *SetNotePadLayer)(HWND, COLORREF, BYTE, DWORD);

bool Client::OnInit()
{
	HINSTANCE hMod;
	wxSize ssz;


	wxImage::AddHandler(new wxPNGHandler);
	// Получаем данные о програме и каталогах
	wxString s = wxApp::argv[0];
	if (!wxApp::OnInit())
		return false;

	wxInitAllImageHandlers();

	InitServerConnection();

	m_path = getPath();
	
	m_Programpath = getPath();
	m_ext = getExt();
	m_name = getName();
	m_sep = getSep();


#ifdef _DEBUG
	m_ext = wxT(".exe");
#else
	m_ext = wxT(".ovl");
#endif // !_DEBUG

	m_name = "GodPool";
	m_version = 0;

	//	m_host = DEFAULTSERVERHOST;
	wxPoint pos(0, 0);
	wxSize sz(300, 220);
	

#ifdef __WIN32__   
	ssz = wxSize(::GetSystemMetrics(SM_CXVIRTUALSCREEN), ::GetSystemMetrics(SM_CYVIRTUALSCREEN));
#else 
	ssz = wxSize(wxSystemSettings::GetMetricwxSYS_SCREEN_X), wxSystemSettings::GetMetricwxSYS_SCREEN_Y));
#endif

	pos.x = ssz.GetWidth() / 2 - sz.GetWidth() / 2;
	pos.y = ssz.GetHeight() / 2 - sz.GetHeight() / 2;

	m_frame = new GodPoolMainFrame(NULL, wxID_ANY, "", pos, sz );
	wxIcon FrameIcon;
	FrameIcon.CopyFromBitmap(GetBitmapFromData("godPoolLogoSquare"));
	m_frame->SetIcon(FrameIcon);

	m_frame->SetBackgroundColour(wxColour(231, 239, 242));

	hMod = LoadLibrary(L"user32.dll");
	if (hMod)
	{
		SetNotePadLayer = (int(__stdcall*)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(hMod, "SetLayeredWindowAttributes");
		if (SetWindowLong(m_frame->GetHWND(), GWL_EXSTYLE, 0x80000))
		{
			SetNotePadLayer(m_frame->GetHWND(), 0, 150, 0x02);
		}
		FreeLibrary(hMod);
	}

	m_frame->Show(true);

	m_frame->AddMessage(_("Загрузка ")+wxT("GodPool ..."));


	return true;
}

int Client::OnRun()
{
	wxFile f;
	bool bLocalDir = false;
	m_frame->AddMessage(_("Проверка версии программы ..."));

	if(f.Exists(m_path + m_sep + m_name + m_ext))
	{
		if (f.Create(m_path + m_sep + wxT("GodPool.tmp"), true))
		{
			f.Close();
			wxRemoveFile(m_path + m_sep + wxT("GodPool.tmp"));
			bLocalDir = true;
		}
	}
	if (!bLocalDir)
	{
		m_path = getConfigPath()+m_sep+m_name;

		if (f.Exists(m_path + m_sep + m_name + m_ext))
		{
			if (f.Create(m_path + m_sep + wxT("GodPool.tmp"), true))
			{
				f.Close();
				wxRemoveFile(m_path + m_sep + wxT("GodPool.tmp"));
				bLocalDir = true;
			}
		}
		if (!bLocalDir)
		{
			m_frame->AddMessage(_("Не хватает прав для сохранения конфигурации."));
			m_frame->AddMessage(_("Завершение работы через 30 сек. ..."));
			Sleep(30 * 1000);
			return 0;
		}

	}



	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION m_pi = { 0 };
	DWORD exitCode;
	int flags;
	wxIPV4address ca;
	wxString sPath;
	wxString sOption;

	int new_version;
	long long new_crc;
	// Проверка версии программы если есть связь с сервером
	wxSocketClient*	clientSocket = new wxSocketClient(wxSOCKET_BLOCK | wxSOCKET_WAITALL);
	ca.Hostname(DEFAULTSERVERHOST);
	ca.Service(DEFAULTSERVERPORT);
	clientSocket->SetTimeout(10);
//	bool failed = false;
	sOption = wxT("");
	// Подключаемся к серверу
	if (clientSocket->Connect(ca))
	{
		sOption = wxT(" -v");
		si.cb = sizeof(si);
		flags = 0;
		sPath = m_path + m_sep + m_name + m_ext + sOption;
		flags |= CREATE_NO_WINDOW;

		// Проверяем версию программы запуск с параметром -v
		if (CreateProcessW(0, (LPWSTR)sPath.wc_str(), 0, 0, false, flags, 0, 0, &si, &m_pi) != 0)
		{
			WaitForSingleObject(m_pi.hProcess, INFINITE);
			GetExitCodeProcess(m_pi.hProcess, &exitCode);
			m_version = exitCode;
			CloseHandle(m_pi.hThread);
			CloseHandle(m_pi.hProcess);
		}
		// Получили версию установленной программы
		m_frame->AddMessage(wxString::Format(_("Установленная версия")+wxT(" %0d.%02d"),m_version/100,m_version%100));

		// Запрашиваем версию с сервера
		sOption = wxT("");
		new_version = m_version;
		new_crc = GetLastVersion(clientSocket, new_version);
		// Сравниваем
		if (new_version > m_version)
		{
			// Если версии не равны проверяем, есть ли скачанная последняя версияс помощью CRC
			m_frame->AddMessage(wxString::Format(_("Найдено обновление до версии")+wxT(" %0d.%02d"), new_version / 100, new_version % 100));
			sPath = m_path + m_sep + UPLOADDIR + m_sep + m_name + ".zip";
			m_crc = CalcCRCFile(sPath.c_str());
			if (m_crc == -999999 || m_crc != new_crc)
			{
				// Если CRC не совпадает запускаем программу в режиме скачивания новой версии
				m_frame->AddMessage(_("Запущено скачивание новой версии ..."));
				if (f.Exists(sPath))
				{
					wxRemoveFile(sPath);
				}
				sOption = wxT(" -u");
			}
			else
			{
				// делаем бэкап старой версии и распаковываем новую
				sOption = wxT("");
				m_frame->AddMessage(_("Обновление программы ..."));
				m_frame->AddMessage(_("Создание резервной копии ..."));
				if (CreateBackup(m_path+ m_sep + BACKUPDIR + m_sep + m_name + BACKUPDIR + ".zip", m_path, m_name, m_sep))
				{
					m_frame->AddMessage(_("Обновление ..."));
					UnzipFile(sPath,m_path);
					m_frame->AddMessage(_("Обновление завершено."));
				}
				else
				{
					m_frame->AddMessage(_("Ошибка создания резервной копии."));
				}
			}
		}
		else
		{
			sOption = wxT("");
		}
	}

	m_frame->AddMessage(_("Запуск")+wxT(" GodPool."));

	// Запускаем программу.
	memset((char *)&si, 0, sizeof(si));
	memset((char *)&m_pi, 0, sizeof(m_pi));
	flags = 0;
	sOption += wxT(" -mp=\"") + wxApp::argv[0] + wxT("\"");
	sOption += wxT(" -lm");
	sPath = m_path + m_sep  + m_name + m_ext + sOption;

	if (CreateProcessW(0, (LPWSTR)sPath.wc_str(), 0, 0, false, flags, 0, 0, &si, &m_pi) != 0)
	{
		WaitForSingleObject(m_pi.hProcess, 10000);
		CloseHandle(m_pi.hThread);
		CloseHandle(m_pi.hProcess);
	}

	return 0; // wxApp::OnRun();
}


int Client::OnExit()
{
	return 0;
}


bool Client::CreateBackup(wxString pCompressedFile, wxString sPath, wxString sName, wxString sSep)
{
	wxDir dir(sPath);
	if (!dir.IsOpened())
		return false;

	wxArrayString dirList;
	wxString target_dir = sPath + sSep + BACKUPDIR;

	dir.GetAllFiles(sPath, &dirList, wxEmptyString, wxDIR_DIRS | wxDIR_FILES);

	if (dirList.GetCount() > 0)
	{
		wxFFileOutputStream out(target_dir + sSep + sName + BACKUPDIR + wxT(".zip"));
		wxZipOutputStream zip(out);

		for (int i = 0; i < dirList.GetCount(); i++)
		{
			wxFileInputStream in(dirList[i]);
			if ((dirList[i].Mid(dir.GetName().Length() + 1, strlen(BACKUPDIR)) != BACKUPDIR) && (dirList[i].Mid(dir.GetName().Length() + 1, strlen(UPLOADDIR)) != UPLOADDIR))
			{
				wxString s = "." + dirList[i].Mid(dir.GetName().Length());
				zip.PutNextEntry(s);
				zip.Write(in);
			}
		}
		zip.Close();
	}

	return true;
}


unsigned long long Client::GetLastVersion(wxSocketClient* clientSocket, int &version)
{
	SERVERPROTOCOL sp;
	sp.askCommand = GETVERSION;
	sp.filecrc = 0;
	sp.version = version;
	int to_process = sizeof(SERVERPROTOCOL);
	char *outbuffer = (char *)&sp;
	clientSocket->SetTimeout(10);

	do {
		clientSocket->Write(outbuffer, to_process);
		if (clientSocket->Error())
		{
			return 0L;
		}
		to_process -= clientSocket->LastCount();
		outbuffer += clientSocket->LastCount();
	} while (!clientSocket->Error() && to_process != 0);

	outbuffer = (char *)&sp;
	to_process = sizeof(SERVERPROTOCOL);
	do {
		clientSocket->Read(outbuffer, to_process);
		if (clientSocket->Error())
		{
			return 0L;
		}
		to_process -= clientSocket->LastCount();
		outbuffer += clientSocket->LastCount();
	} while (!clientSocket->Error() && to_process != 0);

	version = sp.version;
	return sp.filecrc;
}



ThreadWorker::ThreadWorker(const wxString& p_host, int cmd, int p_size, char *fName)
	: wxThread(wxTHREAD_DETACHED)
{
	m_host = p_host;
	m_spo.askCommand = cmd;
	m_spo.askSize = p_size;
	m_Filename = fName;
}

wxThread::ExitCode ThreadWorker::Entry()
{
	wxIPV4address ca;
	wxSocketClient *m_socket;
	ca.Hostname(m_host);
	ca.Service(DOWNLOADSERVERPORT);
	m_socket = new wxSocketClient(wxSOCKET_BLOCK | wxSOCKET_WAITALL);
	bool failed = false;
	if (!m_socket->Connect(ca))
	{
		wxLogError(wxT("Cannot connect to %s:%d"), ca.IPAddress().c_str(), ca.Service());
		failed = true;
	}
	else
	{
		m_socket->SetTimeout(10);
		m_socket->Notify(false);
		int to_process = sizeof(m_spo);
		int count_write = 0;
		do {
			m_socket->Write((char *)&m_spo + count_write, to_process);
			if (m_socket->Error())
			{
				wxLogError(wxT("ThreadWorker: Write error"));
				failed = true;
				break;
			}
			to_process -= m_socket->LastCount();
			count_write += m_socket->LastCount();
		} while (!m_socket->Error() && to_process != 0);
	}
	m_socket->Destroy();
	m_socket = NULL;
	return 0;
}

