// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "plot_pane.h"

#include "line_renderer_2d.h"
#include "span_analyzer_view.h"

/// TEMPORARY
#include "models/transmissionline/catenary.h"

BEGIN_EVENT_TABLE(PlotPane, wxPanel)
  EVT_PAINT(PlotPane::OnPaint)
END_EVENT_TABLE()

PlotPane::PlotPane(wxWindow* parent, wxView* view)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
              wxTAB_TRAVERSAL) {
  view_ = view;

  plot_.set_background(*wxBLACK_BRUSH);
}

PlotPane::~PlotPane() {
}

void PlotPane::Update(wxObject* hint) {
  wxClientDC dc(this);

  // interprets hint
  ViewUpdateHint* hint_update = (ViewUpdateHint*)hint;
  if (hint_update == nullptr) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
       ViewUpdateHint::HintType::kModelAnalysisWeathercaseEdit) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
       ViewUpdateHint::HintType::kModelPreferencesEdit) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
       ViewUpdateHint::HintType::kModelSpansEdit) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
       ViewUpdateHint::HintType::kModelWeathercaseEdit) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
      ViewUpdateHint::HintType::kViewConditionChange) {
    UpdatePlot();
    RenderPlot(dc);
  } else if (hint_update->type() ==
      ViewUpdateHint::HintType::kViewWeathercasesSetChange) {
    UpdatePlot();
    RenderPlot(dc);
  }
}

void PlotPane::ClearPlot(wxDC& dc) {
  dc.Clear();
}

void PlotPane::OnPaint(wxPaintEvent& event) {
  // gets a device context
  wxPaintDC dc(this);

  // renders
  RenderPlot(dc);
}

void PlotPane::RenderPlot(wxDC& dc) {
  plot_.Redraw(dc, GetClientRect());
}

void PlotPane::UpdatePlot() {
  // gets the results
  SpanAnalyzerView* view = (SpanAnalyzerView*)view_;
  const SagTensionAnalysisResultSet& results = view->results();
  if (results.descriptions_weathercase.empty() == true) {
    wxClientDC dc(this);
    ClearPlot(dc);
    return;
  }

  /// \todo change this so that it can display final too
  const SagTensionAnalysisResult& result = *(results.results_initial.cbegin());

  // creates a catenary with the result parameters
  Catenary3d catenary;
  catenary.set_spacing_endpoints(results.span->spacing_catenary);
  catenary.set_tension_horizontal(result.tension_horizontal);
  catenary.set_weight_unit(result.weight_unit);

  // calculates points
  std::list<Point3d> points;
  const int i_max = 100;
  for (int i = 0; i <= i_max; i++) {
    double pos = double(i) / double(i_max);
    Point3d p = catenary.Coordinate(pos, true);
    points.push_back(p);
  }

  // converts points to lines
  std::list<Line2d> lines;
  for (auto iter = points.cbegin(); iter != std::prev(points.cend(), 1);
       iter++) {
    // gets current and next point in the list
    const Point3d p0 = *iter;
    const Point3d p1 = *(std::next(iter, 1));

    // creates a line and maps 3d catenary points to 2d points for drawing
    Line2d line;
    line.p0.x = p0.x;
    line.p0.y = p0.z;
    line.p1.x = p1.x;
    line.p1.y = p1.z;

    lines.push_back(line);
  }

  dataset_catenary_.set_data(lines);

  // creates renderer
  LineRenderer2d renderer;
  renderer.set_dataset(&dataset_catenary_);
  renderer.set_pen(wxCYAN_PEN);

  // adds renderer 2D plot
  plot_.AddRenderer(renderer);
}