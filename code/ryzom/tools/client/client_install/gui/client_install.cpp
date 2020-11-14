// launcher.cpp : fichier projet principal.

#include "stdafx.h"
#include "CClientInstallForm.h"
#include "game_downloader.h"
using namespace client_install;
int test(int argc, char* argv[]);

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Creation of the windows application
 	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 
	
	// Loading of the updater engine
	
	
	if ( !(args->Length > 0 && args[0]->Equals(gcnew System::String("-a")) ) ) 
	{				
		MessageBox::Show("Error: Using wrong parameters.",  L"Error" , MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1, MessageBoxOptions::DefaultDesktopOnly, false);
		return 0; // refuse to start if wrong args	
	}

	if (!System::IO::File::Exists("client.cfg"))
	{
		MessageBox::Show("Error: Using bad directory.",  L"Error" , MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1, MessageBoxOptions::DefaultDesktopOnly, false);		
		return 0;
	}

	IGameDownloader* gd = IGameDownloader::getInstance();
	gd->setAutoInstall(true);
	// loading of the main (on only form)
	client_install::CClientInstallForm^ form = gcnew client_install::CClientInstallForm();

	
	gd->setForm(form);
	Application::Run(form);
	delete gd;	// free of the engine
	return 0;
}
 