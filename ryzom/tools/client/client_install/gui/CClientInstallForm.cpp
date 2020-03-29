#include "stdafx.h"


#include "CClientInstallForm.h"
#include "CInstallPopupForm.h"
#include <stdlib.h>
#include "game_downloader.h"
#include <sstream>
#include <boost/format.hpp>
#include "vcclr.h"


using namespace client_install;

#using <System.dll>
#using <mscorlib.dll>
//#using <System.Configuration.Install.dll>



// helpers functions
//! Transforms a size to a value readable by human ( eg 4.54GB)
static System::String^ sizeToHuman(unsigned long long size)
{
	double total = size;		
	System::String^ unit;
	if (total >= 1000000000.0) // more thant 1GB
	{
		total /= 10000000.0;
		total =  (int)total ;
		total /= 100.0; //display 2 digit after point
		unit = "GB";
	} 
	else if (total >= 1000000.0) // more thant 1MB
	{
		total /= 10000.0;
		total = (int)total ;
		total /= 100.0; //display 2 digit after point
		unit = "MB";
	}
	else if (total >= 1000.0) // more thant 1KB
	{			
		total = int(total/1000.0);
		unit = "KB";
	}
	else
	{
		unit = "B";
	}
	return System::String::Format("{0}{1}", total, unit);
}

//! Transform Microsoft String to stl string
bool toStdString( String^ source, std::string &target )
{
	// alloc memeory (16b char)
	int len = (( source->Length+1) * 2);
	char *ch = new char[ len ];
	bool result ;
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars( source );
		//convert
		result = wcstombs( ch, wch, len ) != -1;
	}
	//set
	target = ch;
	delete [] ch ;
	return result;

} 

/** Wrap text in order to cut it after 50 char (also transform "\\n" int "\n")
Usefull when using text from trad
*/

static System::String^ wrapText(System::String^ content)
{
	// replace "\\n" by "\n"
	content = content->Replace("\\n", "\n");

	std::string tmp1, tmp2;
	toStdString(content, tmp1);
	unsigned int first = 0, last = tmp1.size();
	int count =0;
	for ( ; first != last; ++first)
	{
		if (tmp1[first] == '\n') { count = 0;}
		else { ++count; }
		tmp2.push_back(tmp1[first]);
		// if ligne is longer than 50 cut at the next ' '
		if (count >= 50 && tmp1[first] == ' ')
		{
			tmp2.push_back('\r');
			tmp2.push_back('\n');
			count = 0;
		}
	}
	content = gcnew System::String(tmp2.c_str());
	return content;
}

/** Add to registry specific value in order to identify user for stat purpose
*/
static bool loadProductInfo()
{
	IGameDownloader* gd = IGameDownloader::getInstance();

	bool firstTime = false;
	// Use code in gameshare instead
	std::string installId;
	// read value
	{
		// go/create to the Nevrax node
		Microsoft::Win32::RegistryKey^ root = Microsoft::Win32::Registry::CurrentUser->OpenSubKey("Software", true);
		Microsoft::Win32::RegistryKey^ nevrax = root->OpenSubKey("Nevrax", true);
		if (nevrax == nullptr)
		{
			nevrax = root->CreateSubKey( "Nevrax" );
		}

		// go/create to the RyzomInstall subnode
		Microsoft::Win32::RegistryKey^ ryzomInstall = nevrax->OpenSubKey("RyzomInstall", true);
		if (ryzomInstall == nullptr)
		{
			srand(time(0));
			unsigned int r = rand();
			r <<= 16;
			r |= rand(); // rand() is only 16bit on Windows plateform
			System::String^ str = System::String::Format("{0}", r);
			ryzomInstall = nevrax->CreateSubKey( "RyzomInstall" );
			ryzomInstall->SetValue( "InstallId", str );	
			firstTime = true;
		}

		// Reset the install subnode
		ryzomInstall->SetValue( "LoginStep", gcnew System::Int64(0), Microsoft::Win32::RegistryValueKind::DWord); //at each install restart loginStep info	

		System::Object^ value = ryzomInstall->GetValue("InstallId");
		System::String^ valueStr =  dynamic_cast<System::String^ >( value);
		ryzomInstall->Close();
		
		toStdString(valueStr, installId);
		gd->setInstallId(installId);		
	}
	return firstTime;
}


/** Extract a value from an simple xml vlaue "<token>value</token>"
*/
static std::string extractToken(const std::string& res, const std::string& token)
{
	std::string ret("");
	std::string tokenBegin = std::string("<") + token + ">";
	std::string::size_type tokenBeginLen = tokenBegin.size();
	std::string tokenEnd = std::string("</") + token + ">";
	std::string::size_type tokenEndLen = tokenBegin.size();

	// if res looks like "<token>value</token>" extract value and put it into ret
	std::string::size_type begin = res.find(tokenBegin);
	std::string::size_type end = begin != std::string::npos && res.size() > tokenBeginLen + tokenEndLen ? res.find(tokenEnd, begin+tokenBeginLen) : std::string::npos;
	if (begin != std::string::npos && end != std::string::npos)
	{
		ret = res.substr(begin+tokenBeginLen, end - (begin+tokenBeginLen));
	}
	return ret;
}


//------ Gui Functions


System::Void CClientInstallForm::CClientInstallForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	// if cancel button is clicked display warning windows
	IGameDownloader* gd = IGameDownloader::getInstance();
	if (gd->getState() != IGameDownloader::InstallFinished)
	{
		e->Cancel = true;
		this->onPopupAction(nullptr, "download_cancel", true);	
	}
}
System::Void CClientInstallForm::buttonRepair_Click(System::Object^  sender, System::EventArgs^  e) 
{
	// if cancel button is clicked display warning windows
	IGameDownloader* gd = IGameDownloader::getInstance();
	if (gd->getState() != IGameDownloader::InstallFinished)
	{
		//e->Cancel = true;
		this->onPopupAction(nullptr, "download_cancel", true);	
	}
}

System::Void CClientInstallForm::buttonInstall_Click(System::Object^  sender, System::EventArgs^  e)
{
	// display "start downloading" windows
	onPopupAction(nullptr, "welcome_continue", true);	
}

System::Void CClientInstallForm::webBrowser1_Navigating(System::Object^  sender, System::Windows::Forms::WebBrowserNavigatingEventArgs^  e)
 {
	//refuse to follow link to page other that initial page
	IGameDownloader* gd = IGameDownloader::getInstance();
	System::String^ str = e->Url->ToString();
	
	System::String^ str2 = gcnew System::String(gd->getInstallHttpBackground().c_str());		
	str2 = str2->Replace(":80/", "/");

	if ( str->Equals( str2) )		
	{
		return;
	}

	// if url value different from start value launche the url in the default navigator
	e->Cancel = true;
	System::Diagnostics::Process ^ proc = gcnew System::Diagnostics::Process();
	proc->StartInfo->FileName = e->Url->ToString();
	proc->Start();

	
 }

System::Void CClientInstallForm::checkBoxFullVersion_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	IGameDownloader* gd = IGameDownloader::getInstance();

	unsigned long long value;
	if ( this->checkBoxFullVersion->Checked)
	{
		value = gd->getRyzomPatchSize();		
	}	
	else
	{
 		value =  gd->getRosPatchSize();
	}
	System::String ^str = System::String::Format(_RessourceManager->GetString("uiRequiredSize"), sizeToHuman( value )) ;
	this->labelFullVersion->Text = str;

	//update Required Size lable if Version change from full ton initial version
}

System::Void CClientInstallForm::checkedListBoxPackageSelection_ItemCheck(System::Object^  sender, System::Windows::Forms::ItemCheckEventArgs^  e)
{
	// !! obsolete was for disabling the possibility of unchecking Ros package
	if (e->Index == 0 && e->NewValue != System::Windows::Forms::CheckState::Checked)
	{
		e->NewValue = System::Windows::Forms::CheckState::Checked;
	}
}


System::Void CClientInstallForm::timer1_Tick(System::Object^  sender, System::EventArgs^  e)
{
	// display after few second (wait for the page to be loaded)
	IGameDownloader* gd = IGameDownloader::getInstance();
	//update core (that will update dll, that will update gui)
	gd->update();

	// update labels if changed by IGameDownloader
	// update label "full version"
	if (_LabelFullVersionValue != nullptr)
	{
		this->labelFullVersion->Text =  _LabelFullVersionValue;
		this->labelFullVersion->Visible = true;
		_LabelFullVersionValue = nullptr;
	}
	// update main label
	if (_NewValue != nullptr)
	{		
		this->labelContent->Text = _NewValue;
		_NewValue = nullptr;
	}

	{
		// update "state" label
		static int tick = 0;
		if (_NewState != nullptr)
		{
			this->labelState->Text = _NewState;
			this->labelState->Visible = true;
			_NewState = nullptr;
			tick = 0;
		}

		// hide lavel after 3 sec
		if (this->labelState->Visible)
		{
			++tick;
			if (tick > 30)
			{
				this->labelState->Visible = false;
			}
		}
	}

	// update progress bar value
	if (_NewProgressBarValue != -1 )
	{
		if ( 0 <= _NewProgressBarValue && _NewProgressBarValue <= 100)
		{
			this->progressBar->Value =  _NewProgressBarValue;
		}
		_NewProgressBarValue = -1;
	}
	// at start of application lauch starting windows
	if (gd->getState() == IGameDownloader::Init && gd->getAutoInstall() && !gd->getStarted())
	{
		this->onPopupAction(nullptr, "welcome_continue", true);
		return;
	}
	//At end of init load patch url from login/client_install.php
	if (gd->getState() == IGameDownloader::InitFinished)
	{				
		gd->setState(IGameDownloader::UpdateVersion);

		System::String^ addresse = gcnew System::String(gd->getPatchUrl().c_str());
		System::Net::WebRequest^ webrequest =
			System::Net::WebRequest::Create( addresse );
		webrequest->Timeout=10000;	
		System::Xml::XmlTextReader^ reader = nullptr; 
		System::Xml::XmlDataDocument^ document = nullptr; 
		try 
		{
			// throw an exeption if no web page	
			document = gcnew System::Xml::XmlDataDocument();
			reader = gcnew System::Xml::XmlTextReader(webrequest->GetResponse()->GetResponseStream());
			document->Load(reader);
		}
		catch (System::Exception^ )
		{
			fatalError("uiLoadVersionError", "", "");
			return;
		}
		

		// parse login/client_install.php
		System::Xml::XmlNode^ node = document->FirstChild;
		System::Xml::XmlElement^ version = dynamic_cast<System::Xml::XmlElement^>(node);				
		System::Xml::XmlNodeList^ patchURIs = version->GetElementsByTagName("patchURI");		
		std::vector<std::string> patchUris;
		unsigned int first=0, last = patchURIs->Count;		
		for (; first != last; ++first)
		{
			System::String^ patchURI = patchURIs->Item(first)->InnerText;
			std::string patchUrisStr;
			toStdString(patchURI, patchUrisStr);
			patchUris.push_back(patchUrisStr);
		}
		std::string serverPathStr;
		System::String^ serverPath = version->GetAttribute("serverPath");
		toStdString(serverPath, serverPathStr);


		// update core dpwnload  patch url
		gd->setVersionInfo( patchUris, serverPathStr);		

		// go to next state
		gd->setState(IGameDownloader::UpdateVersionFinished);

		// start to load background web pages (but do not display it)
		this->webBrowser->Url = gcnew System::Uri(gcnew System::String(gd->getInstallHttpBackground().c_str()));
		_NewValue = nullptr;
		this->labelContent->Text = _RessourceManager->GetString("uiUpdateLauncher");
		this->buttonInstall->Visible = false;
		
		return;
	}

	if (gd->getState() == IGameDownloader::UpdateVersionFinished)
	{	//obsolete code to update patch by depacking a bnp that could contain himslef
		gd->setState(IGameDownloader::UpdateLauncher);		
		if (!gd->updateLauncher())
		{  // not used
			gd->setState(IGameDownloader::UpdateLauncherFinished);
			_NewValue = nullptr;
			this->labelContent->Text = _RessourceManager->GetString("uiRestarting");
		}
		else
		{	// alway used
			gd->setState(IGameDownloader::InstallFinished);
			_NewValue = nullptr;
			this->labelContent->Text = _RessourceManager->GetString("uiLoadTorrent");
		}
		return;
	}

	if (gd->getState() == IGameDownloader::UpdateLauncherFinished)
	{	// Look if torrent file exist on server
		gd->setState(IGameDownloader::UpdateTorrent);				
		if (! gd->updateTorrent() )
		{
		//	warningError("uiTorrentUpdateError");
			gd->setTorrentExist(false);
		}
		else
		{
			gd->setTorrentExist(true);
		}

		//go next state
		_NewValue = nullptr;
		this->labelContent->Text =  _RessourceManager->GetString("uiLoadPackageList");
		gd->setState(IGameDownloader::UpdateTorrentFinished);		
		return;
	}

	if (gd->getState() == IGameDownloader::UpdateTorrentFinished)
	{	// start the loading of different package
		// if _NeedRepair (client already downloaded) a MD5 check would be added
		gd->setState(IGameDownloader::UpdatePackage);
		gd->getPackageSelection(_NeedRepair);		
		this->labelContent->Visible = true;
		this->progressBar->Value = 0;
		this->progressBar->Visible = true;
		return;
	}

	if (gd->getState() == IGameDownloader::AskInstallFinishing)
	{
		// automatic after download
		gd->setState(IGameDownloader::InstallFinishing);
		this->onFileInstallFinished();
		return;
	}
	if (gd->getState() == IGameDownloader::InstallFinished)
	{	// At end hide the form, wait 5 seconde then close
		static int tick = 0;
		if (tick == 0)
		{	
			//this->checkedListBoxPackageSelection->Visible = false;
			this->labelFullVersion->Visible = false;
			this->checkBoxFullVersion->Visible = false; 
			this->progressBar->Visible = false;

		}
		++tick;
		
		if (tick == 10)
		{
			this->Close();
		}
		return;
	}

	if (gd->getState() == IGameDownloader::Downloading)
	{
		static int tick = 0;
		++tick;
		if (tick == 10*2)
		{
			// if downloading every 10s send msg to php to inform of progress
			int value = this->progressBar->Value * 2;
			System::String^ val = System::String::Format("update_download&percent={0}", value);
			std::string val2;
			toStdString(val, val2);
			logOnServer(val2.c_str());
			tick = 0;
		}

		return;
	}
	if (gd->getState() == IGameDownloader::Installing)
	{
		static int tick = 0;
		++tick;
		if (tick == 10*2)
		{
			// if installing every 10s send msg to php to inform of progress
			// install bar start at 50
			int value = (this->progressBar->Value - 50)*2;
			System::String^ val = System::String::Format("update_install&percent={0}", value);
			std::string val2;
			toStdString(val, val2);
			logOnServer(val2.c_str());
			tick = 0;
		}
		return;
	}
}
//! Translate duration in seconde to string readable by humain
System::String^ CClientInstallForm::timeToHuman(double t)
{
	
	if (t==0)	
	{
		return  gcnew System::String("");
	}	
	double days = 60*60*24;
	double hours = 60*60;
	double minutes = 60;		

	unsigned int d, h, m, s;

	// comput duration in day hours month secod
	d = static_cast<unsigned int>(t / days);
	t = t - d * days;
	h = static_cast<unsigned int>(t / hours);
	t = t - h * hours;
	m = static_cast<unsigned int>(t / minutes);
	t = t - m * minutes;
	s = static_cast<unsigned int>(t);
	
	System::String^ ret = gcnew System::String("");
	// if bigger thant a day : time is too big to be meaningfull
	if ( d > 0)
	{
		return gcnew System::String("-");
	}
	
	if ( h > 0)
	{	// use translatiion to display hours
		System::String^ format = _RessourceManager->GetString(h==1?"uiHour":"uiHours");
		ret = ret->Concat(ret, " ", System::String::Format(format, h));
	}

	if ( m > 0 && h == 0)
	{		
		// use translatiion to display minutes (if more thant one hours do not display minute because its not meanginfull)
		System::String^ format = _RessourceManager->GetString(m==1?"uiMinute":"uiMinutes");
		ret = ret->Concat(ret, " ", System::String::Format(format, m));
	}

	if ( s >= 0 && h ==0 && m < 3)
	{
		// use translatiion to display seconds (if more than 3minutes its not meaningfull to display seconds)
		System::String^ format = _RessourceManager->GetString(s==1?"uiSeconde":"uiSecondes");
		ret = ret->Concat(ret, " ",  System::String::Format(format, s));
	}
	// display something link "(3 minute restante)"
	ret = System::String::Format(_RessourceManager->GetString("uiTimeLeft"), ret);
	
	return ret;
}

void CClientInstallForm::fatalError(const char* msg, const char* param1, const char* param2)
{

	// do not dispaly fatal Error 2 time
	IGameDownloader* gd = IGameDownloader::getInstance();
	if (gd->getState() == IGameDownloader::InstallFinished)
	{
		return;
	}

	// display Error message
	System::String^ str =  _RessourceManager->GetString( gcnew System::String(msg) );
	System::String^ content = System::String::Format(str, gcnew System::String(param1), gcnew System::String(param2));
	
	//Display a message asking to check support formum (Support Url is different between language)
	System::String^ supportUrl = _RessourceManager->GetString( "uiErrorSupportUrl");
	System::String^ supportUrlFormat = _RessourceManager->GetString( "uiErrorSupportFormat");
	
	content = content->Concat(content, "\n\n", System::String::Format(supportUrlFormat, supportUrl) );

	// Display information how to restart launcher by clicking on ryzom icone
	// only if ryzom has not been already installed
	if ( gd->getFirstInstall())
	{
		content = content->Concat(content, "\n\n", _RessourceManager->GetString( "uiQuitInstallRestart" ));
	}

	// Launch url via web browser
	{
		System::Diagnostics::Process ^ proc = gcnew System::Diagnostics::Process();
		proc->StartInfo->FileName = supportUrl;
		proc->Start();
	}
	// display an Error Modal Window
	System::Windows::Forms::DialogResult r = MessageBox::Show( wrapText(content),  L"Error" , MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1, MessageBoxOptions::DefaultDesktopOnly, false);

	// stop the program
	gd->setState(IGameDownloader::InstallFinished);
}

void CClientInstallForm::displayMsgClientUpdated(bool startClient)
{
	IGameDownloader* gd = IGameDownloader::getInstance();
	// update progress base and description label of main form
	_NewValue =gcnew System::String(_RessourceManager->GetString( "uiInstallFinished") );
	_NewProgressBarValue = 100;	
	
	_NewValue = _NewValue->Replace("\\n", " ");
	client_install::CInstallPopupForm ^ child = gcnew client_install::CInstallPopupForm();
	child->init(this);
	// Display message "installashing finished" with 1 button continue and 1 check box start at end
	child->setText(_RessourceManager->GetString( "uiTitleInstallSuccess" ), _RessourceManager->GetString( "uiContentInstallSuccess" ));
	child->setButton(1, "launch_continue", _RessourceManager->GetString("uiContinue"));
	child->setCheckBox(1, "launch_start_at_end", _RessourceManager->GetString( "uiLaunchAtEnd" ), gd->getStartRyzomAtEnd(), true);
	
	this->webBrowser->Visible = false;
	child->StartPosition = System::Windows::Forms::FormStartPosition::Manual;
	// put the windows at almost the center of the screen
	child->Location = System::Drawing::Point(
		(System::Windows::Forms::Screen::PrimaryScreen->Bounds.Width /2) + 100,
		(System::Windows::Forms::Screen::PrimaryScreen->Bounds.Height /2) + 100);
		
	child->ShowDialog();
	// if cancel selected (finish the depack but do not run)
	if (gd->getState() == IGameDownloader::InstallFinished)
	{
		startClient = false;
		gd->setStartRyzomAtEnd(false);
	}
	// The install thread is waiting for use to finish the install process
	if (this->WaitMutex)
	{
		this->WaitMutex->ReleaseMutex();
	}
	// startClient is true when lauching the installer and client is alread uptodate
	// if this function is called via install process, the ryzom thread take the responsability to launch the exe
	if (startClient && gd->getStartRyzomAtEnd())
	{
		if (System::IO::File::Exists("client_ryzom_rd.exe") )
		{
			// if must start client and client exist launch the client 
			System::Diagnostics::ProcessStartInfo^ psi = gcnew System::Diagnostics::ProcessStartInfo();
			psi->FileName = "./client_ryzom_rd.exe";
			psi->UseShellExecute = false;
			System::Diagnostics::Process::Start(psi); 
		}
	}
	// wait few second before stoping
	this->Visible = false;
	System::Threading::Thread::Sleep(10000);
	gd->setState(IGameDownloader::InstallFinished);
}

void CClientInstallForm::init()
{
	_NewProgressBarValue = -1;
	IGameDownloader* gd = IGameDownloader::getInstance();


	this->_NeedRepair = false;
	
	
	// initiate the windows i18n process
	std::string lang = gd->getLanguage();
	std::string ressource = std::string("client_install.") + lang;
	_RessourceManager = gcnew System::Resources::ResourceManager(gcnew System::String(ressource.c_str()),  System::Reflection::Assembly::GetExecutingAssembly());	
	
	// check if the file "unpack\exedll.bnp" exist. If it exists it means that the install process was finisheed previously..
	if (System::IO::File::Exists("unpack\\exedll.bnp") )
	{
		gd->setFirstInstall(false);
		// the continue button is nename repair in case of repair
		this->buttonInstall->Text = _RessourceManager->GetString( "uiRepair" );;	
	}
	else
	{
		gd->setFirstInstall(true);
		this->buttonInstall->Text = _RessourceManager->GetString( "uiInstall" );;
	}
	// update description label
	System::String^ tmp2 = gcnew System::String( _RessourceManager->GetString( (gd->getFirstInstall()?"uiRyzomInstall":"uiRyzomUpdateRepair") ) );
	this->Text = tmp2->Replace("\n", "");
	
	// if auto install mode do not show the install button at first (because in this case we manualy "click" on this invisible button at start)
	this->buttonInstall->Visible = !gd->getAutoInstall();
	
	this->checkBoxUseTorrent->Text = _RessourceManager->GetString("uiCheckBoxUseTorrentLabel");

	/* The ressource file contains a file like fr.uxt that contains the ryzom  i18n*/
	System::Object^ i18nFile = _RessourceManager->GetObject(gcnew System::String(lang.c_str()));
	if (i18nFile == nullptr)
	{
		fatalError("uiNoRessourceFile", lang.c_str(), "");
		return;
	}
	//load i18n ryzom file
	cli::array<System::Byte>^ bytes = cli::safe_cast<cli::array<System::Byte>^ >(i18nFile);
	pin_ptr<unsigned char> tmp = &bytes[0];
	gd->initI18n(lang.c_str(), tmp, bytes->Length);	

	//lock a mutex (in order to ask the install thread to wait the final Popup to close before finsihing)
	this->WaitMutex = gcnew System::Threading::Mutex();
	this->WaitMutex->WaitOne();
}


std::string CClientInstallForm::logOnServer(const char* msg)
{
	std::string ret;
	try
	{
		IGameDownloader* gd = IGameDownloader::getInstance();
		//get the url to send message to the get format (http://r2linux03/stats/stats.php?param1=42&param2=43")
		std::string str = gd->getLogUrl(msg);
		
		// extract param after the "?" in the url
		std::string page;
		std::string data;
		std::string::size_type sep = str.find("?");
		if (sep != std::string::npos)
		{
			page = str.substr(0, sep);
			data = str.substr(sep + 1);
		}
		else
		{
			page = str;
		}
		System::Net::WebRequest^ webrequest =
			System::Net::WebRequest::Create( gcnew System::String(str.c_str()) );

		// if there is param after the "?" in the url then send param via POST method
		if (!data.empty())
		{
			webrequest->Method = "POST";
			webrequest->ContentType = "application/x-www-form-urlencoded";

			cli::array<System::Byte>^ bytes = nullptr;
			bytes = System::Text::Encoding::ASCII->GetBytes (gcnew System::String(data.c_str()));
			webrequest->ContentLength = bytes->Length;
			webrequest->GetRequestStream ()->Write(bytes, 0, bytes->Length);			
			
		}

		// throw an exeption if no web page	
		webrequest->Timeout=10000;		
		System::IO::Stream^ stream = webrequest->GetResponse()->GetResponseStream();
		System::IO::StreamReader^ streamReader = gcnew System::IO::StreamReader(stream);
		System::String^ value = streamReader->ReadToEnd();
		toStdString(value, ret); //return value if exist		
	}
	catch(...)
	{
	}
	return ret;
}

void CClientInstallForm::packageSelectionUpdated(ICkeckInfoViewer* viewer)
{
	IGameDownloader* gd = IGameDownloader::getInstance();
	// update required size label with the size needed to download
	System::String ^str = System::String::Format(_RessourceManager->GetString("uiRequiredSize"), sizeToHuman( gd->getRosPatchSize())) ;
	this->labelFullVersion->Text = str;
	this->labelFullVersion->Visible = true;


	this->progressBar->Value = 0;
	this->_NewValue = nullptr;
	this->labelContent->Text = _RessourceManager->GetString("uiSelectPackage");	


	this->labelState->Visible = false;
	
	/* Display a description form with 2 buton cancel and continue and 2 check box full version or use torrent
	*/
	{
		
		client_install::CInstallPopupForm ^ child = gcnew client_install::CInstallPopupForm();						
		child->init(this);
		// dynamicaly change the description text when changing option (full version or torrent)
		updateStartProcessText(child);

		{
			// The continue button has a different label if we are in install mode or in repair mode
			if (gd->getFirstInstall())
			{
				child->setButton(1,  "download_continue", _RessourceManager->GetString("uiInstall"));
			}
			else
			{
				child->setButton(1, "download_continue",  _RessourceManager->GetString("uiContinue"));
			}

			child->setButton(2, "download_cancel", _RessourceManager->GetString("uiCancel"));
		}
		{
			child->setCheckBox(1, "download_full_version", _RessourceManager->GetString("uiFullVersion"), gd->getUseFullVersion(), 1);
			child->setCheckBox(2, "download_use_torrent", _RessourceManager->GetString("uiCheckBoxUseTorrentLabel"), gd->getUseTorrent(), gd->getTorrentExist());
		}
		this->buttonRepair->Text = _RessourceManager->GetString("uiCancel");
		this->buttonRepair->Visible = true;

	

		// dispaly the install window almost at the center of the screen
		child->StartPosition = System::Windows::Forms::FormStartPosition::Manual;

		child->Location = System::Drawing::Point(
			(System::Windows::Forms::Screen::PrimaryScreen->Bounds.Width /2 - child->Width / 2) + 60 ,
			(System::Windows::Forms::Screen::PrimaryScreen->Bounds.Height /2 - child->Height / 2) + 10);
		child->ShowDialog();
		this->webBrowser->Visible = true; 
		//this->Visible = true;
	}
	
}

void CClientInstallForm::updateStartProcessText(client_install::CInstallPopupForm ^ child)
{
	// update the Start download install text in function of the mode (install, repaire), the version(full, normal), the protocol(torrent, normal)
	IGameDownloader* gd = IGameDownloader::getInstance();
	System::String^ title = nullptr;		
	System::String^ content = nullptr;
	
	
	if (gd->getFirstInstall())		
	{
		// text install
		title = _RessourceManager->GetString("uiTitleStartProcessInstall");
		content = System::String::Format( _RessourceManager->GetString("uiContentStartProcessInstall"), sizeToHuman( gd->getRyzomInstallSize()));			
	}
	else
	{
		// text repair
		title = _RessourceManager->GetString("uiTitleStartProcessRepair");
		content = System::String::Format( _RessourceManager->GetString("uiContentStartProcessRepair"), sizeToHuman( gd->getRyzomInstallSize()));
	}

	if ( !gd->getUseFullVersion())
	{		
		// text explaining full version
		content = content->Concat(content, "\n\n", _RessourceManager->GetString("uiContentStartProcessFullVersion"));
	}

	if (gd->getUseTorrent())
	{
		// text explaining torrent protocol
		content = content->Concat(content, "\n\n", _RessourceManager->GetString("uiContentStartProcessTorrent"));
	}
	content = content->Replace("\\n", "\n");
	child->setText(title, content);
}
void  CClientInstallForm::addToDownloadList(const char* patchName, const char* sourceName, unsigned int timestamp)
{

}

void  CClientInstallForm::onDownloadListFinished()
{	
	//server give us the list of file to download
	IGameDownloader* gd = IGameDownloader::getInstance();

	this->_NewValue = _RessourceManager->GetString("uiDownloadListUpdated");

	std::string session;
	// set _ProductInfo key in the registry 
	bool firstTime = loadProductInfo();
	firstTime = true;//force the init message each time
	
	// get the sql stat user id from server
	std::string res;
	res = logOnServer("login");
	std::string id = extractToken(res, "user_id");
	gd->setUserId(  id);

	// if user unknow send the hardware description
	if (id.empty())
	{
		res = logOnServer("init");
		gd->setUserId( extractToken(res, "user_id") );
	}

	// creat a new download session and get the sql stars session_id
	res = logOnServer("start_download");
	gd->setSessionId( extractToken(res, "session_id"));
}




// obsolete?
void CClientInstallForm::updateDownloading(const char* actionState, const char* name, unsigned long long progress, unsigned long long rate, unsigned long long done, unsigned long long wanted)
{
	_NewValue = nullptr;
	if (rate)
	{
		this->labelContent->Text = System::String::Format(" {0} {1} at {2}/s - {3} of {4}", gcnew System::String(actionState), gcnew System::String(name), sizeToHuman(rate), sizeToHuman(done), sizeToHuman(wanted)); 
	}
	else
	{
		this->labelContent->Text = System::String::Format(" {0} {1}  {3} of {4}", gcnew System::String(actionState), gcnew System::String(name), sizeToHuman(rate), sizeToHuman(done), sizeToHuman(wanted)); 
	}
	this->progressBar->Value =  int(50.0*progress/100.0);
}

void CClientInstallForm::updateInstalling(unsigned int progress)
{
	//update install progress bar
	_NewValue = nullptr;
	this->labelContent->Text =_RessourceManager->GetString( "uiInstalling" );
	this->progressBar->Value = 50 + int(50.0*progress/100.0);
}

void CClientInstallForm::onFileInstall(const char * filename, unsigned int index, unsigned int filecount)
{
	//message from CGameDownloader is formated
	System::String^format = _RessourceManager->GetString( "uiInstallFormat" );
	_NewValue =  System::String::Format(format, gcnew System::String(filename),  index, filecount );
	_NewProgressBarValue = 50 + ((100 * index) / filecount)/2;
}


void CClientInstallForm::onFileUpdate(const char * trad, const char * filename, unsigned int rate, unsigned int index, unsigned int filecount,  unsigned int firstValue, unsigned int lastValue, unsigned long long size, unsigned long long fullSize, const char * category  )
{
	//function used by install and download process

	System::String^ tr = gcnew System::String(_RessourceManager->GetString( gcnew System::String(trad) ) );
	System::String ^f = gcnew System::String("");
	System::String ^r = gcnew System::String("");
	System::String ^s = gcnew System::String("");
	System::String ^i = gcnew System::String("");
	
	//if filename exist add "of filename" to sentence
	if (filename)
	{
		
		System::String^ filenameFormat = gcnew System::String(_RessourceManager->GetString("uiFilenameFormat"));
		if (filenameFormat == nullptr) { return; }
		f = System::String::Format(filenameFormat, gcnew System::String(filename));
	}

	//if rate exist add "at 18KB/S" to sentence
	if (rate)
	{
		
		System::String^ rateFormat = gcnew System::String(_RessourceManager->GetString("uiRateFormat"));
		r = System::String::Format(rateFormat, sizeToHuman(rate));
	}

	//if filecount exist add "(14/15)" to sentence
	if (filecount)
	{
		i = System::String::Format(" ({0}/{1})", index, filecount);
	}
	//if filecount exist add " - (16MB/205MB)" to sentence
	if (fullSize)
	{
		s = System::String::Format(" - {0}/{1}", sizeToHuman(size), sizeToHuman(fullSize));
	}
	
	//display sentence like "download of toto.bnp at 14MB/S (1/4) -( 200MB/1.6GB)
	_NewValue =  System::String::Format("{0}{1}{2}{3}{4}", tr, f, r, i, s);
	if (fullSize) { _NewProgressBarValue = firstValue + ( (lastValue-firstValue) * size) / fullSize; }
	else if (filecount) { _NewProgressBarValue = firstValue + ( (lastValue-firstValue) * index) / filecount; }

	// if progress bar has invalide value do not update it
	if (_NewProgressBarValue != -1 && ( _NewProgressBarValue < 0 && 100 < _NewProgressBarValue ))
	{
		System::String ^error = nullptr;
	}

	// if category can be equaldownload or upload
	if (category)
	{
		IGameDownloader* gd = IGameDownloader::getInstance();
		static std::string previous("");
		static long long rate2 = 0;

		// smooth a litle bit the rate
		if (previous == category)
		{
			rate2 = (3*rate2 + rate)/4;  
		}
		else
		{
			previous = std::string(category);
			rate2 = rate;
		}

		System::String^ timeleft = gcnew System::String("");
		if (rate2)
		{
			
			double t = double(fullSize - size) /  rate2;
			// when downloading we add the time "estimate" the install process will tack
			if (std::string(category) == std::string("uiCategoryDownload"))
			{
				t +=  double (gd->getUseFullVersion() ?	gd->getRyzomInstallSize() : gd->getRosInstallSize() ) / 7000000.0;				
			}

			timeleft = timeToHuman(t);
		}
		//dispaly at "14hours left" to the "Size requred label"
		timeleft= timeleft->Concat(gcnew System::String(_RessourceManager->GetString(gcnew System::String(category))), gcnew System::String(" "), timeleft);

		_LabelFullVersionValue = timeleft;
	}
}

void CClientInstallForm::onFileInstallFinished()
{
	//inform the stat sever the install is finished
	logOnServer("stop_install");	
	//display a popup that ask the player if he want to launch ryzom
	this->displayMsgClientUpdated(false);	
}

void CClientInstallForm::askForInstall(long long value)
{
	//*WAS* display a message askin to install but now just update label on progessbar and send message to start server

	// update labels
	IGameDownloader* gd = IGameDownloader::getInstance();
	_NewValue =	nullptr;
	this->labelContent->Text = gcnew System::String(_RessourceManager->GetString("uiAskForInstall"));
	_NewState = nullptr;
	this->labelState->Text = gcnew System::String("");
	this->buttonInstall->Visible = false; //done by the other form
	System::String ^str = System::String::Format(_RessourceManager->GetString("uiRequiredSize"), sizeToHuman( value )) ;
	this->labelFullVersion->Text = str;
	this->labelFullVersion->Visible = true;
	gd->setStartRyzomAtEnd(true);
	this->checkBoxFullVersion->Checked = true;
	this->checkBoxFullVersion->Text = _RessourceManager->GetString("uiLaunchAtEnd");
	this->checkBoxFullVersion->Visible = false; // done by the other form
	_NewProgressBarValue =50;	

	// sends message to stat server
	{
		logOnServer("stop_download");		
		//Donwlod Only option is obsolet
		if (!gd->getDownloadOnly())
		{		
			onPopupAction(nullptr, "install_continue", true);
			logOnServer("start_install");
		}		
	}


}

void CClientInstallForm::onPopupAction(client_install::CInstallPopupForm^ form, System::String^ action, bool state)
{
	IGameDownloader* gd = IGameDownloader::getInstance();
	// if click on cancel button of 
	if (action->Equals("download_cancel"))
	{	
		
		// display a "Are you sure to quit" popup message
		System::String^ title = _RessourceManager->GetString( "uiQuitInstallTitle" );
		System::String^ content = _RessourceManager->GetString( "uiQuitInstallContent" );
		content = content->Concat(content, "\n\n", _RessourceManager->GetString( "uiQuitInstallRestart" ));
		System::Windows::Forms::DialogResult r =	MessageBox::Show(  wrapText(content), title , MessageBoxButtons::OKCancel, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1, MessageBoxOptions::DefaultDesktopOnly, false);
		// the user want to quit so close the application
		if  (r == Windows::Forms::DialogResult::OK)
		{
			// hide the windows and change the current state to closing
			if (form)
			{
				if (form->Visible)
				{
					form->Visible = false;
				}
				if (this->Visible)
				{
					this->Visible = false;
				}
				form->closePopup();	
			}
			gd->setState(IGameDownloader::InstallFinished);
			return;				
		}
		else
		{	// the user click on cancel so go back to where we were			
		}
		
		return;
	}

	// Start of application
	if (gd->getState() == IGameDownloader::Init)
	{
		// Update label in function fo the mode
		if (action->Equals("welcome_continue"))
		{
			gd->setStarted(true);
			if ( gd->getFirstInstall() )
			{
				this->_NeedRepair = false;
				_NewValue = nullptr;
				this->labelContent->Text =  _RessourceManager->GetString( "uiLoadVersionInfo" );
				gd->setState(IGameDownloader::InitFinished);
			}
			else
			{
				this->_NeedRepair = true;
				_NewValue = nullptr;
				this->labelContent->Text = _RessourceManager->GetString( "uiCheckData" );
				gd->setState(IGameDownloader::InitFinished);
			}
			
			return;
		}
		return;
	}

	// Download/Install confirmation form
	if (gd->getState() == IGameDownloader::CheckFinished)
	{
		// click on Full version check box -> update the text displayed
		if (action->Equals("download_full_version"))
		{
			gd->setUseFullVersion(state);
			this->updateStartProcessText(form);
			return;
		}

		// click on Torrent protocol box -> update the text displayed
		if (action->Equals("download_use_torrent"))
		{
			gd->setUseTorrent(state);
			this->updateStartProcessText(form);
			return;
		}

		// click on continue button at the download asking -> update the text displayed
		if (action->Equals("download_continue"))
		{			
			form->closePopup();

			this->labelFullVersion->Visible = false;			
			this->labelContent->Text =_RessourceManager->GetString( "uiLoadDownloadList" );
			_NewValue = nullptr;								
			gd->selectPackage(gd->getUseFullVersion());
			gd->setUseTorrent(gd->getUseTorrent());
			gd->getDownloadList();
			return;


		}
		return;
	}

	// Ask install form (no more displayed)
	if (gd->getState() == IGameDownloader::AskInstalling)
	{
		// click on continue button
		if (action->Equals("install_continue"))
		{
			// continue install
			gd->setState(IGameDownloader::InstallOk);
		}
		return;
	}

	// Form telling that the installation is over and asking the player if he want to lauch ryzom
	if (gd->getState() == IGameDownloader::InstallFinishing)
	{
		// if click on continue, continue the install process (with the "Start ryzom At End" setted by the check box)
		if (action->Equals("launch_continue"))
		{	
			form->closePopup();			
			this->progressBar->Visible = false;				
			return;
		}
		// click on Ryzom "Start at End" checkbox
		if (action->Equals("launch_start_at_end"))
		{			
			gd->setStartRyzomAtEnd( state );		
		}
		return;
	}

}