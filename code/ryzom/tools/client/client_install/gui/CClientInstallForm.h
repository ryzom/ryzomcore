#ifndef __CLIENT_INSTALL_FORM_H__
#define __CLIENT_INSTALL_FORM_H__

class ICkeckInfoViewer;
#include <string>
namespace client_install
{

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Description résumée de CClientInstallForm
	///
	/// AVERTISSEMENT : si vous modifiez le nom de cette classe, vous devrez modifier la
	///          propriété 'Nom du fichier de ressources' de l'outil de compilation de ressource managée
	///          pour tous les fichiers .resx dont dépend cette classe. Dans le cas contraire,
	///          les concepteurs ne pourront pas interagir correctement avec les ressources
	///          localisées associées à ce formulaire.
	/// </summary>
	ref class CInstallPopupForm;
	public ref class CClientInstallForm : public System::Windows::Forms::Form
	{

	public:
		CClientInstallForm(void)
		{
			InitializeComponent();
			//
			//TODO : ajoutez ici le code du constructeur
			//
			init();
			_NewProgressBarValue = -1;
		}

	protected:
		/// <summary>
		/// Nettoyage des ressources utilisées.
		/// </summary>
		~CClientInstallForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::Label^  downloadStateLabel;
	private: System::Windows::Forms::Timer^  timer1;


	private: System::Windows::Forms::ProgressBar^  progressBar;
	private: System::Windows::Forms::Button^  buttonInstall;
	private: System::Windows::Forms::Label^  labelContent;
	private: System::Windows::Forms::Button^  buttonRepair;
	public: System::Windows::Forms::CheckBox^  checkBoxUseTorrent;
	private: System::Windows::Forms::Label^  labelState;
	private: System::Windows::Forms::WebBrowser^  webBrowser;
	public: System::Windows::Forms::CheckBox^  checkBoxFullVersion;
	private: System::Windows::Forms::Label^  labelFullVersion;



	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Variable nécessaire au concepteur.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Méthode requise pour la prise en charge du concepteur - ne modifiez pas
		/// le contenu de cette méthode avec l'éditeur de code.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(CClientInstallForm::typeid));
			this->downloadStateLabel = (gcnew System::Windows::Forms::Label());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->progressBar = (gcnew System::Windows::Forms::ProgressBar());
			this->buttonInstall = (gcnew System::Windows::Forms::Button());
			this->labelContent = (gcnew System::Windows::Forms::Label());
			this->buttonRepair = (gcnew System::Windows::Forms::Button());
			this->checkBoxUseTorrent = (gcnew System::Windows::Forms::CheckBox());
			this->labelState = (gcnew System::Windows::Forms::Label());
			this->webBrowser = (gcnew System::Windows::Forms::WebBrowser());
			this->checkBoxFullVersion = (gcnew System::Windows::Forms::CheckBox());
			this->labelFullVersion = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// downloadStateLabel
			// 
			this->downloadStateLabel->AutoSize = true;
			this->downloadStateLabel->Location = System::Drawing::Point(12, 358);
			this->downloadStateLabel->Name = L"downloadStateLabel";
			this->downloadStateLabel->Size = System::Drawing::Size(0, 13);
			this->downloadStateLabel->TabIndex = 1;
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 500;
			this->timer1->Tick += gcnew System::EventHandler(this, &CClientInstallForm::timer1_Tick);
			// 
			// progressBar
			// 
			this->progressBar->Location = System::Drawing::Point(20, 423);
			this->progressBar->Name = L"progressBar";
			this->progressBar->Size = System::Drawing::Size(600, 15);
			this->progressBar->Step = 1;
			this->progressBar->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBar->TabIndex = 10;
			this->progressBar->Visible = false;
			// 
			// buttonInstall
			// 
			this->buttonInstall->Location = System::Drawing::Point(410, 444);
			this->buttonInstall->Name = L"buttonInstall";
			this->buttonInstall->Size = System::Drawing::Size(100, 25);
			this->buttonInstall->TabIndex = 1;
			this->buttonInstall->Text = L"install";
			this->buttonInstall->UseVisualStyleBackColor = true;
			this->buttonInstall->Click += gcnew System::EventHandler(this, &CClientInstallForm::buttonInstall_Click);
			// 
			// labelContent
			// 
			this->labelContent->AutoSize = true;
			this->labelContent->BackColor = System::Drawing::Color::Transparent;
			this->labelContent->ForeColor = System::Drawing::SystemColors::ControlText;
			this->labelContent->Location = System::Drawing::Point(20, 406);
			this->labelContent->Name = L"labelContent";
			this->labelContent->Size = System::Drawing::Size(66, 13);
			this->labelContent->TabIndex = 12;
			this->labelContent->Text = L"labelContent";
			this->labelContent->Visible = false;
			// 
			// buttonRepair
			// 
			this->buttonRepair->Location = System::Drawing::Point(520, 444);
			this->buttonRepair->Name = L"buttonRepair";
			this->buttonRepair->Size = System::Drawing::Size(100, 25);
			this->buttonRepair->TabIndex = 2;
			this->buttonRepair->Text = L"cancel";
			this->buttonRepair->UseVisualStyleBackColor = true;
			this->buttonRepair->Visible = false;
			this->buttonRepair->Click += gcnew System::EventHandler(this, &CClientInstallForm::buttonRepair_Click);
			// 
			// checkBoxUseTorrent
			// 
			this->checkBoxUseTorrent->AutoSize = true;
			this->checkBoxUseTorrent->BackColor = System::Drawing::Color::Transparent;
			this->checkBoxUseTorrent->ForeColor = System::Drawing::SystemColors::ControlText;
			this->checkBoxUseTorrent->Location = System::Drawing::Point(144, 460);
			this->checkBoxUseTorrent->Name = L"checkBoxUseTorrent";
			this->checkBoxUseTorrent->Size = System::Drawing::Size(87, 17);
			this->checkBoxUseTorrent->TabIndex = 4;
			this->checkBoxUseTorrent->Text = L"uiUseTorrent";
			this->checkBoxUseTorrent->UseVisualStyleBackColor = false;
			this->checkBoxUseTorrent->Visible = false;
			// 
			// labelState
			// 
			this->labelState->AutoSize = true;
			this->labelState->BackColor = System::Drawing::Color::Transparent;
			this->labelState->ForeColor = System::Drawing::SystemColors::ControlText;
			this->labelState->Location = System::Drawing::Point(20, 444);
			this->labelState->Name = L"labelState";
			this->labelState->Size = System::Drawing::Size(54, 13);
			this->labelState->TabIndex = 15;
			this->labelState->Text = L"labelState";
			this->labelState->Visible = false;
			// 
			// webBrowser
			// 
			this->webBrowser->AllowWebBrowserDrop = false;
			this->webBrowser->IsWebBrowserContextMenuEnabled = false;
			this->webBrowser->Location = System::Drawing::Point(0, 0);
			this->webBrowser->Margin = System::Windows::Forms::Padding(0);
			this->webBrowser->MaximumSize = System::Drawing::Size(640, 480);
			this->webBrowser->MinimumSize = System::Drawing::Size(640, 480);
			this->webBrowser->Name = L"webBrowser";
			this->webBrowser->ScrollBarsEnabled = false;
			this->webBrowser->Size = System::Drawing::Size(640, 480);
			this->webBrowser->TabIndex = 8;
			this->webBrowser->Url = (gcnew System::Uri(L"", System::UriKind::Relative));
			this->webBrowser->Visible = false;
			this->webBrowser->Navigating += gcnew System::Windows::Forms::WebBrowserNavigatingEventHandler(this, &CClientInstallForm::webBrowser1_Navigating);
			// 
			// checkBoxFullVersion
			// 
			this->checkBoxFullVersion->AutoSize = true;
			this->checkBoxFullVersion->BackColor = System::Drawing::Color::Transparent;
			this->checkBoxFullVersion->ForeColor = System::Drawing::SystemColors::ControlText;
			this->checkBoxFullVersion->Location = System::Drawing::Point(20, 460);
			this->checkBoxFullVersion->Name = L"checkBoxFullVersion";
			this->checkBoxFullVersion->Size = System::Drawing::Size(85, 17);
			this->checkBoxFullVersion->TabIndex = 3;
			this->checkBoxFullVersion->Text = L"uiFullVersion";
			this->checkBoxFullVersion->UseVisualStyleBackColor = false;
			this->checkBoxFullVersion->Visible = false;
			this->checkBoxFullVersion->CheckedChanged += gcnew System::EventHandler(this, &CClientInstallForm::checkBoxFullVersion_CheckedChanged);
			// 
			// labelFullVersion
			// 
			this->labelFullVersion->BackColor = System::Drawing::Color::Transparent;
			this->labelFullVersion->ForeColor = System::Drawing::SystemColors::ControlText;
			this->labelFullVersion->Location = System::Drawing::Point(320, 390);
			this->labelFullVersion->Name = L"labelFullVersion";
			this->labelFullVersion->Size = System::Drawing::Size(300, 13);
			this->labelFullVersion->TabIndex = 17;
			this->labelFullVersion->Text = L"labelFullVersion";
			this->labelFullVersion->TextAlign = System::Drawing::ContentAlignment::TopRight;
			this->labelFullVersion->Visible = false;
			// 
			// CClientInstallForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSize = true;
			this->BackColor = System::Drawing::SystemColors::Control;
			this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"$this.BackgroundImage")));
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
			this->ClientSize = System::Drawing::Size(638, 480);
			this->Controls->Add(this->labelContent);
			this->Controls->Add(this->labelFullVersion);
			this->Controls->Add(this->checkBoxFullVersion);
			this->Controls->Add(this->labelState);
			this->Controls->Add(this->buttonRepair);
			this->Controls->Add(this->checkBoxUseTorrent);
			this->Controls->Add(this->buttonInstall);
			this->Controls->Add(this->progressBar);
			this->Controls->Add(this->downloadStateLabel);
			this->Controls->Add(this->webBrowser);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->MaximumSize = System::Drawing::Size(646, 514);
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(646, 514);
			this->Name = L"CClientInstallForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->TransparencyKey = System::Drawing::Color::Fuchsia;
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &CClientInstallForm::CClientInstallForm_FormClosing);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
// members
public:
		System::Threading::Mutex^ WaitMutex;

private:
	System::String ^ _NewValue;
	System::String ^ _NewState;
	System::String ^ _LabelFullVersionValue;
	
	int _NewProgressBarValue;
	System::Resources::ResourceManager^ _RessourceManager;
	System::Windows::Forms::Form^ _PopupForm;
	bool _NeedRepair;
	

	// members functions
public: 

	//! Update the label "State" that is under the main lable
	void setState(const char* newState)
	{
		_NewState = gcnew System::String(newState);
	}

	//! log a message to the stats.php
	std::string logOnServer(const char* msg);

	/** Called by CGameDownloader the list of availablae package is given,
	Display a form asking if the user is sure to download/installa and what are the option he select (torrent protocol, full version)
	*/
	void packageSelectionUpdated(ICkeckInfoViewer* viewer);

	//! Called via the patch system to know the list of file to download, do nothing
	void addToDownloadList(const char* patchName, const char* sourceName, unsigned int timestamp);

	//! Called when the list of file to download list loaded. Process to inform stat.php the start of download and store/load info from registry
	void onDownloadListFinished();

	//! Displays a nice message msg is a i18n string that must contains the string format to read param1 and parm2
	void fatalError(const char* msg, const char* param1, const char* param2);
	
	//! Send message to stats.php to say that download is finihsed (and install will begin)
	void askForInstall(long long size);
	//obsolete?
	void updateDownloading(const char* actionState, const char* name, unsigned long long progress, unsigned long long rate, unsigned long long done, unsigned long long wanted);	

	/** Update the isntall bar. (progress must be between 0 and 100) but install progress bar is between 50 and 100
	\pram progress must be between 0 and 100
	*/
	void updateInstalling(unsigned int progress);

	/** Generic function to update progress bar and label of install, download, scan...
	The time estimation is made in function of the rate (after smooth function) and 
	- Main label "download of toto.bnp at 14MB/S (1/4) -( 200MB/1.6GB)"
	- Time estimation label: "(3minutes 15 seconds left)
	- Progressbar
	*/
	void onFileUpdate(const char * trad, const char * filename, unsigned int rate, unsigned int index, unsigned int filecount, unsigned int firstValue, unsigned int lastValue, unsigned long long size, unsigned long long fullSize, const char * category);

	/** Update the install progress bar
	*/
	void onFileInstall(const char * filename, unsigned int index, unsigned int filecount);
	/** Called at end of intallation.
	- send message to stats.php
	- opend the "Do you want to launch Ryzom" form
	*/
	void onFileInstallFinished();

	/** Display the "Do you want to launch Ryzom" form (if startClient == 1) and StartRyzomAtEnd == true
	*/
	void displayMsgClientUpdated(bool startClient);

	//!  Display "start downloading" windows (Windows callback when clicking on the install buton)
	System::Void buttonInstall_Click(System::Object^  sender, System::EventArgs^  e);	 
	
	//! obosolete?
	System::Void checkBoxFullVersion_CheckedChanged(System::Object^  sender, System::EventArgs^  e);

	/** Gui send message to application vi this function when button are clicked
	\param form Ptr the popup form if it is the message sender
	\param action The action to realise eg "download_cancel"
	\param state On a checkbox the state of the ckeckbox
	*/
	void onPopupAction(client_install::CInstallPopupForm^ form, System::String^ action, bool state);

	/** Update the text of the form */
	void updateStartProcessText(client_install::CInstallPopupForm^ form);
	/** Display string to display text "3 mintues 4 seconds"
	*/
	System::String^ timeToHuman(double t);
private:
	
	void init();
	//! called every tick (call update functions)
	System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e);

	//! called when click on a link of mini web site (assure to not load other site from form)
	System::Void webBrowser1_Navigating(System::Object^  sender, System::Windows::Forms::WebBrowserNavigatingEventArgs^  e);

	//! obsolete?
	System::Void checkedListBoxPackageSelection_ItemCheck(System::Object^  sender, System::Windows::Forms::ItemCheckEventArgs^  e);
	
	//! obsolete?
	System::Void checkedListBoxPackageSelection_Click(System::Object^  sender, System::EventArgs^  e) { }
	//! obsolete?
	System::Void webBrowser_DocumentCompleted(System::Object^  sender, System::Windows::Forms::WebBrowserDocumentCompletedEventArgs^  e) {}

	//! Cancel the installation
	System::Void buttonRepair_Click(System::Object^  sender, System::EventArgs^  e) ;

	//! Called when form is closing. Open a popup "Are you sure"
	System::Void CClientInstallForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
};

}

#endif