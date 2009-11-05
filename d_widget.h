#ifndef D_WIDGET
#define D_WIDGET

#include <QString>
#include <QGroupBox>
#include <QHBoxLayout>

  //Class for the list widget
template <typename Widget>
class DWidget : public  Widget
{
  private:
    //! Group box
  QGroupBox*    Group_Box;
    //! Layout
  QHBoxLayout*  Layout;

  public:

  DWidget( QString title = QString::fromUtf8("") );

//  ~DWidget();

  /*! \brief Set the title
      \param title
  */
  void setTitle( QString title );

  /*! \brief get the group box
      \return QGroupBox*
  */
  QGroupBox* groupBox();
};

#include "d_widget.cpp"

#endif
