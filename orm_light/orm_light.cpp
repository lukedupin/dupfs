#include <QStringList>
#include <QRegExp>
#include <QFile>

#include "orm_light.h"

using namespace OrmLightType;

  //Used so that we always pass back valid data
OrmLight scratch;


  //OrmLight
OrmLight::OrmLight( OrmLightEnum type, OrmLightStatus status, QString table ) :
    QString("")
{
  Type = type;
  Status = status;
  Table = table;
  Key = QString("");
}

  //OrmLight
OrmLight::OrmLight( QString val, OrmLightEnum type, OrmLightStatus status, 
                    QString table) : 
        QString( val )
{
  Type = type;
  Status = status;
  Table = table;
  Key = QString("");
}

  //Store a const value
OrmLight::OrmLight( const char* val, OrmLightEnum type, OrmLightStatus status, 
                    QString table ) : 
        QString( val )
{
  Type = type;
  Status = status;
  Table = table;
  Key = QString("");
}


  //Return the type
OrmLightEnum& OrmLight::type()
{
  return Type;
}

  //Return or set the status
OrmLightStatus& OrmLight::status()
{
  return Status;
}


  //Return or set the table name
QString& OrmLight::table()
{
  return Table;
}


  //Add this item
OrmLight& OrmLight::add( QString key, OrmLight value )
{
  (*this)[key] = value;
  return *this;
}

  //Add this item to the array
OrmLight& OrmLight::add( OrmLight value )
{
  this->push( value );
  return *this;
}


  //Called to remove an entry from the ormlight object
int OrmLight::remove( QString key )
{
  return Values.remove( key );
}


  //Clear out the object
void OrmLight::clear()
{
  Values.clear();
  Arrays.clear();
}


  //Returns true if both the array and hash are empty
bool OrmLight::isEmpty()
{
  return Values.isEmpty() && Arrays.isEmpty();
}


  //Push a value onto the array 
OrmLight& OrmLight::push( OrmLight val, int idx )
{
    //Set my type to be an array
  Type = ORM_TYPE_ARRAY;
  
    //Store this item
  if ( idx < 0 || idx >= Arrays.size() )
  {
    Arrays.push_back(val);
    return Arrays.last();
  }
  else
  {
    Arrays.insert( idx, val );
    return Arrays[idx];
  }
}


  //Pop the array
void OrmLight::pop( int idx )
{
  if ( idx < 0 || idx >= Arrays.size() )
    Arrays.pop_back();
  else
    Arrays.removeAt(idx);
}


  //Returns the keys from the hash
QList<QString> OrmLight::keys()
{
  return Values.keys();
}


  //Return the first element of the array
OrmLight& OrmLight::first()
{
  return Arrays.front();
}


  //Return the last element in the array
OrmLight& OrmLight::last()
{
  return Arrays.back();
}

  //Return the length of the array
int OrmLight::count()
{
  if ( Arrays.count() > 0 )
    return Arrays.count();

  return Values.count();
}


  //Return the length of the array
int OrmLight::size()
{
  return Arrays.size();
}


  //Return the size of the hash
int OrmLight::sizeHash()
{
  return Values.size();
}


  //Returns true if the key exists
bool OrmLight::contains( QString key )
{
  return Values.contains(key);
}


  //Being interator
QHash<QString, OrmLight>::const_iterator OrmLight::constBegin()
{
  return Values.constBegin();
}

  //End iterator
QHash<QString, OrmLight>::const_iterator OrmLight::constEnd()
{
  return Values.constEnd();
}

  //Join the hash together
QString OrmLight::join( QString mid )
{
  QString result;

    //Loop through my hash
  QHash<QString, OrmLight>::const_iterator i = Values.constBegin();
  while (i != Values.constEnd()) 
  {
    if (i == Values.constBegin())
      result += QString("%1=%3").arg(i.key()).arg(i.value());
    else
      result += QString("%1%2=%3").arg(mid).arg(i.key()).arg(i.value());
    ++i;
  }

  return result;
}

  //Returns the value of the hash or the deafult
QString OrmLight::def( QString key, QString def )
{
  return Values.value(key, def);
}

  //Save the json to a file
bool OrmLight::saveToFile( QString filename )
{
    //Write out my data
  QFile file(filename);

      //Open up this file
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    //Convert the file to a QStringList
  file.write( this->toJson( true ).toAscii() );

    //Close out the file
  file.close();

  return true;
}

  //Load json data form a file
OrmLight* OrmLight::loadFromFile( QString filename, OrmLight* orm )
{
  QFile file( filename );

    //Load up my file
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    throw QString("Failed to open %1").arg(file.fileName());
    return NULL;
  }

    //Load the file file and store it into my system file
  orm = OrmLight::fromJson( QString( file.readAll() ), orm );

    //Close down the file
  file.close();

  return orm;
}

  //Convert this object to json
QString OrmLight::toJson( bool user_readable )
{
  (void)user_readable;
  //return (user_readable)? debugTraverse( *this ): traverse( *this );
  return traverse( *this );
}


  //Create an orm light object from json
OrmLight* OrmLight::fromJson( QString json, OrmLight* orm )
{
  int idx = 0;

    //Generate a data tree
  if ( orm == NULL )
    orm = new OrmLight(ORM_TYPE_OBJECT);
  else
    orm->type() = ORM_TYPE_OBJECT;

    //Empty my object
  orm->clear();

    //Return the newly created tree
  generate( json.replace(QRegExp("\n"), ""), idx, orm );
  return orm;
}


  //Recrusively generate my data tree
OrmLight OrmLight::generate( QString json, int& idx, OrmLight* orm_ptr )
{
  OrmSymbol sym;
  QString match;
  QString tmp;
  int temp_idx;

    //Match some data!
  match = matchSymbol( json, idx, sym );  

    //Go through all possible type
  switch ( sym )
  {
      //My user values
    case SYM_BOOL:
    case SYM_INT:
    case SYM_DOUBLE:
    case SYM_STRING:
    case SYM_STRING_EMPTY:
      return OrmLight( match );
      break;

      //The values I shouldn't encounter here 
    default:
    case SYM_UNKNOWN:
      throw QString("Unknown symbol %3 at Idx (%1) *%2*").arg(idx - match.size()).arg(json.mid(0,12)).arg(sym);
      break;

    case SYM_COLON:
    case SYM_COMMA:
    case SYM_SQUARE_RIGHT:
    case SYM_CURLY_RIGHT:
      throw QString("Invalid symbol %3 at Idx (%1) *%2*").arg(idx - match.size()).arg(match).arg(sym);
      break;

      //Handle an array of values
    case SYM_SQUARE_LEFT:
    {
      OrmLight orm( ORM_TYPE_ARRAY );

        //If we weren't passed a valid orm, then make one
      if ( orm_ptr == NULL )
        orm_ptr = &orm;

        //Test for an empty list
      temp_idx = idx;
      match = matchSymbol( json, temp_idx, sym );  

        //Loop through my array of objects and parse them out
      if ( sym != SYM_SQUARE_RIGHT )
      {
        do { 
            //Recurse for the value of the array
          orm_ptr->push( generate( json, idx ) );
        
            //Step through the array
          match = matchSymbol( json, idx, sym );  
        } while ( sym == SYM_COMMA ); 
      }
      else
        idx = temp_idx;

        //Ensure we have a valid ending char
      if ( sym != SYM_SQUARE_RIGHT )
        throw QString("Invalid symbol at Idx %1, Expected ']' but found %2").arg(idx - match.size()).arg(match);

      return *orm_ptr;
    } break;

      //Handle a hash of values
    case SYM_CURLY_LEFT:
    {
      OrmLight orm( ORM_TYPE_OBJECT );

        //If we weren't passed a valid orm, then make one
      if ( orm_ptr == NULL )
        orm_ptr = &orm;

        //Test for an empty list
      temp_idx = idx;
      match = matchSymbol( json, temp_idx, sym );  

        //Loop through my hash and pull out the values
      if ( sym != SYM_CURLY_RIGHT )
      {
        do { 
            //Step through the hash
          match = matchSymbol( json, idx, sym );  
          if ( sym != SYM_STRING )
            throw QString("Invalid symbol at Idx %1, Expected \"xxx\" but found %2").arg(idx - match.size()).arg(match);

            //Match the colon now
          tmp = matchSymbol( json, idx, sym );  
          if ( sym != SYM_COLON )
            throw QString("Invalid symbol at Idx %1, Expected : but found %2").arg(idx - tmp.size()).arg(tmp);

            //Recurse for the value of the array
          (*orm_ptr)[match] = generate( json, idx );
        
            //Step through the array
          match = matchSymbol( json, idx, sym );  
        } while ( sym == SYM_COMMA ); 
      }
      else
        idx = temp_idx;

        //Ensure we have a valid ending char
      if ( sym != SYM_CURLY_RIGHT )
        throw QString("Invalid symbol at Idx %1, Expected '}' but found %2").arg(idx - match.size()).arg(match);

      return *orm_ptr;
    } break;
  }

    //Shouldn't be possible to get here!
  return OrmLight();
}

  //Match json symbols
QString OrmLight::matchSymbol( QString json, int& idx, OrmLightType::OrmSymbol& sym )
{
    //My symbols
  QRegExp reg_white("[\\t ]*");
  QRegExp reg_bool("(true|false)");
  QRegExp reg_int("([-]?[0-9]+)");
  QRegExp reg_double("([-]?[0-9]+\\.[0-9]+)");
  QRegExp reg_string("\"(.*[^\\\\])\"");
  QRegExp reg_string_empty("\"()\"");
  QRegExp reg_colon("(:)");
  QRegExp reg_comma("(,)");
  QRegExp reg_square_left("(\\[)");
  QRegExp reg_square_right("(\\])");
  QRegExp reg_curly_left("(\\{)");
  QRegExp reg_curly_right("(\\})");
  QRegExp* reg = NULL;

    //Make my string matches none greedy
  reg_string.setMinimal( true );

  //qDebug("Before: %d", idx );

    //Move passed any leading white space
  if ( reg_white.indexIn( json, idx ) == idx )
    idx += reg_white.cap(0).size();

  //qDebug("After: %d", idx );

    //Curly braces for hash's
  if      ( reg_curly_left.indexIn( json, idx) == idx ) 
  {
    sym = SYM_CURLY_LEFT;
    reg = &reg_curly_left;
  }
  else if ( reg_curly_right.indexIn( json, idx) == idx )
  {
    sym = SYM_CURLY_RIGHT;
    reg = &reg_curly_right;
  }

    //Square brackets for arrays
  else if ( reg_square_left.indexIn( json, idx) == idx )
  {
    sym = SYM_SQUARE_LEFT;
    reg = &reg_square_left;
  }
  else if ( reg_square_right.indexIn( json, idx) == idx )
  {
    sym = SYM_SQUARE_RIGHT;
    reg = &reg_square_right;
  }

    //Match a comma
  else if ( reg_comma.indexIn( json, idx) == idx )
  {
    sym = SYM_COMMA;
    reg = &reg_comma;
  }

    //Match a hash colon
  else if ( reg_colon.indexIn( json, idx) == idx )
  {
    sym = SYM_COLON;
    reg = &reg_colon;
  }

    //Match a user value
  else if ( reg_string_empty.indexIn( json, idx) == idx ) 
  {
    sym = SYM_STRING_EMPTY;
    reg = &reg_string_empty;
  }
  else if ( reg_string.indexIn( json, idx) == idx ) 
  {
    sym = SYM_STRING;
    reg = &reg_string;
  }
  else if ( reg_double.indexIn( json, idx) == idx )
  {
    sym = SYM_DOUBLE;
    reg = &reg_double;
  }
  else if ( reg_int.indexIn( json, idx) == idx )
  {
    sym = SYM_INT;
    reg = &reg_int;
  }
  else if ( reg_bool.indexIn( json, idx) == idx )
  {
    sym = SYM_BOOL;
    reg = &reg_bool;
  }

    //Step my Index forward and return my mached text
  if ( reg != NULL )
  {
    //qDebug("%s (%s)", reg->cap(0).toAscii().data(), reg->cap(1).toAscii().data() );
    idx += reg->cap(0).size();
    return reg->cap(1).replace(QRegExp("\\\\\""), "\"");
  }

    //Didn't find much anything, quiting out now
  idx += json.mid( idx, 12).size();
  sym = SYM_UNKNOWN;
  return json.mid( idx - 12, 12);
}

  //Return a value from an orm object
OrmLight& OrmLight::operator[]( const char* buf )
{
  Type = ORM_TYPE_OBJECT;
  return Values[QString(buf)];
}

  //Return a value from an orm object
OrmLight& OrmLight::operator[]( QString key )
{
  Type = ORM_TYPE_OBJECT;
  //return (Values.contains(key))? Values[key]: scratch;
  return Values[key];
}


  //Return a value from my array
OrmLight& OrmLight::operator[]( int idx )
{
  return (idx >= 0 && idx < Arrays.size())? Arrays[idx]: scratch;
}


  //Return the aggeragated result of this data
OrmLight OrmLight::operator+( OrmLight add )
{
  OrmLight obj = *this;

    //Add the incoming to the current
  QHash<QString, OrmLight>::const_iterator i = add.constBegin();
  while (i != add.constEnd()) {
     obj[i.key()] = i.value();
     ++i;
  }

  return obj;
}

  //Add in a key to this orm
OrmLight& OrmLight::operator>>( QString key )
{
    //Store the key
  Key = key;

  return *this;
}

  //Add in a value to this orm
OrmLight& OrmLight::operator<<( OrmLight value )
{
    //Add in either a hash value or an array value
  if ( Key.isEmpty() )  add( value );
  else                add( Key, value );
  
    //Empty out my key value
  Key = QString();

  return *this;
}

  //Create debug out of the contencts of this object and its children
void OrmLight::debugDump()
{
  debugTraverse( *this );
}



  //Tranverse the tree making json output
QString OrmLight::traverse( OrmLight& obj )
{
  int i;
  QStringList ary;

    //If we aren't an object quit something is wrong
  switch ( obj.type() )
  {
      //If we are just a value, then thats easy... dump it out
    case ORM_TYPE_VALUE:
      return QString("\"%1\"").arg(obj);
      break;

    case ORM_TYPE_ARRAY:
        //Loop through all my indexes in the array
      for ( i = 0; i < obj.size(); i++ )
        ary << traverse((obj)[i]);
        
        //Compress down my array items and them into this thing
      return QString("[%1]").arg(ary.join(","));
      break;

      //Should be an orm object if we get here
    default: break;
  }

    //Return an empty string if we aren't an obect, cuz don't know what we are
  if ( obj.type() != ORM_TYPE_OBJECT )
    return QString("");

    //Start everything with an object
  QStringList str;

    //Iterrate through this thing
  QHash<QString, OrmLight>::const_iterator hash = obj.constBegin();
  while (hash != obj.constEnd()) 
  {
      //First add my key into this thing
    str << QString("\"%1\": %2").arg(hash.key()).arg(traverse(const_cast<OrmLight&>(hash.value())));
    ++hash;
  }

    //Add on my ending object and then compress the string down
  return QString("{%1}").arg(str.join(","));
}


  //Tranverse the tree making Debug output
void OrmLight::debugTraverse( OrmLight& obj, int depth, int array )
{
  int i;
  int p;
  QString pad = QString("");
  QString pads = QString("");
  QString str;

    //Go through the pad requested here
  for ( p = 0; p < depth; p++ )
  {
    if ( p != 0 )
      pads += QString("  ");
    pad += QString("  ");
  }

    //If we aren't an object quit something is wrong
  switch ( obj.type() )
  {
      //If we are just a value, then thats easy... dump it out
    case ORM_TYPE_VALUE:
      if ( array )
        qDebug("%s\n", QString("%1\"%2\"").arg(pad).arg(obj).toAscii().data());
      else
        qDebug("%s\n", QString("\"%1\"").arg(obj).toAscii().data());
      return; 
      break;

    case ORM_TYPE_ARRAY:
      qDebug("\n%s\n", QString("%1[").arg(pads).toAscii().data());

        //Loop through all my indexes in the array
      for ( i = 0; i < obj.size(); i++ )
        debugTraverse((obj)[i], depth+1, 1);
        
        //Compress down my array items and them into this thing
      qDebug("%s\n", QString("%1]").arg(pads).toAscii().data());
      return;
      break;

      //Should be an orm object if we get here
    default: break;
  }

    //Return an empty string if we aren't an obect, cuz don't know what we are
  if ( obj.type() != ORM_TYPE_OBJECT )
  {
    qDebug("Error, Unknown data type\n");
    return ;
  }

    //Start everything with an object
  if ( array )
    qDebug("%s\n", QString("%1{").arg(pads).toAscii().data());
  else
    qDebug("\n%s\n", QString("%1{").arg(pads).toAscii().data());

    //Iterrate through this thing
  QHash<QString, OrmLight>::const_iterator hash = obj.constBegin();
  while (hash != obj.constEnd()) 
  {
      //First add my key into this thing
    qDebug("%s", QString("%1\"%2\": ").arg(pad).arg(hash.key()).toAscii().data());
    debugTraverse(const_cast<OrmLight&>(hash.value()), depth+1);
    ++hash;
  }

    //Add on my ending object and then compress the string down
  qDebug("%s\n", QString("%1}").arg(pads).toAscii().data());
}
