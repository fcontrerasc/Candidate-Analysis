#include "RankDialog.h"

RankDialog::RankDialog(wxWindow* parent, const std::vector<std::string>& skills, const std::vector<std::string>& universities, ScoringWeights& scoringWeights)
    : wxDialog(parent, wxID_ANY, "Rank Candidates", wxDefaultPosition, wxSize(400, 600))
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* skillsSizer = new wxBoxSizer(wxHORIZONTAL);
    skillsInput = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxSize(150, 25));
    for (const auto& skill : skills) {
        skillsInput->Append(skill);
    }
    skillsSizer->Add(new wxStaticText(this, wxID_ANY, "Skill"), 0, wxLEFT, 10);
    skillsSizer->Add(skillsInput, 0, wxLEFT, 5);

    skillWeightInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25), 0, wxFloatingPointValidator<double>(2, nullptr, wxNUM_VAL_DEFAULT));
    skillsSizer->Add(new wxStaticText(this, wxID_ANY, "Weight"), 0, wxLEFT, 10);
    skillsSizer->Add(skillWeightInput, 0, wxLEFT, 5);

    mainSizer->Add(skillsSizer, 0, wxEXPAND | wxALL, 10);

    skillsWrapSizer = new wxWrapSizer(wxHORIZONTAL);
    mainSizer->Add(skillsWrapSizer, 0, wxEXPAND | wxALL, 10);

    universitiesSizer = new wxBoxSizer(wxVERTICAL);
    for (const auto& university : universities) {
        wxBoxSizer* universitySizer = new wxBoxSizer(wxHORIZONTAL);
        universitySizer->Add(new wxStaticText(this, wxID_ANY, university), 0, wxLEFT, 10);
        wxTextCtrl* universityWeightInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25), 0, wxFloatingPointValidator<double>(2, nullptr, wxNUM_VAL_DEFAULT));
        universitySizer->Add(universityWeightInput, 0, wxLEFT, 5);
        universityWeightInputs.push_back(universityWeightInput);
        universitiesSizer->Add(universitySizer, 0, wxEXPAND | wxALL, 5);
    }
    mainSizer->Add(universitiesSizer, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer* gpaSizer = new wxBoxSizer(wxHORIZONTAL);
    gpaSizer->Add(new wxStaticText(this, wxID_ANY, "GPA Weight"), 0, wxLEFT, 10);
    gpaWeightInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25), 0, wxFloatingPointValidator<double>(2, nullptr, wxNUM_VAL_DEFAULT));
    gpaSizer->Add(gpaWeightInput, 0, wxLEFT, 5);
    mainSizer->Add(gpaSizer, 0, wxEXPAND | wxALL, 10);

    wxButton* okButton = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(80, 25));
    mainSizer->Add(okButton, 0, wxALIGN_CENTER | wxALL, 10);

    SetSizer(mainSizer);

	skillsInput->Bind(wxEVT_COMBOBOX, &RankDialog::OnSkillSelected, this);
	okButton->Bind(wxEVT_BUTTON, &RankDialog::OnOkButtonPressed, this);

	scoringWeightsPtr = &scoringWeights;
}

void RankDialog::OnOkButtonPressed(wxCommandEvent& event)
{
	scoringWeightsPtr->requiredSkills.clear();
	for (auto skillText : selectedSkills) {
		scoringWeightsPtr->requiredSkills.push_back(skillText->GetLabel().ToStdString());
	}
	if (skillsInput->GetValue().IsEmpty()) {
		scoringWeightsPtr->skillWeight = 0.0f;
	}
	else {
		scoringWeightsPtr->skillWeight = std::stod(skillWeightInput->GetValue().ToStdString());
	}
	scoringWeightsPtr->universityWeights.clear();
	for (size_t i = 0; i < universityWeightInputs.size(); i++) {
		if (universityWeightInputs[i]->GetValue().IsEmpty()) {
			scoringWeightsPtr->universityWeights[universityWeightInputs[i]->GetLabel().ToStdString()] = 0.0f;
		}
        else {
            scoringWeightsPtr->universityWeights[universityWeightInputs[i]->GetLabel().ToStdString()] = std::stod(universityWeightInputs[i]->GetValue().ToStdString());
        }
	}
    if (gpaWeightInput->GetValue().IsEmpty()) {
        scoringWeightsPtr->gpaWeight = 0.0f;
	}
	else {
		scoringWeightsPtr->gpaWeight = std::stod(gpaWeightInput->GetValue().ToStdString());
	}
	EndModal(wxID_OK);
}

void RankDialog::OnSkillSelected(wxCommandEvent& event)
{
    wxString selectedSkill = skillsInput->GetValue();
    if (!selectedSkill.IsEmpty()) {
        skillsInput->SetValue("");
        wxStaticText* skillText = new wxStaticText(this, wxID_ANY, selectedSkill);
        skillText->Bind(wxEVT_LEFT_DCLICK, &RankDialog::OnSkillDoubleClick, this);
        skillsWrapSizer->Add(skillText, 0, wxLEFT, 5);
        selectedSkills.push_back(skillText);
        skillsWrapSizer->Layout();
        this->Layout();
    }
}

void RankDialog::OnSkillDoubleClick(wxMouseEvent& event)
{
    wxStaticText* skillText = dynamic_cast<wxStaticText*>(event.GetEventObject());
    if (skillText) {
        skillsWrapSizer->Detach(skillText);
        skillText->Destroy();
        selectedSkills.erase(std::remove(selectedSkills.begin(), selectedSkills.end(), skillText), selectedSkills.end());
        skillsWrapSizer->Layout();
        this->Layout();
    }
}