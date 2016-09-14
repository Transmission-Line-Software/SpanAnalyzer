// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "results_pane.h"

#include "models/base/helper.h"
#include "models/transmissionline/catenary.h"
#include "wx/xrc/xmlres.h"

#include "span_analyzer_app.h"
#include "span_analyzer_doc.h"
#include "span_analyzer_view.h"

BEGIN_EVENT_TABLE(ResultsPane, wxPanel)
  EVT_CHOICE(XRCID("choice_condition"), ResultsPane::OnChoiceCondition)
  EVT_CHOICE(XRCID("choice_report"), ResultsPane::OnChoiceReport)
  EVT_CHOICE(XRCID("choice_weathercase_group"), ResultsPane::OnChoiceWeathercaseGroup)
  EVT_LIST_ITEM_FOCUSED(wxID_ANY, ResultsPane::OnListCtrlFocus)
END_EVENT_TABLE()

ResultsPane::ResultsPane(wxWindow* parent, wxView* view) {
  // loads dialog from virtual xrc file system
  wxXmlResource::Get()->LoadPanel(this, parent, "results_pane");

  // saves view reference
  view_ = view;

  // initializes report choice
  type_report_ = ReportType::kSagTension;

  wxChoice* choice = XRCCTRL(*this, "choice_report", wxChoice);
  choice->Append("Sag-Tension");
  choice->Append("Tension Distribution");
  choice->Append("Catenary");
  choice->Append("Catenary - Endpoints");
  choice->SetSelection(0);

  // initializes weathercase set choice
  UpdateWeathercaseGroupChoice();
  choice = XRCCTRL(*this, "choice_weathercase_group", wxChoice);

  // intializes condition choice
  choice = XRCCTRL(*this, "choice_condition", wxChoice);
  choice->Append("Initial");
  choice->Append("Load");
  choice->SetSelection(0);

  // creates a report table
  table_ = new ReportTable(this);
  table_->set_data(&data_);

  wxBoxSizer* sizer = (wxBoxSizer*)this->GetSizer();
  sizer->Add(table_, 1, wxEXPAND);

  this->Fit();
}

ResultsPane::~ResultsPane() {
}

void ResultsPane::Update(wxObject* hint) {
  // caches focused index
  const long index_focus = table_->IndexFocused();

  // interprets hint
  UpdateHint* hint_update = (UpdateHint*)hint;
  if (hint_update == nullptr) {
    // do nothing, this is only passed when pane is created
  } else if (hint_update->type() == HintType::kCablesEdit) {
    UpdateReportData();
    table_->Refresh();
  } else if (hint_update->type() == HintType::kPreferencesEdit) {
    UpdateReportData();
    table_->Refresh();
  } else if (hint_update->type() == HintType::kSpansEdit) {
    UpdateReportData();
    table_->Refresh();
  } else if (hint_update->type() == HintType::kWeathercasesEdit) {
    UpdateWeathercaseGroupChoice();
    UpdateReportData();
    table_->Refresh();
  } else if (hint_update->type() == HintType::kWeathercaseSelect) {
    // do nothing
  }
  else if (hint_update->type() == HintType::kWeathercasesSelect) {
    UpdateReportData();
    table_->Refresh();
  }

  table_->set_formatting_column(0, 200, wxLIST_FORMAT_LEFT);
  table_->set_index_focused(index_focus);
}

void ResultsPane::OnChoiceCondition(wxCommandEvent& event) {
  // gets choice selection and updates view cache
  wxChoice* choice = XRCCTRL(*this, "choice_condition", wxChoice);
  wxString str = choice->GetStringSelection();

  SpanAnalyzerView* view = (SpanAnalyzerView*)view_;
  if (str == "Initial") {
    view->set_condition(CableConditionType::kInitial);
  } else if (str == "Load") {
    view->set_condition(CableConditionType::kLoad);
  } else {
    return;
  }

  // updates views
  UpdateHint hint(HintType::kWeathercasesSelect);
  view_->GetDocument()->UpdateAllViews(nullptr, &hint);
}

void ResultsPane::OnChoiceReport(wxCommandEvent& event) {
  // gets choice selection
  wxChoice* choice = XRCCTRL(*this, "choice_report", wxChoice);
  wxString str = choice->GetStringSelection();

  // updates report type
  if (str == "Sag-Tension") {
    type_report_ = ReportType::kSagTension;
  } else if (str == "Tension Distribution") {
    type_report_ = ReportType::kTensionDistribution;
  } else if (str == "Catenary") {
    type_report_ = ReportType::kCatenary;
  } else if (str == "Catenary - Endpoints") {
    type_report_ = ReportType::kCatenaryEndpoints;
  } else {
    return;
  }

  // this update only affects this pane, so a view update is not sent
  // updates the report data and table
  const long index = table_->IndexFocused();
  UpdateReportData();
  table_->Refresh();
  table_->set_formatting_column(0, 200, wxLIST_FORMAT_LEFT);
  table_->set_index_focused(index);
}

void ResultsPane::OnChoiceWeathercaseGroup(wxCommandEvent& event) {
  // gets weathercase set from application data
  wxChoice* choice = XRCCTRL(*this, "choice_weathercase_group", wxChoice);
  wxString str_selection = choice->GetStringSelection();

  // updates the selected/cached weathercases
  UpdateWeathercaseGroupSelected();

  // updates views
  UpdateHint hint(HintType::kWeathercasesSelect);
  view_->GetDocument()->UpdateAllViews(nullptr, &hint);
}

void ResultsPane::OnListCtrlFocus(wxListEvent& event) {
  // gets view
  SpanAnalyzerView* view = (SpanAnalyzerView*)view_;
  view->set_index_weathercase(table_->IndexFocused());

  // updates views
  UpdateHint hint(HintType::kWeathercaseSelect);
  view_->GetDocument()->UpdateAllViews(nullptr, &hint);
}

void ResultsPane::UpdateReportData() {
  // gets view display information
  SpanAnalyzerView* view = (SpanAnalyzerView*)view_;
  const WeatherLoadCaseGroup* group_weathercases = view->group_weathercases();
  const CableConditionType& condition = view->condition();

  // gets weathercases
  const std::list<WeatherLoadCase>* weathercases = nullptr;
  if (group_weathercases != nullptr) {
    weathercases = &group_weathercases->weathercases;
  }

  // gets results from document
  const SpanAnalyzerDoc* doc = (SpanAnalyzerDoc*)view_->GetDocument();
  const std::list<SagTensionAnalysisResult>& results =
      doc->ResultsFiltered(*group_weathercases, condition);

  // selects based on report type
  if (type_report_ == ReportType::kCatenary) {
    UpdateReportDataCatenary(weathercases, results);
  } else if (type_report_ == ReportType::kCatenaryEndpoints) {
    UpdateReportDataCatenaryEndpoints(weathercases, results);
  } else if (type_report_ == ReportType::kSagTension) {
    UpdateReportDataSagTension(weathercases, results);
  } else if (type_report_ == ReportType::kTensionDistribution) {
    UpdateReportDataTensionDistribution(weathercases, results);
  }
}

void ResultsPane::UpdateReportDataCatenary(
    const std::list<WeatherLoadCase>* weathercases,
    const std::list<SagTensionAnalysisResult>& results) {
  // initializes data
  data_.headers.clear();
  data_.rows.clear();

  // fills column headers
  data_.headers.push_back("Weathercase");
  data_.headers.push_back("H");
  data_.headers.push_back("w");
  data_.headers.push_back("H/w");
  data_.headers.push_back("Sag");
  data_.headers.push_back("L");
  data_.headers.push_back("Ls");
  data_.headers.push_back("Swing");

  // checks if weathercases are specified
  if (weathercases == nullptr) {
    return;
  }

  // checks if results has any data
  if (results.empty() == true) {
    return;
  }

  // gets the selected span from the document
  const SpanAnalyzerDoc* doc = (SpanAnalyzerDoc*)view_->GetDocument();
  const Span* span = doc->SpanAnalysis();

  // fills each row with data
  for (auto iter = results.cbegin(); iter != results.cend(); iter++) {
    // creates a report row, which will be filled out by each result
    ReportRow row;

    // gets the weathercase description
    const int index = std::distance(results.cbegin(), iter);
    const WeatherLoadCase& weathercase = *std::next(weathercases->cbegin(),
                                                    index);
    const std::string& description = weathercase.description;

    // gets the sag-tension result
    const SagTensionAnalysisResult& result = *iter;

    // creates a catenary to calculate results
    Catenary3d catenary;
    catenary.set_spacing_endpoints(span->spacing_catenary);
    catenary.set_tension_horizontal(result.tension_horizontal);
    catenary.set_weight_unit(result.weight_unit);

    double value;
    std::string str;

    // adds weathercase
    str = description;
    row.values.push_back(str);

    // adds H
    value = catenary.tension_horizontal();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds w
    value = catenary.weight_unit().Magnitude();
    str = helper::DoubleToFormattedString(value, 3);
    row.values.push_back(str);

    // adds H/w
    value = catenary.Constant();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds sag
    value = catenary.Sag();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds L
    value = catenary.Length();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds Ls
    value = catenary.LengthSlack();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds swing
    value = catenary.SwingAngle();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // appends row to list
    data_.rows.push_back(row);
  }
}

void ResultsPane::UpdateReportDataCatenaryEndpoints(
    const std::list<WeatherLoadCase>* weathercases,
    const std::list<SagTensionAnalysisResult>& results) {
  // initializes data
  data_.headers.clear();
  data_.rows.clear();

  // fills column headers
  data_.headers.push_back("Weathercase");
  data_.headers.push_back("Ts");
  data_.headers.push_back("Tv");
  data_.headers.push_back("A");
  data_.headers.push_back("");
  data_.headers.push_back("Ts");
  data_.headers.push_back("Tv");
  data_.headers.push_back("A");

  // checks if weathercases are specified
  if (weathercases == nullptr) {
    return;
  }

  // checks if results has any data
  if (results.empty() == true) {
    return;
  }

  // gets the selected span from the document
  const SpanAnalyzerDoc* doc = (SpanAnalyzerDoc*)view_->GetDocument();
  const Span* span = doc->SpanAnalysis();

  // fills each row with data
  for (auto iter = results.cbegin(); iter != results.cend(); iter++) {
    // creates a report row, which will be filled out by each result
    ReportRow row;

    // gets the weathercase description
    const int index = std::distance(results.cbegin(), iter);
    const WeatherLoadCase& weathercase = *std::next(weathercases->cbegin(),
                                                    index);
    const std::string& description = weathercase.description;

    // gets the sag-tension result
    const SagTensionAnalysisResult& result = *iter;

    // creates a catenary to calculate results
    Catenary3d catenary;
    catenary.set_spacing_endpoints(span->spacing_catenary);
    catenary.set_tension_horizontal(result.tension_horizontal);
    catenary.set_weight_unit(result.weight_unit);

    double value;
    std::string str;

    // adds weathercase
    str = description;
    row.values.push_back(str);

    // adds Ts
    value = catenary.Tension(0);
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds Tv
    value = catenary.Tension(0, AxisDirectionType::kPositive).z();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds A
    value = catenary.TangentAngleVertical(0, AxisDirectionType::kPositive);
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds blank
    row.values.push_back("");

    // adds Ts
    value = catenary.Tension(1);
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds Tv
    value = catenary.Tension(1, AxisDirectionType::kNegative).z();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds A
    value = catenary.TangentAngleVertical(1, AxisDirectionType::kNegative);
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // appends row to list
    data_.rows.push_back(row);
  }
}

void ResultsPane::UpdateReportDataSagTension(
    const std::list<WeatherLoadCase>* weathercases,
    const std::list<SagTensionAnalysisResult>& results) {
  // initializes data
  data_.headers.clear();
  data_.rows.clear();

  // fills column headers
  data_.headers.push_back("Weathercase");
  data_.headers.push_back("Wv");
  data_.headers.push_back("Wt");
  data_.headers.push_back("Wr");
  data_.headers.push_back("H");
  data_.headers.push_back("H/w");

  // checks if weathercases are specified
  if (weathercases == nullptr) {
    return;
  }

  // checks if results has any data
  if (results.empty() == true) {
    return;
  }

  // fills each row with data
  for (auto iter = results.cbegin(); iter != results.cend(); iter++) {
    // creates a report row, which will be filled out by each result
    ReportRow row;

    // gets the weathercase description
    const int index = std::distance(results.cbegin(), iter);
    const WeatherLoadCase& weathercase = *std::next(weathercases->cbegin(),
                                                    index);
    const std::string& description = weathercase.description;

    // gets the sag-tension result
    const SagTensionAnalysisResult& result = *iter;

    double value;
    std::string str;

    // adds weathercase
    str = description;
    row.values.push_back(str);

    // adds Wv
    value = result.weight_unit.z();
    str = helper::DoubleToFormattedString(value, 3);
    row.values.push_back(str);

    // adds Wt
    value = result.weight_unit.y();
    str = helper::DoubleToFormattedString(value, 3);
    row.values.push_back(str);

    // adds Wr
    value = result.weight_unit.Magnitude();
    str = helper::DoubleToFormattedString(value, 3);
    row.values.push_back(str);

    // adds H
    value = result.tension_horizontal;
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds H/w
    value = result.tension_horizontal / result.weight_unit.Magnitude();
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // appends row to list
    data_.rows.push_back(row);
  }
}

void ResultsPane::UpdateReportDataTensionDistribution(
    const std::list<WeatherLoadCase>* weathercases,
    const std::list<SagTensionAnalysisResult>& results) {
  // initializes data
  data_.headers.clear();
  data_.rows.clear();

  // fills column headers
  data_.headers.push_back("Weathercase");
  data_.headers.push_back("Hs");
  data_.headers.push_back("Hc");

  // checks if weathercases are specified
  if (weathercases == nullptr) {
    return;
  }

  // checks if results has any data
  if (results.empty() == true) {
    return;
  }

  // fills each row with data
  for (auto iter = results.cbegin(); iter != results.cend(); iter++) {
    // creates a report row, which will be filled out by each result
    ReportRow row;

    // gets the weathercase description
    const int index = std::distance(results.cbegin(), iter);
    const WeatherLoadCase& weathercase = *std::next(weathercases->cbegin(),
                                                    index);
    const std::string& description = weathercase.description;

    // gets the sag-tension result
    const SagTensionAnalysisResult& result = *iter;

    double value;
    std::string str;

    // adds weathercase
    str = description;
    row.values.push_back(str);

    // adds Hs
    value = result.tension_horizontal_shell;
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // adds Hc
    value = result.tension_horizontal_core;
    str = helper::DoubleToFormattedString(value, 1);
    row.values.push_back(str);

    // appends row to list
    data_.rows.push_back(row);
  }
}

void ResultsPane::UpdateWeathercaseGroupChoice() {
  // gets choice control
  wxChoice* choice = XRCCTRL(*this, "choice_weathercase_group", wxChoice);

  // saves current choice string
  std::string str_choice = choice->GetStringSelection();

  // clears choice control
  choice->Clear();

  // appends weathercase groups stored in the application data
  const std::list<WeatherLoadCaseGroup>& groups =
      wxGetApp().data()->groups_weathercase;
  for (auto iter = groups.cbegin(); iter != groups.cend(); iter ++) {
    const WeatherLoadCaseGroup& group = *iter;
    choice->Append(group.name);
  }

  // attempts to find the old weathercase set
  choice->SetSelection(choice->FindString(str_choice));

  // updates the selected/cached weathercases
  UpdateWeathercaseGroupSelected();
}

void ResultsPane::UpdateWeathercaseGroupSelected() {
  // initializes weathercases cached in view
  SpanAnalyzerView* view = (SpanAnalyzerView*)view_;
  view->set_group_weathercase(nullptr);

  // searches the choice control to see if a weathercase group is selected
  wxChoice* choice = XRCCTRL(*this, "choice_weathercase_group", wxChoice);
  std::string str_selection = choice->GetStringSelection();

  // gets weathercases from app data
  const std::list<WeatherLoadCaseGroup>& groups_weathercase =
       wxGetApp().data()->groups_weathercase;
  const WeatherLoadCaseGroup* group = nullptr;
  for (auto iter = groups_weathercase.cbegin();
       iter != groups_weathercase.cend(); iter++) {
    const WeatherLoadCaseGroup& group_temp = *iter;
    if (group_temp.name == str_selection) {
      group = &group_temp;
      break;
    }
  }

  // updates view with weathercase group
  view->set_group_weathercase(group);

  if (group == nullptr) {
    return;
  }

  // updates the view if the index if it is not valid anymore
  if ((int)group->weathercases.size() <= view->index_weathercase()) {
    view->set_index_weathercase(-1);
  }
}
