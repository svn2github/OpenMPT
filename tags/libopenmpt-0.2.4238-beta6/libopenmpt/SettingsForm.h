#pragma once

#include "libopenmpt_settings.h"

namespace libopenmpt {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SettingsForm
	/// </summary>
	public ref class SettingsForm : public System::Windows::Forms::Form
	{
	private:
		libopenmpt_settings * settings;
	public:
		SettingsForm( const char * title, libopenmpt_settings * s ) : settings(s)
		{
			InitializeComponent();

			Text = gcnew System::String( title );

			if ( !settings->with_outputformat ) {
				comboBoxSamplerate->Items->Add("default");
			}
			comboBoxSamplerate->Items->Add(6000);
			comboBoxSamplerate->Items->Add(8000);
			comboBoxSamplerate->Items->Add(11025);
			comboBoxSamplerate->Items->Add(16000);
			comboBoxSamplerate->Items->Add(22050);
			comboBoxSamplerate->Items->Add(32000);
			comboBoxSamplerate->Items->Add(44100);
			comboBoxSamplerate->Items->Add(48000);
			comboBoxSamplerate->Items->Add(88200);
			comboBoxSamplerate->Items->Add(96000);
			if ( settings->samplerate == 0 && !settings->with_outputformat ) {
				comboBoxSamplerate->SelectedItem = "default";
			} else {
				comboBoxSamplerate->SelectedItem = settings->samplerate;
			}

			if ( !settings->with_outputformat ) {
				comboBoxChannels->Items->Add("default");
			}
			comboBoxChannels->Items->Add("mono");
			comboBoxChannels->Items->Add("stereo");
			comboBoxChannels->Items->Add("quad");
			if ( settings->channels == 1 ) comboBoxChannels->SelectedItem = "mono";
			if ( settings->channels == 2 ) comboBoxChannels->SelectedItem = "stereo";
			if ( settings->channels == 4 ) comboBoxChannels->SelectedItem = "quad";
			if ( settings->channels == 0 && !settings->with_outputformat ) comboBoxChannels->SelectedItem = "default";

			trackBarGain->Value = settings->mastergain_millibel;

			if ( settings->interpolationfilterlength == 0 ) {
				comboBoxInterpolation->SelectedIndex = 3;
			} else if ( settings->interpolationfilterlength >= 8 ) {
				comboBoxInterpolation->SelectedIndex = 3;
			} else if ( settings->interpolationfilterlength >= 4 ) {
				comboBoxInterpolation->SelectedIndex = 2;
			} else if ( settings->interpolationfilterlength >= 2 ) {
				comboBoxInterpolation->SelectedIndex = 1;
			} else if ( settings->interpolationfilterlength >= 1 ) {
				comboBoxInterpolation->SelectedIndex = 0;
			}

			comboBoxRepeat->SelectedIndex = settings->repeatcount + 1;

			trackBarStereoSeparation->Value = settings->stereoseparation;

			if( settings->ramping < 0 ) {
				comboBoxVolramping->SelectedIndex = 0;
			} else if ( settings->ramping == 0 ) {
				comboBoxVolramping->SelectedIndex = 1;
			} else if ( settings->ramping == 1 ) {
				comboBoxVolramping->SelectedIndex = 2;
			} else if ( settings->ramping == 2 ) {
				comboBoxVolramping->SelectedIndex = 3;
			} else if ( settings->ramping < 5 ) {
				comboBoxVolramping->SelectedIndex = 4;
			} else if ( settings->ramping < 10 ) {
				comboBoxVolramping->SelectedIndex = 5;
			} else {
				comboBoxVolramping->SelectedIndex = 6;
			}

			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SettingsForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ComboBox^  comboBoxSamplerate;
	protected: 
	private: System::Windows::Forms::Label^  labelSamplerate;
	private: System::Windows::Forms::Button^  buttonOK;
	private: System::Windows::Forms::Label^  labelChannels;
	private: System::Windows::Forms::ComboBox^  comboBoxChannels;
	private: System::Windows::Forms::Label^  labelGain;
	private: System::Windows::Forms::TrackBar^  trackBarGain;


	private: System::Windows::Forms::Label^  labelInterpolation;
	private: System::Windows::Forms::ComboBox^  comboBoxInterpolation;
	private: System::Windows::Forms::Label^  labelRepeat;
	private: System::Windows::Forms::ComboBox^  comboBoxRepeat;
	private: System::Windows::Forms::Label^  labelStereoSeparation;
	private: System::Windows::Forms::TrackBar^  trackBarStereoSeparation;
	private: System::Windows::Forms::Label^  labelVolramping;
	private: System::Windows::Forms::ComboBox^  comboBoxVolramping;







	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->comboBoxSamplerate = (gcnew System::Windows::Forms::ComboBox());
			this->labelSamplerate = (gcnew System::Windows::Forms::Label());
			this->buttonOK = (gcnew System::Windows::Forms::Button());
			this->labelChannels = (gcnew System::Windows::Forms::Label());
			this->comboBoxChannels = (gcnew System::Windows::Forms::ComboBox());
			this->labelGain = (gcnew System::Windows::Forms::Label());
			this->trackBarGain = (gcnew System::Windows::Forms::TrackBar());
			this->labelInterpolation = (gcnew System::Windows::Forms::Label());
			this->comboBoxInterpolation = (gcnew System::Windows::Forms::ComboBox());
			this->labelRepeat = (gcnew System::Windows::Forms::Label());
			this->comboBoxRepeat = (gcnew System::Windows::Forms::ComboBox());
			this->labelStereoSeparation = (gcnew System::Windows::Forms::Label());
			this->trackBarStereoSeparation = (gcnew System::Windows::Forms::TrackBar());
			this->labelVolramping = (gcnew System::Windows::Forms::Label());
			this->comboBoxVolramping = (gcnew System::Windows::Forms::ComboBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBarGain))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBarStereoSeparation))->BeginInit();
			this->SuspendLayout();
			// 
			// comboBoxSamplerate
			// 
			this->comboBoxSamplerate->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxSamplerate->FormattingEnabled = true;
			this->comboBoxSamplerate->Location = System::Drawing::Point(106, 9);
			this->comboBoxSamplerate->Name = L"comboBoxSamplerate";
			this->comboBoxSamplerate->Size = System::Drawing::Size(121, 21);
			this->comboBoxSamplerate->TabIndex = 1;
			this->comboBoxSamplerate->SelectedIndexChanged += gcnew System::EventHandler(this, &SettingsForm::comboBoxSamplerate_SelectedIndexChanged);
			// 
			// labelSamplerate
			// 
			this->labelSamplerate->AutoSize = true;
			this->labelSamplerate->Location = System::Drawing::Point(12, 12);
			this->labelSamplerate->Name = L"labelSamplerate";
			this->labelSamplerate->Size = System::Drawing::Size(58, 13);
			this->labelSamplerate->TabIndex = 0;
			this->labelSamplerate->Text = L"samplerate";
			// 
			// buttonOK
			// 
			this->buttonOK->Location = System::Drawing::Point(15, 240);
			this->buttonOK->Name = L"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(212, 23);
			this->buttonOK->TabIndex = 2;
			this->buttonOK->Text = L"OK";
			this->buttonOK->UseVisualStyleBackColor = true;
			this->buttonOK->Click += gcnew System::EventHandler(this, &SettingsForm::buttonOK_Click);
			// 
			// labelChannels
			// 
			this->labelChannels->AutoSize = true;
			this->labelChannels->Location = System::Drawing::Point(12, 39);
			this->labelChannels->Name = L"labelChannels";
			this->labelChannels->Size = System::Drawing::Size(50, 13);
			this->labelChannels->TabIndex = 3;
			this->labelChannels->Text = L"channels";
			// 
			// comboBoxChannels
			// 
			this->comboBoxChannels->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxChannels->FormattingEnabled = true;
			this->comboBoxChannels->Location = System::Drawing::Point(106, 36);
			this->comboBoxChannels->Name = L"comboBoxChannels";
			this->comboBoxChannels->Size = System::Drawing::Size(121, 21);
			this->comboBoxChannels->TabIndex = 4;
			this->comboBoxChannels->SelectedIndexChanged += gcnew System::EventHandler(this, &SettingsForm::comboBoxChannels_SelectedIndexChanged);
			// 
			// labelGain
			// 
			this->labelGain->AutoSize = true;
			this->labelGain->Location = System::Drawing::Point(12, 74);
			this->labelGain->Name = L"labelGain";
			this->labelGain->Size = System::Drawing::Size(27, 13);
			this->labelGain->TabIndex = 5;
			this->labelGain->Text = L"gain";
			// 
			// trackBarGain
			// 
			this->trackBarGain->LargeChange = 300;
			this->trackBarGain->Location = System::Drawing::Point(106, 63);
			this->trackBarGain->Maximum = 1200;
			this->trackBarGain->Minimum = -1200;
			this->trackBarGain->Name = L"trackBarGain";
			this->trackBarGain->Size = System::Drawing::Size(121, 42);
			this->trackBarGain->SmallChange = 100;
			this->trackBarGain->TabIndex = 6;
			this->trackBarGain->TickFrequency = 100;
			this->trackBarGain->TickStyle = System::Windows::Forms::TickStyle::Both;
			this->trackBarGain->Scroll += gcnew System::EventHandler(this, &SettingsForm::trackBarGain_Scroll);
			// 
			// labelInterpolation
			// 
			this->labelInterpolation->AutoSize = true;
			this->labelInterpolation->Location = System::Drawing::Point(12, 114);
			this->labelInterpolation->Name = L"labelInterpolation";
			this->labelInterpolation->Size = System::Drawing::Size(64, 13);
			this->labelInterpolation->TabIndex = 9;
			this->labelInterpolation->Text = L"interpolation";
			// 
			// comboBoxInterpolation
			// 
			this->comboBoxInterpolation->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxInterpolation->FormattingEnabled = true;
			this->comboBoxInterpolation->Items->AddRange(gcnew cli::array< System::Object^  >(4) {L"off / 1 tap (nearest)", L"2 tap (linear)", 
				L"4 tap (cubic)", L"8 tap (polyphase fir)"});
			this->comboBoxInterpolation->Location = System::Drawing::Point(106, 111);
			this->comboBoxInterpolation->Name = L"comboBoxInterpolation";
			this->comboBoxInterpolation->Size = System::Drawing::Size(121, 21);
			this->comboBoxInterpolation->TabIndex = 10;
			this->comboBoxInterpolation->SelectedIndexChanged += gcnew System::EventHandler(this, &SettingsForm::comboBoxInterpolation_SelectedIndexChanged);
			// 
			// labelRepeat
			// 
			this->labelRepeat->AutoSize = true;
			this->labelRepeat->Location = System::Drawing::Point(12, 141);
			this->labelRepeat->Name = L"labelRepeat";
			this->labelRepeat->Size = System::Drawing::Size(37, 13);
			this->labelRepeat->TabIndex = 11;
			this->labelRepeat->Text = L"repeat";
			// 
			// comboBoxRepeat
			// 
			this->comboBoxRepeat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxRepeat->FormattingEnabled = true;
			this->comboBoxRepeat->Items->AddRange(gcnew cli::array< System::Object^  >(3) {L"forever", L"never", L"once"});
			this->comboBoxRepeat->Location = System::Drawing::Point(106, 138);
			this->comboBoxRepeat->Name = L"comboBoxRepeat";
			this->comboBoxRepeat->Size = System::Drawing::Size(121, 21);
			this->comboBoxRepeat->TabIndex = 12;
			this->comboBoxRepeat->SelectedIndexChanged += gcnew System::EventHandler(this, &SettingsForm::comboBoxRepeat_SelectedIndexChanged);
			// 
			// labelStereoSeparation
			// 
			this->labelStereoSeparation->AutoSize = true;
			this->labelStereoSeparation->Location = System::Drawing::Point(12, 178);
			this->labelStereoSeparation->Name = L"labelStereoSeparation";
			this->labelStereoSeparation->Size = System::Drawing::Size(88, 13);
			this->labelStereoSeparation->TabIndex = 13;
			this->labelStereoSeparation->Text = L"stereo separation";
			// 
			// trackBarStereoSeparation
			// 
			this->trackBarStereoSeparation->LargeChange = 100;
			this->trackBarStereoSeparation->Location = System::Drawing::Point(106, 165);
			this->trackBarStereoSeparation->Maximum = 400;
			this->trackBarStereoSeparation->Name = L"trackBarStereoSeparation";
			this->trackBarStereoSeparation->Size = System::Drawing::Size(121, 42);
			this->trackBarStereoSeparation->SmallChange = 25;
			this->trackBarStereoSeparation->TabIndex = 14;
			this->trackBarStereoSeparation->TickFrequency = 100;
			this->trackBarStereoSeparation->TickStyle = System::Windows::Forms::TickStyle::Both;
			this->trackBarStereoSeparation->Value = 100;
			this->trackBarStereoSeparation->Scroll += gcnew System::EventHandler(this, &SettingsForm::trackBarStereoSeparation_Scroll);
			// 
			// labelVolramping
			// 
			this->labelVolramping->AutoSize = true;
			this->labelVolramping->Location = System::Drawing::Point(12, 216);
			this->labelVolramping->Name = L"labelVolramping";
			this->labelVolramping->Size = System::Drawing::Size(81, 13);
			this->labelVolramping->TabIndex = 15;
			this->labelVolramping->Text = L"volume ramping";
			// 
			// comboBoxVolramping
			// 
			this->comboBoxVolramping->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBoxVolramping->FormattingEnabled = true;
			this->comboBoxVolramping->Items->AddRange(gcnew cli::array< System::Object^  >(7) {L"default", L"off", L"1 ms", L"2 ms", 
				L"3 ms", L"5 ms", L"10 ms"});
			this->comboBoxVolramping->Location = System::Drawing::Point(106, 213);
			this->comboBoxVolramping->Name = L"comboBoxVolramping";
			this->comboBoxVolramping->Size = System::Drawing::Size(121, 21);
			this->comboBoxVolramping->TabIndex = 16;
			this->comboBoxVolramping->SelectedIndexChanged += gcnew System::EventHandler(this, &SettingsForm::comboBoxVolramping_SelectedIndexChanged);
			// 
			// SettingsForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSize = true;
			this->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->ClientSize = System::Drawing::Size(436, 477);
			this->Controls->Add(this->comboBoxVolramping);
			this->Controls->Add(this->labelVolramping);
			this->Controls->Add(this->trackBarStereoSeparation);
			this->Controls->Add(this->labelStereoSeparation);
			this->Controls->Add(this->comboBoxRepeat);
			this->Controls->Add(this->labelRepeat);
			this->Controls->Add(this->comboBoxInterpolation);
			this->Controls->Add(this->labelInterpolation);
			this->Controls->Add(this->trackBarGain);
			this->Controls->Add(this->labelGain);
			this->Controls->Add(this->comboBoxChannels);
			this->Controls->Add(this->labelChannels);
			this->Controls->Add(this->buttonOK);
			this->Controls->Add(this->labelSamplerate);
			this->Controls->Add(this->comboBoxSamplerate);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SettingsForm";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"SettingsForm";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBarGain))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->trackBarStereoSeparation))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void comboBoxSamplerate_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
						 try {
							System::String ^ val = (System::String ^)comboBoxSamplerate->SelectedItem;
							if ( val == "default" ) {
								settings->samplerate = 0;
								settings->changed();
								return;
							}
						 } catch ( ... ) {
						 }
						 settings->samplerate = (int)comboBoxSamplerate->SelectedItem;
						 settings->changed();
					 }
	private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e) {
						 this->Close();
						 settings->changed();
					 }
private: System::Void comboBoxChannels_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
					 System::String ^ val = (System::String ^)comboBoxChannels->SelectedItem;
					 if ( val == "mono" ) settings->channels = 1;
					 if ( val == "stereo" ) settings->channels = 2;
					 if ( val == "quad" ) settings->channels = 4;
					 if ( val == "default" ) settings->channels = 0;
					 settings->changed();
				 }
private: System::Void trackBarGain_Scroll(System::Object^  sender, System::EventArgs^  e) {
					 settings->mastergain_millibel = (int)trackBarGain->Value;
					 settings->changed();
				 }
private: System::Void comboBoxInterpolation_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
					switch ( (int)comboBoxInterpolation->SelectedIndex )
					{
					case 0:
						settings->interpolationfilterlength = 1;
						break;
					case 1:
						settings->interpolationfilterlength = 2;
						break;
					case 2:
						settings->interpolationfilterlength = 4;
						break;
					case 3:
						settings->interpolationfilterlength = 8;
						break;
					}
					 settings->changed();
				 }
private: System::Void comboBoxRepeat_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
					 settings->repeatcount = (int)comboBoxRepeat->SelectedIndex - 1;
					 settings->changed();
				 }
private: System::Void trackBarStereoSeparation_Scroll(System::Object^  sender, System::EventArgs^  e) {
					settings->stereoseparation = (int)trackBarStereoSeparation->Value;
					settings->changed();
				}
private: System::Void comboBoxVolramping_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
					 switch ( (int)comboBoxVolramping->SelectedIndex )
					 {
					 case 0:
						 settings->ramping = -1;
						 break;
					 case 1:
						 settings->ramping = 0;
						 break;
					 case 2:
						 settings->ramping = 1;
						 break;
					 case 3:
						 settings->ramping = 2;
						 break;
					 case 4:
						 settings->ramping = 3;
						 break;
					 case 5:
						 settings->ramping = 5;
						 break;
					 case 6:
						 settings->ramping = 10;
						 break;
					 }
					 settings->changed();
				 }

};
}
