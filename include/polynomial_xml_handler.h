// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#ifndef OTLS_SPANANALYZER_POLYNOMIALXMLHANDLER_H_
#define OTLS_SPANANALYZER_POLYNOMIALXMLHANDLER_H_

#include <string>

#include "models/base/polynomial.h"
#include "wx/wx.h"
#include "wx/xml/xml.h"

#include "xml_handler.h"

/// \par OVERVIEW
///
/// This class parses and generates a polynomial XML node. The data is
/// transferred between the XML node and the data object.
///
/// \par VERSION
///
/// This class can parse all versions of the polynomial XML node. However,
/// new polynomial nodes will only be generated with the most recent version.
class PolynomialXmlHandler : public XmlHandler {
 public:
  /// \brief Constructor.
  PolynomialXmlHandler();

  /// \brief Destructor.
  ~PolynomialXmlHandler();

  /// \brief Creates an XML node for the polynomial.
  /// \param[in] polynomial
  ///   The polynomial.
  /// \param[in] name
  ///   The name for the XML node.
  /// \return An XML node for the polynomial.
  static wxXmlNode* CreateNode(const Polynomial& polynomial,
                               const std::string& name_polynomial,
                               const wxXmlAttribute& attribute_coefficients);

  /// \brief Parses an XML node and populates a polynomial.
  /// \param[in] root
  ///   The XML root node for the polynomial.
  /// \param[out] polynomial
  ///   The polynomial that is populated.
  /// \return The file line number of the node if the content could not be
  ///   converted to the expected data type. Returns 0 if no errors were
  ///   encountered.
  static int ParseNode(const wxXmlNode* root, Polynomial& polynomial);

 private:
  /// \brief Parses an XML node and populates a polynomial.
  /// \param[in] root
  ///   The XML root node for the polynomial.
  /// \param[out] polynomial
  ///   The polynomial that is populated.
  /// \return The file line number of the node if the content could not be
  ///   converted to the expected data type. Returns 0 if no errors were
  ///   encountered.  
  static int ParseNodeV1(const wxXmlNode* root, Polynomial& polynomial);
};

#endif  // OTLS_SPANANALYZER_POLYNOMIALXMLHANDLER_H_