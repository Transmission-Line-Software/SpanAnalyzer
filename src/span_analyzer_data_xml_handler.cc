// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "span_analyzer_data_xml_handler.h"

#include "wx/filename.h"

#include "file_handler.h"
#include "weather_load_case_xml_handler.h"

wxXmlNode* SpanAnalyzerDataXmlHandler::CreateNode(
    const SpanAnalyzerData& data,
    const units::UnitSystem& units) {
  // variables used to create XML node
  wxXmlNode* node_root = nullptr;
  wxXmlNode* node_element = nullptr;
  std::string title;
  std::string content;

  // creates a node for the root
  node_root = new wxXmlNode(wxXML_ELEMENT_NODE, "span_analyzer_data");
  node_root->AddAttribute("version", "1");

  // adds child nodes for struct parameters

  // creates cable directory node
  title = "cables";
  node_element = new wxXmlNode(wxXML_ELEMENT_NODE, title);

  for (auto iter = data.cablefiles.cbegin(); iter != data.cablefiles.cend();
       iter++) {
    const CableFile& cablefile = *iter;
    wxXmlNode* sub_node = CreateElementNodeWithContent("file",
                                                       cablefile.filepath);
    node_element->AddChild(sub_node);
  }

  node_root->AddChild(node_element);

  // creates analysis weathercase node
  title = "analysis_weather_load_cases";
  node_element = new wxXmlNode(wxXML_ELEMENT_NODE, title);

  for (auto iter = data.weathercases_analysis.cbegin();
       iter != data.weathercases_analysis.cend(); iter++) {
    // creates a weathercase set node
    wxXmlNode* sub_node = new wxXmlNode(wxXML_ELEMENT_NODE,
                                        "analysis_weather_load_case_set");

    // adds name attribute
    const int index = std::distance(data.weathercases_analysis.cbegin(), iter);
    auto iter_name = std::next(data.descriptions_weathercases_analysis.cbegin(),
                               index);
    sub_node->AddAttribute("name", *iter_name);

    // gets weathercase set, iterates through list
    const std::list<WeatherLoadCase>& weathercases = *iter;
    for (auto it = weathercases.cbegin(); it != weathercases.cend(); it++) {
      // adds a weathercase node to the set node
      const WeatherLoadCase& weathercase = *it;
      wxXmlNode* node_weathercase =
          WeatherLoadCaseXmlHandler::CreateNode(weathercase, "", units);
      sub_node->AddChild(node_weathercase);
    }
    node_element->AddChild(sub_node);
  }
  node_root->AddChild(node_element);

  // returns node
  return node_root;
}

int SpanAnalyzerDataXmlHandler::ParseNode(const wxXmlNode* root,
                                          const std::string& filepath,
                                          const units::UnitSystem& units,
                                          SpanAnalyzerData& data) {
  // checks for valid root node
  if (root->GetName() != "span_analyzer_data") {
    return root->GetLineNumber();
  }

  // gets version attribute
  wxString version;
  if (root->GetAttribute("version", &version) == false) {
    return root->GetLineNumber();
  }

  // sends to proper parsing function
  if (version == "1") {
    return ParseNodeV1(root, filepath, units, data);
  } else {
    return root->GetLineNumber();
  }
}

int SpanAnalyzerDataXmlHandler::ParseNodeV1(const wxXmlNode* root,
                                            const std::string& filepath,
                                            const units::UnitSystem& units,
                                            SpanAnalyzerData& data) {
  wxString message;

  // evaluates each child node
  const wxXmlNode* node = root->GetChildren();
  while (node != nullptr) {
    const wxString title = node->GetName();
    const wxString content = ParseElementNodeWithContent(node);

    if (title == "cables") {
      // gets node for cable file
      wxXmlNode* sub_node = node->GetChildren();
      while (sub_node != nullptr) {
        CableFile cablefile;

        // gets filepath
        cablefile.filepath = ParseElementNodeWithContent(sub_node);

        // loads cable file
        // filehandler function handles all logging
        int status =  FileHandler::LoadCable(cablefile.filepath, units,
                                             cablefile.cable);

        if (status == 0) {
          // adds to data lists
          data.cablefiles.push_back(cablefile);
        }

        sub_node = sub_node->GetNext();
      }
    } else if (title == "analysis_weather_load_cases") {
      // gets node for analysis weather load case set
      wxXmlNode* sub_node = node->GetChildren();
      while (sub_node != nullptr) {
        // creates a new weather load case set
        std::list<WeatherLoadCase> weathercase_set;
        const std::string str_name = sub_node->GetAttribute("name");

        // gets node for individual weathercases
        wxXmlNode* node_weathercase = sub_node->GetChildren();
        while (node_weathercase != nullptr) {
          // creates a weathercase and parses
          WeatherLoadCase weathercase;
          int status = WeatherLoadCaseXmlHandler::ParseNode(
              node_weathercase,
              filepath,
              weathercase);

          // adds to container if parsing was successful
          if (status == 0) {
            weathercase_set.push_back(weathercase);
          } else {
            message = FileAndLineNumber(filepath, node_weathercase)
                      + "Invalid weathercase. Skipping.";
            wxLogError(message);
          }

          node_weathercase = node_weathercase->GetNext();
        }

        // adds weathercase set description to list
        data.descriptions_weathercases_analysis.push_back(str_name);

        // adds weathercase set to analysis weathercases list
        data.weathercases_analysis.push_back(weathercase_set);

        sub_node = sub_node->GetNext();
      }
    } else {
      message = FileAndLineNumber(filepath, node)
                + "XML node isn't recognized.";
      wxLogMessage(message);
    }

    node = node->GetNext();
  }

  // if it gets to this point, no errors were encountered
  return 0;
}
