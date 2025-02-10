#include "DataDownloader.h"
#include "DataParser.h"
#include "DatabaseManager.h"
#include "DataAnalyzer.h"
#include "FilterManager.h"
#include "ScoringEngine.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <wx/wx.h>
#include <wx/listctrl.h>

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
	//int OnRun() override;
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame();
    void PopulateCandidatesList();

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    wxTextCtrl* totalCandidates;
    wxListCtrl* universityStats;
    wxListCtrl* skillsStats;
    wxTextCtrl* skillsInput;
    wxTextCtrl* universityInput;
    wxTextCtrl* minGPAInput;
    wxTextCtrl* maxGPAInput;
    wxListCtrl* candidatesList;
    wxListCtrl* savedFilters;
    wxButton* saveButton;
    wxButton* rankButton;
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

//int MyApp::OnRun()
//{
//	std::cout << "Running app" << std::endl;
//	return wxApp::OnRun();
//}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Candidate Analysis", wxDefaultPosition, wxSize(600, 800))
{
    wxPanel* panel = new wxPanel(this);
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
    mainSizer->Add(skillsStats, 0, wxEXPAND | wxALL, 5);

    wxStaticText* filtersLabel = new wxStaticText(panel, wxID_ANY, "Filters");
    mainSizer->Add(filtersLabel, 0, wxLEFT | wxTOP, 10);

    wxBoxSizer* filtersSizer = new wxBoxSizer(wxHORIZONTAL);
    skillsInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, 25));
    universityInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(100, 25));
    minGPAInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25));
    maxGPAInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(50, 25));
    saveButton = new wxButton(panel, wxID_ANY, "Save", wxDefaultPosition, wxSize(50, 25));
    rankButton = new wxButton(panel, wxID_ANY, "Rank", wxDefaultPosition, wxSize(50, 25));

    filtersSizer->Add(new wxStaticText(panel, wxID_ANY, "Skills"), 0, wxLEFT, 10);
    filtersSizer->Add(skillsInput, 0, wxLEFT, 5);
    filtersSizer->Add(new wxStaticText(panel, wxID_ANY, "University"), 0, wxLEFT, 10);
    filtersSizer->Add(universityInput, 0, wxLEFT, 5);
    filtersSizer->Add(new wxStaticText(panel, wxID_ANY, "GPA Min"), 0, wxLEFT, 10);
    filtersSizer->Add(minGPAInput, 0, wxLEFT, 5);
    filtersSizer->Add(new wxStaticText(panel, wxID_ANY, "Max"), 0, wxLEFT, 10);
    filtersSizer->Add(maxGPAInput, 0, wxLEFT, 5);
    filtersSizer->Add(saveButton, 0, wxLEFT, 10);
    filtersSizer->Add(rankButton, 0, wxLEFT, 5);

    mainSizer->Add(filtersSizer, 0, wxEXPAND | wxALL, 5);

    candidatesList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(500, 200), wxLC_REPORT);
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

    //Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    //Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    //Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::PopulateCandidatesList()
{
    DatabaseManager& db = DatabaseManager::getInstance();
    std::vector<Candidate> candidates = db.getCandidatesByDate(getCurrentDate());

    candidatesList->DeleteAllItems();
    for (const auto& candidate : candidates) {
        long index = candidatesList->InsertItem(candidatesList->GetItemCount(), candidate.name);
        candidatesList->SetItem(index, 1, candidate.university);
        candidatesList->SetItem(index, 2, wxString::FromUTF8(candidate.skills.empty() ? "" : candidate.skills[0].c_str()));
        candidatesList->SetItem(index, 3, std::to_string(candidate.gpa));
    }
}