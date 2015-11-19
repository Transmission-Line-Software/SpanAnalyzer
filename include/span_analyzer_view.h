// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#ifndef OTLS_SPANANALYZER_SPANANALYZERVIEW_H_
#define OTLS_SPANANALYZER_SPANANALYZERVIEW_H_

#include <list>

#include "wx/aui/framemanager.h"
#include "wx/docview.h"

class EditPane;
class MessagesPane;
class ResultsPane;

class ViewUpdateHint : public wxObject {
 public:
  enum class HintType {
	  kModelCableEdit,
	  kModelPreferencesEdit,
	  kModelSpansEdit,
    kModelWeathercaseEdit,
  };

  ViewUpdateHint(HintType type) {type = type_;};

  HintType type_;
};

/// \par OVERVIEW
///
class SpanAnalyzerView : public wxView {
 public:
  /// \brief Constructor.
  SpanAnalyzerView();

  /// \brief Destructor.
  ~SpanAnalyzerView();

  /// Called when the document is created
  virtual bool OnCreate(wxDocument *doc, long flags);

  /// Called to draw the document
  virtual void OnDraw(wxDC *dc);

  /// Called when the view needs to be updated
  virtual void OnUpdate(wxView *sender, wxObject *hint = nullptr);

  /// Called when the view is closed
  virtual bool OnClose(bool deleteWindow = true);

  EditPane* pane_edit();

  ResultsPane* pane_results();

  wxString GetPerspective();

 private:
  /// \manager_
  ///   The AUI manager.
  wxAuiManager manager_;

  EditPane* pane_edit_;
  ResultsPane* pane_results_;

  //std::list<wxCommandEvent> events_;

  wxDECLARE_DYNAMIC_CLASS(SpanAnalyzerView);
};

#endif //  OTLS_SPANANALYZER_SPANANALYZERVIEW_H_