#pragma once
#include "ScoringEngine.h"
#include <wx/wx.h>
#include <wx/valnum.h>
#include <vector>
#include <string>
#include <wx/wrapsizer.h>

class RankDialog : public wxDialog
{
public:
    RankDialog(wxWindow* parent, const std::vector<std::string>& skills, const std::vector<std::string>& universities, ScoringWeights& scoringWeights);

private:
    void OnSkillSelected(wxCommandEvent& event);
	void OnSkillDoubleClick(wxMouseEvent& event);
	void OnOkButtonPressed(wxCommandEvent& event);

    wxComboBox* skillsInput;
    wxTextCtrl* skillWeightInput;
    wxBoxSizer* universitiesSizer;
    wxTextCtrl* gpaWeightInput;
    wxWrapSizer* skillsWrapSizer;
    std::vector<wxStaticText*> selectedSkills;
    std::vector<wxTextCtrl*> universityWeightInputs;
	ScoringWeights* scoringWeightsPtr;
};