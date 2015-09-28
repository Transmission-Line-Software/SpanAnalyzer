// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "polynomial_xml_handler.h"

#include "models/base/helper.h"

PolynomialXmlHandler::PolynomialXmlHandler() {
}

PolynomialXmlHandler::~PolynomialXmlHandler() {
}

wxXmlNode* PolynomialXmlHandler::CreateNode(
    const Polynomial& polynomial,
    const std::string& name_polynomial,
    const wxXmlAttribute& attribute_coefficients) {
  // variables used to create XML node
  std::string content;
  wxXmlNode* node_root = nullptr;
  wxXmlNode* node_element = nullptr;

  // creates a node for the polynomial root
  node_root = new wxXmlNode(wxXML_ELEMENT_NODE, "polynomial");
  node_root->AddAttribute("name", wxString(name_polynomial));
  node_root->AddAttribute("version", "1");

  // creates a child node for each coefficient
  const std::vector<double>* coefficients = polynomial.coefficients();
  for (auto iter = coefficients->begin(); iter != coefficients->end(); iter++) {
    double coefficient = *iter;
    content = helper::DoubleToFormattedString(coefficient, 1);
    wxXmlAttribute* attribute = new wxXmlAttribute(attribute_coefficients);
    node_element = XmlHandler::CreateElementNodeWithContent(
        "coefficient",
        content,
        attribute); 
    node_root->AddChild(node_element);
  }

  return node_root;
}

int PolynomialXmlHandler::ParseNode(const wxXmlNode* root, 
                                    Polynomial& polynomial) {
  // checks for valid root node
  if (root->GetName() != "polynomial") {
    return root->GetLineNumber();
  }

  // gets version attribute
  wxString version;
  if (root->GetAttribute("version", &version) == false) {
    return root->GetLineNumber();
  }

  // sends to proper parsing function
  if (version == "1") {
    return ParseNodeV1(root, polynomial);
  } else {
    return root->GetLineNumber();
  }
}

int PolynomialXmlHandler::ParseNodeV1(const wxXmlNode* root, 
                                      Polynomial& polynomial) {
  // variables used to parse XML node
  wxString name;
  wxString content;
  double coefficient = -999999;
  std::vector<double>* coefficients = new std::vector<double>;

  // evaluates each child node and fills coefficients vector
  wxXmlNode* node = root->GetChildren();
  while (node != nullptr) {
    name = node->GetName();
    content = node->GetChildren()->GetContent();

    if (name == "coefficient") {
      if (content.ToDouble(&coefficient) == true) {
        coefficients->push_back(coefficient);
      } else {
        return node->GetLineNumber();
      }
    }
    node = node->GetNext();
  }

  // sets polynomial coefficients
  polynomial.set_coefficients(coefficients);

  // if it reaches this point, no errors were encountered
  return 0;
}