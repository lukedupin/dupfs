#ifndef ORM_LIGHT
#define ORM_LIGHT

#include <QString>
#include <QHash>
#include <QList>

  //My name spaces
namespace OrmLightType {
  enum OrmLightEnum { ORM_TYPE_VALUE, ORM_TYPE_OBJECT, ORM_TYPE_ARRAY };
  enum OrmLightStatus { ORM_VOID, ORM_LOADED, ORM_TAINTED };
  enum OrmSymbol {SYM_BOOL, SYM_INT, SYM_DOUBLE, SYM_STRING, 
                  SYM_COLON, SYM_COMMA,
                  SYM_SQUARE_LEFT, SYM_SQUARE_RIGHT,
                  SYM_CURLY_LEFT, SYM_CURLY_RIGHT,
                  SYM_UNKNOWN };
};

  //Define the default static fuctions
class OrmLight : public QString {
  private:
  QHash<QString, OrmLight> Values;
  QList<OrmLight> Arrays;
  OrmLightType::OrmLightStatus Status;
  OrmLightType::OrmLightEnum Type;
  QString Table;
  QString Key;

  public:

    //! \brief Bad ass contructors
  OrmLight( OrmLightType::OrmLightEnum type = OrmLightType::ORM_TYPE_VALUE,
            OrmLightType::OrmLightStatus status = OrmLightType::ORM_VOID, 
            QString table = QString("") );
  OrmLight( const char* val, 
            OrmLightType::OrmLightEnum type = OrmLightType::ORM_TYPE_VALUE,
            OrmLightType::OrmLightStatus status = OrmLightType::ORM_VOID, 
            QString table = QString("") );
  OrmLight( QString val, 
            OrmLightType::OrmLightEnum type = OrmLightType::ORM_TYPE_VALUE,
            OrmLightType::OrmLightStatus status = OrmLightType::ORM_VOID, 
            QString table = QString("") );

    //Get the orm type
  OrmLightType::OrmLightEnum& type();

    //Get the orm type
  OrmLightType::OrmLightStatus& status();

    //The table name of this Orm object
  QString& table();

    /*! \brief Add an item to this object and return this object
        \param key The key of my hash
        \param value The value to be stored
        \return OrmLight& This object
    */
  OrmLight& add( QString key, OrmLight value );

    /*! \brief Push this value onto the array
        \param value OrmLight object
        \return OrmLight& This object
    */
  OrmLight& add( OrmLight value );

    //! \brief Push an orm object onto the array
  OrmLight& push( OrmLight val, int idx = -1 );

    //! \brief Pop the last object of the array
  void pop( int idx = -1 );

    //! \brief Return the first element in my array
  OrmLight& first();

    //! \brief Return the last element in my array
  OrmLight& last();

    //! \brief Return the size of the array stack
  int size();

    //! \brief return the size of the hash stack
  int sizeHash();

    //! \brief Returns true if the key exists
  bool contains( QString key );

    /*! \brief Convert this orm_light object to json formatted data
        \param user_readable True if you want the output to be user readable
    */
  virtual QString toJson( bool user_readable = false );

    //! \brief Load JSON data into an orm light object
  static OrmLight* fromJson( QString json, OrmLight* orm = NULL );

    //! \brief Used for parsing json data
  static OrmLight generate( QString json, int& idx, OrmLight* orm_ptr = NULL );

    //! \brief Returns symbol match based on the string
  static QString matchSymbol( QString json, int& idx, OrmLightType::OrmSymbol &sym );

    //! \brief Return the const begin interator from the internal hash
  QHash<QString, OrmLight>::const_iterator constBegin();

    //! \brief Return the const end interator from the internal hash
  QHash<QString, OrmLight>::const_iterator constEnd();

    //! \brief Joins the hash together, using mid as the seperator
  QString join( QString mid = QString("") );

    /*! \brief Returns the value of the hash or the default one passed in
        \param key The key to look up
        \param def The default value if none is in the hash
        \return QString The string from this value
    */
  QString def( QString key, QString def );

    /*! \brief Access a field using a constant char
        \param buf A constant char string
        \return The resulting OrmLight object 

        Access a hash name returned the next level down in the data tree
    */
  OrmLight& operator[]( const char* buf );

    /*! \brief Access a field using a QString
        \param key A QString object
        \return The resulting OrmLight object 

        Access a hash name returned the next level down in the data tree
    */
  OrmLight& operator[]( QString key );

    /*! \brief Access the OrmLight object like an array
        \param idx The array index to access
        \return The resulting OrmLight object 

        OrmLight objects can act like arrays dynamically. This operator 
        accessses the object like an array, has no knowledge of the hash
        like access
    */
  OrmLight& operator[]( int idx );

    /*! \brief Add two OrmLight objects together and give the result

      Add two orm_light modules together and return the result as a new one.
      Overlaps are preferanced with the right hand side OrmLight object.
    */
  OrmLight operator+( OrmLight add );

    /*! \brief Add a key value which will turn into a hash when paired
        \param key QString
        \return OrmLight&
    */
  OrmLight& operator>>( QString key );

    /*! \brief Add a value to this orm, if a key is waiting, a hash is paired,
                else, an array item is inserted
        \param value OrmLight
        \return OrmLight&
    */
  OrmLight& operator<<( OrmLight value );

    /*! \brief Traverse this object and print its contence to the screen
        
        This method is used for debugging.  It will traverse the contents
        of this OrmLight object, and print the contents to the screen using
        qDebug and spaces to align the data
    */
  void debugDump();

  private:
    /*! \brief Traverse the tree creating json output
        \param obj The OrmLight object to read from

        This is used to recursively go through the tree creating a single
        string that has a JSON representation of the data in this OrmLight obj
    */
  QString traverse( OrmLight& obj );

    /*! \brief Traverse the tree creating debug output
        \param obj The OrmLight object to read from
        \param depth The depth of this object, used for formating
        \param array True if the last call was from an array

        This is used to recursively go through the tree creating debug output
        used to view the contents of this object cleanly and easily
    */
  void debugTraverse( OrmLight& obj, int depth = 1, int array = 0 );
};

#endif
