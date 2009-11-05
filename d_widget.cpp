#include "d_widget.h"

template <typename Widget>
DWidget<Widget>::DWidget( QString title )
{
  Layout = new QHBoxLayout();
  Layout->setObjectName(QString::fromUtf8("Layout"));

  Group_Box = new QGroupBox();
  Group_Box->setObjectName(QString::fromUtf8("Group_Box"));
  Group_Box->setLayout(Layout);
  Group_Box->setTitle(title);

  Layout->addWidget( this );
}

  //Kill my allocated memory
/*
template <typename Widget>
DWidget<Widget>::~DWidget()
{
  delete Layout;
//  delete Group_Box;
}
*/

  //Set the title of the group box
template <typename Widget>
void DWidget<Widget>::setTitle( QString title )
{
  Group_Box->setTitle( title );
}

  //Return the group box
template <typename Widget>
QGroupBox* DWidget<Widget>::groupBox()
{
  return Group_Box;
}
