#include <stdio.h>

#include "svn_xml_reader.h"

  //Reads an svn log xml from a given svn command
OrmLight SvnXmlReader::readSvnXml( QString cmd, SvnXmlType type )
{
  QString data;
  SvnXmlReader xml( type );
  FILE* handle;
  char* p;
  char buffer[BUFFER_SIZE];
  bool result;

    //Pipe in my command
  if ( (handle = popen( cmd.toAscii().data(), "r" )) == NULL )
  {
    qDebug("Invalid popen for: %s", cmd.toAscii().data() );
    return OrmLight();
  }

    //Read all the data the file has for me
  for ( p = fgets( buffer, BUFFER_SIZE, handle ); !feof( handle );
        p = fgets( buffer, BUFFER_SIZE, handle ) )
    data += buffer;

    //Close down my pipe
  pclose( handle );

    //Now pass my QFile, IE, QIODevice to start reading the data
  result = xml.read( data.toAscii() );

    //Return data to the user or a blank object depending on our read
  return (result)? xml.data(): OrmLight();
}

  //Init my object
SvnXmlReader::SvnXmlReader( SvnXmlType type )
{
  Xml_Type = type;
}

  //Gets/sets the object
SvnXmlReader::SvnXmlType SvnXmlReader::xmlType( SvnXmlType type )
{
    //Check if I should set my value
  if ( type != SVN_XML_UNKNOWN && type < SVN_XML_TYPE_COUNT )
    Xml_Type = type;

  return Xml_Type;
}

  //Returns orm data
OrmLight  SvnXmlReader::data()
{
  return Data;
}

  //Assign my io object
bool SvnXmlReader::read(QByteArray ary)
{
    //Pass my device to the parent object
  addData( ary );

    //Start reading my xml
  while (!atEnd()) 
  {
      //Read the first element
    readNext();

      //Ensure we are on a starting element
    if ( !isStartElement()) continue;

      //Figure out what kinda parsing we should do on this guy
    switch ( Xml_Type )
    {
        //Read output from an svn log
      case SVN_XML_LOG:
        readSvnLog();
        break;

        //not sure what we found
      default:  
        raiseError(QObject::tr("Invalid SVN parse type selected"));
        break;
    }
  }

  return !error();
}

  //Reads invalid elements
void SvnXmlReader::readUnknownElement()
{
  Q_ASSERT(isStartElement());

  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement())
      readUnknownElement();
  }
}

  //Reads an svn log tag
void SvnXmlReader::readSvnLog()
{
  Q_ASSERT(isStartElement() && name() == "log");

    //Clear out my orm Object
  Data.clear();

    //Read the entrie xml doc
  while (!atEnd()) 
  {
      //Read an element
    readNext();

      //If this is a closing element, quit out entirely
    if (isEndElement() && name() == "log" )
      break;

      //If we aren't on a starting element, then skip it 
    if ( !isStartElement()) continue;

      //Figure out what kinda element we got
    if (name() == "logentry")
    {
      QString key = attributes().value("revision").toString();
      Data[key.rightJustified(10, '0')] = readSvnLog_LogEntry();
    }
    else
      readUnknownElement();
  }
}

  //Reads an svn log entr tag
OrmLight SvnXmlReader::readSvnLog_LogEntry()
{
  Q_ASSERT(isStartElement() && name() == "logentry");
  OrmLight orm;

    //Read the entrie xml doc
  while (!atEnd()) 
  {
      //Read an element
    readNext();

      //If this is a closing element, quit out entirely
    if (isEndElement() && name() == "logentry" )
      break;

      //If we aren't on a starting element, then skip it 
    if ( !isStartElement()) continue;

      //Figure out what kinda element we got
    if (name() == "author" || name() == "date" || name() == "msg")
      orm[name().toString()] = readElementText();
    else if ( name() == "paths" )
      orm["paths"] = readSvnLog_Paths();
    else
      readUnknownElement();
  }

    //Give back my orm object
  return orm;
}

  //Reads svn paths entries
OrmLight SvnXmlReader::readSvnLog_Paths()
{
  Q_ASSERT(isStartElement() && name() == "paths");
  OrmLight orm;

    //Read the entrie xml doc
  while (!atEnd()) 
  {
      //Read an element
    readNext();

      //If this is a closing element, quit out entirely
    if (isEndElement() && name() == "paths" )
      break;

      //If we aren't on a starting element, then skip it 
    if ( !isStartElement()) continue;

      //Figure out what kinda element we got
    if (name() == "path")
      orm.push( readElementText() );
    else
      readUnknownElement();
  }

    //Give back my orm object
  return orm;
}
