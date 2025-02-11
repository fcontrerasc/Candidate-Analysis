#include "DataDownloader.h"
#include "DataParser.h"
#include "DatabaseManager.h"
#include "DataAnalyzer.h"
#include "FilterManager.h"
#include "PersistenceHandler.h"
#include "ScoringEngine.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/valnum.h>
#include <wx/wrapsizer.h>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");  // Format: YYYY-MM-DD
    return ss.str();
}

class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame();
    void PopulateCandidatesList();

private:
    void OnSkillSelected(wxCommandEvent& event);
    void OnSkillDoubleClick(wxMouseEvent& event);
    void OnFilterChanged(wxCommandEvent& event);
    void OnClearButtonClicked(wxCommandEvent& event);
	void OnSaveButtonClicked(wxCommandEvent& event);
    void OnSavedFilterDoubleClick(wxListEvent& event);

    wxTextCtrl* totalCandidates;
    wxListCtrl* universityStats;
    wxListCtrl* skillsStats;
    wxComboBox* skillsInput;
    wxComboBox* universityInput;
    wxTextCtrl* minGPAInput;
    wxTextCtrl* maxGPAInput;
    wxCheckBox* skillsCheckBox;
    wxCheckBox* universityCheckBox;
    wxCheckBox* gpaCheckBox;
    wxListCtrl* candidatesList;
    wxListCtrl* savedFilters;
	wxButton* clearButton;
    wxButton* saveButton;
    wxButton* rankButton;
    wxWrapSizer* skillsWrapSizer;
    std::vector<wxStaticText*> selectedSkills;
    wxPanel* panel;
};

enum
{
    ID_Hello = 1
};

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();
    frame->Show(true);

    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.open("candidates.db")) {
        return false; // Exit on failure
    }
    if (!db.createTables()) {
        return false;
    }

    // URLs to download
    std::vector<std::string> urls = {
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/Polytechnic-University-of-Bucharest.xml",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-S%C3%A3o-Paulo.xml",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-Florida.json",
        "https://chromium-case-study.s3.us-east-1.amazonaws.com/candidate+feeds/University-of-Havana.json"
    };

    // Download data
    DataDownloader downloader;
    downloader.downloadAllFeedsAsync(urls);

    // Batch insert with transaction  
    std::string today = getCurrentDate();
    for (const auto& url : urls) {
        // Determine file type from URL
        std::string fileType = downloader.getFileExtension(url);
        // Create the appropriate parser
        Parser* parser = ParserFactory::createParser(fileType);
        if (parser) {
            // Load parsed candidates from file  
            std::string rawData = downloader.loadFile(downloader.url_to_filename(url));
            std::string university = downloader.extractUniversityName(url);
            auto candidates = parser->parse(rawData, university);
            if (!db.insertCandidates(candidates, today)) {
                delete parser; // Clean up the parser
            }
        }
    }

    frame->PopulateCandidatesList();

    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Candidate Analysis", wxDefaultPosition, wxSize(600, 800))
{
    panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* statsLabel = new wxStaticText(panel, wxID_ANY, "Stats");
    mainSizer->Add(statsLabel, 0, wxLEFT | wxTOP, 10);

    wxBoxSizer* statsSizer = new wxBoxSizer(wxHORIZONTAL);
    totalCandidates = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, 25), wxTE_READONLY);
    statsSizer->Add(new wxStaticText(panel, wxID_ANY, "Total of candidates"), 0, wxLEFT, 10);
    statsSizer->Add(totalCandidates, 0, wxLEFT, 5);
    mainSizer->Add(statsSizer, 0, wxEXPAND | wxALL, 5);

    universityStats = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(280, 100), wxLC_REPORT);
    universityStats->InsertColumn(0, "University");
    universityStats->InsertColumn(1, "Total");
    universityStats->InsertColumn(2, "GPA");
    mainSizer->Add(universityStats, 0, wxEXPAND | wxALL, 5);

    skillsStats = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(280, 100), wxLC_REPORT);
    skillsStats->InsertColumn(0, "Skill");
	skillsStats->InsertColumn(1, "Frecuency");
    mainSizer->Add(skillsStats, 0, wxEXPAND | wxALL, 5);

    wxStaticText* filtersLabel = new wxStaticText(panel, wxID_ANY, "Filters");
    mainSizer->Add(filtersLabel, 0, wxLEFT | wxTOP, 10);

    wxBoxSizer* filtersSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* optionsSizer = new wxBoxSizer(wxHORIZONTAL);
	clearButton = new wxButton(panel, wxID_ANY, "Clear", wxDefaultPosition, wxSize(50, 25));
    saveButton = new wxButton(panel, wxID_ANY, "Save", wxDefaultPosition, wxSize(50, 25));
    rankButton = new wxButton(panel, wxID_ANY, "Rank", wxDefaultPosition, wxSize(50, 25));
	optionsSizer->Add(clearButton, 0, wxLEFT, 10);
	optionsSizer->Add(saveButton, 0, wxLEFT, 10);
	optionsSizer->Add(rankButton, 0, wxLEFT, 5);
	filtersSizer->Add(optionsSizer, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* skillsSizer = new wxBoxSizer(wxHORIZONTAL);
    skillsInput = new wxComboBox(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, 25));
    skillsCheckBox = new wxCheckBox(panel, wxID_ANY, "");
    skillsSizer->Add(new wxStaticText(panel, wxID_ANY, "Skills"), 0, wxLEFT, 10);
    skillsSizer->Add(skillsInput, 0, wxLEFT, 5);
    skillsSizer->Add(skillsCheckBox, 0, wxLEFT, 5);

    filtersSizer->Add(skillsSizer, 0, wxEXPAND | wxALL, 5);

    skillsWrapSizer = new wxWrapSizer(wxHORIZONTAL);
    filtersSizer->Add(skillsWrapSizer, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* universitySizer = new wxBoxSizer(wxHORIZONTAL);
    universityInput = new wxComboBox(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, 25));
    universityCheckBox = new wxCheckBox(panel, wxID_ANY, "");
    universitySizer->Add(new wxStaticText(panel, wxID_ANY, "University"), 0, wxLEFT, 10);
    universitySizer->Add(universityInput, 0, wxLEFT, 5);
    universitySizer->Add(universityCheckBox, 0, wxLEFT, 5);
    filtersSizer->Add(universitySizer, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* gpaSizer = new wxBoxSizer(wxHORIZONTAL);

    // Create validators for minGPAInput and maxGPAInput
    wxFloatingPointValidator<double> gpaValidator(2, nullptr, wxNUM_VAL_DEFAULT);
    gpaValidator.SetRange(0.0, 4.0);

    minGPAInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25), 0, gpaValidator);
    maxGPAInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25), 0, gpaValidator);
    gpaCheckBox = new wxCheckBox(panel, wxID_ANY, "");
    gpaSizer->Add(new wxStaticText(panel, wxID_ANY, "GPA Min"), 0, wxLEFT, 10);
    gpaSizer->Add(minGPAInput, 0, wxLEFT, 5);
    gpaSizer->Add(new wxStaticText(panel, wxID_ANY, "Max"), 0, wxLEFT, 10);
    gpaSizer->Add(maxGPAInput, 0, wxLEFT, 5);
    gpaSizer->Add(gpaCheckBox, 0, wxLEFT, 5);
	filtersSizer->Add(gpaSizer, 0, wxEXPAND | wxALL, 5);

    mainSizer->Add(filtersSizer, 0, wxEXPAND | wxALL, 5);

    candidatesList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(500, 100), wxLC_REPORT);
    candidatesList->InsertColumn(0, "Name");
    candidatesList->InsertColumn(1, "University");
    candidatesList->InsertColumn(2, "Skills");
    candidatesList->InsertColumn(3, "GPA");
    mainSizer->Add(candidatesList, 0, wxEXPAND | wxALL, 5);

    wxStaticText* savedFiltersLabel = new wxStaticText(panel, wxID_ANY, "Saved Filters");
    mainSizer->Add(savedFiltersLabel, 0, wxLEFT | wxTOP, 10);

    savedFilters = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(500, 100), wxLC_REPORT);
    savedFilters->InsertColumn(0, "Name");
    savedFilters->InsertColumn(1, "Date");
    mainSizer->Add(savedFilters, 0, wxEXPAND | wxALL, 5);

    panel->SetSizer(mainSizer);
	skillsInput->Bind(wxEVT_COMBOBOX, &MyFrame::OnSkillSelected, this);
    skillsCheckBox->Bind(wxEVT_CHECKBOX, &MyFrame::OnFilterChanged, this);
    universityCheckBox->Bind(wxEVT_CHECKBOX, &MyFrame::OnFilterChanged, this);
    gpaCheckBox->Bind(wxEVT_CHECKBOX, &MyFrame::OnFilterChanged, this);
    clearButton->Bind(wxEVT_BUTTON, &MyFrame::OnClearButtonClicked, this);
	saveButton->Bind(wxEVT_BUTTON, &MyFrame::OnSaveButtonClicked, this);
    savedFilters->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MyFrame::OnSavedFilterDoubleClick, this);
}

void MyFrame::OnSavedFilterDoubleClick(wxListEvent& event)
{
    long itemIndex = event.GetIndex();
    wxString filterName = savedFilters->GetItemText(itemIndex, 0);

    // Load the filter from the file
    PersistenceHandler handler;
    FilterData filterData;
    std::vector<Candidate> candidates = handler.loadFromFile(filterName.ToStdString() + ".json", filterData);

    // Update the UI with the loaded filter data
    skillsCheckBox->SetValue(filterData.skillsChecked);
    universityCheckBox->SetValue(filterData.universityChecked);
    gpaCheckBox->SetValue(filterData.gpaChecked);

    skillsInput->SetValue("");
	if (filterData.skillsChecked) {
        for (const auto& skill : filterData.skills) {
            wxStaticText* skillText = new wxStaticText(panel, wxID_ANY, skill);
            skillText->Bind(wxEVT_LEFT_DCLICK, &MyFrame::OnSkillDoubleClick, this);
            skillsWrapSizer->Add(skillText, 0, wxLEFT, 5);
            selectedSkills.push_back(skillText);
        }
        skillsWrapSizer->Layout();
        panel->Layout();
	}
	else {
		for (auto skillText : selectedSkills) {
			skillsWrapSizer->Detach(skillText);
			skillText->Destroy();
		}
		selectedSkills.clear();
		skillsWrapSizer->Layout();
		panel->Layout();
	}

	universityInput->SetValue("");
	if (filterData.universityChecked) {
		universityInput->SetValue(filterData.university);
	}

	minGPAInput->SetValue("");
	maxGPAInput->SetValue("");
	if (filterData.gpaChecked) {
		minGPAInput->SetValue(std::to_string(filterData.minGPA));
		maxGPAInput->SetValue(std::to_string(filterData.maxGPA));
	}

    // Update the candidate list
    candidatesList->DeleteAllItems();
    for (const auto& candidate : candidates) {
        long index = candidatesList->InsertItem(candidatesList->GetItemCount(), candidate.name);
        candidatesList->SetItem(index, 1, candidate.university);

        // Join all skills into a single string separated by commas
        std::string skills;
        for (const auto& skill : candidate.skills) {
            if (!skills.empty()) {
                skills += ", ";
            }
            skills += skill;
        }
        candidatesList->SetItem(index, 2, wxString::FromUTF8(skills.c_str()));
        candidatesList->SetItem(index, 3, std::to_string(candidate.gpa));
    }
}

void MyFrame::OnSaveButtonClicked(wxCommandEvent& event)
{
	// Open an input dialog to get the filter name
	wxTextEntryDialog dialog(this, "Enter a name for the filter", "Save Filter", "", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		std::string filterName = dialog.GetValue().ToStdString();
		// Save the current filter settings
		FilterData filterData;
		filterData.date = getCurrentDate();
		filterData.skillsChecked = skillsCheckBox->IsChecked();
		filterData.universityChecked = universityCheckBox->IsChecked();
		filterData.gpaChecked = gpaCheckBox->IsChecked();
		if (filterData.skillsChecked) {
			for (const auto& skillText : selectedSkills) {
				filterData.skills.push_back(skillText->GetLabel().ToStdString());
			}
		}
		if (filterData.universityChecked) {
			filterData.university = universityInput->GetValue().ToStdString();
		}
		if (filterData.gpaChecked) {
			double minGPA, maxGPA;
			minGPAInput->GetValue().ToDouble(&minGPA);
			maxGPAInput->GetValue().ToDouble(&maxGPA);
			filterData.minGPA = minGPA;
			filterData.maxGPA = maxGPA;
		}
		// Retrieve the current candidates inside candidatesList
		std::vector<Candidate> candidates;
		for (int i = 0; i < candidatesList->GetItemCount(); i++) {
			Candidate c;
			c.name = candidatesList->GetItemText(i, 0).ToStdString();
			c.university = candidatesList->GetItemText(i, 1).ToStdString();
			c.gpa = std::stod(candidatesList->GetItemText(i, 3).ToStdString());
			std::string skills = candidatesList->GetItemText(i, 2).ToStdString();
			std::stringstream ss(skills);
			std::string skill;
			while (std::getline(ss, skill, ',')) {
				c.skills.push_back(skill);
			}
			candidates.push_back(c);
		}
		PersistenceHandler handler;
		if (handler.saveToFile(candidates, filterName + ".json", filterData)) {
			// Update the saved filters list
			long index = savedFilters->InsertItem(savedFilters->GetItemCount(), filterName);
			savedFilters->SetItem(index, 1, filterData.date);
			wxMessageBox("Filter saved successfully", "Success", wxOK | wxICON_INFORMATION);
		}
		else {
			wxMessageBox("Failed to save filter", "Error", wxOK | wxICON_ERROR);
		}
	}
}

void MyFrame::OnClearButtonClicked(wxCommandEvent& event)
{
    // Clear all text inputs
    skillsInput->SetValue("");
    universityInput->SetValue("");
    minGPAInput->SetValue("");
    maxGPAInput->SetValue("");

    // Uncheck all checkboxes
    skillsCheckBox->SetValue(false);
    universityCheckBox->SetValue(false);
    gpaCheckBox->SetValue(false);

    // Clear selected skills
    for (auto skillText : selectedSkills) {
        skillsWrapSizer->Detach(skillText);
        skillText->Destroy();
    }
    selectedSkills.clear();
    skillsWrapSizer->Layout();
    panel->Layout();

    DatabaseManager& db = DatabaseManager::getInstance();
    std::vector<Candidate> candidates = db.getCandidatesByDate(getCurrentDate());

    // Update the candidate list
    candidatesList->DeleteAllItems();
    for (const auto& candidate : candidates) {
        long index = candidatesList->InsertItem(candidatesList->GetItemCount(), candidate.name);
        candidatesList->SetItem(index, 1, candidate.university);

        // Join all skills into a single string separated by commas
        std::string skills;
        for (const auto& skill : candidate.skills) {
            if (!skills.empty()) {
                skills += ", ";
            }
            skills += skill;
        }
        candidatesList->SetItem(index, 2, wxString::FromUTF8(skills.c_str()));
        candidatesList->SetItem(index, 3, std::to_string(candidate.gpa));
    }
}

void MyFrame::OnFilterChanged(wxCommandEvent& event)
{
    DatabaseManager& db = DatabaseManager::getInstance();
    FilterManager filter;

    std::vector<Candidate> candidates = db.getCandidatesByDate(getCurrentDate());

    if (skillsCheckBox->IsChecked()) {
        std::vector<std::string> selectedSkillsList;
        for (const auto& skillText : selectedSkills) {
            selectedSkillsList.push_back(std::string(skillText->GetLabel().mb_str()));
        }
        if (!selectedSkillsList.empty()) {
            candidates = filter.filterBySkills(candidates, selectedSkillsList);
		}
        else {
            wxMessageBox("Please select a skill", "Validation Error", wxOK | wxICON_ERROR);
			// Clear the checkbox
			skillsCheckBox->SetValue(false);
            return;
        }
    }

    if (universityCheckBox->IsChecked()) {
        wxString selectedUniversity = universityInput->GetValue();
        if (!selectedUniversity.IsEmpty()) {
            candidates = filter.filterByUniversity(candidates, selectedUniversity.ToStdString());
		}
        else {
            wxMessageBox("Please select a university", "Validation Error", wxOK | wxICON_ERROR);
			// Clear the checkbox
			universityCheckBox->SetValue(false);
            return;
        }
    }

    if (gpaCheckBox->IsChecked()) {
        double minGPA, maxGPA;
        if (minGPAInput->GetValue().ToDouble(&minGPA) && maxGPAInput->GetValue().ToDouble(&maxGPA)) {
            if (minGPA > maxGPA) {
                wxMessageBox("Min GPA should be less than or equal to Max GPA", "Validation Error", wxOK | wxICON_ERROR);
				// Clear the checkbox
				gpaCheckBox->SetValue(false);
                return;
            }
            candidates = filter.filterByGPA(candidates, minGPA, maxGPA);
        }
        else {
            wxMessageBox("Please enter valid GPA values", "Validation Error", wxOK | wxICON_ERROR);
			// Clear the checkbox
			gpaCheckBox->SetValue(false);
            return;
        }
    }

	// Check if candidates is empty
	if (candidates.empty()) {
		wxMessageBox("No candidates found", "Validation Error", wxOK | wxICON_ERROR);
		// Clear the checkboxes
		skillsCheckBox->SetValue(false);
		universityCheckBox->SetValue(false);
		gpaCheckBox->SetValue(false);
		return;
	}

    // Update candidatesList with candidates
    candidatesList->DeleteAllItems();
    for (const auto& candidate : candidates) {
        long index = candidatesList->InsertItem(candidatesList->GetItemCount(), candidate.name);
        candidatesList->SetItem(index, 1, candidate.university);

        // Join all skills into a single string separated by commas
        std::string skills;
        for (const auto& skill : candidate.skills) {
            if (!skills.empty()) {
                skills += ", ";
            }
            skills += skill;
        }
        candidatesList->SetItem(index, 2, wxString::FromUTF8(skills.c_str()));
        candidatesList->SetItem(index, 3, std::to_string(candidate.gpa));
    }
}

void MyFrame::OnSkillSelected(wxCommandEvent& event)
{
    wxString selectedSkill = skillsInput->GetValue();
    if (!selectedSkill.IsEmpty()) {
		skillsInput->SetValue("");
		//skillsCheckBox->SetValue(false);
        wxStaticText* skillText = new wxStaticText(panel, wxID_ANY, selectedSkill);
        skillText->Bind(wxEVT_LEFT_DCLICK, &MyFrame::OnSkillDoubleClick, this);
        skillsWrapSizer->Add(skillText, 0, wxLEFT, 5);
        selectedSkills.push_back(skillText);
        skillsWrapSizer->Layout();
        panel->Layout();
    }
}

void MyFrame::OnSkillDoubleClick(wxMouseEvent& event)
{
    wxStaticText* skillText = dynamic_cast<wxStaticText*>(event.GetEventObject());
    if (skillText) {
        skillsWrapSizer->Detach(skillText);
        skillText->Destroy();
        selectedSkills.erase(std::remove(selectedSkills.begin(), selectedSkills.end(), skillText), selectedSkills.end());
        skillsWrapSizer->Layout();
        panel->Layout();
    }
}

void MyFrame::PopulateCandidatesList()
{
    DatabaseManager& db = DatabaseManager::getInstance();
    DataAnalyzer analyzer;

    std::vector<Candidate> candidates = db.getCandidatesByDate(getCurrentDate());

    // Populate totalCandidates
	int total = analyzer.getTotalCandidates(candidates);
    totalCandidates->SetValue(std::to_string(total));

    // Populate universityStats
    universityStats->DeleteAllItems();
    auto universityData = analyzer.getUniversityStats(candidates);
    for (const auto& entry : universityData) {
        long index = universityStats->InsertItem(universityStats->GetItemCount(), entry.first);
        universityStats->SetItem(index, 1, std::to_string(entry.second.totalCandidates));
        universityStats->SetItem(index, 2, std::to_string(entry.second.averageGpa));
    }

	// Populate universityInput
	universityInput->Clear();
	for (const auto& entry : universityData) {
		universityInput->Append(entry.first);
	}

    // Populate skillsStats
    skillsStats->DeleteAllItems();
    auto skillsData = analyzer.getSkillsStats(candidates);
    for (const auto& entry : skillsData) {
        long index = skillsStats->InsertItem(skillsStats->GetItemCount(), entry.first);
        skillsStats->SetItem(index, 1, std::to_string(entry.second));
    }

    // Populate skillsInput
    skillsInput->Clear();
    for (const auto& entry : skillsData) {
        skillsInput->Append(entry.first);
    }

    // Populate candidatesList
    candidatesList->DeleteAllItems();
    for (const auto& candidate : candidates) {
        long index = candidatesList->InsertItem(candidatesList->GetItemCount(), candidate.name);
        candidatesList->SetItem(index, 1, candidate.university);

        // Join all skills into a single string separated by commas
        std::string skills;
        for (const auto& skill : candidate.skills) {
            if (!skills.empty()) {
                skills += ", ";
            }
            skills += skill;
        }
        candidatesList->SetItem(index, 2, wxString::FromUTF8(skills.c_str()));
        candidatesList->SetItem(index, 3, std::to_string(candidate.gpa));
    }

}