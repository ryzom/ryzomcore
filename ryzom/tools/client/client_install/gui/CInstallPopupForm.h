#pragma once

#include <string>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace client_install {

	ref class CClientInstallForm;
	/// <summary>
	/// Description résumée de CInstallPopupForm
	///
	/// AVERTISSEMENT : si vous modifiez le nom de cette classe, vous devrez modifier la
	///          propriété 'Nom du fichier de ressources' de l'outil de compilation de ressource managée
	///          pour tous les fichiers .resx dont dépend cette classe. Dans le cas contraire,
	///          les concepteurs ne pourront pas interagir correctement avec les ressources
	///          localisées associées à ce formulaire.
	/// </summary
	public ref class CInstallPopupForm : public System::Windows::Forms::Form
	{
	public:
		CInstallPopupForm(void)
		{
			InitializeComponent();
			//
			//TODO : ajoutez ici le code du constructeur
			//
		}

	protected:
		/// <summary>
		/// Nettoyage des ressources utilisées.
		/// </summary>
		~CInstallPopupForm()
		{
			if (components)
			{
				delete components;
			}
		}
	

	private:
		/// <summary>
		/// Variable nécessaire au concepteur.
		/// </summary>

	private: System::ComponentModel::Container ^components;	
	private: System::Windows::Forms::CheckBox^  _CheckBox3;
	private: System::Windows::Forms::PictureBox^  pictureBox2;
	private: System::Windows::Forms::Button^  _Button1;
	private: System::Windows::Forms::CheckBox^  _CheckBox1;
	private: System::Windows::Forms::CheckBox^  _CheckBox2;
	private: System::Windows::Forms::Button^  _Button2;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::SplitContainer^  splitContainer1;
	private: System::Windows::Forms::RichTextBox^  _RichTextBox;
	private: System::Windows::Forms::GroupBox^  groupBox1;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Méthode requise pour la prise en charge du concepteur - ne modifiez pas
		/// le contenu de cette méthode avec l'éditeur de code.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(CInstallPopupForm::typeid));
			this->_CheckBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->_CheckBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->_Button2 = (gcnew System::Windows::Forms::Button());
			this->_CheckBox3 = (gcnew System::Windows::Forms::CheckBox());
			this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
			this->_Button1 = (gcnew System::Windows::Forms::Button());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->_RichTextBox = (gcnew System::Windows::Forms::RichTextBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->BeginInit();
			this->panel1->SuspendLayout();
			this->groupBox1->SuspendLayout();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			this->SuspendLayout();
			// 
			// _CheckBox1
			// 
			this->_CheckBox1->AutoSize = true;
			this->_CheckBox1->Location = System::Drawing::Point(12, 58);
			this->_CheckBox1->Name = L"_CheckBox1";
			this->_CheckBox1->Size = System::Drawing::Size(80, 17);
			this->_CheckBox1->TabIndex = 1;
			this->_CheckBox1->Text = L"checkBox1";
			this->_CheckBox1->UseVisualStyleBackColor = true;
			this->_CheckBox1->Visible = false;
			this->_CheckBox1->CheckedChanged += gcnew System::EventHandler(this, &CInstallPopupForm::checkBox1_CheckedChanged);
			// 
			// _CheckBox2
			// 
			this->_CheckBox2->AutoSize = true;
			this->_CheckBox2->Location = System::Drawing::Point(12, 41);
			this->_CheckBox2->Name = L"_CheckBox2";
			this->_CheckBox2->Size = System::Drawing::Size(80, 17);
			this->_CheckBox2->TabIndex = 2;
			this->_CheckBox2->Text = L"checkBox2";
			this->_CheckBox2->UseVisualStyleBackColor = true;
			this->_CheckBox2->Visible = false;
			this->_CheckBox2->CheckedChanged += gcnew System::EventHandler(this, &CInstallPopupForm::checkBox2_CheckedChanged);
			// 
			// _Button2
			// 
			this->_Button2->Location = System::Drawing::Point(389, 43);
			this->_Button2->Name = L"_Button2";
			this->_Button2->Size = System::Drawing::Size(106, 32);
			this->_Button2->TabIndex = 3;
			this->_Button2->Text = L"button2";
			this->_Button2->UseVisualStyleBackColor = true;
			this->_Button2->Visible = false;
			this->_Button2->Click += gcnew System::EventHandler(this, &CInstallPopupForm::button2_Click);
			// 
			// _CheckBox3
			// 
			this->_CheckBox3->AutoSize = true;
			this->_CheckBox3->Location = System::Drawing::Point(12, 23);
			this->_CheckBox3->Name = L"_CheckBox3";
			this->_CheckBox3->Size = System::Drawing::Size(80, 17);
			this->_CheckBox3->TabIndex = 5;
			this->_CheckBox3->Text = L"checkBox3";
			this->_CheckBox3->UseVisualStyleBackColor = true;
			this->_CheckBox3->Visible = false;
			this->_CheckBox3->CheckedChanged += gcnew System::EventHandler(this, &CInstallPopupForm::checkBox3_CheckedChanged);
			// 
			// pictureBox2
			// 
			this->pictureBox2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBox2->Dock = System::Windows::Forms::DockStyle::Left;
			this->pictureBox2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox2.Image")));
			this->pictureBox2->Location = System::Drawing::Point(0, 0);
			this->pictureBox2->Margin = System::Windows::Forms::Padding(0);
			this->pictureBox2->Name = L"pictureBox2";
			this->pictureBox2->Size = System::Drawing::Size(205, 371);
			this->pictureBox2->TabIndex = 7;
			this->pictureBox2->TabStop = false;
			// 
			// _Button1
			// 
			this->_Button1->Location = System::Drawing::Point(501, 43);
			this->_Button1->Name = L"_Button1";
			this->_Button1->Size = System::Drawing::Size(106, 32);
			this->_Button1->TabIndex = 6;
			this->_Button1->Text = L"button1";
			this->_Button1->UseVisualStyleBackColor = true;
			this->_Button1->Visible = false;
			this->_Button1->Click += gcnew System::EventHandler(this, &CInstallPopupForm::button1_Click);
			// 
			// panel1
			// 
			this->panel1->BackColor = System::Drawing::SystemColors::Menu;
			this->panel1->Controls->Add(this->groupBox1);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->panel1->Location = System::Drawing::Point(0, 371);
			this->panel1->Margin = System::Windows::Forms::Padding(0);
			this->panel1->Name = L"panel1";
			this->panel1->Padding = System::Windows::Forms::Padding(0, 2, 0, 0);
			this->panel1->Size = System::Drawing::Size(638, 95);
			this->panel1->TabIndex = 8;
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->_Button2);
			this->groupBox1->Controls->Add(this->_Button1);
			this->groupBox1->Controls->Add(this->_CheckBox1);
			this->groupBox1->Controls->Add(this->_CheckBox2);
			this->groupBox1->Controls->Add(this->_CheckBox3);
			this->groupBox1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->groupBox1->Location = System::Drawing::Point(0, 2);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(638, 93);
			this->groupBox1->TabIndex = 7;
			this->groupBox1->TabStop = false;
			// 
			// splitContainer1
			// 
			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Top;
			this->splitContainer1->Location = System::Drawing::Point(0, 0);
			this->splitContainer1->Margin = System::Windows::Forms::Padding(0);
			this->splitContainer1->Name = L"splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this->splitContainer1->Panel1->Controls->Add(this->pictureBox2);
			this->splitContainer1->Panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &CInstallPopupForm::splitContainer1_Panel1_Paint);
			// 
			// splitContainer1.Panel2
			// 
			this->splitContainer1->Panel2->Controls->Add(this->_RichTextBox);
			this->splitContainer1->Size = System::Drawing::Size(638, 371);
			this->splitContainer1->SplitterDistance = 202;
			this->splitContainer1->SplitterWidth = 1;
			this->splitContainer1->TabIndex = 9;
			// 
			// _RichTextBox
			// 
			this->_RichTextBox->BackColor = System::Drawing::SystemColors::Window;
			this->_RichTextBox->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->_RichTextBox->Location = System::Drawing::Point(20, 21);
			this->_RichTextBox->Margin = System::Windows::Forms::Padding(0);
			this->_RichTextBox->Name = L"_RichTextBox";
			this->_RichTextBox->ReadOnly = true;
			this->_RichTextBox->Size = System::Drawing::Size(394, 334);
			this->_RichTextBox->TabIndex = 1;
			this->_RichTextBox->Text = L"xcbvxvcbxbvc";
			// 
			// CInstallPopupForm
			// 
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(638, 466);
			this->Controls->Add(this->splitContainer1);
			this->Controls->Add(this->panel1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->MaximumSize = System::Drawing::Size(646, 500);
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(646, 500);
			this->Name = L"CInstallPopupForm";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->TopMost = true;
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &CInstallPopupForm::CInstallPopupForm_FormClosing);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->EndInit();
			this->panel1->ResumeLayout(false);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->splitContainer1->Panel1->ResumeLayout(false);
			this->splitContainer1->Panel2->ResumeLayout(false);
			this->splitContainer1->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	public: 
		//< init with the main form
		void init(client_install::CClientInstallForm^ parent);
		//! Set the content and the title of the windows (will be display with special fonts)
		void setText(System::String^ title, System::String^ content);

		/** Set the look of displayed button
		\param id The index of the button (1 to 3)
		\param action The action to send to this->_InstallForm->onPopupAction(
		\param text The text displayed on the button
		*/
		void setButton( unsigned int id, System::String^ action, System::String^ text);

		/** Set the look of checkbock
		\param id The index of the checkbox (1 to 3)
		\param action The action to send to this->_InstallForm->onPopupAction(
		\param text The text displayed on the button
		\param checked True if the checkbox is initialy checked
		\param enabled True if the checkbox is enbaled (disable is displayed in grey)
		*/
		void setCheckBox(unsigned int id, System::String^ action, System::String^ text, System::Boolean checked, System::Boolean enable);
		
		/** Ask the windows to close
		Called by the main Form
		*/
		void closePopup();

		/** Get the States of the different checkbox
		{
		*/
		bool getChecked1() { return this->_CheckBox1->Checked;} 
		bool getChecked2() { return this->_CheckBox2->Checked;} 
		bool getChecked3() { return this->_CheckBox3->Checked;} 
		//!}

	private: 
		// the 1 button has been clicked
		System::Void button1_Click(System::Object^  sender, System::EventArgs^  e);		
		// the 2 button has been clicked
		System::Void button2_Click(System::Object^  sender, System::EventArgs^  e);
		// the 1 Checkbox has been changed
		System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
		// the 2 Checkbox has been changed
		System::Void checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
		// the 3 Checkbox has been changed
		System::Void checkBox3_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
		// the form is closing
		System::Void CInstallPopupForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);

	private:
		System::String^ _CheckBox1Action; //Action passed to this->_InstallForm->onPopupAction if button 1 is clicked
		System::String^ _CheckBox2Action;//Action passed to this->_InstallForm->onPopupAction if button 2 is clicked
		System::String^ _CheckBox3Action;//Action passed to this->_InstallForm->onPopupAction if button 3 is clicked
		System::String^ _Button1Action;//Action passed to this->_InstallForm->onPopupAction if Checkbox 1is clicked
		System::String^ _Button2Action;//Action passed to this->_InstallForm->onPopupAction if Checkbox 2is clicked		
		CClientInstallForm^ _InstallForm; // The main form
		bool _MustCloseApplication; // True if the form has been asked to close by the main form
		//obsolete	
	private: System::Void splitContainer1_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		 }



};
}
