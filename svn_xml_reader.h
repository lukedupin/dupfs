#ifndef SVN_XML_READER
#define SVN_XML_READER

#include <QXmlStreamReader>
#include <QByteArray>

#include "orm_light/orm_light.h"

class SvnXmlReader : public QXmlStreamReader
{
  public:
    //! \brief The enum of all possible svn xml outputs we can read
  enum SvnXmlType {
    SVN_XML_UNKNOWN,
    SVN_XML_LOG,
    SVN_XML_TYPE_COUNT
  };

    //! \brief Reads an svn log and returns it as an ormlight object  
  static OrmLight readSvnXml( QString cmd, SvnXmlType type );

    //! \brief the size of my buffer for reading a single line
  static const int BUFFER_SIZE = 4096;

  private:
    //! \brief The xml type we are gong to be reading
  SvnXmlType  Xml_Type;
    //! \brief My OrmLight object that holds my svn log data
  OrmLight    Data;

  public:
    //! \brief Loads up the svn type  we are looking for
  SvnXmlReader( SvnXmlType type = SVN_XML_UNKNOWN );

    //! \brief Set the xml type we should read
  SvnXmlType xmlType( SvnXmlType type = SVN_XML_UNKNOWN );

    //! \brief Returns my OrmLight object
  OrmLight data();

    //! \brief Read xml from a QIODevice
  bool read( QByteArray ary);

  private:
    //! \brief Read an unknown element
  void readUnknownElement();

    //! \brief Called after a IODev is set to read svn log xml
  void readSvnLog();
  OrmLight readSvnLog_LogEntry();
  OrmLight readSvnLog_Paths();
};

#endif
