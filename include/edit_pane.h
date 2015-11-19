// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#ifndef OTLS_SPANANALYZER_EDITPANE_H_
#define OTLS_SPANANALYZER_EDITPANE_H_

#include "models/transmissionline/weather_load_case.h"
#include "wx/docview.h"
#include "wx/treectrl.h"
#include "wx/wx.h"

#include "span.h"

class EditTreeItemData : public wxTreeItemData {
 public:
  enum class Type {
    kSpan,
    kWeathercase
  };

  EditTreeItemData(Type type, const wxString& description) {
    type_ = type;
    description_ = description;
  };

  wxString description() {return description_;}
  void set_description(wxString description) {description_ = description;};
  Type type() {return type_;}

 private:
  wxString description_;
  Type type_;
};

class WeathercaseTreeCtrl : public wxTreeCtrl {
 public:
  WeathercaseTreeCtrl(wxWindow* parent, wxView* view);
  void Update(wxObject* hint);

 private:
  void AddWeathercase();
  void CopyWeathercase(const wxTreeItemId& id);
  void DeleteWeathercase(const wxTreeItemId& id);
  void DeleteWeathercases();
  void EditWeathercase(const wxTreeItemId& id);
  void InitWeathercases();
  void OnContextMenuSelect(wxCommandEvent& event);
  void OnItemMenu(wxTreeEvent& event);

  std::vector<WeatherLoadCase>* weathercases_;
  wxView* view_;

  DECLARE_EVENT_TABLE()
};

class SpanTreeCtrl : public wxTreeCtrl {
 public:
  SpanTreeCtrl(wxWindow* parent, wxView* view);
  void Update(wxObject* hint);

 private:
  void ActivateSpan(const wxTreeItemId& id);
  void AddSpan();
  void CopySpan(const wxTreeItemId& id);
  void DeleteSpan(const wxTreeItemId& id);
  void DeleteSpans();
  void EditSpan(const wxTreeItemId& id);
  void InitSpans();
  void OnContextMenuSelect(wxCommandEvent& event);
  void OnItemActivate(wxTreeEvent& event);
  void OnItemMenu(wxTreeEvent& event);

  std::vector<Span>* spans_;
  wxView* view_;

  DECLARE_EVENT_TABLE()
};

/// \par OVERVIEW
class EditPane : public wxPanel {
 public:
  EditPane(wxWindow* parent, wxView* view);
  ~EditPane();

  Span* ActivatedSpan();
  void Update(wxObject* hint = nullptr);

 private:
  wxView* view_;

  /// \var
  SpanTreeCtrl* treectrl_spans_;
  WeathercaseTreeCtrl* treectrl_weathercases_;
};

# endif //  OTLS_SPANANALYZER_EDITPANE_H_
