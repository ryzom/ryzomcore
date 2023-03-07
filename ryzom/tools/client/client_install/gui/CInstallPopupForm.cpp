#include "stdafx.h"

#include "CInstallPopupForm.h"
#include "CClientInstallForm.h"
#include "game_downloader.h"

using namespace client_install;



void CInstallPopupForm::init(client_install::CClientInstallForm^ parent)
{
	this->_InstallForm = parent;
	this->_MustCloseApplication = true;
	this->Text = this->_InstallForm->Text;
}

void CInstallPopupForm::setText(System::String^ title, System::String^ content)
{	// set the title with 14 bold font
	// set the content with 12 font
	this->_RichTextBox->Clear();
	this->_RichTextBox->SelectionFont = gcnew System::Drawing::Font("Verdana", 14, System::Drawing::FontStyle::Bold);
	this->_RichTextBox->AppendText(title->Replace("\\n", "\n"));
	this->_RichTextBox->AppendText(L"\n\n");
	this->_RichTextBox->SelectionFont =  gcnew System::Drawing::Font("Verdana", 12, System::Drawing::FontStyle::Regular);
	this->_RichTextBox->AppendText(content->Replace("\\n", "\n"));
}

void CInstallPopupForm::setCheckBox(unsigned int index, System::String^ action,  System::String^ text, System::Boolean checked, System::Boolean enabled)
{
	System::Windows::Forms::CheckBox^ checkBox = nullptr;
	//select the widget that will be modified
	switch (index)
	{
	case 1:
		checkBox = this->_CheckBox1;
		this->_CheckBox1Action = action;
		break;
	case 2:
		checkBox = this->_CheckBox2;
		this->_CheckBox2Action = action;
		break;
	case 3:
		checkBox = this->_CheckBox3;
		this->_CheckBox3Action = action;
		break;
	default:
		System::Diagnostics::Debug::Assert( 0 && "bad index ");
	}

	//set the selected widget with function parameter(text, visible, checked, enable)
	if (text != nullptr)
	{
		checkBox->Text = text;
		checkBox->Visible = true;
		checkBox->Checked = checked;
		checkBox->Enabled = enabled;
	}
	else
	{
		checkBox->Visible = false;
	}
	
}

void CInstallPopupForm::setButton( unsigned int index, System::String^ action, System::String^ text)
{
	System::Windows::Forms::Button^ button = nullptr;
	//select the widget that will be modified
	switch (index)
	{
	case 1:
		button = this->_Button1;
		_Button1Action = action;	
		// The first button is alwayas active (pre-selected)
		this->ActiveControl = button;
		//button->Focus();
		break;

	case 2:
		button = this->_Button2;
		_Button2Action = action;
		break;

	default:
		System::Diagnostics::Debug::Assert( 0 && "bad index ");
	}
	// set the visibility and the content of the text of the button
	if (text != nullptr)
	{
		button->Text = text;
		button->Visible = true;		
	}
	else
	{
		button->Visible = false;
	}
}




System::Void CInstallPopupForm::button1_Click(System::Object^  sender, System::EventArgs^  e)
{
	//Forward message to main form
	this->_InstallForm->onPopupAction(this, this->_Button1Action, true);
}


System::Void CInstallPopupForm::button2_Click(System::Object^  sender, System::EventArgs^  e)
{
	//Forward message to main form
	this->_InstallForm->onPopupAction(this, this->_Button2Action, true);
}

System::Void CInstallPopupForm::checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	//Forward message to main form
	this->_InstallForm->onPopupAction(this, this->_CheckBox1Action, this->_CheckBox1->Checked);
}

System::Void CInstallPopupForm::checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	//Forward message to main form
	this->_InstallForm->onPopupAction(this, this->_CheckBox2Action, this->_CheckBox2->Checked);
}
System::Void CInstallPopupForm::checkBox3_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	//Forward message to main form
	this->_InstallForm->onPopupAction(this, this->_CheckBox3Action, this->_CheckBox3->Checked);
}

System::Void CInstallPopupForm::CInstallPopupForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	// If the user click on the close button of the window we display a warning popup
	if (this->_MustCloseApplication)
	{
		e->Cancel= true;
		this->_InstallForm->onPopupAction(this, "download_cancel", true);
		
	}
}

void  CInstallPopupForm::closePopup()
{
	// if we chose to close the windwos we do not dippaly a warning popup
	this->_MustCloseApplication = false;
	this->Close();
}